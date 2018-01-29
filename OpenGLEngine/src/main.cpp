#define GLM_ENABLE_EXPERIMENTAL
#include <Windows.h>
#include <mmsystem.h>

#include "glew/glew.h"
#include "freeglut/freeglut.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/transform.hpp"

#include "Camera.h"
#include "ModelMatrixTransformations.h"
#include "ModelLoader.h"
#include "ErrorHandling.h"
#include "Shader.h"

#include <iostream>
#include <math.h>
#include <fstream>
#include <string>
#include <string.h>

using namespace std;

Camera FPcamera(glm::vec3(0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, 1.0f, 0.0f));
Camera TPcamera;

int window_height = 0;
int window_width = 0;
int mouse_x, mouse_y;

//A matrix that is updated in the keyboard function, allows moving model around in the scene
glm::mat4 model_transform = glm::mat4(1.0f);

//TODO this is temp
glm::mat4 rotation = glm::mat4(1.0f);

//Holds the information loaded from the obj files
vector<SceneInfo> Scene;

//Modified in the mouse function
bool left_mouse_down;
bool middle_mouse_down;
bool use_quaternions = false;

//NOTE it might be more effective to refactor this to Enum and allow many camera types
bool use_fp_camera = false;


enum class eRenderType {
	RT_blinn,
	RT_cel,
	RT_minnaert
};

eRenderType render_type = eRenderType::RT_blinn;

render::CommonShader* blinn_phong;
render::CommonShader* silhoutte;
render::CommonShader* cel;
render::CommonShader* minnaert;

void CreateShaders() {
    const std::string shaderfile = "shadernames.txt";
    blinn_phong = new render::CommonShader("blinn_phong", shaderfile);
    silhoutte = new render::CommonShader("silhouette", shaderfile);
    cel = new render::CommonShader("cel", shaderfile);
    minnaert = new render::CommonShader("minnaert", shaderfile);
}

void ReloadShaders() {
    blinn_phong->Reload();
    silhoutte->Reload();
    cel->Reload();
    minnaert->Reload();
}

void LoadModels() {
	ifstream model_names("modelnames.txt");
	string name;
	int i = 0;
	while (getline(model_names, name)) {
		Scene.push_back(SceneInfo());
		if (!Scene[i].LoadModelFromFile(name)) {
			fprintf(stderr, "Did not correctly read %s\n", name.c_str());
			exit(-1);
		}
		Scene[i].InitBuffersAndArrays();
		++i;
	}
	model_names.close();
}

void RenderBlinn();
void RenderCel();
void RenderMinnaert();

void Render() {
	switch (render_type) {
		case eRenderType::RT_blinn :
			RenderBlinn();
			break;
		case eRenderType::RT_cel :
			RenderCel();
			break;
		case eRenderType::RT_minnaert :
			RenderMinnaert();
			break;
		default:
			break;
	}
}

//REFERENCE http://sunandblackcat.com/tipFullView.php?l=eng&topicid=15

