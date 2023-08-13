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
#include <unistd.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb/stb_image_write.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

struct OffscreenTarget
{
	GLuint framebuffer;
	GLuint texture;
};

EGLContext initializeEGLDRMOpenGL(char *card)
{
	EGLConfig eglConfig;
	EGLint eglNumConfig;
	EGLContext eglContext;
	EGLDisplay *eglDisplay;
	EGLint eglContextAttrs[] = {
		EGL_CONTEXT_OPENGL_DEBUG, EGL_TRUE,
		EGL_CONTEXT_MAJOR_VERSION, 4,
		EGL_CONTEXT_MINOR_VERSION, 1,
		EGL_NONE};
	EGLint eglConfigAttrs[] = {
		EGL_RENDERABLE_TYPE, EGL_OPENGL_BIT,
		EGL_NONE};

	int32_t fd = open(card, O_RDWR);
	if (fd == -1)
		die("Can't open dri device %s", card);

	struct gbm_device *gbm = gbm_create_device(fd);
	if (!gbm)
		die("Can't gbm_create_device");

	eglDisplay = eglGetPlatformDisplay(EGL_PLATFORM_GBM_MESA, gbm, NULL);
	if (!eglDisplay)
		die("Can't eglGetDisplay: %s", eglGetErrorString(eglGetError()));
	if (!eglInitialize(eglDisplay, NULL, NULL))
		die("Can't eglInitialize: %s", eglGetErrorString(eglGetError()));

	printf("Initialized EGL on DRM\n");
	printf("EGL version: %s\n", eglQueryString(eglDisplay, EGL_VERSION));
	printf("EGL client APIs: %s\n", eglQueryString(eglDisplay, EGL_CLIENT_APIS));
	printf("EGL vendor: %s\n", eglQueryString(eglDisplay, EGL_VENDOR));

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

	return eglContext;
}

struct OffscreenTarget initializeOffscreenTarget(uint8_t *data, int width, int height)
{
	struct OffscreenTarget target;

	glViewport(0, 0, width, height); // important
	glGenFramebuffers(1, &target.framebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, target.framebuffer);

	glGenTextures(1, &target.texture);
	glBindTexture(GL_TEXTURE_2D, target.texture);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, target.texture, 0);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		die("Framebuffer not complete");

	printf("Initialized offscreen target\n");

	return target;
}

GLFWwindow *initializeGLFWOpenGL(int width, int height)
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);
	glfwWindowHint(GLFW_VISIBLE, GLFW_TRUE);
	glfwWindowHint(GLFW_DOUBLEBUFFER, GL_FALSE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	GLFWwindow *window = glfwCreateWindow(width, height, "OpenGL", NULL, NULL);
	glfwMakeContextCurrent(window);
	GLenum glewErr = glewInit();
	if (glewErr != GLEW_OK)
		die("Can't glewInit: %s", glewGetErrorString(glewErr));

	printf("Initialized GLFW OpenGL\n");
	return window;
}

GLuint initializeShaders(char *vertexShaderFile, char *fragmentShaderFile)
{
	char *vertexShaderSource = readFile(vertexShaderFile);
	unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, (char const *const *)&vertexShaderSource, NULL);
	glCompileShader(vertexShader);

	char *fragmentShaderSource = readFile(fragmentShaderFile);
	unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, (char const *const *)&fragmentShaderSource, NULL);
	glCompileShader(fragmentShader);

	GLuint shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	free(vertexShaderSource);
	free(fragmentShaderSource);

	glUseProgram(shaderProgram);

	printf("Initialized shaders\n");
	return shaderProgram;
}

void renderEmptyTriangle()
{
	GLuint VAO;
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	glDrawArrays(GL_TRIANGLES, 0, 3);
}

const char *usage = "Usage: \n"
					"-i <inputfile>     - input file name (required)						\n"
					"-g                 - display output to screen (-o or -g is required)	\n"
					"-o <outputfile>    - write output to file (-o or -g is required)		\n"
					"-c <card>          - use specific dri device (allowed only with -o)	\n"
					"-f <shaderfile>    - set the fragment shader to use					\n"
					"-v <shaderfile>    - set the vertex shader to use						\n";

