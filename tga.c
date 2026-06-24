#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "gl.h"

/**
 * gl_tga.c
 * --------
 * loads tga images into memory
 * supports data types 1, 2, 9, & 10
 * and pixel / color depths of 16, 24, & 32
 * 
 */

/*********************************************************************
 *                                                                   *
 *                              structs                              *
 *                                                                   *
 *********************************************************************/

/**********
 * header *
 **********/

/* holds the header of a tga in memory */

struct header {
   uint8_t idlen;
   uint8_t cmap_type;
   uint8_t img_type;
   int16_t cmap_start;
   int16_t cmap_len;
   uint8_t cmap_depth;
   int16_t img_x;
   int16_t img_y;
   int16_t img_w;
   int16_t img_h;
   uint8_t img_depth;
   uint8_t img_desc;
};

struct tga {
   struct header header;
   uint8_t *color_map;
   uint8_t *image_data;
};

/**************************************************************
 *                                                            *
 *                      private helpers                       *
 *                                                            *
 **************************************************************/

/******
 * u8 *
 ******/

void
u8(uint8_t *dst, FILE *fp)
{
    fread(dst, 1, 1, fp);
}

/*******
 * u16 *
 *******/

void
u16(uint16_t *dst, FILE *fp)
{
    fread(dst, 2, 1, fp);
}

/**********
 * unpack *
 **********/

/* 
 * takes in a varying representation of color in 'data'
 * and converts to the standard color struct
 */

static uint32_t
unpack(uint8_t *bytes, int n_bytes)
{
    uint8_t a, r, g, b;

    a = 0;
    r = 0;
    g = 0;
    b = 0;

    if (n_bytes == 2) {
        
	    /* grab 5-bit representation of each color */
         
	    r = (bytes[1] >> 2) & 0x1F;
        g = ((bytes[1] << 3) & 0x18) | ((bytes[0] >> 5) & 0x07);
        b = (bytes[0]) & 0x1F;

        /* scale channels by 8.2258 */
         
	    r = (r << 3) | (r >> 2);
        g = (g << 3) | (g >> 2);
        b = (b << 3) | (b >> 2);

        /* attribute channel */
         
	    a = 255 * ((bytes[0] & 0x80) >> 7);
    }

    if (n_bytes == 3) {   
        a = 255;
        r = bytes[2];
        g = bytes[1];
        b = bytes[0];
    }
        
    if (n_bytes == 4) {        
        a = bytes[3];
        r = bytes[2];
        g = bytes[1];
        b = bytes[0];
    }

   return a << 24 | r << 16 | g << 8 | b;
}

static void
mapped(uint32_t *colors, struct tga tga)
{
    int color_depth = tga.header.cmap_depth / 8;
    uint8_t *color_tga;    /* raw color bytes read from tga */
    for (int i = 0; i < tga.header.img_w * tga.header.img_h; i++) {
        color_tga = tga.color_map + tga.image_data[i] * color_depth;
        colors[i] = unpack(color_tga, color_depth);
    }
}

static void
rgb(uint32_t *colors, struct tga tga)
{
    int pixel_depth = tga.header.img_depth / 8;
    int color_depth = tga.header.cmap_depth / 8;
    uint8_t *color_tga;    /* raw color bytes read from tga */

    color_tga = tga.image_data;
    for (int i = 0; i < tga.header.img_w * tga.header.img_h; i++) {
        colors[i] = unpack(color_tga, pixel_depth);
        color_tga += pixel_depth;
    }
}

/*******
 * rle *
 *******/

