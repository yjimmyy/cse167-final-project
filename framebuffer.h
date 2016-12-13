#pragma once

#include "gl-inl.h"
#include "window.h"

struct TexUnit {
  const GLenum   unit;
  const unsigned idx;
  TexUnit(GLenum unit, unsigned idx) : unit(unit), idx(idx) {}
};

class Framebuffer {
  GLuint fb;
  GLuint tex_color_buffer;
  GLuint rbo;
  GLuint depth_buffer;

  Window const& window;
  bool const with_depth, full_rgb;
  const float scale;

public:
  Framebuffer(Window& window, bool with_depth, bool full_rgb, float scale = 1.0f);

  void bind_for_writing() const;
  void bind_for_reading(GLenum texture_unit) const;

  std::pair<int, int> get_fb_size() const;

  virtual ~Framebuffer();

private:
    void init();
};
