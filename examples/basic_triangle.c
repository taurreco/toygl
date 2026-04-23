
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "driver.h"
#include "sr.h"

/**
 * basic_triangle.c
 * --------
 * this is *almost* the bare minimum setup to
 * produce some cool graphics with the sr library!
 * 
 */

/***************************************************************
 *                                                             *
 *                    indexed triangle list                    *
 *                                                             *
 ***************************************************************/

float pts[18] = {
    /* x y z */         /* r g b */
    0.5, -0.5, 0.0,     0.0, 1.0, 0.0,
    0.0, 0.5, 0.0,      1.0, 0.0, 0.0,
    -0.5, -0.5, 0.0,    0.0, 0.0, 1.0
};

int indices[3] = {0, 1, 2};

/***************************************************************
 *                                                             *
 *                        vertex shader                        *
 *                                                             *
 ***************************************************************/

/******
 * vs *
 ******/

/* emits a clip space vertex given an object space vertex */
static void 
vs(float* out, float* in, void* uniform)
{
    float tmp[4] = { in[0], in[1], in[2], 1 };   /* homogenize */ 
    memcpy(out, tmp, 4 * sizeof(float));         /* position */
    memcpy(out + 4, in + 3, 3 * sizeof(float));  /* colors */
}

/***************************************************************
 *                                                             *
 *                      fragment shader                        *
 *                                                             *
 ***************************************************************/

/******
 * fs *
 ******/

/* emits a pixel color per screen space vertex */

static void
fs(uint32_t* out, float* in, void* uniform) 
{   
    uint8_t a = 255;
    uint8_t r = in[4] * 255;
    uint8_t g = in[5] * 255;
    uint8_t b = in[6] * 255;
    *out = a << 24 | r << 16 | g << 8 | b << 0; 
}

/***************************************************************
 *                                                             *
 *                      prepare context                        *
 *                                                             *
 ***************************************************************/

/*********
 * start *
 *********/

/* runs only once in the begining */
extern void
start()  
{
    sr_bind_pts(pts, 3, 6);
    sr_bind_framebuffer(SCREEN_WIDTH, SCREEN_HEIGHT, colors, depths);
    sr_bind_vs(vs, 7);
    sr_bind_fs(fs);
}

/***************************************************************
 *                                                             *
 *                        render image                         *
 *                                                             *
 ***************************************************************/

/**********
 * update *
 **********/

/* runs every frame */
extern void
update(float dt)
{
    sr_renderl(indices, 3, SR_TRIANGLE_LIST);
}

/***************************************************************
 *                                                             *
 *                     free program data                       *
 *                                                             *
 ***************************************************************/

/*******
 * end *
 *******/

/* runs once at the end of the program */
extern void
end() { /* not implemented */ }