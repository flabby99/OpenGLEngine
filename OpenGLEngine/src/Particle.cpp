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
    mesh_->SetTranslation(position_);
    mesh_->UpdateModelMatrix();
  }
}