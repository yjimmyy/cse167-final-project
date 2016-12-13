#pragma once

#include <iostream>
#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>

#include "gl-inl.h"

void set_vec3_uniform(GLuint shader, std::string const& uname, glm::vec3 const& val);
void set_mat4_uniform(GLuint shader, std::string const& uname, glm::mat4 const& val);

std::ostream& operator<<(std::ostream& ostr, glm::vec3 const& vec);

template<typename Cbs, typename... Args>
static void call_all(Cbs const& cbs, Args&&... args) {
  for(auto const& cb : cbs) {
    cb(std::forward<Args>(args)...);
  }
}

template <typename cb>
void set_generic_uniform(GLuint shader, std::string const& uname, cb const& set_cb) {
    auto const uniform = glGetUniformLocation(shader, uname.c_str());
    if(uniform == -1) {
      std::cerr << "didn't find " << uname << " in " << shader << std::endl;
      assert(false);
    }
    else {
        set_cb(uniform);
    }
}
