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