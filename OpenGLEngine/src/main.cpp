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
#include "Renderer.h"
#include "Object.h"
#include "Maths.h"
#include "CathmullRomChain.h"
#include "CausticMapping.h"

#include <memory>
#include <iostream>
#include <math.h>
#include <fstream>
#include <string>
#include <string.h>
#include <random>
#include <time.h>

const glm::mat4 core::CathmullRomChain::CatmullRomCoeffs = glm::mat4(
  0, 2, 0, 0,
  -1, 0, 1, 0,
  2, -5, 4, -1,
  -1, 3, -3, 1
);

Camera TPcamera;

int window_height = 0;
int window_width = 0;
int mouse_x, mouse_y;

//A matrix that is updated in the keyboard function, allows moving model around in the scene
glm::mat4 model_transform = glm::mat4(1.0f);

//Holds the information loaded from the obj files
std::vector<core::SceneInfo> Scene;

//Modified in the mouse function
bool left_mouse_down;
bool middle_mouse_down;
bool use_quaternions = true;

//Could use this for the camera position
std::shared_ptr<core::CathmullRomChain> CMRchain;
std::shared_ptr<scene::Object> sky_box;
std::shared_ptr<scene::Object> box_root;

void InitSkyBox(std::shared_ptr<scene::Object> box) {
    const char* front = "res/Models/textures/Storforsen4/negz.jpg";
    const char* back = "res/Models/textures/Storforsen4/posz.jpg";
    const char* top = "res/Models/textures/Storforsen4/posy.jpg";
    const char* bottom = "res/Models/textures/Storforsen4/negy.jpg";
    const char* left = "res/Models/textures/Storforsen4/negx.jpg";
    const char* right = "res/Models/textures/Storforsen4/posx.jpg";
    std::shared_ptr<scene::Texture> texture = std::make_shared<scene::Texture>();
    texture->CreateCubeMap(front, back, top, bottom, left, right);
    box->SetDiffuseTexture(texture);
}

std::shared_ptr<render::FrameBuffer> colour_buffer;
void InitFrameBuffers() {
  colour_buffer = std::make_shared<render::FrameBuffer>();
  auto final_texture = std::make_shared<scene::Texture>(window_width, window_height, GL_TEXTURE_2D);
  colour_buffer->AttachTexture(final_texture, GL_COLOR_ATTACHMENT0);
  colour_buffer->BufferStatusCheck();
}

std::shared_ptr<scene::Object> ss_quad;
//Sets up a quad, which covers the screen exactly in clip co-ordinates
//This can be used to see your textures on the screen
void CreateScreenQuad() {
  //Positions of two triangles that cover the screen
  float ss_quad_pos[] = {
    -1.f, -1.f,
    1.f, -1.f,
    1.f, 1.f,
    -1.f, 1.f,
  };
  //Texture co ordinates of these two triangles
  float ss_quad_st[] = {
    0.f, 0.f,
    1.f, 0.f,
    1.f, 1.f,
    0.f, 1.f,
  };
  GLuint ss_quad_indices[] = {
    0, 1, 2, 2, 3, 0
  };
  
  auto points_vb = std::make_shared<render::VertexBuffer>(
    &ss_quad_pos[0],
    (unsigned int)sizeof(float) * 8
    );
  auto tex_co_ords_vb = std::make_shared<render::VertexBuffer>(
    &ss_quad_st[0],
    (unsigned int)sizeof(float) * 8
    );
  auto ib = std::make_shared<render::IndexBuffer>(&ss_quad_indices[0], 6);
  auto va = std::make_shared<render::VertexArray>();
  va->Addbuffer_2f(points_vb, 0);
  va->Addbuffer_2f(tex_co_ords_vb, 1);
  ss_quad = std::make_shared<scene::Object>(va, ib);
}

std::unique_ptr<render::CausticMapping> caustic_mapping;

void CreateCausticMapper() {
  caustic_mapping = std::make_unique<render::CausticMapping>(&window_width, &window_height, false);
}

enum class eRenderType {
  RT_blinn,
  RT_cel,
  RT_minnaert,
  RT_reflection
};

eRenderType render_type = eRenderType::RT_blinn;

render::CommonShader* blinn_phong;
render::CommonShader* silhoutte;
render::CommonShader* cel;
render::CommonShader* minnaert;
render::Shader* cube_map;
render::CommonShader* reflection;
render::Shader* post_process;

void CreateShaders() {
    const std::string shaderfile = "config/shadernames.txt";
    blinn_phong = new render::CommonShader("blinn_phong", shaderfile);
    silhoutte = new render::CommonShader("silhouette", shaderfile);
    cel = new render::CommonShader("cel", shaderfile);
    minnaert = new render::CommonShader("minnaert", shaderfile);
    reflection = new render::CommonShader("reflection", shaderfile);
    cube_map = new render::Shader("cube_map", shaderfile);
    post_process = new render::Shader("post_process", shaderfile);
    cube_map->Bind();
    cube_map->SetUniform4fv("scale", glm::scale(glm::mat4(1.0f), glm::vec3(5.0f)));
}

//TODO maybe I could make a map of shaders if I am going to have a lot
void ReloadShaders() {
    blinn_phong->Reload();
    silhoutte->Reload();
    cel->Reload();
    minnaert->Reload();
    reflection->Reload();
    cube_map->Reload();
    post_process->Reload();
    cube_map->Bind();
    cube_map->SetUniform4fv("scale", glm::scale(glm::mat4(1.0f), glm::vec3(5.0f)));
}

