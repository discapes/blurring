#include <GL/glew.h>
#include <EGL/egl.h>
#include <GL/gl.h>
#include <png.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>

static EGLint const attribute_list[] = {
    EGL_NONE};

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

#define die(...)                      \
    {                                 \
        fprintf(stderr, __VA_ARGS__); \
        exit(1);                      \
    }                                 \
    0

void initializeOpenGL(void)
{
    EGLDisplay eglDisplay;
    EGLConfig eglConfig;
    EGLint num_config;
    EGLContext context;

    if (!(eglDisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY)))
        die("Can't eglGetDisplay: %s", eglGetErrorString(eglGetError()));
    if (!eglInitialize(eglDisplay, NULL, NULL))
        die("Can't eglInitialize: %s", eglGetErrorString(eglGetError()));

    const char *egl_extension_st = eglQueryString(eglDisplay, EGL_EXTENSIONS);
    // printf("EGL extensions: %s", egl_extension_st);
    assert(strstr(egl_extension_st, "EGL_KHR_create_context") != NULL);
    assert(strstr(egl_extension_st, "EGL_KHR_surfaceless_context") != NULL);

    if (!eglChooseConfig(eglDisplay, NULL, &eglConfig, 1, &num_config))
        die("Can't eglChooseConfig: %s", eglGetErrorString(eglGetError()));

    eglBindAPI(EGL_OPENGL_API); // important
    if (!(context = eglCreateContext(eglDisplay, eglConfig, EGL_NO_CONTEXT, NULL)))
        die("Can't eglCreateContext: %s", eglGetErrorString(eglGetError()));

    if (!eglMakeCurrent(eglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, context))
        die("Can't eglMakeCurrent: %s", eglGetErrorString(eglGetError()));

    GLenum glewErr = glewInit();
    if (glewErr != GLEW_OK)
        die("Can't glewInit: %s", glewGetErrorString(glewErr));
}

int main(int argc, char *argv[])
{

    GLuint FramebufferName = 0;
    glGenFramebuffers(1, &FramebufferName);
    glBindFramebuffer(GL_FRAMEBUFFER, FramebufferName);
    GLuint renderedTexture;
    glGenTextures(1, &renderedTexture);
    glBindTexture(GL_TEXTURE_2D, renderedTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1024, 768, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, renderedTexture, 0);
    GLenum DrawBuffers[1] = {GL_COLOR_ATTACHMENT0};
    glDrawBuffers(1, DrawBuffers); // "1" is the size of DrawBuffers

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        printf("Went wrong!\n");
        exit(1);
    }

    GLuint VertexArrayID;
    glGenVertexArrays(1, &VertexArrayID);
    glBindVertexArray(VertexArrayID);
    static const GLfloat g_vertex_buffer_data[] = {
        -1.0f,
        -1.0f,
        0.0f,
        1.0f,
        -1.0f,
        0.0f,
        0.0f,
        1.0f,
        0.0f,
    }; // This will identify our vertex buffer
    GLuint vertexbuffer;
    // Generate 1 buffer, put the resulting identifier in vertexbuffer
    glGenBuffers(1, &vertexbuffer);
    // The following commands will talk about our 'vertexbuffer' buffer
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
    // Give our vertices to OpenGL.
    glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
    glVertexAttribPointer(
        0,        // attribute 0. No particular reason for 0, but must match the layout in the shader.
        3,        // size
        GL_FLOAT, // type
        GL_FALSE, // normalized?
        0,        // stride
        (void *)0 // array buffer offset
    );
    // Draw the triangle !
    glDrawArrays(GL_TRIANGLES, 0, 3); // Starting from vertex 0; 3 vertices total -> 1 triangle
    glDisableVertexAttribArray(0);

    printf("initalized!\n");

    return 0;
}