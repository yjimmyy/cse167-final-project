#include "camera.h"
#include "util.h"

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_transform.hpp>

extern const glm::vec3 cam_up(0.0f, 1.0f, 0.0f);       // up | What orientation "up" is
extern const glm::vec3 cam_init_lookat(0.0f, 0.0f, 0.0f);

Camera::Camera(glm::vec3 const& pos, glm::vec3 const& lookat) : pos_(pos) {
    auto const dir = glm::normalize(pos - lookat);
    this->pitch = glm::asin(-dir.y);
    this->yaw   = atan2(dir.x, dir.z) + glm::pi<float>();
    this->update_viewm();
}

std::pair<double, double> Camera::aero_dir() const {
    return {this->pitch, this->yaw};
}

void Camera::set_aero_dir(std::pair<double, double> const& d) {
  this->pitch = glm::clamp(d.first, -glm::pi<double>()/2+0.001, glm::pi<double>()/2-0.001);
  this->yaw =   d.second;
  this->update_viewm();
}

glm::vec3 const& Camera::pos() const {
    return this->pos_;
}

glm::vec3 const& Camera::up() const {
  return cam_up;
}

glm::vec3 Camera::dir() const {
  glm::vec3 ret;
  ret.x = cos(pitch) * cos(yaw);
  ret.y = sin(pitch);
  ret.z = cos(pitch) * sin(yaw);
  return ret;
}

void Camera::set_pos(glm::vec3 const& new_pos) {
  bool signal_changed = new_pos != this->pos_;
  this->pos_ = new_pos;
  if(signal_changed) {
    this->update_viewm();
    call_all(this->on_change_listeners);
  }
}

void Camera::update_viewm() {
  this->viewm = glm::lookAt(this->pos_, this->pos_+this->dir(), this->up());
}

glm::mat4 const& Camera::get_viewm() const {
  return this->viewm;
}

void Camera::on_cam_change(ocl_cb const& listener) {
  this->on_change_listeners.push_back(listener);
}
