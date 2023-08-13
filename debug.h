#pragma once
#include <EGL/egl.h>

void initializeDebugOutput(void);
void printStats(void);
const char *eglGetErrorString(EGLint error);