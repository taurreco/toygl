#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "tga.h"

#define CMAP            1
#define RGB             2
#define RLE_CMAP        9
#define RLE_RGB         10

/***************************************************************
 *                                                             *
 *                   private data structures                   *
 *                                                             *
 ***************************************************************/

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

/***************************************************************
 *                                                             *
 *                      private helpers                        *
 *                                                             *
 ***************************************************************/

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

/* unpacks given bit representation of color to uint32  */

uint32_t
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

/**********
 * mapped *
 **********/

void
mapped(
    struct header *ihdr,
    uint32_t *data, 
    uint8_t *cmap,
    uint8_t *img)
{
    for (int i = 0; i < header->img_w * header->img_h; i++) {
        bytes = cmap + img[i] * n_bytes;
        data[i] = unpack(bytes, n_bytes);
    }
}

/*******
 * rgb *
 *******/

void
rgb(struct header *hdr, uint32_t *data, uint8_t *img)
{
    uint8_t* bytes;
    int n_bytes;
    
    bytes = img;
    n_bytes = hdr->img_depth >> 3;
    
    for (int i = 0; i < hdr->img_w * hdr->img_h; i++) {
        data[i] = unpack(bytes, n_bytes);
        bytes += n_bytes;
    }
}

/*******
 * rle *
 *******/

static void
rle(struct header *hdr, uint32_t *data, uint8_t *cmap, uint8_t *img)
{
    int pixel_bytes;
    uint8_t* packet;
    
    pixel_bytes = tga->pixel_depth >> 3;
    packet = tga->pixels;

    for (int i = 0; i < header.w * header.h; i++) {
        
        int len;
        len = (*packet & 0x7F) + 1;
        
	    if (*packet & 0x80) {    /* run length packet */
            
            uint32_t color;
            
	        color = unpack(bytes, stride);
            for (int j = 0; j < len; j++) {
                data[i + j] = color;
            }

            /* next packet */

            packet += pixel_bytes + 1;
        } else {                /* raw packet */
            for (int j = 0; j < len; j++) {
                data[i + j] = unpack(bytes, stride);
                bytes += pixel_bytes;
            }

            /* next packet */
            packet += len * pixel_bytes + 1;  
        }
        i += len - 1;
    }
}

/*********
 * parse *
 *********/

/* placeholder */

static int
parse(
    struct header *ihdr,
    uint32_t *data, 
    uint8_t *cmap,
    uint8_t *img)
{
    stride = color_bytes;
    bytes = tga.colors + packet[1];

    switch (header.data_type) {
	
	/* uncompressed color mapped */
	case MAPPED:
	    mapped(data, &tga);	
	    break;
	/* uncompressed RGB */
	case RGB:
            rgb(data, &tga);
	    break;
        case RLE_MAPPED:    /* run length encoded & color mapped */
            stride = pixel_bytes;
            bytes = packet + 1;
        case RLE_RGB:   /* run length encoded RGB */
            rle(data, tga, bytes, stride);
            break;
        default:
            free(colors);
            return 0;
    }
    return 1;
}

/***************************************************************
 *                                                             *
 *                         public api                          *
 *                                                             *
 ***************************************************************/

/************
 * tga_load *
 ************/

/* placeholder */

int
tga_load(char *file, uint32_t **data_p, int *w_p, int *h_p)
{
    FILE *fp;
    struct header ihdr;
    uint8_t *cmap, *img;
    int size, cmap_bytes, img_bytes;

    fp = fopen(fp, "r");

    /* fill header */

    u8(&ihdr->idlen, fp);
    u8(&ihdr->cmap_type, fp);
    u8(&ihdr->img_type, fp);
    u16(&ihdr->cmap_start, fp);
    u16(&ihdr->cmap_len, fp);
    u8(&ihdr->cmap_depth, fp);
    u16(&ihdr->img_x, fp);
    u16(&ihdr->img_y, fp);
    u16(&ihdr->img_w, fp);
    u16(&ihdr->img_h, fp);
    u8(&ihdr->img_depth, fp);
    u8(&ihdr->img_desc, fp);

    cmap_bytes = ihdr.cmap_depth >> 3;
    img_bytes = ihdr.img_depth >> 3;

    /* fill color map and image data */

    colors = calloc(ihdr.cmap_len * cmap_bytes, 1);
    pixels = calloc(ihdr.img_w * ihdr.img_h * img_bytes, 1);

    fseek(fp, ihdr.idlen, SEEK_CUR);
    fread(cmap, ihdr.cmap_len * cmap_bytes, 1, fp);
    fread(img, ihdr.img_w * ihdr.img_h * img_bytes, 1, fp);
   
    fclose(fp);

    parse(&ihdr, *data_p, cmap, img);
    *w_p = ihdr.img_w;
    *h_p = ihdr.img_h;

    free(cmap);
    free(img)
}