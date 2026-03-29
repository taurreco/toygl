#ifndef TGA_H
#define TGA_H

#include <stdint.h>

int tga_load(char *file, uint32_t **colors, int *width, int *height);

#endif    /* TGA_H */
