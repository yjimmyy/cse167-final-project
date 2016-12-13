#include <iostream>
#include <vector>
#include <iomanip>
#include <array>

// TODO: replace rand() with actual c++ random number generator
#include <cstdlib>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "gl-inl.h"
#include "window.h"
#include "OBJObject.h"
#include "quad_mesh.h"
#include "skybox.h"
#include "util.h"
#include "shader.h"
#include "framebuffer.h"

void error_callback(int error, const char* description)
{
  // Print error
  std::cerr << "err " << error << ": " << description << std::endl;
}

void print_versions()
{
  std::cout << "GLFW version: " << GLFW_VERSION_MAJOR << "." << GLFW_VERSION_MINOR << "." << GLFW_VERSION_REVISION << std::endl;
  // Get info of GPU and supported OpenGL version
  std::cout << "Renderer: " << glGetString(GL_RENDERER) << std::endl;
  std::cout << "OpenGL version supported " << glGetString(GL_VERSION) << std::endl;

  //If the shading language symbol is defined
#ifdef GL_SHADING_LANGUAGE_VERSION
  std::cout << "Supported GLSL version is " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;
#endif
}

enum {
  W_KEY,
  A_KEY,
  S_KEY,
  D_KEY,
  SP_KEY,
  SF_KEY
};

#define LOAD_SHADER(ident, vert_path, frag_path) \
  auto ident = load_shaders(vert_path, frag_path); \
  if(!ident) { \
    std::cerr << "unable to load " #ident << std::endl; \
    return -5; \
  }

