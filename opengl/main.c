#include <GL/glew.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GL/gl.h>
#include <png.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <gbm.h>
#include <fcntl.h>
#include <unistd.h>
#include "debug.h"
#include "util.h"

void initializeDebugOutput()
{
    int flags;
    glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
    if (flags & GL_CONTEXT_FLAG_DEBUG_BIT)
    {
        fprintf(stderr, "initialized debug output!\n");
        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        glDebugMessageCallback(glDebugOutput, NULL);
        glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, GL_TRUE);
    }
    else
    {

        fprintf(stderr, "not enabling debug output\n");
    }
}

void initializeOpenGL(void)
{
    EGLDisplay eglDisplay;
    EGLConfig eglConfig;
    EGLint eglNumConfig;
    EGLContext eglContext;
    EGLint eglContextAttrs[] = {EGL_CONTEXT_OPENGL_DEBUG, EGL_TRUE, EGL_NONE};

    int32_t fd = open("/dev/dri/renderD128", O_RDWR);
    if (!fd)
        die("Can't open render node");

    struct gbm_device *gbm = gbm_create_device(fd);
    if (!gbm)
        die("Can't gbm_create_device");

    if (!(eglDisplay = eglGetPlatformDisplay(EGL_PLATFORM_GBM_MESA, gbm, NULL)))
        die("Can't eglGetDisplay: %s", eglGetErrorString(eglGetError()));
    if (!eglInitialize(eglDisplay, NULL, NULL))
        die("Can't eglInitialize: %s", eglGetErrorString(eglGetError()));

    const char *egl_extension_st = eglQueryString(eglDisplay, EGL_EXTENSIONS);
    // printf("EGL extensions: %s", egl_extension_st);
    assert(strstr(egl_extension_st, "EGL_KHR_create_context") != NULL);
    assert(strstr(egl_extension_st, "EGL_KHR_surfaceless_context") != NULL);

    if (!eglChooseConfig(eglDisplay, NULL, &eglConfig, 1, &eglNumConfig))
        die("Can't eglChooseConfig: %s", eglGetErrorString(eglGetError()));

    eglBindAPI(EGL_OPENGL_API); // important
    if (!(eglContext = eglCreateContext(eglDisplay, eglConfig, EGL_NO_CONTEXT, eglContextAttrs)))
        die("Can't eglCreateContext: %s", eglGetErrorString(eglGetError()));

    if (!eglMakeCurrent(eglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, eglContext))
        die("Can't eglMakeCurrent: %s", eglGetErrorString(eglGetError()));

    GLenum glewErr = glewInit();
    if (glewErr != GLEW_OK)
        die("Can't glewInit: %s", glewGetErrorString(glewErr));

    printf("initialized OpenGL!\n");
}

void initializeBuffers(int width, int height)
{
    // https://www.khronos.org/opengl/wiki/Framebuffer
    GLuint FramebufferName = 0;
    glGenFramebuffers(1, &FramebufferName);
    glBindFramebuffer(GL_FRAMEBUFFER, FramebufferName);

    GLuint renderedTexture;
    glGenTextures(1, &renderedTexture);
    glBindTexture(GL_TEXTURE_2D, renderedTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, renderedTexture, 0);
    GLenum DrawBuffers[1] = {GL_COLOR_ATTACHMENT0};
    glDrawBuffers(1, DrawBuffers); // "1" is the size of DrawBuffers
    glReadBuffer(GL_COLOR_ATTACHMENT0);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        die("Framebuffer not complete");

    printf("initialized buffers!\n");
}

void render()
{
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
    printf("rendered!\n");
}

void initializeShaders()
{
    char *vertexShaderSource = readFile("vertex.glsl");
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, (char const *const *)&vertexShaderSource, NULL);
    glCompileShader(vertexShader);

    char *fragmentShaderSource = readFile("frag.glsl");
    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, (char const *const *)&fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);

    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    glUseProgram(shaderProgram);

    // all hail LeakSanitizer
    free(vertexShaderSource);
    free(fragmentShaderSource);
    printf("initialized shaders!\n");
}

int main(int argc, char *argv[])
{
    int width = 30;
    int height = 30;

    initializeOpenGL();
    initializeDebugOutput();
    initializeBuffers(width, height);
    initializeShaders();
    // render();

    int pixels[width * height * 3];
    pixels[1] = 5;
    pixels[0] = 9;
    pixels[2] = 512;
    glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_UNSIGNED_BYTE, pixels);

    printf("done!\n");

    return 0;
}