#pragma once
#include "Particle.h"
#include "Object.h"
#include <random>

namespace physics {
  class ParticleSpawner {
    ParticlePool* particle_pool_;
    scene::Object* mesh_;
    float cor_;
    float radius_;
    float mass_;
    float range_;
    int min_life_;
    int max_life_;
  public:
    inline ParticleSpawner(scene::Object* mesh, float cor, float radius, float mass, 
      float range, physics::ParticlePool* particle_pool, int min_life, int max_life) :
      mesh_(mesh), cor_(cor), radius_(radius), mass_(mass), 
      range_(range), particle_pool_(particle_pool), min_life_(min_life), max_life_(max_life) {}
    physics::Particle* Spawn();
  };
}