//TODO this does not work if the model gets reversed since the order of the faces gets reversed
void RenderCel() {
	GLuint viewmatrix_id = 0, projmatrix_id = 1, modelmatrix_id = 2, modelview_inversetranspose = 3;
	GLuint offset_id = 5, colour_id = 4;
	silhoutte->Bind();
	static float angle = 0.0;
	glm::mat4 global1 = glm::mat4(1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);

	//Render the black outline silhoutte
	glCullFace(GL_FRONT); //We need to able culling of front faces for the black outline
	glDepthMask(GL_TRUE); //We want to write to the depth buffer here
	silhoutte->SetUniform3f("colour", glm::vec3(0.0f));
	glm::mat4 view;
	for (size_t j = 0; j != Scene.size(); ++j) {
		if (j == 0) { //Render the plane
			for (size_t i = 0; i < Scene[j].GetNumMeshes(); ++i) {
				glUniform1f(offset_id, 0.12f);
				//Bind the correct VAO and VBO
				Scene[j].GetData(i).first.Bind();
				Scene[j].GetData(i).second.Bind();
				glm::mat4 persp_proj = glm::perspective(glm::radians(45.0f), (float)window_width / (float)window_height, 0.1f, 100.0f);
				glm::mat4 local1 = glm::mat4(1.0f);
				//If the plane is too big and too far forward, make it smaller before applying the keyboard defined transformation, and then move it backwards
				local1 = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -20.0f)) * model_transform;

				// for the root, we orient it in global space
				global1 = local1;
				if (use_fp_camera) {
					FPcamera.updatePosition(glm::vec3(global1 * glm::vec4(-0.007894f, 2.238691f, 2.166406f, 1.0f)));
					FPcamera.updateDirection(glm::vec3(model_transform * glm::vec4(0.0f, 0.0f, 1.0f, 0.0f)));
					view = FPcamera.getMatrix();
				}
				else {
					view = TPcamera.getMatrix();
				}
				// update uniforms & draw
				glUniformMatrix4fv(projmatrix_id, 1, GL_FALSE, &persp_proj[0][0]);
				glUniformMatrix4fv(viewmatrix_id, 1, GL_FALSE, &view[0][0]);
				glUniformMatrix4fv(modelmatrix_id, 1, GL_FALSE, &global1[0][0]);
				//Needed so that normals behave correctly under non uniform scaling
				glm::mat4 modelview_it = glm::transpose(glm::inverse(view * global1));
				glUniformMatrix4fv(modelview_inversetranspose, 1, GL_FALSE, &modelview_it[0][0]);
				glm::mat4 global2;
				//The centre of the propellor
				if (i == 29) {
					angle = fmod(angle + 0.05f, FULLROTATIONINRADIANS);
					//The translation is defined by the first point for this mesh in the obj file
					//This is not exactly right, but it is close, how do I know it exactly?
					glm::mat4 local2 = glm::translate(glm::mat4(1.0), glm::vec3(0.007894f, 1.238691f, 3.366406f));
					local2 = glm::rotate(local2, angle, glm::vec3(0.0f, 0.0f, 1.0f));
					local2 = glm::translate(local2, -glm::vec3(0.007894f, 1.238691f, 3.366406f));
					global2 = global1 * local2;
					glUniformMatrix4fv(modelmatrix_id, 1, GL_FALSE, &global2[0][0]);
					//Needed so that normals behave correctly under non uniform scaling
					glm::mat4 modelview_it = glm::transpose(glm::inverse(view * global2));
					glUniformMatrix4fv(modelview_inversetranspose, 1, GL_FALSE, &modelview_it[0][0]);
					glUniform1f(offset_id, 0.05f);
				}
				//The blades of the propellor
				else if (i == 31 || i == 32) {
					glUniformMatrix4fv(modelmatrix_id, 1, GL_FALSE, &global2[0][0]);
					//Needed so that normals behave correctly under non uniform scaling
					glm::mat4 modelview_it = glm::transpose(glm::inverse(view * global2));
					glUniformMatrix4fv(modelview_inversetranspose, 1, GL_FALSE, &modelview_it[0][0]);
					glUniform1f(offset_id, 0.05f);
				}

        GLCall(glDrawElements(GL_TRIANGLES, Scene[j].GetNumIndices(i), GL_UNSIGNED_INT, (void*)0));
			}
		}
		else if (j == 1) { //Render the knot
			for (size_t i = 0; i < Scene[j].GetNumMeshes(); ++i) {
        Scene[j].GetData(i).first.Bind();
        Scene[j].GetData(i).second.Bind();
				glm::mat4 local1 = glm::mat4(1.0f);
				local1 = glm::translate(glm::mat4(1.0f), glm::vec3(10.0f, 1.0f, -20.0f)) * model_transform * glm::scale(glm::vec3(3.0f));
				global1 = local1;
				glUniformMatrix4fv(modelmatrix_id, 1, GL_FALSE, &global1[0][0]);
				//Needed so that normals behave correctly under non uniform scaling
				glm::mat4 modelview_it = glm::transpose(glm::inverse(view * global1));
				glUniformMatrix4fv(modelview_inversetranspose, 1, GL_FALSE, &modelview_it[0][0]);
				glDrawElements(GL_TRIANGLES, Scene[j].GetNumIndices(i), GL_UNSIGNED_INT, (void*)0);
			}
		}
		else if (j == 2) { //Render the sphere
			for (size_t i = 0; i < Scene[j].GetNumMeshes(); ++i) {
        Scene[j].GetData(i).first.Bind();
        Scene[j].GetData(i).second.Bind();
				glm::mat4 local1 = glm::mat4(1.0f);
				local1 = glm::translate(glm::mat4(1.0f), glm::vec3(-10.0f, -2.0f, -20.0f)) * model_transform * glm::scale(glm::vec3(0.3f));
				global1 = local1;
				glUniformMatrix4fv(modelmatrix_id, 1, GL_FALSE, &global1[0][0]);
				//Needed so that normals behave correctly under non uniform scaling
				glm::mat4 modelview_it = glm::transpose(glm::inverse(view * global1));
				glUniformMatrix4fv(modelview_inversetranspose, 1, GL_FALSE, &modelview_it[0][0]);
				glDrawElements(GL_TRIANGLES, Scene[j].GetNumIndices(i), GL_UNSIGNED_INT, (void*)0);
			}
		}
	}

	//Render the colours
	cel->Bind();
	//glClear(GL_DEPTH_BUFFER_BIT);
	glCullFace(GL_BACK);
	glDepthMask(GL_TRUE);
	glEnable(GL_DEPTH_TEST);
	GLuint num_shades_id = 5, base_colour_id = 6;
	glUniform3fv(base_colour_id, 1, &(glm::vec3(1.0f, 0.5f, 1.0f)[0]));
	glUniform1f(num_shades_id, 6);
	for (size_t j = 0; j != Scene.size(); ++j) {
		if (j == 0) { //Render the plane
			for (size_t i = 0; i < Scene[j].GetNumMeshes(); ++i) {
				//Bind the correct VAO and VBO
        Scene[j].GetData(i).first.Bind();
        Scene[j].GetData(i).second.Bind();
				glm::mat4 persp_proj = glm::perspective(glm::radians(45.0f), (float)window_width / (float)window_height, 0.1f, 100.0f);
				glm::mat4 local1 = glm::mat4(1.0f);
				//If the plane is too big and too far forward, make it smaller before applying the keyboard defined transformation, and then move it backwards
				local1 = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -20.0f)) * model_transform;

				// for the root, we orient it in global space
				global1 = local1;
				if (use_fp_camera) {
					FPcamera.updatePosition(glm::vec3(global1 * glm::vec4(-0.007894f, 2.238691f, 2.166406f, 1.0f)));
					FPcamera.updateDirection(glm::vec3(model_transform * glm::vec4(0.0f, 0.0f, 1.0f, 0.0f)));
					view = FPcamera.getMatrix();
				}
				else {
					view = TPcamera.getMatrix();
				}
				// update uniforms & draw
				glUniformMatrix4fv(projmatrix_id, 1, GL_FALSE, &persp_proj[0][0]);
				glUniformMatrix4fv(viewmatrix_id, 1, GL_FALSE, &view[0][0]);
				glUniformMatrix4fv(modelmatrix_id, 1, GL_FALSE, &global1[0][0]);
				//Needed so that normals behave correctly under non uniform scaling
				glm::mat4 modelview_it = glm::transpose(glm::inverse(view * global1));
				glUniformMatrix4fv(modelview_inversetranspose, 1, GL_FALSE, &modelview_it[0][0]);
				glm::mat4 global2;
				//The centre of the propellor
				if (i == 29) {
					glUniform3fv(colour_id, 1, &(glm::vec3(1.0f, 0.0f, 0.1f)[0]));
					//The translation is defined by the first point for this mesh in the obj file
					//This is not exactly right, but it is close, how do I know it exactly?
					glm::mat4 local2 = glm::translate(glm::mat4(1.0), glm::vec3(0.007894f, 1.238691f, 3.366406f));
					local2 = glm::rotate(local2, angle, glm::vec3(0.0f, 0.0f, 1.0f));
					local2 = glm::translate(local2, -glm::vec3(0.007894f, 1.238691f, 3.366406f));
					global2 = global1 * local2;
					glUniformMatrix4fv(modelmatrix_id, 1, GL_FALSE, &global2[0][0]);
					//Needed so that normals behave correctly under non uniform scaling
					glm::mat4 modelview_it = glm::transpose(glm::inverse(view * global2));
					glUniformMatrix4fv(modelview_inversetranspose, 1, GL_FALSE, &modelview_it[0][0]);
				}
				//The blades of the propellor
				else if (i == 31 || i == 32) {
					glUniform3fv(colour_id, 1, &(glm::vec3(1.0f, 0.0f, 0.1f)[0]));
					glUniformMatrix4fv(modelmatrix_id, 1, GL_FALSE, &global2[0][0]);
					//Needed so that normals behave correctly under non uniform scaling
					glm::mat4 modelview_it = glm::transpose(glm::inverse(view * global2));
					glUniformMatrix4fv(modelview_inversetranspose, 1, GL_FALSE, &modelview_it[0][0]);
				}
				//The wheels
				else if (i == 82 || i == 106 || i == 116) {
					glUniform3fv(colour_id, 1, &(glm::vec3(0.0f, 0.0f, 0.0f)[0]));
				}
				else glUniform3fv(colour_id, 1, &(glm::vec3(0.0f, 0.0f, 1.0f)[0]));
				glDrawElements(GL_TRIANGLES, Scene[j].GetNumIndices(i), GL_UNSIGNED_INT, (void*)0);
			}
		}
		else if (j == 1) { //Render the knot
			for (size_t i = 0; i < Scene[j].GetNumMeshes(); ++i) {
        Scene[j].GetData(i).first.Bind();
        Scene[j].GetData(i).second.Bind();
				glm::mat4 local1 = glm::mat4(1.0f);
				local1 = glm::translate(glm::mat4(1.0f), glm::vec3(10.0f, 1.0f, -20.0f)) * model_transform * glm::scale(glm::vec3(3.0f));
				global1 = local1;
				glUniformMatrix4fv(modelmatrix_id, 1, GL_FALSE, &global1[0][0]);
				//Needed so that normals behave correctly under non uniform scaling
				glm::mat4 modelview_it = glm::transpose(glm::inverse(view * global1));
				glUniformMatrix4fv(modelview_inversetranspose, 1, GL_FALSE, &modelview_it[0][0]);
				glUniform3fv(colour_id, 1, &(glm::vec3(1.0f, 3.0f, 0.0f)[0]));
				glDrawElements(GL_TRIANGLES, Scene[j].GetNumIndices(i), GL_UNSIGNED_INT, (void*)0);
			}
		}
		else if (j == 2) { //Render the sphere
			for (size_t i = 0; i < Scene[j].GetNumMeshes(); ++i) {
        Scene[j].GetData(i).first.Bind();
        Scene[j].GetData(i).second.Bind();
				glm::mat4 local1 = glm::mat4(1.0f);
				local1 = glm::translate(glm::mat4(1.0f), glm::vec3(-10.0f, -2.0f, -20.0f)) * model_transform * glm::scale(glm::vec3(0.3f));
				global1 = local1;
				glUniformMatrix4fv(modelmatrix_id, 1, GL_FALSE, &global1[0][0]);
				//Needed so that normals behave correctly under non uniform scaling
				glm::mat4 modelview_it = glm::transpose(glm::inverse(view * global1));
				glUniformMatrix4fv(modelview_inversetranspose, 1, GL_FALSE, &modelview_it[0][0]);
				glUniform3fv(colour_id, 1, &(glm::vec3(0.0f, 1.0f, 0.0f)[0]));
				glDrawElements(GL_TRIANGLES, Scene[j].GetNumIndices(i), GL_UNSIGNED_INT, (void*)0);
			}
		}
	}
	glutSwapBuffers();
}

