#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glext.h>
#include <EGL/egl.h>
#include <stdio.h>
#include "debug_output.h"

void initializeDebugOutput(void)
{
    int flags;
    glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
    if (flags & GL_CONTEXT_FLAG_DEBUG_BIT)
    {
        fprintf(stderr, "Initialized debug output\n");
        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        glDebugMessageCallback(glDebugOutput, NULL);
        glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, GL_TRUE);
    }
    else
    {

        fprintf(stderr, "Not enabling debug output\n");
    }
}

static void printExtensions(void)
{
    GLint n = 0;
    glGetIntegerv(GL_NUM_EXTENSIONS, &n);

    printf("Extensions: ");
    for (GLint i = 0; i < n; i++)
    {
        const char *extension =
            (const char *)glGetStringi(GL_EXTENSIONS, i);
        if (i)
            printf(", ");
        printf("%s", extension);
    }
    printf("\n");
}

void printStats(void)
{

    printf("Vendor: %s\n", glGetString(GL_VENDOR));
    printf("Version: %s\n", glGetString(GL_VERSION));
    printf("Renderer: %s\n", glGetString(GL_RENDERER));
    printf("Shading language version: %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));
    printExtensions();

    GLint work_group_count[3] = {0};
    for (unsigned i = 0; i < 3; i++)
        glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT,
                        i,
                        &work_group_count[i]);
    printf("GL_MAX_COMPUTE_WORK_GROUP_COUNT: %d, %d, %d\n",
           work_group_count[0],
           work_group_count[1],
           work_group_count[2]);

    GLint work_group_size[3] = {0};
    for (unsigned i = 0; i < 3; i++)
        glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, i, &work_group_size[i]);
    printf("GL_MAX_COMPUTE_WORK_GROUP_SIZE: %d, %d, %d\n",
           work_group_size[0],
           work_group_size[1],
           work_group_size[2]);

    GLint max_invocations;
    glGetIntegerv(GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS, &max_invocations);
    printf("GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS: %d\n", max_invocations);

    GLint mem_size;
    glGetIntegerv(GL_MAX_COMPUTE_SHARED_MEMORY_SIZE, &mem_size);
    printf("GL_MAX_COMPUTE_SHARED_MEMORY_SIZE: %d\n", mem_size);
}

#define CASE_STR(value) \
    case value:         \
        return #value;
const char *eglGetErrorString(EGLint error)
{
    switch (error)
    {
        CASE_STR(EGL_SUCCESS)
        CASE_STR(EGL_NOT_INITIALIZED)
        CASE_STR(EGL_BAD_ACCESS)
        CASE_STR(EGL_BAD_ALLOC)
        CASE_STR(EGL_BAD_ATTRIBUTE)
        CASE_STR(EGL_BAD_CONTEXT)
        CASE_STR(EGL_BAD_CONFIG)
        CASE_STR(EGL_BAD_CURRENT_SURFACE)
        CASE_STR(EGL_BAD_DISPLAY)
        CASE_STR(EGL_BAD_SURFACE)
        CASE_STR(EGL_BAD_MATCH)
        CASE_STR(EGL_BAD_PARAMETER)
        CASE_STR(EGL_BAD_NATIVE_PIXMAP)
        CASE_STR(EGL_BAD_NATIVE_WINDOW)
        CASE_STR(EGL_CONTEXT_LOST)
    default:
        return "Unknown";
    }
}
#undef CASE_STR