static void
rle(uint32_t *colors, struct tga tga)
{
    uint8_t* packet;
    packet = tga.image_data;
    int pixel_depth = tga.header.img_depth / 8;
    int color_depth = tga.header.cmap_depth / 8;


    for (int i = 0; i < tga.header.img_w * tga.header.img_h; i++) {

        int len = (*packet & 0x7F) + 1;

        int depth = tga.header.cmap_type ? color_depth : pixel_depth;
        uint8_t* color_addr = tga.header.cmap_type ? tga.color_map + packet[1] : packet + 1;

        if (*packet & 0x80) {    /* run length packet */

            uint32_t color = unpack(color_addr, depth);
            for (int j = 0; j < len; j++) {
                colors[i + j] = color;
            }

            /* next packet */
            packet += pixel_depth + 1;  
        } else {                /* raw packet */

            for (int j = 0; j < len; j++) {
                colors[i + j] = unpack(color_addr, depth);
                color_addr += pixel_depth;
            }

            /* next packet */
            packet += len * pixel_depth + 1;  
        }

        i += len - 1;
    }
}

/********
 * read *
 ********/

/* 
 * reads tga data into RAM for quick parsing,
 * allocates image and color tga on the heap
 */
static void
read(struct tga* tga, FILE* fp)
{
   int n = 0;  // used to supress fread warnings :(
   /* fill header */
   n += fread(&tga->header.idlen, 1, 1, fp);
   n += fread(&tga->header.cmap_type, 1, 1, fp);
   n += fread(&tga->header.img_type, 1, 1, fp);
   n += fread(&tga->header.cmap_start, 1, 2, fp);
   n += fread(&tga->header.cmap_len, 1, 2, fp);
   n += fread(&tga->header.cmap_depth, 1, 1, fp);
   n += fread(&tga->header.img_x, 1, 2, fp);
   n += fread(&tga->header.img_y, 1, 2, fp);
   n += fread(&tga->header.img_w, 1, 2, fp);
   n += fread(&tga->header.img_h, 1, 2, fp);
   n += fread(&tga->header.img_depth, 1, 1, fp);
   n += fread(&tga->header.img_desc, 1, 1, fp);

   int pixel_depth = tga->header.img_depth / 8;
   int color_depth = tga->header.cmap_depth / 8;
   
   tga->color_map = calloc(tga->header.cmap_len * color_depth, 1);
   tga->image_data = calloc(tga->header.img_w * 
                                tga->header.img_h * 
                                pixel_depth, 1);

   fseek(fp, tga->header.idlen, SEEK_CUR);
   n += fread(tga->color_map, 1, tga->header.cmap_len * color_depth, fp);
   n += fread(tga->image_data, 1, 
         tga->header.img_w * 
         tga->header.img_h * 
         pixel_depth, 
         fp);
}

/*********************************************************************
 *                                                                   *
 *                         public definition                         *
 *                                                                   *
 *********************************************************************/

/***************
 * gl_load_tga *
 ***************/

/*
 * allocates a texture buffer, fills with color contents,
 * and sets it to the given pointer
 * 1 on success, 0 on failure
 */
int
gl_load_tga(char* file, uint32_t **colors_out, int *width_out, int *height_out)
{
   struct tga tga;

   /* open file */
   FILE* fp = fopen(file, "rb");
   if (!fp)
      return 0;

   read(&tga, fp);
   fclose(fp);

   int pixel_depth = tga.header.img_depth / 8;
   int color_depth = tga.header.cmap_depth / 8;
   uint8_t* color_tga;    /* raw color bytes read from tga */
   uint8_t* packet;

   uint32_t* colors = calloc(tga.header.img_w * tga.header.img_h, 4);
   

   switch (tga.header.img_type) {
        case 1:    /* uncompressed color mapped */
            mapped(colors, tga);
            break;

        case 2:    /* uncompressed RGB */
            rgb(colors, tga);
            break;

        case 9:    /* run length encoded & color mapped */
        case 10:   /* run length encoded RGB */
            rle(colors, tga);
            break;

        default:
            printf("unsupported tga data type code");
            free(colors);
            return 0;
   }

   /* return */
   *colors_out = colors;
   *width_out = tga.header.img_w;
   *height_out= tga.header.img_h;
   free(tga.color_map);
   free(tga.image_data);

   return 0;
}