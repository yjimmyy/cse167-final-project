#pragma once

#include "gl-inl.h"
#include "mesh.h"
#include "OBJObject.h"

#include <string>
#include <experimental/optional>

class Camera;
class Window;

class ShaderOpts {
    std::experimental::optional<std::string> camera_pos_uniform_name;
public:
    GLuint const shader;
    GLenum const draw_type;
    ShaderOpts(GLuint shader, GLenum draw_type)
        : shader(shader), draw_type(draw_type) {}

    void setup(Camera const&) const;

    void set_camera_pos_uniform(std::string const& name) {
        this->camera_pos_uniform_name = name;
    }

    GLuint get_shader() const { return shader; }
};

class Scene {
public:
    // void draw(Camera const& camera, Window const& window);

    static void render(
        Camera const& camera,
        Window const& window,
        OBJObject& mesh,
        ShaderOpts const& shader
    );
};
