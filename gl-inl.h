#pragma once

#define GLFW_INCLUDE_GLEXT
#define GLFW_INCLUDE_GLCOREARB

#ifndef GLM_FORCE_RADIANS
#define GLM_FORCE_RADIANS
#endif

#include <GL/glew.h>
#ifdef __APPLE__
#include <OpenGL/gl3.h>
#else
#include <GL/gl.h>
#endif /* __APPLE__ */
#include <GL/glext.h>

#include <GLFW/glfw3.h>
