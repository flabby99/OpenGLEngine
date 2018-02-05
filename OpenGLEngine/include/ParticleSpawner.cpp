#include "ParticleSpawner.h"
#include "util.h"

namespace physics {
  Particle * ParticleSpawner::Spawn()
  {
    int lifetime = (rand() % (max_life_ - min_life_ + 1)) + min_life_;
    glm::vec3 position = range_ * glm::vec3(core::frand_s(), core::frand_s(), core::frand_s());
    glm::vec3 velocity = glm::vec3(core::frand_s(), core::frand_s(), core::frand_s());
    return particle_pool_->Create(position, velocity, mass_, radius_, cor_, lifetime, mesh_);
  }
}