int main(int const argc, char const** argv)
{
  glfwInit();
  glfwSetErrorCallback(error_callback);

  auto glfw_window = Window::create_window();
  print_versions();

  // camera({position}, {direction})
  Camera camera({2, 2, 2}, {0, 0, 0});
  auto skybox = Skybox::create("./skybox/hd");
  if(!skybox) {
    std::cerr << "error loading skybox :(" << std::endl;
    return -2;
  }
  std::unique_ptr<Window> window = Window::create(640, 480, camera, *skybox, glfw_window);

  if(!window) {
    std::cerr << "unable to create window!" << std::endl;
    return -3;
  }

  Window& w = *window;

  w.on_key(GLFW_KEY_Q, [&](int action, int mods) {
    w.set_should_close();
    std::cout << "exiting application..." << std::endl;
  });

  std::array<bool, 6> dir_buttons_down = {false};
  w.on_key(GLFW_KEY_W, [&](int action, int mods) {
    dir_buttons_down[W_KEY] = (action != GLFW_RELEASE);
  });
  w.on_key(GLFW_KEY_A, [&](int action, int mods) {
    dir_buttons_down[A_KEY] = (action != GLFW_RELEASE);
  });
  w.on_key(GLFW_KEY_S, [&](int action, int mods) {
    dir_buttons_down[S_KEY] = (action != GLFW_RELEASE);
  });
  w.on_key(GLFW_KEY_D, [&](int action, int mods) {
    dir_buttons_down[D_KEY] = (action != GLFW_RELEASE);
  });
  w.on_key(GLFW_KEY_SPACE, [&](int action, int mods) {
    dir_buttons_down[SP_KEY] = (action != GLFW_RELEASE);
  });
  w.on_key(GLFW_KEY_LEFT_SHIFT, [&](int action, int mods) {
    dir_buttons_down[SF_KEY] = (action != GLFW_RELEASE);
  });

  bool is_paused = false;
  w.on_key(GLFW_KEY_ESCAPE, [&](int action, int) {
    if(action != GLFW_PRESS) return;

    is_paused = !is_paused;
    if(is_paused) {
      dir_buttons_down = {false};
      glfwSetInputMode(glfw_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    } else {
      glfwSetInputMode(glfw_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    }
  });

  bool fullscreen = false;
  w.on_key(GLFW_KEY_F, [&](int action, int mods) {
    if (action == GLFW_PRESS) {
      fullscreen = !fullscreen;
      w.set_fullscreen(fullscreen);
    }
  });

#if OSX
  // for osx
  static const float mouse_const = 0.1f;
#else
  // for linux
  static const float mouse_const = 0.1f;
#endif

  std::pair<double, double> last_pos = w.get_mouse_coords();
  w.on_mouse_move([&last_pos, &camera, &is_paused](double const xpos, double const ypos) {
    auto xdiff = xpos - last_pos.first;
    auto ydiff = last_pos.second - ypos;
    const float velocity = glm::length(glm::vec2(xdiff, ydiff));
    auto dir = camera.aero_dir();
    if(!is_paused && velocity > 0.001) {
      dir.first  += glm::radians(ydiff * mouse_const);
      dir.second += glm::radians(xdiff * mouse_const);
      camera.set_aero_dir(dir);
    }
    last_pos = std::make_pair(xpos, ypos);
  });

  QuadMesh quad_mesh;
  auto const obj_path = "objs/bust1.obj";
  auto scene_mesh = OBJObject::build_from_path(obj_path);
  if(!scene_mesh) {
    std::cerr << "unable to load " << obj_path << std::endl;
    return -4;
  }

  auto const sphere_path = "objs/untitled6.obj";
  auto light_obj = OBJObject::build_from_path(sphere_path);
  if(!light_obj) {
    std::cerr << "unable to load " << sphere_path << std::endl;
    return -4;
  }
  light_obj->set_pos(glm::vec3(4.0f, 4.0f, -4.0f));
  light_obj->set_light(true);

  auto normal_shader = load_shaders("shaders/identity_geom.vert", "shaders/yellow_normal_line.frag", {"shaders/normals.geom"});
  if(!normal_shader) {
    std::cerr << "unable to load normal shader" << std::endl;
    return -6;
  }

  LOAD_SHADER(dir_lighting_shader, "shaders/identity.vert",    "shaders/dir_lighting.frag");
  LOAD_SHADER(geom_pass_shader,    "shaders/geometry.vert",    "shaders/geometry.frag");
  LOAD_SHADER(ssao_shader,         "shaders/ssao.vert",        "shaders/ssao.frag");
  LOAD_SHADER(blur_shader,         "shaders/quad_ident.vert",  "shaders/blur.frag");
  LOAD_SHADER(bright_shader,       "shaders/quad_ident.vert",  "shaders/bright.frag");
  LOAD_SHADER(combine_shader,      "shaders/quad_ident.vert",  "shaders/combine2_fb.frag");
  LOAD_SHADER(tex_ident_shader,    "shaders/quad_ident.vert",  "shaders/tex_ident.frag");
  LOAD_SHADER(light_source_shader,    "shaders/identity.vert",  "shaders/light_source.frag");
  LOAD_SHADER(godray_shader,    "shaders/quad_ident.vert",  "shaders/light_scattering.frag");

  // glm::vec3 obj_color      ( 0.25f, 0.5f,  0.75f);
  glm::vec3 obj_color      ( 0.9f,  0.9f,  0.91f);
  glm::vec3 dir_light_pos  ( 5.0f,  5.0f,  5.0f);
  glm::vec3 dir_light_color( 1.0f,  1.0f,  1.0f);

  // set up dir/color of directional lighting
  {
    glUseProgram(*dir_lighting_shader);
    set_vec3_uniform(*dir_lighting_shader, "dir_light.pos",   dir_light_pos);
    set_vec3_uniform(*dir_lighting_shader, "dir_light.color", dir_light_color);
    set_vec3_uniform(*dir_lighting_shader, "obj_color",       obj_color);
  }

  double last_time = glfwGetTime();

  auto handle_camera_movement = [&] {
    if(is_paused) return;

    auto const dir = glm::normalize(camera.dir());
    auto       pos = camera.pos();
    auto const up  = camera.up();
    auto cross_dir = glm::normalize(glm::cross(dir, up));

    auto const cam_march_mult = 0.1f;

    if(dir_buttons_down[W_KEY]) {
      pos += dir*cam_march_mult;
    }
    if(dir_buttons_down[S_KEY]) {
      pos -= dir*cam_march_mult;
    }
    if(dir_buttons_down[A_KEY]) {
      pos -= cross_dir*cam_march_mult;
    }
    if(dir_buttons_down[D_KEY]) {
      pos += cross_dir*cam_march_mult;
    }
    if(dir_buttons_down[SP_KEY]) {
      pos += up*cam_march_mult;
    }
    if(dir_buttons_down[SF_KEY]) {
      pos -= up*cam_march_mult;
    }
    camera.set_pos(pos);
  };

  // for basic directional lighting
  ShaderOpts dir_light_opts(*dir_lighting_shader, GL_TRIANGLES);
  dir_light_opts.set_camera_pos_uniform("cam_pos");

  ShaderOpts light_source_opts(*light_source_shader, GL_TRIANGLES);

  // for rendering into the depth buffer for SSAO pass later
  ShaderOpts geom_pass_options(*geom_pass_shader, GL_TRIANGLES);

  // for rendering normal directions as yellow lines
  ShaderOpts normal_opts(*normal_shader, GL_POINTS);

  Framebuffer g_framebuffer      (w, true,  true);  // geometry fragment position
  Framebuffer ao_framebuffer     (w, false, true); // AO pixel value multiplier into this
  Framebuffer blurred_fb_x       (w, false, true, 0.5f);  // AO after being gaussian blurred in X direction
  Framebuffer blurred_fb_y       (w, false, true, 0.8f);  // AO after being gaussian blurred in Y direction
  // Framebuffer bloom_fb1          (w, false, true);  // bloom framebuffer
  Framebuffer bloom_fb           (w, false, true);  // bloom framebuffer
  Framebuffer blur_bloom_fb_x1  (w, false, true, 0.6f);  // combine AO and full scene into this
  Framebuffer blur_bloom_fb_y1  (w, false, true, 0.6f);  // combine AO and full scene into this
  Framebuffer blur_bloom_fb_x2  (w, false, true, 0.2f);  // combine AO and full scene into this
  Framebuffer blur_bloom_fb_y2  (w, false, true, 0.2f);  // combine AO and full scene into this
  Framebuffer scene_framebuffer  (w, false, true);  // render fully lit scence into this
  Framebuffer combine_framebuffer(w, false, true);  // combine AO and full scene into this
  Framebuffer light_source_fb    (w, false, true);  // combine AO and full scene into this
  Framebuffer godray_fb          (w, false, true);  // combine AO and full scene into this

  const TexUnit g_tex_pair     = {GL_TEXTURE1, 1};
  const TexUnit ao_tex_pair    = {GL_TEXTURE2, 2};
  const TexUnit blur_pair      = {GL_TEXTURE3, 3};
  const TexUnit combine_a_pair = {GL_TEXTURE4, 4};
  const TexUnit combine_b_pair = {GL_TEXTURE5, 5};
  const TexUnit combine_c_pair = {GL_TEXTURE6, 6};
  const TexUnit bright_pair    = {GL_TEXTURE7, 7};
  const TexUnit tex_ident_pair = {GL_TEXTURE8, 8};
  const TexUnit godray_pair    = {GL_TEXTURE9, 9};
  const TexUnit combine_d_pair = {GL_TEXTURE10, 10};

  bool debug_channels = true;
  auto const set_debug_channels = [&] {
    glUseProgram(*combine_shader);
    set_generic_uniform(*combine_shader, "debug_channels", [&](auto const loc) {
      std::cout << "debug channels? " << debug_channels << std::endl;
      glUniform1i(loc, debug_channels ? 1 : 0);
    });
  };

  w.on_key(GLFW_KEY_J, [&](auto const action, auto const mods) {
    if(action != GLFW_PRESS) return;
    debug_channels = !debug_channels;
    set_debug_channels();
  });

  {
    glUseProgram(*combine_shader);
    set_generic_uniform(*combine_shader, "fba", [&](auto const loc) {
      glUniform1i(loc, combine_a_pair.idx);
    });
    set_generic_uniform(*combine_shader, "fbb", [&](auto const loc) {
      glUniform1i(loc, combine_b_pair.idx);
    });
    set_generic_uniform(*combine_shader, "fbc", [&](auto const loc) {
      glUniform1i(loc, combine_c_pair.idx);
    });
    set_generic_uniform(*combine_shader, "fbd", [&](auto const loc) {
      glUniform1i(loc, combine_d_pair.idx);
    });
    // set_debug_channels();

    glUseProgram(*blur_shader);
    set_generic_uniform(*blur_shader, "tex", [&](auto const loc) {
      glUniform1i(loc, blur_pair.idx);
    });

    glUseProgram(*bright_shader);
    set_generic_uniform(*bright_shader, "tex", [&](auto const loc) {
      glUniform1i(loc, bright_pair.idx);
    });

    glUseProgram(*tex_ident_shader);
    set_generic_uniform(*tex_ident_shader, "tex", [&](auto const loc) {
      glUniform1i(loc, tex_ident_pair.idx);
    });

    glUseProgram(*godray_shader);
    set_generic_uniform(*godray_shader, "screen_texture", [&](auto const loc) {
      glUniform1i(loc, godray_pair.idx);
    });
  }

  // configure ssao shader
  {
    glUseProgram(*ssao_shader);
    auto const kernel_size = 48;
    glm::vec3 kernel[kernel_size];

    for (uint i = 0 ; i < kernel_size ; i++) {
      float scale = (float)i / (float)(kernel_size);
      glm::vec3 v;
      v.x = (2.0f * (float)rand()/RAND_MAX) - 1.0f;
      v.y = (2.0f * (float)rand()/RAND_MAX) - 1.0f;
      v.z = (2.0f * (float)rand()/RAND_MAX) - 1.0f;
      // Use an acceleration function so more points are
      // located closer to the origin
      v = v * (0.1f + 0.9f * scale * scale);

      kernel[i] = v;
      std::cout << "setting to " << v << std::endl;
    }

    set_generic_uniform(*ssao_shader, "gSampleRad", [&](auto const loc) {
      glUniform1f(loc, 1.5f);
    });
    set_generic_uniform(*ssao_shader, "gPositionMap", [&](auto const loc) {
      glUniform1i(loc, g_tex_pair.idx);
    });
    set_generic_uniform(*ssao_shader, "gKernel", [&](auto const loc) {
      glUniform3fv(loc, kernel_size, (const GLfloat*)&kernel[0]);
    });
  }

  // configure directional lighting shader
  {
    auto const on_resize = [&](int, int) {
      std::cout << "sending new projection matrix to SSAO matrix" << std::endl;
      glUseProgram(*ssao_shader);
      set_generic_uniform(*ssao_shader, "gProj", [&](auto const loc) {
        glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(w.get_projm()));
      });
    };

    w.on_resize(on_resize);
    on_resize(0, 0);
  }

  std::function<void(void)> draw_cb = [&] {
    // geometry pass
    glUseProgram(geom_pass_options.shader);
    g_framebuffer.bind_for_writing();
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    Scene::render(camera, w, *scene_mesh, geom_pass_options); // sets up model/view/proj matrixes in geom pass shader

    // ao pass
    glUseProgram(*ssao_shader);
    g_framebuffer.bind_for_reading(g_tex_pair.unit);
    ao_framebuffer.bind_for_writing();
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    quad_mesh.draw(GL_TRIANGLES);

    for(int i = 0; i < 1; i++) {
      // blur the ao in the 'x' direction
      glUseProgram(*blur_shader);
      set_generic_uniform(*blur_shader, "dimention", [](auto const loc) {
        glUniform1i(loc, 0); // x dimention blur, first
      });
      blurred_fb_x.bind_for_writing(); // just using this as a temp buffer
      ao_framebuffer.bind_for_reading(blur_pair.unit);
      glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
      quad_mesh.draw(GL_TRIANGLES);

      // blur the ao in the 'y' direction (back into AO FB)
      set_generic_uniform(*blur_shader, "dimention", [](auto const loc) {
        glUniform1i(loc, 1); // y dimention blur
      });
      ao_framebuffer.bind_for_writing(); // just using this as a temp buffer
      blurred_fb_x.bind_for_reading(blur_pair.unit);
      glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
      quad_mesh.draw(GL_TRIANGLES);
    }

    // render full scene into scene buffer
    glUseProgram(dir_light_opts.shader);
    scene_framebuffer.bind_for_writing();
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    w.draw_skybox();
    Scene::render(camera, w, *scene_mesh, dir_light_opts);

    glUseProgram(*light_source_shader);
    light_source_fb.bind_for_writing();
    glClearColor(0.21f, 0.21f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    Scene::render(camera, w, *scene_mesh, light_source_opts);
    Scene::render(camera, w, *light_obj, light_source_opts);

    glDisable(GL_DEPTH_TEST);
    glUseProgram(*godray_shader);
    glm::vec3 light_screen_pos = glm::project(
      glm::vec3(0.0f),
      camera.get_viewm() * light_obj->get_modelm(),
      window->get_projm(),
      glm::vec4(0.0f, 0.0f, 1.0f, 1.0f)
    );
    glUniform2fv(glGetUniformLocation(*godray_shader, "light_pos"), 1, glm::value_ptr(light_screen_pos));
    godray_fb.bind_for_writing();
    //glBindFramebuffer(GL_FRAMEBUFFER, 0);
    light_source_fb.bind_for_reading(godray_pair.unit);
    quad_mesh.draw(GL_TRIANGLES);

    {
      // blur the ao in the 'x' direction
      glUseProgram(*blur_shader);
      set_generic_uniform(*blur_shader, "dimention", [](auto const loc) {
        glUniform1i(loc, 0); // x dimention blur, first
      });
      blur_bloom_fb_x2.bind_for_writing(); // just using this as a temp buffer
      godray_fb.bind_for_reading(blur_pair.unit);
      glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
      quad_mesh.draw(GL_TRIANGLES);

      // blur the ao in the 'y' direction (back into AO FB)
      set_generic_uniform(*blur_shader, "dimention", [](auto const loc) {
        glUniform1i(loc, 1); // y dimention blur
      });
      godray_fb.bind_for_writing(); // just using this as a temp buffer
      blur_bloom_fb_x2.bind_for_reading(blur_pair.unit);
      glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
      quad_mesh.draw(GL_TRIANGLES);
    }

    {
      // pass full scene through brigness to bring out bloom portions
      glUseProgram(*bright_shader);
      bloom_fb.bind_for_writing();
      scene_framebuffer.bind_for_reading(bright_pair.unit);
      glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
      quad_mesh.draw(GL_TRIANGLES);

      // blur the brightness in the 'x' direction
      glUseProgram(*blur_shader);
      set_generic_uniform(*blur_shader, "dimention", [](auto const loc) {
        glUniform1i(loc, 0); // x dimention blur, first
      });
      blur_bloom_fb_x1.bind_for_writing(); // just using this as a temp buffer
      bloom_fb.bind_for_reading(blur_pair.unit);
      glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
      quad_mesh.draw(GL_TRIANGLES);

      // blur the brightness in the 'y' direction
      set_generic_uniform(*blur_shader, "dimention", [](auto const loc) {
        glUniform1i(loc, 1); // y dimention blur
      });
      blur_bloom_fb_y1.bind_for_writing(); // just using this as a temp buffer
      blur_bloom_fb_x1.bind_for_reading(blur_pair.unit);
      glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
      quad_mesh.draw(GL_TRIANGLES);

      // blur the brightness in the 'x' direction
      glUseProgram(*blur_shader);
      set_generic_uniform(*blur_shader, "dimention", [](auto const loc) {
        glUniform1i(loc, 0); // x dimention blur, first
      });
      blur_bloom_fb_x2.bind_for_writing(); // just using this as a temp buffer
      blur_bloom_fb_y1.bind_for_reading(blur_pair.unit);
      glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
      quad_mesh.draw(GL_TRIANGLES);

      // blur the brightness in the 'y' direction
      set_generic_uniform(*blur_shader, "dimention", [](auto const loc) {
        glUniform1i(loc, 1); // y dimention blur
      });
      blur_bloom_fb_y2.bind_for_writing(); // just using this as a temp buffer
      blur_bloom_fb_x2.bind_for_reading(blur_pair.unit);
      glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
      quad_mesh.draw(GL_TRIANGLES);
    }

    glDisable(GL_DEPTH_TEST);
    // draw combined scene and AO into back buffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glUseProgram(*combine_shader);
    scene_framebuffer.bind_for_reading(combine_a_pair.unit);
       ao_framebuffer.bind_for_reading(combine_b_pair.unit);
            godray_fb.bind_for_reading(combine_c_pair.unit);
     blur_bloom_fb_y2.bind_for_reading(combine_d_pair.unit);
    set_generic_uniform(*combine_shader, "b_is_single_channel", [&](auto const loc) {
      glUniform1i(loc, 1);
    });
    {
      auto const dims = w.get_framebuffer_size();
      glViewport(0, 0, dims.first, dims.second);
      glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
      glDisable(GL_DEPTH_TEST);
      quad_mesh.draw(GL_TRIANGLES);
      glEnable(GL_DEPTH_TEST);
    }
  };

  while(!window->should_close()) {
    window->display(draw_cb);

    window->pump();
    handle_camera_movement();

    window->idle();

    // Idle callback. Updating objects, etc. can be done here.
    double now = glfwGetTime();
    double diff = now - last_time;
    last_time = now;
    std::cout << "render time: " << std::setw(3) << std::setprecision(3) << (diff*1000.0f) << "ms           " << '\r';
    std::cout.flush();
  }

  window.reset();
  glfwTerminate();

  return 0;
}
