#include <string.h>
#include <stdint.h>
#include <stddef.h>

#include "sr.h"
#include "clip.h"

/**
 * sr_clip.c
 * --------
 * defines functions to clip a polygon of vectors (x, y, z, w) in
 * homogeneous space
 * 
 */

/*********************************************************************
 *                                                                   *
 *                       private definitions                         *
 *                                                                   *
 *********************************************************************/

/********
 * swap *
 ********/

/* swaps what the pointers point to */

static void
swap(float **a, float **b)
{
    void* tmp = *a;
    *a = *b;
    *b = tmp;
}

/********
 * lerp *
 ********/

/* interpolates a point some length up an edge */

static void
lerp(float *dest, float *from, float *to, float a, int n_attr)
{
    for (int i = 0; i < n_attr; i++) {
        dest[i] = from[i] + (to[i] - from[i]) * a;
    }
}

/****************
 * clip_routine *
 ****************/

/**
 * a variant of cohen-sutherland for one plane as determined by an
 * axis and a sign to indicate direction
 */

static void
clip_routine(
    float *dest,
    float *src, 
    int *n_pts,
    int n_attr, 
    int axis,
    int sign)
{
    float *tmp = dest;    /* walk along dest with this pointer */

    float *prev = src + (*n_pts - 1) * n_attr;  /* last pt */
    int prev_inside = prev[axis] * sign <= prev[3];

    for (int i = 0; i < *n_pts; i++) {

        float* cur = src + i * n_attr;
        int cur_inside = cur[axis] * sign <= cur[3];

        /* intersection */

        if (cur_inside ^ prev_inside) {
            float a = (prev[3]  - prev[axis] * sign) / 
                      ((prev[3] - prev[axis] * sign) - 
                      (cur[3]   - cur[axis]  * sign));
            lerp(tmp, prev, cur, a, n_attr);
            tmp += n_attr;
        }

        /* point inside */

        if (cur_inside) {
            memcpy(tmp, cur, n_attr * sizeof(float));
            tmp += n_attr;
        }
        
        prev = cur;
        prev_inside = cur_inside;
    }
    *n_pts = (tmp - dest) / n_attr;    /* new number of points after clip */
}

/*********************************************************************
 *                                                                   *
 *                        public definitions                         *
 *                                                                   *
 *********************************************************************/

/*************
 * clip_poly *
 *************/

/**
 * matches a one-hot clip flag to its clip routine updates source points
 * to the clipped ones in place
 */

void
clip_poly(
    float *src,
    int *n_pts, 
    int n_attr,
    uint8_t clip_flags)
{
    float dest[16 * SR_MAX_ATTRIBUTE_COUNT];
    float tmp[16 * SR_MAX_ATTRIBUTE_COUNT];
    memcpy(tmp, src, *n_pts * n_attr * sizeof(float));
    float *tmp_src = (float*)tmp;
    float *tmp_dest = (float*)dest;

    if (clip_flags & SR_CLIP_LEFT_PLANE) {        /* left */
        clip_routine(tmp_dest, tmp_src, n_pts, n_attr, 0, -1);
        swap(&tmp_src, &tmp_dest);
    }

    if (clip_flags & (SR_CLIP_BOTTOM_PLANE)) {    /* bottom */
         clip_routine(tmp_dest, tmp_src, n_pts, n_attr, 1, -1);
        swap(&tmp_src, &tmp_dest);
    }

    if (clip_flags & SR_CLIP_NEAR_PLANE) {        /* near */
        clip_routine(tmp_dest, tmp_src, n_pts, n_attr, 2, -1);
        swap(&tmp_src, &tmp_dest);
    }

    if (clip_flags & SR_CLIP_RIGHT_PLANE) {       /* right */
        clip_routine(tmp_dest, tmp_src, n_pts, n_attr, 0, 1);
        swap(&tmp_src, &tmp_dest);
    }

    if (clip_flags & SR_CLIP_TOP_PLANE) {         /* top */
        clip_routine(tmp_dest, tmp_src, n_pts, n_attr, 1, 1);
        swap(&tmp_src, &tmp_dest);
    }

    memcpy(src, tmp_src, *n_pts * n_attr * sizeof(float));
}

/*************
 * clip_test *
 *************/

/* assigns proper clip flags to a point */

void
clip_test(float *pt, uint8_t *flags)
{
    uint8_t left   = (pt[3] + pt[0] < 0) << 0;
    uint8_t bottom = (pt[3] + pt[1] < 0) << 1;
    uint8_t near   = (pt[3] + pt[2] < 0) << 2;
    uint8_t right  = (pt[3] - pt[0] < 0) << 3;
    uint8_t top    = (pt[3] - pt[1] < 0) << 4;

    *flags = left | bottom | near | right | top;
}