void RenderBlinn() {
	GLuint viewmatrix_id = 0, projmatrix_id = 1, modelmatrix_id = 2;
	GLuint modelview_inversetranspose = 3, diffuse_colour_id = 4;
	blinn_phong->Bind();
	static float angle = 0.0;
	glm::mat4 global1 = glm::mat4(1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glDepthMask(GL_TRUE);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glDisable(GL_CULL_FACE);
	glm::mat4 view;
	for (size_t j = 0; j != Scene.size(); ++j) {
		if (j == 0) { //Render the plane
			for (size_t i = 0; i < Scene[j].GetNumMeshes(); ++i) {
				//Bind the correct VAO and VBO
        Scene[j].GetData(i).first.Bind();
        Scene[j].GetData(i).second.Bind();
				glm::mat4 persp_proj = glm::perspective(glm::radians(45.0f), (float)window_width / (float)window_height, 0.1f, 100.0f);
				glm::mat4 local1 = glm::mat4(1.0f);
				//If the plane is too big and too far forward, make it smaller before applying the keyboard defined transformation, and then move it backwards
				local1 = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -20.0f)) * model_transform;

				// for the root, we orient it in global space
				global1 = local1;
				if (use_fp_camera) {
					FPcamera.updatePosition(glm::vec3(global1 * glm::vec4(-0.007894f, 2.238691f, 2.166406f, 1.0f)));
					FPcamera.updateDirection(glm::vec3(model_transform * glm::vec4(0.0f, 0.0f, 1.0f, 0.0f)));
					view = FPcamera.getMatrix();
				}
				else {
					view = TPcamera.getMatrix();
				}
				// update uniforms & draw
				glUniformMatrix4fv(projmatrix_id, 1, GL_FALSE, &persp_proj[0][0]);
				glUniformMatrix4fv(viewmatrix_id, 1, GL_FALSE, &view[0][0]);
				glUniformMatrix4fv(modelmatrix_id, 1, GL_FALSE, &global1[0][0]);
				//Needed so that normals behave correctly under non uniform scaling
				glm::mat4 modelview_it = glm::transpose(glm::inverse(view * global1));
				glUniformMatrix4fv(modelview_inversetranspose, 1, GL_FALSE, &modelview_it[0][0]);
				glm::mat4 global2;
				//The centre of the propellor
				if (i == 29) {
					angle = fmod(angle + 0.05f, FULLROTATIONINRADIANS);
					glUniform3fv(diffuse_colour_id, 1, &(glm::vec3(1.0f, 0.0f, 0.1f)[0]));
					//The translation is defined by the first point for this mesh in the obj file
					//This is not exactly right, but it is close, how do I know it exactly?
					glm::mat4 local2 = glm::translate(glm::mat4(1.0), glm::vec3(0.007894f, 1.238691f, 3.366406f));
					local2 = glm::rotate(local2, angle, glm::vec3(0.0f, 0.0f, 1.0f));
					local2 = glm::translate(local2, -glm::vec3(0.007894f, 1.238691f, 3.366406f));
					global2 = global1 * local2;
					glUniformMatrix4fv(modelmatrix_id, 1, GL_FALSE, &global2[0][0]);
					//Needed so that normals behave correctly under non uniform scaling
					glm::mat4 modelview_it = glm::transpose(glm::inverse(view * global2));
					glUniformMatrix4fv(modelview_inversetranspose, 1, GL_FALSE, &modelview_it[0][0]);
				}
				//The blades of the propellor
				else if (i == 31 || i == 32) {
					glUniform3fv(diffuse_colour_id, 1, &(glm::vec3(1.0f, 0.0f, 0.1f)[0]));
					glUniformMatrix4fv(modelmatrix_id, 1, GL_FALSE, &global2[0][0]);
					//Needed so that normals behave correctly under non uniform scaling
					glm::mat4 modelview_it = glm::transpose(glm::inverse(view * global2));
					glUniformMatrix4fv(modelview_inversetranspose, 1, GL_FALSE, &modelview_it[0][0]);
				}
				//The wheels
				else if (i == 82 || i == 106 || i == 116) {
					glUniform3fv(diffuse_colour_id, 1, &(glm::vec3(0.0f, 0.0f, 0.0f)[0]));
				}
				else glUniform3fv(diffuse_colour_id, 1, &(glm::vec3(0.0f, 0.0f, 1.0f)[0]));
 				GLCall(glDrawElements(GL_TRIANGLES, Scene[j].GetData(i).second.GetCount(), GL_UNSIGNED_INT, (void*)0));
			}
		}
		else if (j == 1) { //Render the knot
			for (size_t i = 0; i < Scene[j].GetNumMeshes(); ++i) {
        Scene[j].GetData(i).first.Bind();
        Scene[j].GetData(i).second.Bind();
				glm::mat4 local1 = glm::mat4(1.0f);
				local1 = glm::translate(glm::mat4(1.0f), glm::vec3(10.0f, 1.0f, -20.0f)) * model_transform * glm::scale(glm::vec3(3.0f));
				global1 = local1;
				glUniformMatrix4fv(modelmatrix_id, 1, GL_FALSE, &global1[0][0]);
				//Needed so that normals behave correctly under non uniform scaling
				glm::mat4 modelview_it = glm::transpose(glm::inverse(view * global1));
				glUniformMatrix4fv(modelview_inversetranspose, 1, GL_FALSE, &modelview_it[0][0]);
				glUniform3fv(diffuse_colour_id, 1, &(glm::vec3(1.0f, 3.0f, 0.0f)[0]));
				glDrawElements(GL_TRIANGLES, Scene[j].GetNumIndices(i), GL_UNSIGNED_INT, (void*)0);
			}
		}
		else if (j == 2) { //Render the sphere
			for (size_t i = 0; i < Scene[j].GetNumMeshes(); ++i) {
        Scene[j].GetData(i).first.Bind();
        Scene[j].GetData(i).second.Bind();
				glm::mat4 local1 = glm::mat4(1.0f);
				local1 = glm::translate(glm::mat4(1.0f), glm::vec3(-10.0f, -2.0f, -20.0f)) * model_transform * glm::scale(glm::vec3(0.3f));
				global1 = local1;
				glUniformMatrix4fv(modelmatrix_id, 1, GL_FALSE, &global1[0][0]);
				//Needed so that normals behave correctly under non uniform scaling
				glm::mat4 modelview_it = glm::transpose(glm::inverse(view * global1));
				glUniformMatrix4fv(modelview_inversetranspose, 1, GL_FALSE, &modelview_it[0][0]);
				glUniform3fv(diffuse_colour_id, 1, &(glm::vec3(0.0f, 1.0f, 0.0f)[0]));
				glDrawElements(GL_TRIANGLES, Scene[j].GetNumIndices(i), GL_UNSIGNED_INT, (void*)0);
			}
		}
	}
	glutSwapBuffers();
}

