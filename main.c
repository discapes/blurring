#include <GL/glew.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GL/gl.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <gbm.h>
#include <fcntl.h>
#include <unistd.h>
#include "debug.h"
#include "util.h"
#include <dlfcn.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb/stb_image_write.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

static struct OffscreenTarget
{
    GLuint framebuffer;
    GLuint texture;
} target;

static GLuint shader;

void initializeEGLDRMOpenGL(void)
{
    EGLConfig eglConfig;
    EGLint eglNumConfig;
    EGLContext eglContext;
    EGLDisplay *eglDisplay;
    EGLint eglContextAttrs[] = {
        EGL_CONTEXT_OPENGL_DEBUG, EGL_TRUE,
        EGL_CONTEXT_MAJOR_VERSION, 4,
        EGL_CONTEXT_MINOR_VERSION, 6,
        EGL_NONE};
    EGLint eglConfigAttrs[] = {
        EGL_RENDERABLE_TYPE, EGL_OPENGL_BIT,
        EGL_NONE};

    // 128 for intel, 129 for nvidia
    int32_t fd = open("/dev/dri/renderD129", O_RDWR);
    if (!fd)
        die("Can't open render node");

    struct gbm_device *gbm = gbm_create_device(fd);
    if (!gbm)
        die("Can't gbm_create_device");

    eglDisplay = eglGetPlatformDisplay(EGL_PLATFORM_GBM_MESA, gbm, NULL);
    if (!eglDisplay)
        die("Can't eglGetDisplay: %s", eglGetErrorString(eglGetError()));
    if (!eglInitialize(eglDisplay, NULL, NULL))
        die("Can't eglInitialize: %s", eglGetErrorString(eglGetError()));

    printf("Initialized EGL on DRM\n");
    printf("EGL_VERSION: %s\n", eglQueryString(eglDisplay, EGL_VERSION));
    printf("EGL_CLIENT_APIS: %s\n", eglQueryString(eglDisplay, EGL_CLIENT_APIS));
    printf("EGL_VENDOR: %s\n", eglQueryString(eglDisplay, EGL_VENDOR));

    const char *eglExtensions = eglQueryString(eglDisplay, EGL_EXTENSIONS);
    // printf("EGL extensions: %s", eglExtensions);
    assert(strstr(eglExtensions, "EGL_KHR_create_context") != NULL);
    assert(strstr(eglExtensions, "EGL_KHR_surfaceless_context") != NULL);

    if (!eglChooseConfig(eglDisplay, eglConfigAttrs, &eglConfig, 1, &eglNumConfig))
        die("Can't eglChooseConfig: %s", eglGetErrorString(eglGetError()));

    if (!eglBindAPI(EGL_OPENGL_API))
        die("Can't eglBindAPI: %s", eglGetErrorString(eglGetError()));

    eglContext = eglCreateContext(eglDisplay, eglConfig, EGL_NO_CONTEXT, eglContextAttrs);
    if (!eglContext)
        die("Can't eglCreateContext: %s", eglGetErrorString(eglGetError()));

    if (!eglMakeCurrent(eglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, eglContext))
        die("Can't eglMakeCurrent: %s", eglGetErrorString(eglGetError()));

    printf("Initialized OpenGL context\n");

    GLenum glewErr = glewInit();
    if (glewErr != GLEW_OK)
        die("Can't glewInit: %s", glewGetErrorString(glewErr));
}

void initializeOffscreenTarget(int width, int height)
{
    glViewport(0, 0, width, height); // important
    glGenFramebuffers(1, &target.framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, target.framebuffer);

    glGenTextures(1, &target.texture);
    glBindTexture(GL_TEXTURE_2D, target.texture);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, target.texture, 0);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        die("Framebuffer not complete");

    printf("Initialized buffers\n");
}

GLFWwindow *initializeGLFWOpenGL(int width, int height)
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);
    glfwWindowHint(GLFW_VISIBLE, GLFW_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    GLFWwindow *window = glfwCreateWindow(width, height, "OpenGL", NULL, NULL);
    glfwMakeContextCurrent(window);
    GLenum glewErr = glewInit();
    if (glewErr != GLEW_OK)
        die("Can't glewInit: %s", glewGetErrorString(glewErr));

    printf("Initialized GLFW OpenGL\n");
    return window;
}

void render()
{
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    float vertices[] = {
        -0.5f, -0.5f, 0.0f, // left
        0.5f, -0.5f, 0.0f,  // right
        0.0f, 0.5f, 0.0f    // top
    };

    unsigned int VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);

    glDrawArrays(GL_TRIANGLES, 0, 3);

    printf("Rendered\n");
}

void initializeShaders()
{
    char *vertexShaderSource = readFile("shader.vert");
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, (char const *const *)&vertexShaderSource, NULL);
    glCompileShader(vertexShader);

    char *fragmentShaderSource = readFile("shader.frag");
    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, (char const *const *)&fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);

    shader = glCreateProgram();
    glAttachShader(shader, vertexShader);
    glAttachShader(shader, fragmentShader);
    glLinkProgram(shader);
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    free(vertexShaderSource);
    free(fragmentShaderSource);

    glUseProgram(shader);

    printf("Initialized shaders\n");
}

int main(int argc, char *argv[])
{
    int iwidth, iheight, inrChannels;
    stbi_set_flip_vertically_on_load(true);
    uint8_t *data = stbi_load("big.jpg", &iwidth, &iheight, &inrChannels, 0);

    int width = iwidth;
    int height = iheight;
    printf("width: %d, height: %d\n", width, height);
    GLFWwindow *window = NULL;
    bool onScreen = false;

    if (onScreen)
        window = initializeGLFWOpenGL(width, height);
    else
        initializeEGLDRMOpenGL();
    initializeDebugOutput();
    printStats();

    initializeShaders();

    if (!onScreen)
        initializeOffscreenTarget(width, height);

    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
    stbi_image_free(data);
    printf("Loaded texture\n");

    GLuint VAO;
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    glDrawArrays(GL_TRIANGLES, 0, 3);
    printf("Finished drawing\n");

    if (onScreen)
    {
        while (!glfwWindowShouldClose(window))
        {
            glfwSwapBuffers(window);
            glfwPollEvents();
        }
    }
    else
    {

        uint8_t *pixels = malloc(width * height * 3);
        glBindTexture(GL_TEXTURE_2D, target.texture);
        glPixelStorei(GL_PACK_ALIGNMENT, 1);
        glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_UNSIGNED_BYTE, pixels);
        printf("Got tex image\n");

        stbi_flip_vertically_on_write(true);
        stbi_write_bmp("output.bmp", width, height, 3, pixels);
        free(pixels);
    }

    printf("Done\n");

    return 0;
}