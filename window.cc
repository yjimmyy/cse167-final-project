#include "window.h"
#include "shader.h"
#include "util.h"

#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <string>
#include <memory>

#if GLFW_VERSION_MAJOR != 3
#error "cannot compile without 3.x GLFW"
#endif

std::string const window_title = "GLFW Starter Project";

static Window* current_window;

Window::~Window() {
  if(this != current_window) {
    std::cerr << "internal error! expected the only constructed window to be current_window" << std::endl;
    assert(this == current_window);
  }
  else {
    // Destroy the window
    glfwDestroyWindow(this->window);
    current_window = nullptr;
  }
}

static void raw_scroll_callback(GLFWwindow *window, double xoffset, double yoffset) {
  assert(window == current_window->raw_window());
  current_window->scroll_callback(xoffset, yoffset);
}
static void raw_key_callback(GLFWwindow *window, int key, int scancode, int action, int mods) {
  assert(window == current_window->raw_window());
  current_window->key_callback(key, scancode, action, mods);
}
static void raw_mouse_press_callback(GLFWwindow *window, int button, int action, int mods) {
  assert(window == current_window->raw_window());
  current_window->mouse_press_callback(button, action, mods);
}
static void raw_cursor_pos_callback(GLFWwindow *window, double xpos, double ypos) {
  assert(window == current_window->raw_window());
  current_window->cursor_pos_callback(xpos, ypos);
}
static void raw_resize_callback(GLFWwindow *window, int width, int height) {
  assert(window == current_window->raw_window());
  current_window->resize_callback(width, height);
}

static void setup_callbacks(GLFWwindow* window)
{
  // Set the key callback
  glfwSetScrollCallback(window, raw_scroll_callback);
  glfwSetKeyCallback(window, raw_key_callback);
  glfwSetMouseButtonCallback(window, raw_mouse_press_callback);
  glfwSetCursorPosCallback(window, raw_cursor_pos_callback);
  glfwSetFramebufferSizeCallback(window, raw_resize_callback);
}

static GLFWmonitor* get_current_monitor(GLFWwindow *window)
{
  int nmonitors, i;
  int wx, wy, ww, wh;
  int mx, my, mw, mh;
  int overlap, bestoverlap;
  GLFWmonitor *bestmonitor;
  GLFWmonitor **monitors;
  const GLFWvidmode *mode;

  bestoverlap = 0;
  bestmonitor = NULL;

  glfwGetWindowPos(window, &wx, &wy);
  glfwGetWindowSize(window, &ww, &wh);
  monitors = glfwGetMonitors(&nmonitors);

  return monitors[nmonitors-1];

  for (i = 0; i < nmonitors; i++) {
    mode = glfwGetVideoMode(monitors[i]);
    glfwGetMonitorPos(monitors[i], &mx, &my);
    mw = mode->width;
    mh = mode->height;

    overlap =
      std::max(0, std::min(wx + ww, mx + mw) - std::max(wx, mx)) *
      std::max(0, std::min(wy + wh, my + mh) - std::max(wy, my));

    if(bestoverlap < overlap) {
      bestoverlap = overlap;
      bestmonitor = monitors[i];
    }
  }

  return bestmonitor;
}

static void init_glew() {
  // Initialize GLEW. Not needed on OSX systems.
  glewExperimental = GL_TRUE;
  GLenum err = glewInit();
  if (GLEW_OK != err)
  {
    /* Problem: glewInit failed, something is seriously wrong. */
    std::cerr << "GLEW error: " << glewGetErrorString(err) << std::endl;
    glfwTerminate();
    return;
  }

  std::cerr << "Current GLEW version: " << glewGetString(GLEW_VERSION) << std::endl;
}

static void init_opengl() {
  // Enable depth buffering
  glEnable(GL_DEPTH_TEST);
  // Related to shaders and z value comparisons for the depth buffer
  glDepthFunc(GL_LEQUAL);
  // Set polygon drawing mode to fill front and back of each polygon
  // You can also use the paramter of GL_LINE instead of GL_FILL to see wireframes
  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  // Disable backface culling to render both sides of polygons
  glEnable(GL_CULL_FACE);
  glCullFace(GL_BACK);

  // Set clear color
  glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // Black - the dark of ages past!
}


GLFWwindow* Window::create_window() {
  // Initialize GLFW.
  if (!glfwInit())
  {
    std::cerr << "Failed to initialize GLFW" << std::endl;
    return nullptr;
  }

  // 4x antialiasing
  glfwWindowHint(GLFW_SAMPLES, 4);

  // Ensure that minimum OpenGL version is 3.3
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  // Enable forward compatibility and allow a modern OpenGL context
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

  GLFWwindow* window = glfwCreateWindow(640, 480, window_title.c_str(), nullptr, nullptr);

  // Check if the window could not be created
  if(!window)
  {
    std::cerr << "Failed to open GLFW window." << std::endl;
    glfwTerminate();
    return nullptr;
  }

  // Make the context of the window
  glfwMakeContextCurrent(window);

  // Set swap interval to 1
  glfwSwapInterval(1);

  init_opengl();
  init_glew();

  return window;
}

