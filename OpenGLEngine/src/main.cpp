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

#include <iostream>
#include <math.h>
#include <fstream>
#include <string>
#include <string.h>
#include <random>
#include <time.h>

using namespace std;

Camera FPcamera(glm::vec3(0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, 1.0f, 0.0f));
Camera TPcamera;

int window_height = 0;
int window_width = 0;
int mouse_x, mouse_y;

//A matrix that is updated in the keyboard function, allows moving model around in the scene
glm::mat4 model_transform = glm::mat4(1.0f);

//Holds the information loaded from the obj files
vector<core::SceneInfo> Scene;

//Modified in the mouse function
bool left_mouse_down;
bool middle_mouse_down;
bool use_quaternions = false;

//NOTE it might be more effective to refactor this to Enum and allow many camera types
bool use_fp_camera = false;
bool use_gravity = true;
bool use_euler = false;
bool use_vortex = false;
bool use_box = true;

std::vector<physics::Plane* > cube;

void InitCube() {
  glm::vec3 right(100.f, 0.0f, 0.0f);
  glm::vec3 top(0.0f, 100.f, 0.0f);
  glm::vec3 front(0.0f, 0.0f, 100.f);
  glm::vec3* planes[3] = { &right, &top, &front };
  for (auto &plane : planes) {
    physics::Plane* side = new physics::Plane;
    side->position_ = *plane;
    side->normal_ = -glm::normalize(*plane);
    side->threshold_ = 0.02;
    cube.push_back(side);
    side = new physics::Plane;
    side->position_ = -*plane;
    side->normal_ = glm::normalize(*plane);
    side->threshold_ = 0.02;
    cube.push_back(side);
  }
}

scene::Object* sky_box;
scene::Object* particle_mesh;
physics::ParticleSpawner* sphere_spawner;

