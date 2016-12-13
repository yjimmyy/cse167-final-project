#pragma once

#include "gl-inl.h"
#include "mesh.h"

#include <glm/glm.hpp>
#include <glm/mat4x4.hpp>
#include <vector>
#include <string>
#include <memory>

class OBJObject : public Mesh
{
  using vertlist = std::vector<GLfloat>;
  using normlist  = std::vector<GLfloat>;
  using idxlist  = std::vector<GLuint>;
  const std::size_t num_idxes;

  // friend class RasterObject;
  GLuint VBO, VAO;

  glm::mat4 modelm;
  glm::vec3 pos;
  float scale;

  bool light;

  OBJObject(
    GLuint vbo, GLuint vao, /*GLuint ebo,*/
    std::size_t num_idxes
  );

public:
  void set_pos(glm::vec3 pos);
  glm::vec3 const& get_pos() const;
  void set_scale(float scale);
  glm::mat4 const& get_modelm() const;
  void set_light(bool is_light);
  bool is_light() const;
  void draw(GLenum draw_type) const;
  void update();

  static std::unique_ptr<OBJObject> build_from_path(std::string const& filepath);

  virtual ~OBJObject();
};
