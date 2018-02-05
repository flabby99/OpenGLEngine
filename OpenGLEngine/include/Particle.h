#pragma 
#include "glm\glm.hpp"
#include "Object.h"
#include "Renderer.h"
#include <vector>

namespace physics {
  //The normal points in the legal direction
  struct Plane {
    glm::vec3 position_;
    glm::vec3 normal_;
    float threshold_;
  };

  //These particle could shrink in size over time, eventually dying
  //And freeing part of a pool, allowing for more particles to spawn
  //TODO See the prototype pattern in game programming patterns page 59 for spawning ideas
  //Furthermore page 305 gives ideas for a pool!
  class Particle {
    float mass_ = 1.0f;
    float radius_ = 0.0f;
    float cor_ = 0.7f;
    glm::vec3 position_;
    glm::vec3 velocity_;
    glm::vec3 force_;
    scene::Object* mesh_;
    //For a paricle to die, it stores a frames remaining
    int frames_remaining_;

  public:
    inline Particle() : position_(glm::vec3(0.0f)), velocity_(glm::vec3(0.0f)), force_(glm::vec3(0.0f)) {}
    inline ~Particle() {}
    void Init(glm::vec3 position, glm::vec3 velocity,
      float mass, float radius, float cor,
      int lifetime, scene::Object* mesh);
    inline void ClearForces() { force_ = glm::vec3(0.0f); }
    inline void AddForce(const glm::vec3& force) { 
      if (!inUse()) return;
      force_ += force; 
    }
    inline void SetMesh(scene::Object* mesh) { mesh_ = mesh; }
    inline void SetMass(float mass) { mass_ = mass; }
    inline void SetVelocity(glm::vec3 velocity) { velocity_ = velocity; }
    inline void SetRadius(float radius) { radius_ = radius; }
    inline void SetFramesRemaining(int frames) { frames_remaining_ = frames; }
    inline bool inUse() const { return frames_remaining_ > 0; }
    inline void Draw(render::Renderer renderer) { 
      if (!inUse()) return;
      renderer.Render(*mesh_);
    }
    inline scene::Object* GetMesh() const { return mesh_; }
    void SimpleUpdateStep();
    void HandleCollision(std::vector<Plane*> planes);
    void HandleCollision(std::vector<Plane*> planes, int ignore_index);
    void UpdateMesh();
  };

  //REFERENCE see Bob nystroms game programming patterns
  class ParticlePool {
    static const int POOL_SIZE = 1000;
    Particle particles_[POOL_SIZE];
  public:
    Particle* Create(glm::vec3 position, glm::vec3 velocity,
      float mass, float radius, float cor,
      int lifetime, scene::Object* mesh);
    void Update();
    void HandleCollision(std::vector<Plane*> planes);
    void Draw(render::Renderer renderer);
    void ClearForces();
  };

  class Force {
    glm::vec3 force_;
    std::vector<Particle*> particles_;

  public:
    inline Force(glm::vec3 force) : force_(force) {}
    inline ~Force() {}
    inline void AddParticle(Particle* particle) { particles_.push_back(particle); }
    inline void AccumulateForces() { 
      for (auto& particle : particles_) 
        particle->AddForce(force_); }
  };
}