void RenderMinnaert() {
	GLuint viewmatrix_id = 0, projmatrix_id = 1, modelmatrix_id = 2;
	GLuint modelview_inversetranspose = 3, diffuse_colour_id = 4;
	minnaert->Bind();
	static float angle = 0.0;
	glm::mat4 global1 = glm::mat4(1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glDepthMask(GL_TRUE);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glDisable(GL_CULL_FACE);
	glm::mat4 view;
	for (size_t j = 0; j != Scene.size(); ++j) {
		if (j == 0) { //Render the plane
			for (size_t i = 0; i < Scene[j].GetNumMeshes(); ++i) {
				//Bind the correct VAO and VBO
        Scene[j].GetData(i).first.Bind();
        Scene[j].GetData(i).second.Bind();
				glm::mat4 persp_proj = glm::perspective(glm::radians(45.0f), (float)window_width / (float)window_height, 0.1f, 100.0f);
				glm::mat4 local1 = glm::mat4(1.0f);
				//If the plane is too big and too far forward, make it smaller before applying the keyboard defined transformation, and then move it backwards
				local1 = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -20.0f)) * model_transform;

				// for the root, we orient it in global space
				global1 = local1;
				if (use_fp_camera) {
					FPcamera.updatePosition(glm::vec3(global1 * glm::vec4(-0.007894f, 2.238691f, 2.166406f, 1.0f)));
					FPcamera.updateDirection(glm::vec3(model_transform * glm::vec4(0.0f, 0.0f, 1.0f, 0.0f)));
					view = FPcamera.getMatrix();
				}
				else {
					view = TPcamera.getMatrix();
				}
				// update uniforms & draw
				glUniformMatrix4fv(projmatrix_id, 1, GL_FALSE, &persp_proj[0][0]);
				glUniformMatrix4fv(viewmatrix_id, 1, GL_FALSE, &view[0][0]);
				glUniformMatrix4fv(modelmatrix_id, 1, GL_FALSE, &global1[0][0]);
				//Needed so that normals behave correctly under non uniform scaling
				glm::mat4 modelview_it = glm::transpose(glm::inverse(view * global1));
				glUniformMatrix4fv(modelview_inversetranspose, 1, GL_FALSE, &modelview_it[0][0]);
				glm::mat4 global2;
				//The centre of the propellor
				if (i == 29) {
					angle = fmod(angle + 0.05f, FULLROTATIONINRADIANS);
					glUniform3fv(diffuse_colour_id, 1, &(glm::vec3(1.0f, 0.0f, 0.1f)[0]));
					//The translation is defined by the first point for this mesh in the obj file
					//This is not exactly right, but it is close, how do I know it exactly?
					glm::mat4 local2 = glm::translate(glm::mat4(1.0), glm::vec3(0.007894f, 1.238691f, 3.366406f));
					local2 = glm::rotate(local2, angle, glm::vec3(0.0f, 0.0f, 1.0f));
					local2 = glm::translate(local2, -glm::vec3(0.007894f, 1.238691f, 3.366406f));
					global2 = global1 * local2;
					glUniformMatrix4fv(modelmatrix_id, 1, GL_FALSE, &global2[0][0]);
					//Needed so that normals behave correctly under non uniform scaling
					glm::mat4 modelview_it = glm::transpose(glm::inverse(view * global2));
					glUniformMatrix4fv(modelview_inversetranspose, 1, GL_FALSE, &modelview_it[0][0]);
				}
				//The blades of the propellor
				else if (i == 31 || i == 32) {
					glUniform3fv(diffuse_colour_id, 1, &(glm::vec3(1.0f, 0.0f, 0.1f)[0]));
					glUniformMatrix4fv(modelmatrix_id, 1, GL_FALSE, &global2[0][0]);
					//Needed so that normals behave correctly under non uniform scaling
					glm::mat4 modelview_it = glm::transpose(glm::inverse(view * global2));
					glUniformMatrix4fv(modelview_inversetranspose, 1, GL_FALSE, &modelview_it[0][0]);
				}
				//The wheels
				else if (i == 82 || i == 106 || i == 116) {
					glUniform3fv(diffuse_colour_id, 1, &(glm::vec3(0.0f, 0.0f, 0.0f)[0]));
				}
				else glUniform3fv(diffuse_colour_id, 1, &(glm::vec3(0.0f, 0.0f, 1.0f)[0]));
				glDrawElements(GL_TRIANGLES, Scene[j].GetNumIndices(i), GL_UNSIGNED_INT, (void*)0);
			}
		}
		else if (j == 1) { //Render the knot
			for (size_t i = 0; i < Scene[j].GetNumMeshes(); ++i) {
        Scene[j].GetData(i).first.Bind();
        Scene[j].GetData(i).second.Bind();
				glm::mat4 local1 = glm::mat4(1.0f);
				local1 = glm::translate(glm::mat4(1.0f), glm::vec3(10.0f, 1.0f, -20.0f)) * model_transform * glm::scale(glm::vec3(3.0f));
				global1 = local1;
				glUniformMatrix4fv(modelmatrix_id, 1, GL_FALSE, &global1[0][0]);
				//Needed so that normals behave correctly under non uniform scaling
				glm::mat4 modelview_it = glm::transpose(glm::inverse(view * global1));
				glUniformMatrix4fv(modelview_inversetranspose, 1, GL_FALSE, &modelview_it[0][0]);
				glUniform3fv(diffuse_colour_id, 1, &(glm::vec3(1.0f, 3.0f, 0.0f)[0]));
				glDrawElements(GL_TRIANGLES, Scene[j].GetNumIndices(i), GL_UNSIGNED_INT, (void*)0);
			}
		}
		else if (j == 2) { //Render the sphere
			for (size_t i = 0; i < Scene[j].GetNumMeshes(); ++i) {
        Scene[j].GetData(i).first.Bind();
        Scene[j].GetData(i).second.Bind();
				glm::mat4 local1 = glm::mat4(1.0f);
				local1 = glm::translate(glm::mat4(1.0f), glm::vec3(-10.0f, -2.0f, -20.0f)) * model_transform * glm::scale(glm::vec3(0.3f));
				global1 = local1;
				glUniformMatrix4fv(modelmatrix_id, 1, GL_FALSE, &global1[0][0]);
				//Needed so that normals behave correctly under non uniform scaling
				glm::mat4 modelview_it = glm::transpose(glm::inverse(view * global1));
				glUniformMatrix4fv(modelview_inversetranspose, 1, GL_FALSE, &modelview_it[0][0]);
				glUniform3fv(diffuse_colour_id, 1, &(glm::vec3(0.0f, 1.0f, 0.0f)[0]));
				glDrawElements(GL_TRIANGLES, Scene[j].GetNumIndices(i), GL_UNSIGNED_INT, (void*)0);
			}
		}
	}
	glutSwapBuffers();
}

