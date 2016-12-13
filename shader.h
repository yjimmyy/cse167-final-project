#pragma once

#include "gl-inl.h"

#include <experimental/optional>
#include <string>

std::experimental::optional<GLuint> load_shaders(
  std::string const& vertex_file_path,
  std::string const& fragment_file_path,
  std::experimental::optional<std::string> const& geom_file_path = {}
);
