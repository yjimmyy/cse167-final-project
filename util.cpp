#include "util.h"

#include <iomanip>

#include <glm/gtc/type_ptr.hpp>

std::ostream& operator<<(std::ostream& ostr, glm::vec3 const& vec) {
    return ostr << "(" << std::setw(3) << std::setprecision(3) << vec.x << ", " << vec.y << ", " << vec.z << ")";
}

void set_mat4_uniform(GLuint shader, std::string const& uname, glm::mat4 const& val) {
    set_generic_uniform(shader, uname, [&](auto const uniform) {
        glUniformMatrix4fv(uniform,  1, GL_FALSE, glm::value_ptr(val));
    });
}

void set_vec3_uniform(GLuint shader, std::string const& uname, glm::vec3 const& val) {
  set_generic_uniform(shader, uname, [&](auto const uniform) {
      glUniform3fv(uniform, 1, glm::value_ptr(val));
  });
}