void UpdateScene() {
	// Wait until at least 16ms passed since start of last frame (Effectively caps framerate at ~60fps)
	static DWORD  last_time = 0;
	DWORD  curr_time = timeGetTime();
	DWORD delta = (curr_time - last_time);
	if (delta > 16)
	{
		delta = 0;
		last_time = curr_time;
		glutPostRedisplay();
	}
}

void Keyboard(unsigned char key, int x, int y) {
	static ModelMatrixTransformations translationmatrices;
	static GLfloat xtranslate = -0.3f;
	bool changedmatrices = false;
	switch (key) {

	case 13: //Enter key
		use_fp_camera = !use_fp_camera;
		if(use_fp_camera) FPcamera.mouseUpdate(glm::vec2(mouse_x, mouse_y));
		else FPcamera.first_click = false;
		break;

	case 27: //ESC key
		exit(0);
		break;

	case 32: //Space key
		use_quaternions = !use_quaternions;
		changedmatrices = true;
		break;

		//Reload vertex and fragment shaders during runtime
	case 'P':
    ReloadShaders();
		std::cout << "Reloaded shaders" << endl;
		break;

		//Rotations
	case 'i':
		translationmatrices.UpdateRotate(glm::vec3(glm::radians(10.0f), 0.0f, 0.0f), use_quaternions);
		changedmatrices = true;
		break;
	case 'I':
		translationmatrices.UpdateRotate(glm::vec3(glm::radians(-10.0f), 0.0f, 0.0f), use_quaternions);
		changedmatrices = true;
		break;
	case 'j':
		translationmatrices.UpdateRotate(glm::vec3(0.0f, glm::radians(10.0f), 0.0f), use_quaternions);
		changedmatrices = true;
		break;
	case 'J':
		translationmatrices.UpdateRotate(glm::vec3(0.0f, glm::radians(-10.0f), 0.0f), use_quaternions);
		changedmatrices = true;
		break;
	case 'k':
		translationmatrices.UpdateRotate(glm::vec3(0.0f, 0.0f, glm::radians(10.0f)), use_quaternions);
		changedmatrices = true;
		break;
	case 'K':
		translationmatrices.UpdateRotate(glm::vec3(0.0f, 0.0f, glm::radians(-10.0f)), use_quaternions);
		changedmatrices = true;
		break;

		//Scalings
	case 'x':
		translationmatrices.UpdateScale(glm::vec3(-0.2f, 0.0f, 0.0f));
		changedmatrices = true;
		break;
	case 'X':
		translationmatrices.UpdateScale(glm::vec3(0.2f, 0.0f, 0.0f));
		changedmatrices = true;
		break;
	case 'y':
		translationmatrices.UpdateScale(glm::vec3(0.0f, -0.2f, 0.0f));
		changedmatrices = true;
		break;
	case 'Y':
		translationmatrices.UpdateScale(glm::vec3(0.0f, 0.2f, 0.0f));
		changedmatrices = true;
		break;
	case 'z':
		translationmatrices.UpdateScale(glm::vec3(0.0f, 0.0f, -0.2f));
		changedmatrices = true;
		break;
	case 'Z':
		translationmatrices.UpdateScale(glm::vec3(0.0f, 0.0f, 0.2f));
		changedmatrices = true;
		break;
	case 'r':
		translationmatrices.UpdateScale(glm::vec3(-0.2f, -0.2f, -0.2f));
		changedmatrices = true;
		break;
	case 'R':
		translationmatrices.UpdateScale(glm::vec3(0.2f, 0.2f, 0.2f));
		changedmatrices = true;
		break;

	//Camera Translations
	case 'd':
		TPcamera.Move(glm::vec3(1.0f, 0.0f, 0.0f));
		changedmatrices = true;
		break;
	case 'a':
		TPcamera.Move(glm::vec3(-1.0f, 0.0f, 0.0f));
		changedmatrices = true;
		break;
	case 'w':
		TPcamera.Move(glm::vec3(0.0f, 1.0f, 0.0f));
		changedmatrices = true;
		break;
	case 's':
		TPcamera.Move(glm::vec3(0.0f, -1.0f, 0.0f));
		changedmatrices = true;
		break;
	case 'q':
		TPcamera.Move(glm::vec3(0.0f, 0.0f, -1.0f));
		changedmatrices = true;
		break;
	case 'e':
		TPcamera.Move(glm::vec3(0.0f, 0.0f, 1.0f));
		changedmatrices = true;
		break;

	//model translations
	case 'D':
		translationmatrices.UpdateTranslate(glm::vec3(0.2f, 0.0f, 0.0f));
		changedmatrices = true;
		break;
	case 'A':
		translationmatrices.UpdateTranslate(glm::vec3(-0.2f, 0.0f, 0.0f));
		changedmatrices = true;
		break;
	case 'W':
		translationmatrices.UpdateTranslate(glm::vec3(0.0f, 0.2f, 0.0f));
		changedmatrices = true;
		break;
	case 'S':
		translationmatrices.UpdateTranslate(glm::vec3(0.0f, -0.2f, 0.0f));
		changedmatrices = true;
		break;
	case 'Q':
		translationmatrices.UpdateTranslate(glm::vec3(0.0f, 0.0f, -0.2f));
		changedmatrices = true;
		break;
	case 'E':
		translationmatrices.UpdateTranslate(glm::vec3(0.0f, 0.0f, 0.2f));
		changedmatrices = true;
		break;
	//change rendering mode
	case 'l':
		render_type = eRenderType::RT_blinn;
		break;
	case 't':
		render_type = eRenderType::RT_cel;
		break;
	case 'T':
		render_type = eRenderType::RT_minnaert;
		break;
	default:
		break;
	}
	//Apply the changes
	if (changedmatrices) {
		model_transform = translationmatrices.UpdateModelMatrix();
		rotation = translationmatrices.GetRotationMatrix();
		glutPostRedisplay();
	}
}

