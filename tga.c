#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "gl.h"

#define MAPPED          1
#define RGB             2
#define RLE_MAPPED      9
#define RLE_RGB         10

/**************************************************************
 *                                                            *
 *                     struct definitions                     *
 *                                                            *
 **************************************************************/

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
 * takes in a varying representation of color in 'bytes'
 * and converts to the standard 4 byte argb color struct
 */

static uint32_t
unpack(uint8_t *bytes, int stride)
{
    uint8_t a, r, g, b;

    a = 0;
    r = 0;
    g = 0;
    b = 0;

    if (stride == 2) {
        
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

    if (stride == 3) {   
        a = 255;
        r = bytes[2];
        g = bytes[1];
        b = bytes[0];
    }
        
    if (stride == 4) {        
        a = bytes[3];
        r = bytes[2];
        g = bytes[1];
        b = bytes[0];
    }

   return a << 24 | r << 16 | g << 8 | b;
}

/**********
 * mapped *
 **********/

static void
mapped(uint32_t *data, struct header ihdr, uint8_t *cmap, uint8_t *img)
{
    int width, height;
    int stride;
    
    width = ihdr.img_w;
    height = ihdr.img_h;
    stride = ihdr.cmap_depth >> 3;

    for (int i = 0; i < width * height; i++) {
        uint8_t *color;
        color = cmap + img[i] * stride;
        data[i] = unpack(color, stride);
    }
}

/*******
 * rgb *
 *******/

static void
rgb(uint32_t *data, struct header ihdr, uint8_t *img)
{
    uint8_t *color;
    int width, height;
    int stride;

    color = img;
    width = ihdr.img_w;
    height = ihdr.img_h;
    stride = ihdr.img_depth >> 3;
    
    for (int i = 0; i < width * height; i++) {
        data[i] = unpack(color, stride);
        color += stride;
    }
}

/*******
 * rle *
 *******/

static void
rle(uint32_t *data, struct header ihdr, uint8_t *cmap, uint8_t *img)
{
    uint8_t *packet;
    int width, height;
    int stride, img_stride, cmap_stride;
    
    packet = img;

    width = ihdr.img_w;
    height = ihdr.img_h;

    img_stride = ihdr.img_depth >> 3;
    cmap_stride = ihdr.cmap_depth >> 3;
    stride = ihdr.cmap_type ? cmap_stride : img_stride;
    
    for (int i = 0; i < width * height; i++) {

        uint8_t *color;
        int len;

        len = (*packet & 0x7F) + 1;
        color = ihdr.cmap_type ? cmap + packet[1] : packet + 1;

        if (*packet & 0x80) {    /* run length packet */

            for (int j = 0; j < len; j++) {
                data[i + j] = unpack(color, stride);;
            }

            /* next packet */
            packet += img_stride + 1;  
        } else {                /* raw packet */

            for (int j = 0; j < len; j++) {
                data[i + j] = unpack(color, stride);
                color += img_stride;
            }

            /* next packet */
            packet += len * img_stride + 1;  
        }

        i += len - 1;
    }
}

/*********
 * parse *
 *********/

static void
parse(uint32_t *data, struct header ihdr, uint8_t *cmap, uint8_t *img)
{
    switch (ihdr.img_type) {
        case MAPPED:        /* uncompressed color mapped */
            mapped(data, ihdr, cmap, img);
            break;

        case RGB:           /* uncompressed RGB */
            rgb(data, ihdr, img);
            break;

        case RLE_MAPPED:    /* run length encoded & color mapped */
        case RLE_RGB:       /* run length encoded RGB */
            rle(data, ihdr, cmap, img);
            break;

        default:
            printf("unsupported tga data type code");
    }
}

/**************************************************************
 *                                                            *
 *                    public definitions                      *
 *                                                            *
 **************************************************************/

/***************
 * gl_load_tga *
 ***************/

/*
 * allocates a texture buffer, fills with color contents,
 * and sets it to the given pointer
 * 1 on success, 0 on failure
 */

int
gl_load_tga(
    char *path,
    uint32_t **data_out,
    int *width_out,
    int *height_out)
{
    FILE *fp;
    uint32_t *data;
    uint8_t *cmap, *img;
    struct header ihdr;
    int width, height;
    int cmap_stride, img_stride;

    fp = fopen(path, "rb");

    /* fill header */

    u8(&ihdr.idlen, fp);
    u8(&ihdr.cmap_type, fp);
    u8(&ihdr.img_type, fp);
    u16(&ihdr.cmap_start, fp);
    u16(&ihdr.cmap_len, fp);
    u8(&ihdr.cmap_depth, fp);
    u16(&ihdr.img_x, fp);
    u16(&ihdr.img_y, fp);
    u16(&ihdr.img_w, fp);
    u16(&ihdr.img_h, fp);
    u8(&ihdr.img_depth, fp);
    u8(&ihdr.img_desc, fp);

    width = ihdr.img_w;
    height = ihdr.img_h;

    /* depth is bits per pixel, stride is bytes per pixel */

    cmap_stride = ihdr.cmap_depth >> 3;
    img_stride = ihdr.img_depth >> 3;

    /* fill color map and image data */

    cmap = calloc(ihdr.cmap_len * cmap_stride, 1);
    img = calloc(width * height * img_stride, 1);

    fseek(fp, ihdr.idlen, SEEK_CUR);
    fread(cmap, ihdr.cmap_len * cmap_stride, 1, fp);
    fread(img, width * height * img_stride, 1, fp);
   
    fclose(fp);

    /* fill contents of the image */

    data = calloc(width * height, 4);
    parse(data, ihdr, cmap, img);

    /* return */

    *data_out = data;
    *width_out = width;
    *height_out = height;

    free(cmap);
    free(img);

    return 0;
}