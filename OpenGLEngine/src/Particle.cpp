#include "Particle.h"

namespace physics {
  void Particle::Init(glm::vec3 position, glm::vec3 velocity,
    float mass, float radius, float cor,
    int lifetime, scene::Object* mesh) {
    position_ = position;
    velocity_ = velocity;
    mass_ = mass;
    radius_ = radius;
    cor_ = cor;
    force_ = glm::vec3(0.0f);
    frames_remaining_ = lifetime;
    mesh_ = mesh;
    force_ = glm::vec3(0.0f);
  }

  //solve dx/dt = v, dv/dt = f/m using euler
  void Particle::SimpleUpdateStep() {
    if (!inUse()) return;
    velocity_ += force_ / mass_;
    position_ += velocity_;
    --frames_remaining_;
  }
  void Particle::HandleCollision(std::vector<Plane*> planes)
  {
    if (!inUse()) return;
    for (int i = 0; i < planes.size(); ++i) {
      Plane* plane = planes[i];
      float perpendicular_distance = glm::dot((position_ - plane->position_), plane->normal_);
      if (perpendicular_distance - radius_ < plane->threshold_) {
        float perpendicular_speed = glm::dot(velocity_, plane->normal_);
        //Close contact, is the particle moving into the wall?
        if (perpendicular_speed < plane->threshold_) {
          //Repulse the object
          glm::vec3 perpendicular_velocity = perpendicular_speed * plane->normal_;
          glm::vec3 tangential_velocity = velocity_ - perpendicular_velocity;
          velocity_ = tangential_velocity - cor_ * perpendicular_velocity;
          glm::vec3 positional_correction = (radius_ + plane->threshold_ - perpendicular_distance + 0.001f) * plane->normal_;
          position_ += positional_correction;
          //Recursively handle multiple collisions
          HandleCollision(planes, i % planes.size());
          break;
        }
      }
    }
  }
  //This is so that multiple collisions can be handled
  void Particle::HandleCollision(std::vector<Plane*> planes, int ignore_index) {
    for (int i = (ignore_index + 1) % planes.size(); i != ignore_index; i = (i + 1) % planes.size()) {
      Plane* plane = planes[i];
      float perpendicular_distance = glm::dot((position_ - plane->position_), plane->normal_);
      if (perpendicular_distance - radius_ < plane->threshold_) {
        float perpendicular_speed = glm::dot(velocity_, plane->normal_);
        //If moving further into plane
        if (perpendicular_speed < plane->threshold_) {
          //Repulse the object
          glm::vec3 perpendicular_velocity = perpendicular_speed * plane->normal_;
          glm::vec3 tangential_velocity = velocity_ - perpendicular_velocity;
          velocity_ = tangential_velocity - cor_ * perpendicular_velocity;
          glm::vec3 positional_correction = (radius_ + plane->threshold_ - perpendicular_distance + 0.001f) * plane->normal_;
          position_ += positional_correction;
          HandleCollision(planes, i);
        }
      }
    }
  }

  void Particle::UpdateMesh()
  {
    if (!inUse()) return;
    mesh_->SetTranslation(position_);
    mesh_->UpdateModelMatrix();
  }
  Particle* ParticlePool::Create(glm::vec3 position, glm::vec3 velocity, float mass, float radius, float cor, int lifetime, scene::Object* mesh)
  {
    for (int i = 0; i < POOL_SIZE; ++i) {
      if (!particles_[i].inUse()) {
        particles_[i].Init(position, velocity, mass, radius, cor, lifetime, mesh);
        return &particles_[i];
      }
    }
    return NULL;
  }
  void ParticlePool::Update()
  {
    for (int i = 0; i < POOL_SIZE; ++i) {
      particles_[i].SimpleUpdateStep();
    }
  }
  void ParticlePool::HandleCollision(std::vector<Plane*> planes)
  {
    for (int i = 0; i < POOL_SIZE; ++i) {
      particles_[i].HandleCollision(planes);
    }
  }
  void ParticlePool::Draw(render::Renderer renderer)
  {
    for (int i = 0; i < POOL_SIZE; ++i) {
      particles_[i].UpdateMesh();
      particles_[i].Draw(renderer);
    }
  }
  void ParticlePool::ClearForces()
  {
    for (int i = 0; i < POOL_SIZE; ++i) {
      particles_[i].ClearForces();
    }
  }
}