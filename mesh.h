#pragma once

#include "gl-inl.h"

struct Mesh {
    virtual void draw(GLenum draw_primitive_type) const = 0;
};
