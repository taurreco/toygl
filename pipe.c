
#include <stdlib.h>
#include <string.h>

#include "sr.h"
#include "sr_priv.h"

/**
 * sr_pipe.c
 * --------
 * implementation of the internal graphics pipeline,
 * resembling roughly how its done in hardware
 * 
 */

/*********************************************************************
 *                                                                   *
 *                       private definitions                         *
 *                                                                   *
 *********************************************************************/

/*****************
 * winding_order *
 *****************/

/* false for triangles against the winding order */

static int
winding_order(int winding, float* v0, float* v1, float* v2)
{
    float e01 = (v1[0] - v0[0]) * (v1[1] + v0[1]);
    float e12 = (v2[0] - v1[0]) * (v2[1] + v1[1]);
    float e20 = (v0[0] - v2[0]) * (v0[1] + v2[1]);

    return (e01 + e12 + e20) * winding > 0;  /* same sign */
}

/*************
 * draw_prim *
 *************/

/* matches the correct drawing routine with the primitive type */

static void 
draw_prim(struct raster_context* rast, float* pts, 
          int n_pts, enum sr_primitive prim_type)
{
    switch (prim_type) {
        case SR_POINT_LIST:    /* point list */
            for (int i = 0; i < n_pts; i++) {
                draw_pt(rast, pts + i * rast->n_attr);
            }
            break;

        case SR_LINE_LIST:
        case SR_LINE_STRIP:    /* line list */
            for (int i = 1; i < n_pts; i++) {
                /* draw_ln(rast, tmp_p, tmp_p + i * n_attr_out); */
            }
            break;

        case SR_TRIANGLE_LIST:
        case SR_TRIANGLE_STRIP:    /* triangle fan */
            {
                float* v0 = pts;
                float* v1 = pts + 1 * rast->n_attr;
                for (int i = 2; i < n_pts; i++) {
                    float* v2 = pts + i * rast->n_attr;
                    if (winding_order(rast->winding, v0, v1, v2))
                        draw_tr(rast, v0, v1, v2);
                    v1 = v2;
                }
            }
            break;
    }
}

/**************
 * split_prim *
 **************/

/* fills relevant traversal data about a primitive type */

static void
split_prim(enum sr_primitive prim_type, int* prim_size)
{
    switch (prim_type) {
        case SR_POINT_LIST:
            *prim_size = 1;
            break;

        case SR_LINE_LIST:
        case SR_LINE_STRIP:
            *prim_size = 2;
            break;
            
        case SR_TRIANGLE_LIST:
        case SR_TRIANGLE_STRIP:
            *prim_size = 3;
            break;
    }
}

/****************
 * screen_space *
 ****************/

/* moves coordinates from clip space to screen space */

static void
screen_space(struct sr_framebuffer* fbuf, float* pt)
{
    /* to ndc space */

    pt[3] = 1 / pt[3];
    pt[0] *= pt[3];
    pt[1] *= pt[3];
    pt[2] *= pt[3]; /* should equal 1 now */

    /* to screen space */
    pt[0] = (fbuf->width / 2) * (pt[0] + 1);
    pt[1] = (fbuf->height / 2) * (1 - pt[1]);
    pt[2] = (pt[2] + 1) / 2;
}

/*********************************************************************
 *                                                                   *
 *                         public definition                         *
 *                                                                   *
 *********************************************************************/

/*************
 * sr_render *
 *************/

/**
 * entry point of the sr pipeline, 
 * refines indexed vertex data to be sent to rasterizer
 */

void
sr_render(struct sr_pipeline* pipe, int* indices, 
          int n_indices, enum sr_primitive prim_type)
{
    /* setup variables */
    
    struct raster rast = {
        .fbuf = pipe->fbuf, 
        .uniform = pipe->uniform, 
        .fs = pipe->fs, 
        .n_attr = pipe->n_attr_out,
        .winding = pipe->winding
    };

    int prim_size, n_prims;

    float* pts_out;
    float tmp[16 * SR_MAX_ATTRIBUTE_COUNT]; /* holds current face */;
    uint8_t* clip_flags;

    prim_size = 0;
    split_prim(prim_type, &prim_size);
    n_prims = n_indices / prim_size;

    /* vertex processing */
    
    pts_out = malloc(pipe->n_pts * pipe->n_attr_out * sizeof(float));
    clip_flags = malloc(pipe->n_pts * sizeof(uint8_t));

    for (int i = 0; i < pipe->n_pts; i++) {    /* per point */

        /* vertex shader pass */
        pipe->vs(pts_out + i * pipe->n_attr_out,
                 pipe->pts_in + i * pipe->n_attr_in, 
                 pipe->uniform);

        /* grab clip flags while vertex is still hot */
        clip_test(pts_out + i * pipe->n_attr_out, clip_flags + i);
    }

    for (int i = 0; i < n_prims * prim_size; i += prim_size) {

        /* primitive assembly */

        uint8_t clip_and, clip_or;

        clip_and = 0;
        clip_or = 0;

        for (int j = 0; j < prim_size; j++) {
            /* fill buffer with primitive data */
            memcpy(tmp + j * pipe->n_attr_out,
                   pts_out + indices[i + j] * pipe->n_attr_out, 
                   pipe->n_attr_out * sizeof(float));

            /* accumulate point clip flags */
            clip_and &= clip_flags[indices[i + j]];
            clip_or |= clip_flags[indices[i + j]];
        }

        /* clipping */

        if (clip_and != 0)  /* outside frustum */
            continue;

        int clipped_prim_size = prim_size;
        if (clip_or != 0)     /* if intersect frustum */
            clip_poly(tmp, &clipped_prim_size, pipe->n_attr_out, clip_or);

        /* perspective divide */
        for (int j = 0; j < clipped_prim_size; j++)
            screen_space(pipe->fbuf, tmp + j * pipe->n_attr_out);
        
        draw_prim(&rast, tmp, clipped_prim_size, prim_type);
    }
    free(pts_out);
    free(clip_flags);
}
