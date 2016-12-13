#include "OBJObject.h"

#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <fstream>
#include <sstream>
#include <numeric>
#include <array>
#include <algorithm>
#include <unordered_map>

#include <glm/gtc/matrix_transform.hpp>

#include "tiny_obj_loader.h"

OBJObject::OBJObject(
  GLuint VAO, GLuint VBO, /*GLuint EBO,*/
  std::size_t num_idxes_
) :
  VAO(VAO), VBO(VBO),/* EBO(EBO), */
  num_idxes(num_idxes_),
  light(false),
  pos(glm::vec3(0.0f)),
  scale(1.0f),
  modelm(glm::mat4(1.0f))
{}

OBJObject::~OBJObject() {
  glDeleteVertexArrays(1, &this->VAO);
  glDeleteBuffers(1, &this->VBO);
  // glDeleteBuffers(1, &this->EBO);
}

std::unique_ptr<OBJObject>
OBJObject::build_from_path(std::string const& filepath)
{
  //TODO parse the OBJ file
  // Populate the face indexes, vertices, and normals vectors with the OBJ Object data
  std::cerr << "loading OBJ " << filepath << std::endl;

  tinyobj::attrib_t attrib;
  std::vector<tinyobj::shape_t> shapes;
  std::vector<tinyobj::material_t> materials;

  std::string err;
  bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &err, filepath.c_str());

  if (!err.empty()) { // `err` may contain warning message.
    std::cerr << err << std::endl;
  }

  if (!ret) {
    return nullptr;
  }

  if(shapes.size() != 1) {
    std::cerr << "invalid number of shapes in obj: " << shapes.size() << std::endl;
    return nullptr;
  }

  auto& shape = shapes[0];

  GLuint VAO, VBO;

  std::vector<glm::vec3> verts_and_norms;

  std::cout << attrib.vertices.size() << " vertexes in this object" << std::endl;
  std::cout << attrib.normals.size() << " normals in this object" << std::endl;

  for (size_t f = 0; f < shape.mesh.indices.size(); f++) {
    tinyobj::index_t idx = shape.mesh.indices[f];

    float vx = attrib.vertices[(3*idx.vertex_index)+0];
    float vy = attrib.vertices[(3*idx.vertex_index)+1];
    float vz = attrib.vertices[(3*idx.vertex_index)+2];

    float nx =  attrib.normals[(3*idx.normal_index)+0];
    float ny =  attrib.normals[(3*idx.normal_index)+1];
    float nz =  attrib.normals[(3*idx.normal_index)+2];

    verts_and_norms.push_back({vx, vy, vz});
    verts_and_norms.push_back({nx, ny, nz});
  }

  std::cout << verts_and_norms.size() << " vertexes to draw" << std::endl;
  std::cout << shape.mesh.num_face_vertices.size() << " faces in the object" << std::endl;

  glGenVertexArrays(1, &VAO);
  glGenBuffers(1, &VBO);

  // working with the VAO object, which will interpret the VBO buffer
  glBindVertexArray(VAO);

  // set up VBO
  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(
    GL_ARRAY_BUFFER,
    verts_and_norms.size()*sizeof(verts_and_norms[0]),
    verts_and_norms.data(),
    GL_STATIC_DRAW
  );
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(
    0, // 'position' layout in shader.vert
    3, // elements per vertex
    GL_FLOAT, // they are floating point values
    GL_FALSE, // don't normalize values
    2*sizeof(glm::vec3), // stride between elements
    (GLvoid*)(0 * sizeof(glm::vec3)) // offset to the first element
  );
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(
    1, // 'normal' layout in shader.vert
    3, // elements per vertex
    GL_FLOAT, // they are floating point values
    GL_FALSE, // don't normalize values
    2*sizeof(glm::vec3), // stride between elements
    (GLvoid*)(1 * sizeof(glm::vec3)) // offset to the first element
  );

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);

  return std::unique_ptr<OBJObject>(new OBJObject(
    VAO, VBO,
    verts_and_norms.size() / 2
  ));
}

// shader_draw_type is GL_TRIANGLES, GL_POINTS, etc
void OBJObject::draw(GLenum shader_draw_type) const {
  glBindVertexArray(VAO);
  glDrawArrays(shader_draw_type, 0, this->num_idxes);
  glBindVertexArray(0);
}

void OBJObject::set_pos(glm::vec3 pos) {
  this->pos = pos;
  //this->modelm = glm::scale(glm::translate(glm::mat4(1.0f), this->pos), glm::vec3(this->scale));
  this->modelm = glm::translate(glm::mat4(1.0f), this->pos);
}

glm::vec3 const& OBJObject::get_pos() const {
  return this->pos;
}

void OBJObject::set_scale(float scale) {
  this->scale = scale;
  //this->modelm = glm::translate(glm::mat4(1.0f), this->pos), glm::vec3(this->scale));
}

glm::mat4 const& OBJObject::get_modelm() const {
  return this->modelm;
}

void OBJObject::set_light(bool is_light) {
  this->light = is_light;
}

bool OBJObject::is_light() const {
  return this->light;
}
