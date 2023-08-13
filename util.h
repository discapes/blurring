#pragma once

char *readFile(char *fileName);

#define die(...)                      \
	{                                 \
		fprintf(stderr, "ERR: ");     \
		fprintf(stderr, __VA_ARGS__); \
		fprintf(stderr, "\n");        \
		exit(1);                      \
	}
