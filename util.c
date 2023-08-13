#include <GL/glew.h>
#include <stdio.h>
#include <stdlib.h>
#include "util.h"

char *readFile(char *fileName)
{
	FILE *f = fopen(fileName, "r");
	if (!f)
		die("Couldn't open file %s!", fileName);
	fseek(f, 0, SEEK_END);
	long length = ftell(f);
	fseek(f, 0, SEEK_SET);
	char *buffer = malloc(length + 1); // remember the nul!
	buffer[length] = 0;
	fread(buffer, 1, length, f);
	fclose(f);
	return buffer;
}

void renderTriangle(void)
{
	glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	float vertices[] = {
		-0.5f, -0.5f, 0.0f, // left
		0.5f, -0.5f, 0.0f,	// right
		0.0f, 0.5f, 0.0f	// top
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

	printf("Rendered triangle\n");
}