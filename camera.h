#pragma once

#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <utility>
#include <vector>
#include <functional>

extern const glm::vec3 cam_up;
extern const glm::vec3 cam_init_lookat;

class Camera {
    glm::vec3 pos_;
    double pitch, yaw;

    using ocl_cb = std::function<void(void)>;
    using ocl_vec = std::vector<ocl_cb>;
    ocl_vec on_change_listeners;

    void update_viewm();
    glm::mat4 viewm;

public:
    using aero_dir_type = std::pair<double, double>;

    Camera(glm::vec3 const& pos, glm::vec3 const& lookat);

    glm::vec3 const& up()  const;
    glm::vec3 const& pos() const;
    glm::vec3        dir() const;
    aero_dir_type aero_dir() const;
    void set_aero_dir(aero_dir_type const&);

    void on_cam_change(ocl_cb const& listener);
    void set_pos(glm::vec3 const& new_pos);

    glm::mat4 const& get_viewm() const;
};