void Mouse(int button, int state, int x, int y) {
	if (!use_fp_camera) {
		if (button == GLUT_LEFT && state == GLUT_DOWN)
		{
			left_mouse_down = true;
			TPcamera.mouseUpdate(glm::vec2(x, y));
		}
		if (button == GLUT_LEFT && state == GLUT_UP)
		{
			left_mouse_down = false;
			TPcamera.first_click = false;
		}
		if (button == GLUT_MIDDLE_BUTTON && state == GLUT_DOWN)
		{
			middle_mouse_down = true;
			TPcamera.mouseMove(glm::vec2(x, y));
		}
		if (button == GLUT_MIDDLE_BUTTON && state == GLUT_UP)
		{
			middle_mouse_down = false;
			TPcamera.first_click = false;
		}
	}
}

void MouseMovement(int x, int y) {
	if (!use_fp_camera) {
		if (left_mouse_down) {
			TPcamera.mouseUpdate(glm::vec2(x, y));
		}
		if (middle_mouse_down) {
			TPcamera.mouseMove(glm::vec2(x, y));
		}
	}
}

void PassiveMouseMovement(int x, int y) {
	if (use_fp_camera) {
		//NOTE - A potential solution could be to make FPcamera and TP camera children of camera class
		//And the update function is different in FPcamera
		//FPcamera.mouseUpdate(glm::vec2(x, y));
	}
	mouse_x = x;
	mouse_y = y;
}

