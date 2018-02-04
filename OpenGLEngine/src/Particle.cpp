#include "Particle.h"

namespace physics {
  //solve dx/dt = v, dv/dt = f/m using euler
  void Particle::SimpleUpdateStep() {
    velocity_ += force_ / mass_;
    position_ += velocity_;
    ClearForces();
  }
  void Particle::HandleCollision(std::vector<Plane*> planes)
  {
    bool done = false;
    while (!done) {
      done = true;
      for (auto& plane : planes) {
        float perpendicular_distance = glm::dot((position_ - plane->position_), plane->normal_);
        if (perpendicular_distance - radius_ < plane->threshold_) {
          float perpendicular_speed = glm::dot(velocity_, plane->normal_);
          //If moving further into plane
          if (perpendicular_speed - radius_ < plane->threshold_) {
            //Repulse the object
            glm::vec3 perpendicular_velocity = perpendicular_speed * plane->normal_;
            glm::vec3 tangential_velocity = velocity_ - perpendicular_velocity;
            velocity_ = tangential_velocity - cor_ * perpendicular_velocity;
            //TODO later handle multiple collisions
            //TODO note that I need to ignore the original plane collided with
            done = true;
            break;
          }
        }
      }
    }
  }
  void Particle::UpdateMesh()
  {
    mesh_->SetTranslation(position_);
    mesh_->UpdateModelMatrix();
  }
}