GLuint createTexture(uint8_t *data, int width, int height)
{
	GLuint texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
	glGenerateMipmap(GL_TEXTURE_2D);
	return texture;
}

int main(int argc, char *argv[])
{
	char *inputFile = NULL;
	char *outputFile = NULL;
	char *card = NULL;
	char *fragmentShaderFile = NULL;
	char *vertexShaderFile = NULL;
	bool offScreen = true;

	if (argc == 1)
	{
		printf("%s", usage);
		exit(1);
	}

	char currentOption;
	while ((currentOption = getopt(argc, argv, "hi:o:gc:f:v:")) != -1)
		switch (currentOption)
		{
		case 'i':
			inputFile = optarg;
			break;
		case 'o':
			outputFile = optarg;
			break;
		case 'f':
			fragmentShaderFile = optarg;
			break;
		case 'v':
			vertexShaderFile = optarg;
			break;
		case 'g':
			offScreen = false;
			break;
		case 'c':
			card = optarg;
			break;
		case 'h':
			printf("%s", usage);
			exit(0);
			break;
		case '?':
			die("%s", usage);
			break;
		default:
			die("Unknown option %c \n%s", currentOption, usage);
		}

	if (!inputFile)
		die("Input file is required.");
	if (offScreen && !outputFile)
		die("Output file is required with offscreen rendering.");
	if (card && !offScreen)
		die("Card can only be specified with offscreen rendering");

	if (!card)
		card = "/dev/dri/renderD128";
	if (!fragmentShaderFile)
		fragmentShaderFile = "shader.frag";
	if (!vertexShaderFile)
		vertexShaderFile = "shader.vert";

	if (!offScreen)
	{
		stbi_set_flip_vertically_on_load(true);
		stbi_flip_vertically_on_write(true);
	}

	int width, height, inputChannels;
	GLFWwindow *window = NULL;

	printf("PID: %d\n", getpid());
	printf("Using %s target\n", offScreen ? "offscreen" : "onscreen");
	printf("Loading image %s... ", inputFile);
	fflush(stdout);
	uint8_t *inputData = stbi_load(inputFile, &width, &height, &inputChannels, 0);
	if (!inputData)
		die("Failed to load %s", inputFile);
	if (inputChannels != 3)
		die("Input image must have 3 channels. Try 'convert %s -alpha off %s' with ImageMagick", inputFile, inputFile);
	printf("DONE - width: %d, height: %d\n", width, height);

	if (offScreen)
		initializeEGLDRMOpenGL(card);
	else
		window = initializeGLFWOpenGL(width, height);

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glPixelStorei(GL_PACK_ALIGNMENT, 1);

	initializeDebugOutput();
	printStats();
	initializeShaders(vertexShaderFile, fragmentShaderFile);
	if (offScreen)
		// we use the render target texture as the source texture
		initializeOffscreenTarget(inputData, width, height);
	else
		createTexture(inputData, width, height);
	stbi_image_free(inputData);

	printf("Initialization complete\n");

	printf("Rendering... ");
	fflush(stdout);
	renderEmptyTriangle();
	printf("DONE\n");

	if (offScreen)
	{
		printf("Retrieving image from GPU... ");
		fflush(stdout);
		uint8_t *outputData = malloc(width * height * 3);
		glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_UNSIGNED_BYTE, outputData);
		printf("DONE\n");

		printf("Writing image to %s... ", outputFile);
		fflush(stdout);
		stbi_write_bmp(outputFile, width, height, 3, outputData);
		printf("DONE\n");
		free(outputData);

		eglDestroyContext(eglGetCurrentDisplay(), eglGetCurrentContext());
	}
	else
	{
		glFlush();
		while (!glfwWindowShouldClose(window))
			glfwPollEvents();

		glfwDestroyWindow(window);
	}

	printf("Finished\n");

	return 0;
}