#pragma once

#include "mesh.h"

class QuadMesh : public Mesh {
    GLuint quad_vao; // quad to display texture
    GLuint quad_vbo;

public:
    QuadMesh();
    void draw(GLenum draw_primitive_type) const;
    ~QuadMesh();
};
