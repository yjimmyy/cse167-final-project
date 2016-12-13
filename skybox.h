#pragma once

#include "gl-inl.h"

#include <glm/mat4x4.hpp>
#include <memory>

class Skybox {
    GLuint shader;
    GLuint texture;
    GLuint vao, vbo;
    Skybox(GLuint shader, GLuint texture, GLuint vao, GLuint vbo)
        : shader(shader), texture(texture), vao(vao), vbo(vbo) {}

public:
    static std::unique_ptr<Skybox> create(std::string const& texture_dir);
    void draw(glm::mat4 const& viewm, glm::mat4 const& projm) const;

    virtual ~Skybox() {
        glDeleteProgram(shader);
        glDeleteTextures(1, &texture);
        glDeleteVertexArrays(1, &vao);
        glDeleteBuffers(1, &vbo);
    }
};