void InitSkyBox() {
    core::SceneInfo sceneinfo;
    char* filename = "res/Models/cube.obj";
    if (!sceneinfo.LoadModelFromFile(filename)) {
        fprintf(stderr, "ERROR: Could not load %s", filename);
        exit(-1);
    }
    sceneinfo.InitBuffersAndArrays();
    sky_box = sceneinfo.GetObject_(0);
    //TODO set the textures for the sky box
    const char* front = "res/Models/textures/Storforsen4/negz.jpg";
    const char* back = "res/Models/textures/Storforsen4/posz.jpg";
    const char* top = "res/Models/textures/Storforsen4/posy.jpg";
    const char* bottom = "res/Models/textures/Storforsen4/negy.jpg";
    const char* left = "res/Models/textures/Storforsen4/negx.jpg";
    const char* right = "res/Models/textures/Storforsen4/posx.jpg";
    scene::Texture* texture = new scene::Texture();
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

physics::ParticlePool* particle_pool;
void LoadModels() {
  core::SceneInfo sceneinfo;
  char* filename = "res/Models/unit_sphere.obj";
  if (!sceneinfo.LoadModelFromFile(filename)) {
    fprintf(stderr, "ERROR: Could not load %s", filename);
    exit(-1);
  }
  sceneinfo.InitBuffersAndArrays();
  scene::Object* root = new scene::Object();
  root->SetTranslation(glm::vec3(0.0f, 0.0f, -20.0f));
  root->UpdateModelMatrix();
  particle_mesh = sceneinfo.GetObject_(0);
  particle_mesh->SetColour(glm::vec3(1.0f, 1.0f, 1.0f));
  particle_mesh->SetParent(root);
  scene::Texture* texture = new scene::Texture();
  texture->SetSlot(GL_TEXTURE0);
  texture->Load("res/Models/textures/white.jpg");
  particle_mesh->SetDiffuseTexture(texture);
  scene::Texture* normal_texture = new scene::Texture();
  normal_texture->SetSlot(GL_TEXTURE1);
  normal_texture->Load("res/Models/textures/golf_test.jpg");
  particle_mesh->SetNormalTexture(normal_texture);
}

//-9.8f / 60.0f would be correct for framerate of 60
physics::Gravity gravity(glm::vec3(0.0f, -0.03f, 0.0f));
physics::Circulation circle(glm::vec3(0.0f));
void InitSpawners() {
  float radius = 0.05f;
  particle_mesh->SetScale(glm::vec3(radius));
  particle_pool = new physics::ParticlePool();
  sphere_spawner = new physics::ParticleSpawner(particle_mesh, 0.7f, radius, 1.0f, 5.0f, particle_pool, 60, 300);
  gravity.AddPool(particle_pool);
  circle.AddPool(particle_pool);
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

void Spawn() {
  int max_spawns = 4;
  int num_spawns = rand() % (max_spawns + 1);
  for (int i = 0; i <= max_spawns; ++i) {
    physics::Particle* particle = sphere_spawner->Spawn();
    if (particle == NULL) cout << "Can't spawn any more particles" << endl;
  }
}

void UpdateScene() {
  // Wait until at least 16ms passed since start of last frame (Effectively caps framerate at ~60fps)
  static DWORD  last_time = 0;
  static int simulation_time = 0;
  DWORD curr_time = timeGetTime();
  DWORD delta = (curr_time - last_time);
  if (delta > 16)
  {
    Spawn();
    particle_pool->ClearForces();
    if(use_gravity)
      gravity.AccumulateForces(0);
    else
      circle.AccumulateForces(simulation_time);
    if (use_euler)
      particle_pool->Update();
    else
      particle_pool->UpdateLeap();
    if(use_vortex)
     particle_pool->Vortex(glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f), 4.0f, 0.8f);
    if(use_box)
      particle_pool->HandleCollision(cube);
    delta = 0;
    last_time = curr_time;
    glutPostRedisplay();
    simulation_time = (simulation_time + 1) % 360;
  }
}

void RenderWithShader(render::Shader* shader) {
  shader->Bind();
  glm::mat4 view = TPcamera.getMatrix();
  shader->SetUniform4fv("view", view);
  glm::mat4 persp_proj = glm::perspective(glm::radians(45.0f), (float)window_width / (float)window_height, 0.1f, 100.0f);
  shader->SetUniform4fv("proj", persp_proj);
  //Draw the particles
  render::Renderer renderer(shader, view);
  particle_pool->Draw(renderer);
}

void Render() {
  render::Renderer::Clear();
  DrawSkyBox();
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
  switch (key) {

  case 13: //Enter key
    //use_fp_camera = !use_fp_camera;
    //if(use_fp_camera) FPcamera.mouseUpdate(glm::vec2(mouse_x, mouse_y));
    //else FPcamera.first_click = false;
    use_euler = !use_euler;
    cout << "Using euler?" << use_euler << endl;
    break;

  case 27: //ESC key
    exit(0);
    break;

  case 32: //Space key
    use_quaternions = !use_quaternions;
    use_gravity = !use_gravity;
    changedmatrices = true;
    break;
  
  case 9: //Tab key
    use_vortex = !use_vortex;
    break;

  case 'o':
    use_box = !use_box;
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
  LoadModels();
  InitSkyBox();
  InitCube();
  InitSpawners();
  CreateShaders();
}

void CleanUp() {
  delete(blinn_phong);
  delete(minnaert);
  delete(cel);
  delete(silhoutte);
  delete(cube_map);
  delete(reflection);
  delete(particle_pool);
  delete(particle_mesh);
  delete(sphere_spawner);
  for (int i = 0; i < Scene.size(); ++i)
  {
    for (int j = 0; j < Scene[i].GetNumMeshes(); ++j) {
      delete(Scene[i].GetObject_(j)->GetDiffuseTexture());
      delete(Scene[i].GetObject_(j));
    }
  }
}

int main(int argc, char** argv) {
  srand(time(NULL));
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