std::shared_ptr<scene::Object> sphere;
std::shared_ptr<scene::Object> scene_root;
std::shared_ptr<scene::Object> plane;
void LoadModels() {
  std::shared_ptr<scene::Texture> white = std::make_shared<scene::Texture>("res/Models/textures/white.jpg");
 
  scene_root = std::make_shared<scene::Object>();
  scene_root->SetTranslation(glm::vec3(0.f, 0.0f, 0.f));
  scene_root->UpdateModelMatrix();
  std::string sphere_filename = "unit_sphere.obj";
  core::SceneInfo sphere_scene(sphere_filename, white);
  sphere = sphere_scene.GetObject_(0);
  sphere->SetColour(glm::vec3(1.0f, 0.f, 0.f));
  sphere->SetParent(scene_root);
  sphere->SetScale(glm::vec3(0.2f));

  std::string plane_filename = "flat_plane.obj";
  core::SceneInfo plane_scene(plane_filename, white);
  plane = plane_scene.GetObject_(0);
  plane->SetColour(glm::vec3(0.1f, 0.1f, 0.1f));
  plane->SetParent(scene_root);
  plane->SetTranslation(glm::vec3(0.0f, -0.0f, 0.0f));
  plane->UpdateModelMatrix();

  std::string filename = "cube.obj";
  core::SceneInfo box_scene(filename, white);
  sky_box = box_scene.GetObject_(0);
}

void DrawSkyBox() {
  glDepthMask(GL_FALSE);
  glDisable(GL_CULL_FACE);
  cube_map->Bind();
  glm::mat4 view;
  view = TPcamera.getMatrix();
  cube_map->SetUniform4fv("view", view);
  glm::mat4 persp_proj = glm::perspective(glm::radians(45.0f), (float)window_width / (float)window_height, 0.1f, 300.0f);
  cube_map->SetUniform4fv("proj", persp_proj);
  render::Renderer::Draw(*sky_box);
  glDepthMask(GL_TRUE);
  glEnable(GL_CULL_FACE);
}

void UpdateScene() {
  // Wait until at least 16ms passed since start of last frame (Effectively caps framerate at ~60fps)
  static const DWORD start_time = timeGetTime();
  static DWORD last_time = 0;
  DWORD curr_time = timeGetTime();
  DWORD delta = (curr_time - last_time);
  if (delta > 16)
  {
    delta = 0;
    last_time = curr_time;
    glutPostRedisplay();
  }
}

void RenderWithShader(render::Shader* shader) {
  shader->Bind();
  glm::mat4 view = TPcamera.getMatrix();
  shader->SetUniform4fv("view", view);
  glm::mat4 persp_proj = glm::perspective(glm::radians(45.0f), (float)window_width / (float)window_height, 0.1f, 300.0f);
  shader->SetUniform4fv("proj", persp_proj);
  shader->SetUniform4fv("model_view_matrix", persp_proj * view * sphere->GetGlobalModelMatrix());
  render::Renderer::Draw(*sphere, shader, view);
}

std::shared_ptr<render::Shader> receiver_shader;

void LoadReceiver() {
  const std::string caustic_shaders = "config/causticshaders.txt";
  receiver_shader = std::make_shared<render::Shader>("receiver", caustic_shaders);
}

void Render() {
  render::Renderer::Clear();
  std::vector <std::shared_ptr<scene::Object>> receivers;
  receivers.push_back(sphere);
  std::vector<std::shared_ptr<scene::Object>> producers;
  producers.push_back(plane);
 /* 
  DrawSkyBox();*/
  caustic_mapping->CalculateCaustics(receivers, producers, post_process, ss_quad.get());
  //NB need this
  render::Renderer::SetScreenAsRenderTarget();
  glutSwapBuffers();
}

void Keyboard(unsigned char key, int x, int y) {
  static ModelMatrixTransformations translationmatrices;
  static GLfloat xtranslate = -0.3f;
  bool changedmatrices = false;
  switch (key) {

  case 13: //Enter key
    break;

  case 27: //ESC key
    exit(0);
    break;

  case 32: //Space key
    break;
  
  case 9: //Tab key
    break;

    //Reload vertex and fragment shaders during runtime
  case 'P':
    ReloadShaders();
    caustic_mapping->LoadShaders();
    LoadReceiver();
    std::cout << "Reloaded shaders" << std::endl;
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
  case 'L':
    render_type = eRenderType::RT_reflection;
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
    sphere->SetModelMatrix(model_transform);
    glutPostRedisplay();
  }
}

void Mouse(int button, int state, int x, int y) {
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

void MouseMovement(int x, int y) {
  if (left_mouse_down) {
    TPcamera.mouseUpdate(glm::vec2(x, y));
  }
  if (middle_mouse_down) {
    TPcamera.mouseMove(glm::vec2(x, y));
  }
}

void PassiveMouseMovement(int x, int y) {
  mouse_x = x;
  mouse_y = y;
}

void Init() {
  glClearColor(0.3f, 0.3f, 0.3f, 1.0f); //Grey clear colour
  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LESS);
  glEnable(GL_CULL_FACE);
  glCullFace(GL_BACK);
  LoadModels();
  InitSkyBox(sky_box);
  CreateScreenQuad();
  CreateShaders();
  CreateCausticMapper();
  InitFrameBuffers();
  LoadReceiver();
}

void CleanUp() {
  delete(blinn_phong);
  delete(minnaert);
  delete(cel);
  delete(silhoutte);
  delete(cube_map);
  delete(reflection);
  delete(post_process);
}

int main(int argc, char** argv) {
  srand((unsigned int)time(NULL));
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
  window_width = 1600;
  window_height = 900;
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
  if (glewIsSupported("GL_VERSION_4_5")) std::cout << "GLEW version supports 4.5" << std::endl;
  else fprintf(stderr, "glew version is not 4.5\n");
  if (glewIsSupported("GL_VERSION_4_3")) std::cout << "GLEW version supports 4.3" << std::endl;
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

