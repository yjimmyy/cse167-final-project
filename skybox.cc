#include "skybox.h"
#include "shader.h"
#include "window.h"
#include "util.h"
#include "framebuffer.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <Magick++.h>

#include <array>

static GLfloat skybox_verts[] = {
    // Positions
    -1.0f,  1.0f, -1.0f,
    -1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,

    -1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,

     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,

    -1.0f, -1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,

    -1.0f,  1.0f, -1.0f,
     1.0f,  1.0f, -1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f, -1.0f,

    -1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
     1.0f, -1.0f,  1.0f
};

const TexUnit skybox_tex_pair = {GL_TEXTURE0, 0};

struct ImagePixels {
  std::size_t width;
  std::size_t height;
  std::unique_ptr<unsigned char[]> pixels;
};

static ImagePixels get_pixels(std::string const& path) {
  ImagePixels ret;
  Magick::Image image;

  image.read(path);

  std::size_t w = image.columns();
  std::size_t h = image.rows();
  auto pixels = image.getConstPixels(0, 0, w, h);

  ret.width = w;
  ret.height = h;
  ret.pixels = std::make_unique<unsigned char[]>(w*h*3);

  image.write(0, 0, w, h, "RGB", Magick::CharPixel, ret.pixels.get());

  return ret;
}

std::unique_ptr<Skybox> Skybox::create(std::string const& texture_dir) {
    Magick::InitializeMagick(nullptr);

    // load skybox textures
    auto front_pixels = get_pixels(texture_dir + "/front.jpg");
    auto back_pixels  = get_pixels(texture_dir + "/back.jpg");
    auto left_pixels  = get_pixels(texture_dir + "/left.jpg");
    auto right_pixels = get_pixels(texture_dir + "/right.jpg");
    auto up_pixels    = get_pixels(texture_dir + "/top.jpg");
    auto down_pixels  = get_pixels(texture_dir + "/bottom.jpg");

    auto skybox_shader = load_shaders("skybox.vert", "skybox.frag");
    if(!skybox_shader) {
        std::cerr << "error loading skybox shader" << std::endl;
        return nullptr;
    }

    std::array<ImagePixels*, 6> sides = {
        &right_pixels,
        &left_pixels,
        &up_pixels,
        &down_pixels,
        &back_pixels,
        &front_pixels,
    };


    GLuint skybox_texture;
    GLuint skybox_vao;
    GLuint skybox_vbo;

    glGenTextures(1, &skybox_texture);
    glBindTexture(GL_TEXTURE_CUBE_MAP, skybox_texture);
    glActiveTexture(GL_TEXTURE0);

    for(std::size_t i = 0; i < sides.size(); i++) {
        auto ip = sides[i];
        glTexImage2D(
            GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
            0,
            GL_RGB,
            ip->width, ip->height, 0,
            GL_RGB, GL_UNSIGNED_BYTE,
            ip->pixels.get()
        );
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    // Make sure no bytes are padded:
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    // Select GL_MODULATE to mix texture with polygon color for shading:
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

    // set up VAO/VBO
    glGenVertexArrays(1, &skybox_vao);
    glGenBuffers(1, &skybox_vbo);
    glBindVertexArray(skybox_vao);
    glBindBuffer(GL_ARRAY_BUFFER, skybox_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skybox_verts), &skybox_verts, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0); // enable location=0 in shader
    glVertexAttribPointer(
        0, // location=0 is `position` in vertex shader
        3, // 3 components per vertex
        GL_FLOAT, // type of components
        GL_FALSE, // don't normalize values
        3 * sizeof(GLfloat), // offset between elements
        (GLvoid*)0 // offset to first component
    );
    glBindVertexArray(0);

    set_generic_uniform(*skybox_shader, "skybox", [&](auto const loc) {
      glUniform1i(loc, skybox_tex_pair.idx);
    });

    return std::unique_ptr<Skybox>(new Skybox(*skybox_shader, skybox_texture, skybox_vao, skybox_vbo));
}

void Skybox::draw(glm::mat4 const& viewm, glm::mat4 const& projm) const {
    // GLuint old;
    // glGetUint(GL_DEPTH_FUNC, &old);
    // glDepthFunc(GL_LEQUAL);

    glUseProgram(this->shader);
    set_mat4_uniform(this->shader, "view", viewm);
    set_mat4_uniform(this->shader, "proj", projm);

    glBindVertexArray(this->vao);

    glActiveTexture(skybox_tex_pair.unit);
    glBindTexture(GL_TEXTURE_CUBE_MAP, this->texture);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);

    // glDepthFunc(old);
}