void Init() {
	glClearColor(0.3f, 0.3f, 0.3f, 1.0f); //Grey clear colour
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	LoadModels();
  CreateShaders();
}

void CleanUp() {
	delete(blinn_phong);
	delete(minnaert);
	delete(cel);
	delete(silhoutte);
}

int main(int argc, char** argv) {
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	window_width = 1440;
	window_height = 810;
	glutInitWindowPosition(100, 100);//optional
	glutInitWindowSize(window_width, window_height); //optional
	glutCreateWindow("Plane Rotations - Sean Martin 13319354");

	glewExperimental = GL_TRUE;
	GLenum res = glewInit();
	// Check for any errors in initialising glew
	if (res != GLEW_OK) {
		fprintf(stderr, "Error: '%s'\n", glewGetErrorString(res));
		return 1;
	}
	//Make sure which version of openGL you can use!
	if (glewIsSupported("GL_VERSION_4_5")) std::cout << "GLEW version supports 4.5" << endl;
	else fprintf(stderr, "glew version is not 4.5\n");
	if (glewIsSupported("GL_VERSION_4_3")) std::cout << "GLEW version supports 4.3" << endl;
	else fprintf(stderr, "glew version is not 4.3\n");

	#ifdef GLM_FORCE_RADIANS
		fprintf(stderr, "Glm is forced using radians\n");
	#else
		fprintf(stderr, "Glm is not forced using radians\n");
	#endif

	glutDisplayFunc(Render);
	glutIdleFunc(UpdateScene);
	glutKeyboardFunc(Keyboard);
	glutMouseFunc(Mouse);
	glutMotionFunc(MouseMovement);
	glutPassiveMotionFunc(PassiveMouseMovement);
	Init();
	glutMainLoop();
	CleanUp();
	return 0;
}
