
#include <stdio.h>
#include <stdlib.h>

#include "sr.h"
#include "sr_priv.h"

/**
 * sr_shaders.c
 * --------
 * vertex and fragment shaders
 * 
 */

/**************************************************************
 *                                                            *
 *                      color & blending                      *
 *                                                            *
 **************************************************************/

/*********
 * clamp *
 *********/

/* clamps float value between 0 and 1 */

static float
clamp(float v)
{
    return fmin(fmax(v, 0), 1);
}

/***********
 * argb_int *
 ***********/

/* converts a float representation of a color into an int */

static uint32_t
rgb_int(float *color)
{
    uint8_t a = floorf(clamp(color[0]) * 255);;
    uint8_t r = floorf(clamp(color[1]) * 255);
    uint8_t g = floorf(clamp(color[2]) * 255);
    uint8_t b = floorf(clamp(color[3]) * 255);

    return a << 24 | r << 16 | g << 8 | b << 0; 
}

/*************
 * argb_float *
 *************/

/* converts one int into a float representation of a color */

static void
rgb_float(float *a, uint32_t b)
{
    a[0] = ((b & 0xFF000000) >> 24) / (float)255;
    a[1] = ((b & 0x00FF0000) >> 16) / (float)255;
    a[2] = ((b & 0x0000FF00) >> 8)  / (float)255;
    a[3] = ( b & 0x000000FF)        / (float)255;
}

/**************************************************************
 *                                                            *
 *                        shader helpers                      *
 *                                                            *
 **************************************************************/

/**************
 * clip_space *
 **************/

/**
 * sends the vector 'in' to clip space by applying an mvp 
 * sets the first four indices of 'out' to these coordinates
 */

static void
clip_space(float *out, float *in, struct sr_uniform *uniform)
{
    /* homogenize vector */
    float tmp[4] = { in[0], in[1], in[2], 1 };
    vec4_matmul(out, uniform->mvp, tmp);
}

/***************
 * world_space *
 ***************/

/**
 * sends the vector 'in' to world space by applying a model matrix 
 * sets the first three indices of 'out' to these coordinates
 */

static void
world_space(float *out, float *in, struct sr_uniform *uniform)
{
    /* homogenize vector */
    float tmp_in[4] = { in[0], in[1], in[2], 1 };
    float tmp_out[4];
    vec4_matmul(tmp_out, uniform->model, tmp_in);
    memcpy(out, tmp_out, 3 * sizeof(float));
}

/**********************
 * world_space_normal *
 **********************/

/**
 * sends the normal vector 'in' to world space by applying a normal 
 * transform the first three indices of 'out' to these coordinates
 */

static void
world_space_normal(float *out, float *in, struct sr_uniform *uniform)
{
    /* homogenize vector */
    float tmp_in[4] = { in[0], in[1], in[2], 1 };
    float tmp_out[4];
    vec4_matmul(tmp_out, uniform->normal_transform, tmp_in);
    memcpy(out, tmp_out, 3 * sizeof(float));
}

/******************
 * sample_texture *
 ******************/

/**
 * sets 'color' to the float triple color of the 
 * texture at coordinates 'u' and 'v' 
 */

static void
sample_texture(struct sr_texture *texture, float *color, float u, float v)
{
    int x = floorf(u * texture->width);
    int y = texture->height - 1 - floorf(v * texture->height);
    rgb_float(color, texture->colors[y * texture->width + x]);
}

/***********
 * falloff *
 ***********/

static float 
falloff(float x, float inner, float outer) {
    return -2 * powf((x - inner) / (outer - inner), 3) + 
            3 * powf((x - inner) / (outer - inner), 2);
}

/*********
 * phong *
 *********/

/**
 * given a world position, normal, and light, calculates 
 * an ambient, diffuse, and specular intensites to blend 
 * with a base color 
 */

static void
phong(
    float *color,
    float *pos,
    float *uv, 
    float *normal,
    struct sr_uniform *uniform)
{
    float fatt, intensity, dist;
    float I[4], L[3], R[3], V[3], tmp[4];
    float *Oa, *Od, *Os;
    float n, ka, kd, ks;

    memset(color, 0, 4 * sizeof(float));
    fatt = 1;
    intensity = 1;

    Oa = uniform->material->ambient;
    Od = uniform->material->diffuse;
    Os = uniform->material->specular;
    n = uniform->material->shininess;
    ka = uniform->ka;
    kd = uniform->kd;
    ks = uniform->ks;

    /* ambient */
    vec4_scale(tmp, Oa, ka);
    vec4_add(color, color, tmp);

