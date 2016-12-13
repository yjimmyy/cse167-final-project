#include "scene.h"

#include "window.h"
#include "camera.h"
#include "util.h"

#include <glm/gtc/type_ptr.hpp>

void ShaderOpts::setup(Camera const& camera) const {
    glUseProgram(this->shader);

    if(this->camera_pos_uniform_name) {
        set_vec3_uniform(
            this->shader,
            *this->camera_pos_uniform_name,
            camera.pos()
        );
    }
}

static void draw_one(
    Camera const& camera,
    Window const& window,
    OBJObject const& object,
    ShaderOpts const shader_opts
) {
    shader_opts.setup(camera);

    auto modelm = glm::mat4(1.0f);

    set_mat4_uniform(shader_opts.get_shader(), "proj",  window.get_projm());
    set_mat4_uniform(shader_opts.get_shader(), "view",  camera.get_viewm());
    set_mat4_uniform(shader_opts.get_shader(), "model", object.get_modelm());
    
    glUniform1i(glGetUniformLocation(shader_opts.get_shader(), "is_light"), object.is_light());

    object.draw(shader_opts.draw_type);
}

void Scene::render(
    Camera const& camera,
    Window const& window,
    OBJObject& mesh,
    ShaderOpts const& shader
) {
    return draw_one(camera, window, mesh, shader);
}
