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
#include "Particle.h"
#include "ParticleSpawner.h"
#include "Bone.h"
#include "IK_Solver.h"

#include <memory>
#include <iostream>
#include <math.h>
#include <fstream>
#include <string>
#include <string.h>
#include <random>
#include <time.h>

Camera FPcamera(glm::vec3(0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, 1.0f, 0.0f));
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
bool use_quaternions = false;

//NOTE it might be more effective to refactor this to Enum and allow many camera types
bool use_fp_camera = false;
bool use_mip = true;
bool do_rotate = false;

glm::vec3 target = glm::vec3(0.0f);
std::shared_ptr<IK::BoneChain> cone_chain;
std::shared_ptr<scene::Object> sky_box;
std::vector<std::shared_ptr<IK::Bone>> cone_bones;

void InitSkyBox() {
    core::SceneInfo sceneinfo;
    char* filename = "res/Models/cube.obj";
    if (!sceneinfo.LoadModelFromFile(filename)) {
        fprintf(stderr, "ERROR: Could not load %s", filename);
        exit(-1);
    }
    sky_box = sceneinfo.GetObject_(0);
    const char* front = "res/Models/textures/Storforsen4/negz.jpg";
    const char* back = "res/Models/textures/Storforsen4/posz.jpg";
    const char* top = "res/Models/textures/Storforsen4/posy.jpg";
    const char* bottom = "res/Models/textures/Storforsen4/negy.jpg";
    const char* left = "res/Models/textures/Storforsen4/negx.jpg";
    const char* right = "res/Models/textures/Storforsen4/posx.jpg";
    std::shared_ptr<scene::Texture> texture = std::make_shared<scene::Texture>();
    texture->CreateCubeMap(front, back, top, bottom, left, right);
    sky_box->SetDiffuseTexture(texture);
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

void CreateShaders() {
    const std::string shaderfile = "config/shadernames.txt";
    blinn_phong = new render::CommonShader("blinn_phong", shaderfile);
    silhoutte = new render::CommonShader("silhouette", shaderfile);
    cel = new render::CommonShader("cel", shaderfile);
    minnaert = new render::CommonShader("minnaert", shaderfile);
    reflection = new render::CommonShader("reflection", shaderfile);
    cube_map = new render::Shader("cube_map", shaderfile);
    cube_map->Bind();
    cube_map->SetUniform4fv("scale", glm::scale(glm::mat4(1.0f), glm::vec3(5.0f)));
}

void ReloadShaders() {
    blinn_phong->Reload();
    silhoutte->Reload();
    cel->Reload();
    minnaert->Reload();
    reflection->Reload();
    cube_map->Reload();
    cube_map->Bind();
    cube_map->SetUniform4fv("scale", glm::scale(glm::mat4(1.0f), glm::vec3(5.0f)));
}

std::shared_ptr<core::SceneInfo> peter;
std::shared_ptr<scene::Object> cone;
std::shared_ptr<scene::Object> cone_root;
void LoadModels() {
  std::shared_ptr<scene::Texture> white = std::make_shared<scene::Texture>("res/Models/textures/white.jpg");
 

  cone_root = std::make_shared<scene::Object>();
  cone_root->SetTranslation(glm::vec3(0.f, -1.0f, -18.f));
  cone_root->UpdateModelMatrix();
  std::string cone_filename = "unit_sphere.obj";
  core::SceneInfo cone_scene(cone_filename, white);
  cone = cone_scene.GetObject_(0);
  cone->SetColour(glm::vec3(1.0f, 0.f, 0.f));
  cone->SetParent(cone_root);
  cone->SetScale(glm::vec3(0.2f));
  std::string base_dir = "res/Models/Peter/";
  std::string peter_filename = "peter.obj";
  peter = std::make_shared<core::SceneInfo>(base_dir, peter_filename, white);
}

void DrawSkyBox() {
  glDepthMask(GL_FALSE);
  glDisable(GL_CULL_FACE);
  cube_map->Bind();
  glm::mat4 view;
  if (use_fp_camera) {
    FPcamera.updatePosition(glm::vec3(model_transform * glm::vec4(-0.007894f, 2.238691f, 2.166406f, 1.0f)));
    FPcamera.updateDirection(glm::vec3(model_transform * glm::vec4(0.0f, 0.0f, 1.0f, 0.0f)));
    view = FPcamera.getRotation();
  }
  else {
    view = TPcamera.getMatrix();
  }
  cube_map->SetUniform4fv("view", view);
  glm::mat4 persp_proj = glm::perspective(glm::radians(45.0f), (float)window_width / (float)window_height, 0.1f, 100.0f);
  cube_map->SetUniform4fv("proj", persp_proj);
  render::Renderer::Draw(*sky_box);
  glDepthMask(GL_TRUE);
  glEnable(GL_CULL_FACE);
}

void UpdateScene() {
  // Wait until at least 16ms passed since start of last frame (Effectively caps framerate at ~60fps)
  static const DWORD start_time = timeGetTime();
  static DWORD  last_time = 0;
  static int simulation_time = 0;
  static bool first1 = true;
  static bool first2 = true;
  DWORD curr_time = timeGetTime();
  DWORD delta = (curr_time - last_time);
  const DWORD time_reset = 7680 * 4;
  if (delta > 16)
  {
    delta = 0;
    last_time = curr_time;
    IK::CCD_Solver ccd(1000, 0.01f, 0.001f);
    ccd.Solve(cone_chain, target);
    glutPostRedisplay();
  }
}

void RenderWithShader(render::Shader* shader) {
  shader->Bind();
  glm::mat4 view = TPcamera.getMatrix();
  shader->SetUniform4fv("view", view);
  glm::mat4 persp_proj = glm::perspective(glm::radians(45.0f), (float)window_width / (float)window_height, 0.1f, 300.0f);
  shader->SetUniform4fv("proj", persp_proj);
  //render::Renderer::Draw(*cone, shader, view);
  for (unsigned int i = 0; i < peter->GetNumMeshes(); ++i) {
    render::Renderer::Draw(*peter->GetObject_(i), shader, view);
  }
  for (auto& bone : cone_bones) {
    render::Renderer::Draw(*bone->GetBoneObject(), shader, view);
  }
  cone->SetColour(glm::vec3(0.f, 1.f, 0.f));
  cone->SetTranslation(glm::vec3(target));
  cone->UpdateModelMatrix();
  render::Renderer::Draw(*cone, shader, view);
  cone->SetColour(glm::vec3(0.f, 0.f, 1.f));
  cone->SetTranslation(glm::vec3(cone_chain->GetEndEffector()));
  cone->UpdateModelMatrix();
  render::Renderer::Draw(*cone, shader, view);
}

void Render() {
  render::Renderer::Clear();
  //DrawSkyBox();
  switch (render_type) {
    case eRenderType::RT_blinn :
      RenderWithShader(blinn_phong);
      break;
    case eRenderType::RT_cel :
      glCullFace(GL_FRONT);
      silhoutte->Bind();
      silhoutte->SetUniform3f("const_colour", glm::vec3(0.0f));
      silhoutte->SetUniform1f("offset", 0.15f);
      RenderWithShader(silhoutte);
      cel->Bind();
      cel->SetUniform1f("num_shades", 8);
      cel->SetUniform3f("base_colour", glm::vec3(1.0f));
      glCullFace(GL_BACK);
      RenderWithShader(cel);
      break;
    case eRenderType::RT_minnaert :
      RenderWithShader(minnaert);
      break;
    case eRenderType::RT_reflection :
      RenderWithShader(reflection);
      break;
    default:
      break;
  }
  glutSwapBuffers();
}

void Keyboard(unsigned char key, int x, int y) {
  static ModelMatrixTransformations translationmatrices;
  static GLfloat xtranslate = -0.3f;
  bool changedmatrices = false;
  static glm::vec3 first_target = target;
  switch (key) {

  case 13: //Enter key
    //use_fp_camera = !use_fp_camera;
    //if(use_fp_camera) FPcamera.mouseUpdate(glm::vec2(mouse_x, mouse_y));
    //else FPcamera.first_click = false;
    break;

  case 27: //ESC key
    exit(0);
    break;

  case 32: //Space key
    use_quaternions = !use_quaternions;
    changedmatrices = true;
    break;
  
  case 9: //Tab key
    break;

    //Reload vertex and fragment shaders during runtime
  case 'P':
    ReloadShaders();
    std::cout << "Reloaded shaders" << std::endl;
    break;

  case 'h':
    do_rotate = !do_rotate;
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
    target = glm::vec3(model_transform * glm::vec4(first_target, 1.0f));
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
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LESS);
  glEnable(GL_CULL_FACE);
  glCullFace(GL_BACK);
  InitSkyBox();
  LoadModels();
  CreateShaders();
}

void CleanUp() {
  delete(blinn_phong);
  delete(minnaert);
  delete(cel);
  delete(silhoutte);
  delete(cube_map);
  delete(reflection);
}

void Bones() {
  //TODO make a rectangle object and attach a copy of it to each bone in the chain
  target = glm::vec3(1.0f, 8.0f, 0.0f);

  //Create a bone chain and test it.
  std::shared_ptr<IK::Bone> bone_base =
    std::make_shared<IK::Bone>(glm::vec3(0.f, 0.f, 0.f), glm::vec3(1.0f, 0.0f, 0.0f));
  std::shared_ptr<IK::Bone> bone_end =
    std::make_shared<IK::Bone>(glm::vec3(1.f, 0.f, 0.f), glm::vec3(1.0f, -1.0f, 0.0f));

  bone_base->AddChild(bone_end);
  std::shared_ptr<IK::BoneChain> chain =
    std::make_shared<IK::BoneChain>(bone_base, bone_end);

  IK::CCD_Solver ccd(1000, 0.01f, 0.001f);

  ccd.Solve(chain, glm::vec3(2.1f, 0.0f, 0.f));

  glm::vec3 ef = chain->GetEndEffector();
  std::cout << ef.x << " " << ef.y << " " << ef.z << std::endl;

  std::vector<glm::vec3> test_points;
  test_points.push_back(glm::vec3(27, -7, 4));
  test_points.push_back(glm::vec3(30, 10, 3));
  test_points.push_back(glm::vec3(45, 2, 4));
  test_points.push_back(glm::vec3(-10, 15, -30));
  std::shared_ptr<IK::BoneChain> test_chain = std::make_shared<IK::BoneChain>();
  std::vector<std::shared_ptr<IK::Bone>> test_bones;
  test_bones = test_chain->MakeChain(test_points);
  ccd.Solve(test_chain, glm::vec3(25, -18.2, -24));

  ef = test_chain->GetEndEffector();
  std::cout << ef.x << " " << ef.y << " " << ef.z << std::endl;

  //TODO draw the bones
  std::vector<glm::vec3> cone_points;
  cone_points.push_back(glm::vec3(0.0f));
  cone_points.push_back(glm::vec3(-1.0f, 4.0f, 1.0f));
  cone_points.push_back(glm::vec3(0.0f, 9.0f, 0.0f));
  cone_points.push_back(glm::vec3(0.0f, 10.0f, 0.0f));
  cone_points.push_back(glm::vec3(0.0f, 10.0f, -5.0f));
  cone_chain = std::make_shared<IK::BoneChain>();
  cone_bones = cone_chain->MakeChain(cone_points, cone);
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
  //TODO remove
  Bones();
  glutMainLoop();
  CleanUp();
  return 0;
}

