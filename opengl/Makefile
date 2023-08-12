all: prog

CC = clang
CFLAGS = -std=c17 \
	-D_POSIX_C_SOURCE=200112L \
	-Wall -Wextra -Werror -Wno-unused-parameter -Wno-unused-const-variable \
	-fsanitize=undefined -lGL -lEGL -lGLEW -DGLEW_EGL

prog: main.c 
	$(CC) $(CFLAGS) main.c -o prog