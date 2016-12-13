#pragma once

#include <iostream>
#include <utility>
#include <vector>
#include <functional>
#include <unordered_map>
#include <experimental/optional>

#include "gl-inl.h"
#include "camera.h"
#include "skybox.h"

#include "scene.h"
#include "framebuffer.h"

#include <glm/mat4x4.hpp>

class Window
{
  GLFWwindow* const window;

  glm::mat4 projm;  // projection
  float scale;  // HDPI scale amount

  // using kcb_vec0 = std::vector<std::function<void(void)>>;
  using kcb2_vec      = std::vector<std::function<void(int, int)>>;
  using mouse_cb_vec  = std::vector<std::function<void(float const, float const)>>;
  using resize_cb_vec = std::vector<std::function<void(int, int)>>;

  // <key code, callbacks>
  std::unordered_map<int, kcb2_vec> some_key_callbacks;
  mouse_cb_vec                      mouse_move_callbacks;
  resize_cb_vec                     resize_callbacks;

  int width;
  int height;

  Window(GLFWwindow* window, Camera const& camera, Skybox const& skybox);

  Camera const& camera;
  Skybox const& skybox;

  Scene scene;

public:
  void scroll_callback(double xoffset, double yoffset);
  void key_callback(int key, int scancode, int action, int mods);
  void mouse_press_callback(int button, int action, int mods);
  void cursor_pos_callback(double xpos, double ypos);
  void resize_callback(int width, int height);

  static GLFWwindow *create_window();

  static std::unique_ptr<Window> create(
    int width,
    int height,
    Camera& camera,
    Skybox const& skybox,
    GLFWwindow* window
  );

  void set_fullscreen(bool fullscreen);

  GLFWwindow* raw_window() const { return this->window; }

  void on_key(int key, kcb2_vec::value_type const& callback) {
    auto iter = some_key_callbacks.find(key);
    if(iter == some_key_callbacks.end()) {
      some_key_callbacks[key] = kcb2_vec();
      iter = some_key_callbacks.find(key);
    }

    iter->second.emplace_back(callback);
  }

  void on_mouse_move(mouse_cb_vec::value_type const& callback) {
    mouse_move_callbacks.emplace_back(callback);
  }

  void on_resize(resize_cb_vec::value_type const& callback) {
    resize_callbacks.emplace_back(callback);
  }

  std::pair<double, double> get_mouse_coords() {
    double last_x, last_y;
    glfwGetCursorPos(this->window, &last_x, &last_y);
    return {last_x, last_y};
  }

  void set_should_close();
  bool should_close() const;

  void draw_skybox() const;
  void display(std::function<void(void)> const& draw_cb) const;
  void pump();
  void idle();

  Scene& get_scene() { return this->scene; }

  glm::mat4 const& get_projm() const { return this->projm; }

  std::pair<int, int> get_framebuffer_size() const;

  virtual ~Window();
};
