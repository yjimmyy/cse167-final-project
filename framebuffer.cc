#include "framebuffer.h"
#include "shader.h"

#include <cassert>
#include <iostream>

Framebuffer::Framebuffer(Window& window, bool with_depth, bool full_rgb, float scale)
  :
  window(window),
  with_depth(with_depth),
  full_rgb(full_rgb),
  scale(scale)
{
  this->fb = 0;
  this->tex_color_buffer = 0;
  this->rbo = 0;

  window.on_resize([this](int, int) {
    this->init();
  });
  this->init();
}

std::pair<int, int> Framebuffer::get_fb_size() const {
  auto const dims = window.get_framebuffer_size();
  return {
    static_cast<int>(scale * dims.first),
    static_cast<int>(scale * dims.second)
  };
}

void Framebuffer::init() {
  auto const dims = this->get_fb_size();
  std::cout << "resizing framebuffer to (" << dims.first << ", " << dims.second << ")" << std::endl;

  if(this->fb) {
    glDeleteFramebuffers(1, &this->fb);
    this->fb = 0;
  }
  if(this->tex_color_buffer) {
    glDeleteTextures(1, &this->tex_color_buffer);
    this->tex_color_buffer = 0;
  }
  if(this->depth_buffer) {
    glDeleteTextures(1, &this->depth_buffer);
    this->depth_buffer = 0;
  }
  if(this->rbo) {
    glDeleteRenderbuffers(1, &this->rbo);
    this->rbo = 0;
  }

  GLenum internal_type, format;
  if(full_rgb) {
    internal_type = GL_RGB32F;
    format        = GL_RGB;
  }
  else {
    internal_type = GL_R32F;
    format        = GL_RED;
  }

  // framebuffer
  glGenFramebuffers(1, &this->fb);
  glBindFramebuffer(GL_FRAMEBUFFER, this->fb);

  // texture
  glGenTextures(1, &this->tex_color_buffer);
  glBindTexture(GL_TEXTURE_2D, this->tex_color_buffer);
  glTexImage2D(GL_TEXTURE_2D, 0, internal_type, dims.first, dims.second, 0, format, GL_FLOAT, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, this->tex_color_buffer, 0);

  if(with_depth) {
    glGenTextures(1, &this->depth_buffer);
    glBindTexture(GL_TEXTURE_2D, this->depth_buffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, dims.first, dims.second, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, this->depth_buffer, 0);
  }

  // renderbuffer
  glGenRenderbuffers(1, &this->rbo);
  glBindRenderbuffer(GL_RENDERBUFFER, this->rbo);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, dims.first, dims.second);
  glBindRenderbuffer(GL_RENDERBUFFER, 0);

  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, this->rbo);

  if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
    std::cerr << "error completing framebuffer!" << std::endl;
    assert(false);
    return;
  }

  glBindTexture(GL_TEXTURE_2D, 0);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Framebuffer::bind_for_writing() const {
  glBindFramebuffer(GL_FRAMEBUFFER, this->fb);
  auto const dims = this->get_fb_size();
  glViewport(0, 0, dims.first, dims.second);
}

void Framebuffer::bind_for_reading(GLenum texture_unit) const {
  glActiveTexture(texture_unit);
  glBindTexture(GL_TEXTURE_2D, this->tex_color_buffer);
}

// void Framebuffer::draw(GLuint shader) const {
//   glDisable(GL_DEPTH_TEST);

//   glUseProgram(shader);
//   glBindTexture(GL_TEXTURE_2D, this->tex_color_buffer);

//   glEnable(GL_DEPTH_TEST);
// }

Framebuffer::~Framebuffer() {
  std::cout << "Destroying framebuffer" << std::endl;
  glDeleteFramebuffers(1, &this->fb);
  glDeleteTextures(1, &this->tex_color_buffer);
  if(with_depth) {
    glDeleteTextures(1, &this->depth_buffer);
  }
  glDeleteRenderbuffers(1, &this->rbo);

  this->fb = 0;
  this->tex_color_buffer = 0;
  this->rbo = 0;
}