std::unique_ptr<Window> Window::create(
  int width,
  int height,
  Camera&       camera,
  Skybox const& skybox,
  GLFWwindow* window
) {
  if(current_window != nullptr) {
    std::cerr << "attempt to construct multiple active window contexts!" << std::endl;
    return nullptr;
  }

  setup_callbacks(window);

  auto ret = new Window(window, camera, skybox);
  current_window = ret;

#ifdef OSX
  ret->set_fullscreen(true);
#endif
#ifdef Linux
  ret->set_fullscreen(false);
#endif

  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

  return std::unique_ptr<Window>(ret);
}

void Window::set_fullscreen(bool fullscreen) {
  int width = 640;
  int height = 480;
  GLFWmonitor* primary = get_current_monitor(window);
  assert(primary);

  if(fullscreen) {
    // resize to max size of monitor
    // GLFWmonitor* primary = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(primary);
    assert(mode);
    width = mode->width;
    height = mode->height;
#ifdef Linux
    width -= 100;
    height -= 100;
#endif
  }

#if GLFW_VERSION_MINOR >= 2
  if(fullscreen) {
    glfwSetWindowMonitor(window, primary, 0, 0, width, height, GLFW_DONT_CARE);
    glfwFocusWindow(window);
  }
  else {
    glfwSetWindowSize(window, width, height);
    glfwSetWindowPos(window, 0, 0);
  }
#else
  glfwSetWindowSize(window, width, height);
  glfwSetWindowPos(window, 0, 0);
#endif

  // Call the resize callback to make sure things get drawn immediately
  int fwidth, fheight;
  glfwGetFramebufferSize(window, &fwidth, &fheight);
  current_window->resize_callback(fwidth, fheight);
}


Window::Window(GLFWwindow* window, Camera const& camera, Skybox const& skybox)
 : window(window), camera(camera), skybox(skybox)
{}

std::pair<int, int> Window::get_framebuffer_size() const {
  return {this->width, this->height};
}

void Window::resize_callback(int width, int height)
{
  this->width = width;
  this->height = height;

  int wwidth, wheight;
  glfwGetWindowSize(this->window, &wwidth, &wheight);
  this->scale = static_cast<float>(width) / static_cast<float>(wwidth);

  // update opengl viewport matrix
  glViewport(0, 0, width, height);

  if (height > 0)
  {
    this->projm = glm::perspective(45.0f, (float)width / (float)height, 0.1f, 1000.0f);
  }

  call_all(this->resize_callbacks, width, height);
}

void Window::cursor_pos_callback(double x, double y)
{
  call_all(mouse_move_callbacks, static_cast<float>(x), static_cast<float>(y));
}

void Window::mouse_press_callback(int button, int action, int mods)
{}

void Window::scroll_callback(double xoffset, double yoffset)
{}

void Window::display(std::function<void(void)> const& draw_cb) const {
  draw_cb();
  glfwSwapBuffers(window);
}

void Window::draw_skybox() const {
  glDepthMask(GL_FALSE);
  auto const skybox_pos = glm::translate(this->camera.get_viewm(), camera.pos());
  this->skybox.draw(skybox_pos, this->projm);
  glDepthMask(GL_TRUE);
}

void Window::pump() {
  glfwPollEvents();
}

void Window::idle() {
}

void Window::set_should_close() {
  glfwSetWindowShouldClose(this->window, GL_TRUE);
}

bool Window::should_close() const {
  return glfwWindowShouldClose(this->window);
}

enum {
  W_KEY,
  A_KEY,
  S_KEY,
  D_KEY,
  SP_KEY,
  SF_KEY
};
static bool dir_buttons_down[6] = {false};

void Window::key_callback(int key, int scancode, int action, int mods)
{
  const bool shift_pressed = mods & GLFW_MOD_SHIFT;

  auto kb = this->some_key_callbacks.find(key);
  if(kb != this->some_key_callbacks.end()) {
    call_all(kb->second, action, mods);
  }
}

static glm::vec3 track_ball_mapping(double width, double height, double x, double y)
{
  glm::vec3 v;
  float d;
  v.x = (2.0*x - width) / width;
  v.y = (height - 2.0*y) / height;
  v.z = 0.0;
  d = glm::clamp(glm::length(v), 0.0f, 1.0f);
  v.z = glm::sqrt(1.0 - d*d);

  return glm::normalize(v);
}
