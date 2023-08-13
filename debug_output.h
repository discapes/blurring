#pragma once
#include <GL/gl.h>

void glDebugOutput(GLenum source,
                   GLenum type,
                   unsigned int id,
                   GLenum severity,
                   GLsizei length,
                   const char *message,
                   const void *userParam);
