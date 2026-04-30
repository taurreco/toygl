#include <math.h>
#include <string.h>
#include <stdlib.h>

#include "sr_priv.h"

/**
 * sr_rast.c
 * --------
 * rasterizes triangles, lines, and points
 * 
 */

/***************************************************************
 *                                                             *
 *                    private utility helpers                  *
 *                                                             *
 ***************************************************************/

/*********
 * is_tl *
 *********/

/**
 * determines whether an edge is top or left 
 * where the edge is directed, starting at v0 pointing to v1
 */

static int 
is_tl(float *v0, float *v1) 
{
    int is_top = (v0[1] == v1[1]) && (v0[0] > v1[0]); 
    int is_left = v0[1] < v1[1];
    return is_top | is_left;
}

/***************************************************************
 *                                                             *
 *                    private constructors                     *
 *                                                             *
 ***************************************************************/

/*************
 * edge_init *
 *************/

/* make an edge struct instance, return the initial determinant */

static float 
edge_init(
    struct edge *edge,
    int winding, 
    float *v0,
    float *v1,
    float *pt)
{
    edge->is_tl = is_tl(v0, v1);

    float A = v1[1] - v0[1];
    float B = v0[0] - v1[0];
    float C = v0[1] * v1[0] - v0[0] * v1[1];

    A *= winding;
    B *= winding;
    C *= winding;

    /* step sizes */
    edge->step_x = A;
    edge->step_y = B;

    /* edge function output at origin */
    return A * pt[0] + B * pt[1] + C;
}

/*************
 * bbox_init *
 *************/

/* define a pixel-aligned bounding box for triangle rasterization */

static void
bbox_init(struct bbox *bbox, float *v0, float *v1, float *v2)
{    
    /* naiive values */

    bbox->min_x = fmin(v0[0], fmin(v1[0], v2[0]));
    bbox->min_y = fmin(v0[1], fmin(v1[1], v2[1]));
    bbox->max_x = fmax(v0[0], fmax(v1[0], v2[0]));
    bbox->max_y = fmax(v0[1], fmax(v1[1], v2[1]));

    /* align to pixel centers */

    bbox->min_x = floorf(bbox->min_x) + 0.5;
    bbox->min_y = floorf(bbox->min_y) + 0.5;
    bbox->max_x = floorf(bbox->max_x) + 0.5;
    bbox->max_y = floorf(bbox->max_y) + 0.5;
}

/***************************************************************
 *                                                             *
 *                         public api                          *
 *                                                             *
 ***************************************************************/

/***********
 * draw_pt *
 ***********/

/* render point to framebuffer */

void 
draw_pt(struct raster_context *rast, float *pt)
{
    uint32_t color = 0; /* color dest */
    rast->fs(&color, pt, rast->uniform);  /* fragment shader */
    size_t fbuf_idx = floorf(pt[1]) * rast->fbuf->width + floorf(pt[0]);
    
    if (pt[2] < rast->fbuf->depths[fbuf_idx]) {  /* depth buffer */
        rast->fbuf->colors[fbuf_idx] = color;
        rast->fbuf->depths[fbuf_idx] = pt[2];
    }
}

/***********
 * draw_tr *
 ***********/

/* rasterize a triangle to framebuffer */

void 
draw_tr(struct raster_context *rast, float *v0, float *v1, float *v2)
{   
    /* find bounding box */

    struct bbox bbox; 
    bbox_init(&bbox, v0, v1, v2);

    /* store current point */

    float pt[SR_MAX_ATTRIBUTE_COUNT];
    pt[0] = bbox.min_x;
    pt[1] = bbox.min_y;

    /* grab edge information */

    struct edge e12, e20, e01;

    float w0_row = edge_init(&e12, rast->winding, v1, v2, pt);
    float w1_row = edge_init(&e20, rast->winding, v2, v0, pt);
    float w2_row = edge_init(&e01, rast->winding, v0, v1, pt);

    /* rasterize */

    for (pt[1] = bbox.min_y; pt[1] <= bbox.max_y; pt[1]++) {

        float w0 = w0_row;
        float w1 = w1_row;
        float w2 = w2_row;

        for (pt[0] = bbox.min_x; pt[0] <= bbox.max_x; pt[0]++) {

            float f0 = (w0 == 0) && !e12.is_tl ? -1 : 0;
            float f1 = (w1 == 0) && !e20.is_tl ? -1 : 0;
            float f2 = (w2 == 0) && !e01.is_tl ? -1 : 0;

            if ( (w0 + f0 >= 0) && (w1 + f1 >= 0) && (w2 + f2 >= 0) ) {

                /* normalize barycentric weights */

                float area = w0 + w1 + w2;
                
                float b0 = w0 / area;
                float b1 = w1 / area;
                float b2 = w2 / area;

                /* interpolate z and w */

                float a = b0 * v0[3];
                float b = b1 * v1[3];
                float c = b2 * v2[3];

                float Z = a + b + c;

                pt[2] = 1 / Z;
                pt[3] = Z;

                /* interpolate rest of points */

                for (int i = 4; i < (int)rast->n_attr; i++) {
                    float P = (a * v0[i] + b * v1[i] + c * v2[i]);
                    pt[i] = P * pt[2]; /* to clip space */
                }

                draw_pt(rast, pt);
            }

            w0 += e12.step_x;
            w1 += e20.step_x;
            w2 += e01.step_x;
        }

        w0_row += e12.step_y;
        w1_row += e20.step_y;
        w2_row += e01.step_y;
    }
}