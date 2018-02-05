#pragma once 
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
    glm::vec3 old_acceleration_ = glm::vec3(0.0f);
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
    inline float GetMass() const { return mass_; }
    inline glm::vec3 GetPosition() const{ return position_; }
    inline void Draw(render::Renderer renderer) { 
      if (!inUse()) return;
      renderer.Render(*mesh_);
    }
    inline scene::Object* GetMesh() const { return mesh_; }
    void SimpleUpdateStep();
    void LeapFrogUpdateStep();
    void HandleCollision(std::vector<Plane*> planes);
    void HandleCollision(std::vector<Plane*> planes, int ignore_index);
    void UpdateMesh();
  };

  //REFERENCE see Bob nystroms game programming patterns
  class ParticlePool {
    static const int POOL_SIZE = 1000;
    Particle particles_[POOL_SIZE];
  public:
    inline ParticlePool() {}
    inline ~ParticlePool() {}
    Particle* Create(glm::vec3 position, glm::vec3 velocity,
      float mass, float radius, float cor,
      int lifetime, scene::Object* mesh);
    inline int GetSize() { return POOL_SIZE; }
    inline Particle* GetParticle(int index) { return &particles_[index]; }
    void Update();
    void UpdateLeap();
    void HandleCollision(std::vector<Plane*> planes);
    void Draw(render::Renderer renderer);
    void ClearForces();
  };

  class Force {
    glm::vec3 force_;
    std::vector<ParticlePool*> particle_pools;

  public:
    inline Force(glm::vec3 force) : force_(force) {}
    inline ~Force() {}
    inline glm::vec3 const GetForce() { return force_; }
    inline void AddPool(ParticlePool* pool) { particle_pools.push_back(pool); }
    virtual inline glm::vec3 ForceFn(Particle* particle, float time) { return force_; }
    //TODO remove forces from particles when they are no longer in use
    inline void AccumulateForces(float time) {
      for (auto& particle_pool : particle_pools) {
        for (int i = 0; i < particle_pool->GetSize(); ++i) {
          Particle* particle = particle_pool->GetParticle(i);
          if (particle->inUse()) {
            glm::vec3 force = ForceFn(particle, time);
            particle->AddForce(force);
          }
        }
      }
    }
  };

  class Gravity : public Force {
  public:
    inline Gravity(glm::vec3 force) : Force(force) {}
    inline glm::vec3 ForceFn(Particle* particle, float time) override{
      return particle->GetMass() * GetForce(); 
    }
  };

  class Circulation : public Force {
  public:
    inline Circulation(glm::vec3 force) : Force(force) {}
    //If multiplied by position, splits the particles
    inline glm::vec3 ForceFn(Particle* particle, float time) override {
      float radian_time = glm::radians(time);
      return glm::vec3(cos(radian_time), sin(radian_time), sin(radian_time) * cos(radian_time) );
    }
  };
}