    for (int i = 0; i < SR_MAX_LIGHT_COUNT; i++) {
        if (uniform->light_state & (1 << i)) {
            struct light light = uniform->lights[i];

            memcpy(I, light.color, 4 * sizeof(float));

            /* directional light and spot light */
            if (0x1 & light.type) 
                vec3_scale(L, light.dir, -1);
            
            /* point light and spot light */
            if (0x6 & light.type) {
                vec3_sub(L, light.pos, pos);
                dist = magnitude(L);
                fatt = 1 / (light.attn_quad * dist * dist + 
                            light.attn_lin * dist + 
                            light.attn_const);
            }

            /* spot light */
            if (0x4 & light.type) {
                vec3_scale(L, light.dir, -1);
                float light_dir[3];
                memcpy(light_dir, light.dir, 3 * sizeof(float));
                vec3_sub(tmp, pos, light.pos);
                normalize(tmp);
                normalize(light_dir);
                float x = acos(dot(light_dir, tmp));
                float inner = light.spot_angle - light.spot_penumbra;
                float outer = light.spot_angle;
                if (x <= inner) {
                    intensity = 1;
                } else if (x <= outer) {
                    intensity = (1 - falloff(x, inner, outer));
                } else {
                    intensity = 0;
                }
                vec4_scale(I, light.color, intensity);
            }

            normalize(L);

            /* diffuse color */
            vec4_scale(tmp, Od, kd);
            if (uniform->has_texture) {
                float tex_color[4];
                sample_texture(uniform->texture, tex_color, uv[0], uv[1]);
                lerp(tmp, tmp, tex_color, uniform->material->blend);
            }
            vec4_scale(tmp, tmp, clamp(dot(normal, L)));
            vec4_mul(tmp, tmp, I);
            vec4_scale(tmp, tmp, fatt);
            vec4_add(color, color, tmp);

            /* specular color */
            reflect(R, L, normal);
            vec3_sub(V, uniform->cam_pos, pos);
            normalize(V);
            normalize(R);
            
            vec4_scale(tmp, Os, powf(clamp(dot(R, V)), n));
            vec4_scale(tmp, tmp, ks);
            vec4_mul(tmp, tmp, I);
            vec4_scale(tmp, tmp, fatt);
            vec4_add(color, color, tmp);
        }
    }
}

/**************************************************************
 *                                                            *
 *                            color                           *
 *                                                            *
 **************************************************************/

/**
 * in (3):
 *    float x, y, z;
 * 
 * out (7):
 *    float x, y, z, w;
 *    float r, g, b;
 */

/************
 * color_vs *
 ************/

/* copies argb coords over from 'in' to 'out' */

static void 
color_vs(float *out, float *in, void *uniform)
{
    clip_space(out, in, uniform);  /* position */
    memcpy(out + 4, in + 3, 3 * sizeof(float));  /* color */
}

/************
 * color_fs *
 ************/

/* uses argb coords to fit color representation */
static void
color_fs(uint32_t *out, float *in, void *uniform)
{
    float color[4] = { in[4], in[5], in[6], 1 };
    *out = rgb_int(color);  /* frag color */
}

/**************************************************************
 *                                                            *
 *                           texture                          *
 *                                                            *
 **************************************************************/

/**
 * in (5):
 *    float x, y, z;
 *    float u, v;
 * 
 * out (6):
 *    float x, y, z, w;
 *    float u, v;
 */

/**************
 * texture_vs *
 **************/

/* copies argb coords over from 'in' to 'out' */

static void 
texture_vs(float *out, float *in, void *uniform)
{
    clip_space(out, in, uniform);  /* position */
    memcpy(out + 4, in + 3, 2 * sizeof(float));  /* texture */
}

/**************
 * texture_fs *
 **************/

/* uses argb coords to fit color representation */

static void
texture_fs(uint32_t *out, float *in, void *uniform)
{   
    struct sr_uniform *sr_uniform = (struct sr_uniform *)uniform;
    float color[4];
    sample_texture(sr_uniform->texture, color, in[4], in[5]); 
    *out = rgb_int(color);  /* frag color */
}

/**************************************************************
 *                                                            *
 *                           phong                            *
 *                                                            *
 **************************************************************/

/**
 * in (8):
 *    float x, y, z;
 *    float u, v;
 *    float nx, ny, nz;
 * 
 * out (12):
 *    float x, y, z, w;
 *    float wx, wy, wz;
 *    float u, v;
 *    float nx, ny, nz;
 */

/**********
 * std_vs *
 **********/

/* copies argb coords over from 'in' to 'out' */

static void 
std_vs(float *out, float *in, void *uniform)
{
    struct sr_uniform *sr_uniform = (struct sr_uniform *)uniform;

    /* x y z w */
    clip_space(out, in, sr_uniform);  /* position */

    /* wx, wy, wz */
    world_space(out + 4, in, sr_uniform);

    /* u v */
    memcpy(out + 7, in + 3, 2 * sizeof(float));

    /* nx ny nz */
    world_space_normal(out + 9, in + 5, sr_uniform);

    /* normalize them */
    normalize(out + 9);
}

/************
 * phong_fs *
 ************/

/* blends a phong sample with color base */

static void
phong_fs(uint32_t *out, float *in, void *uniform)
{
    struct sr_uniform *sr_uniform = (struct sr_uniform *)uniform;

    float color[4];
    normalize(in + 9);
    phong(color, in + 4, in + 7, in + 9, sr_uniform);
    *out = rgb_int(color);
}

/**************************************************************
 *                                                            *
 *                         bindings                           *
 *                                                            *
 **************************************************************/

/*********
 * color *
 *********/

void
sr_bind_color_vs()
{
    sr_bind_vs(color_vs, 7);
}

void
sr_bind_color_fs()
{
    sr_bind_fs(color_fs);
}

/***********
 * texture *
 ***********/

void
sr_bind_texture_vs()
{
    sr_bind_vs(texture_vs, 6);
}

void
sr_bind_texture_fs()
{
    sr_bind_fs(texture_fs);
}

/*********
 * phong *
 *********/

void
sr_bind_std_vs()
{
    sr_bind_vs(std_vs, 12);
}

void
sr_bind_phong_fs()
{
    sr_bind_fs(phong_fs);
}