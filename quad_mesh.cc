#include "quad_mesh.h"

QuadMesh::QuadMesh() {
    // quad to attach texture to
    static const GLfloat quad_verts[] = {
      // Positions   // TexCoords
      -1.0f,  1.0f,  0.0f, 1.0f,
      -1.0f, -1.0f,  0.0f, 0.0f,
       1.0f, -1.0f,  1.0f, 0.0f,

      -1.0f,  1.0f,  0.0f, 1.0f,
       1.0f, -1.0f,  1.0f, 0.0f,
       1.0f,  1.0f,  1.0f, 1.0f
    };

    glGenVertexArrays(1, &this->quad_vao);
    glGenBuffers(1, &this->quad_vbo);
    glBindVertexArray(this->quad_vao);
    glBindBuffer(GL_ARRAY_BUFFER, this->quad_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quad_verts), &quad_verts, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (GLvoid*)(2 * sizeof(GLfloat)));
    glBindVertexArray(0);
}

void QuadMesh::draw(GLenum draw_prim_type) const {
    glBindVertexArray(this->quad_vao);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}

QuadMesh::~QuadMesh() {
    glDeleteVertexArrays(1, &this->quad_vao);
    glDeleteBuffers(1, &this->quad_vbo);

    this->quad_vao = 0;
    this->quad_vbo = 0;
}
