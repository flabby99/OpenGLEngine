#include "ParticleSpawner.h"

namespace physics {
  Particle * ParticleSpawner::Spawn()
  {
    int lifetime = (rand() % (max_life_ - min_life_)) + min_life_;
    glm::vec3 position = range_ * glm::vec3(rand() / double(RAND_MAX), rand() / double(RAND_MAX), rand() / double(RAND_MAX));
    glm::vec3 velocity = glm::vec3(rand() / double(RAND_MAX), rand() / double(RAND_MAX), rand() / double(RAND_MAX));
    return particle_pool_->Create(position, velocity, mass_, radius_, cor_, lifetime, mesh_);
  }
}
