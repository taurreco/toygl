#include <stdlib.h>
#include <string.h>
#include <math.h>    /* TODO: remove this dependency */

#include "gl.h"
#include "mat.h"

/***************************************************************
 *                                                             *
 *                     private structures                      *
 *                                                             *
 ***************************************************************/

struct texture {
    uint32_t *colors;
    int width;
    int height;
};

struct material {
    struct texture texture;
    float ambient[4];
    float diffuse[4];
    float specular[4];
    float blend;
    float shininess;
};

struct uniform {
    struct mat4 *model;
    struct mat4 *normal;
    struct mat4 *mvp;
    float cam_pos[3];
    int has_texture;
    struct material *material;
    struct texture *texture;
    uint8_t light_state;
    struct light *lights;
    float ka;
    float kd;
    float ks;
};

struct light {
    enum gl_light_type type;
    float pos[3];
    float color[4];
    float dir[3];
    float spot_angle;
    float spot_penumbra;
    float attn_const;
    float attn_lin;
    float attn_quad;
};

/***************************************************************
 *                                                             *
 *                      global variables                       *
 *                                                             *
 ***************************************************************/


/* identity matrix */
static const struct mat4 identity = {
    1, 0, 0, 0,
    0, 1, 0, 0,
    0, 0, 1, 0,
    0, 0, 0, 1,
};

/* model matrix */
static struct mat4 model = {
    1, 0, 0, 0,
    0, 1, 0, 0,
    0, 0, 1, 0,
    0, 0, 0, 1,
};

/* normal transform matrix */
static struct mat4 normal = {
    1, 0, 0, 0,
    0, 1, 0, 0,
    0, 0, 1, 0,
    0, 0, 0, 1,
};

/* camera view matrix */
static struct mat4 view = {
    1, 0, 0, 0,
    0, 1, 0, 0,
    0, 0, 1, 0,
    0, 0, 0, 1,
};

/* projection matrix */
static struct mat4 proj = {
    1, 0, 0, 0,
    0, 1, 0, 0,
    0, 0, 1, 0,
    0, 0, 0, 1,
};

/* model view projection matrix */
static struct mat4 mvp = {
    1, 0, 0, 0,
    0, 1, 0, 0,
    0, 0, 1, 0,
    0, 0, 0, 1,
};

static struct light g_lights[GL_MAX_LIGHT_COUNT];
static struct material g_material;
static struct mat4 *cur_mat;

static struct texture g_texture = {
    .colors = NULL,
    .width  = 0,
    .height = 0,
};

/* framebuffer */
static struct gl_framebuffer g_fbuf = {
    .colors = NULL,
    .depths = NULL,
    .width  = 0,
    .height = 0,
};

/* uniform */
static struct uniform g_uniform = {
    .model            = &model,
    .normal           = &normal,
    .mvp              = &mvp,
    .has_texture      = 0,
    .material         = &g_material,
    .texture          = &g_texture,
    .lights           = g_lights,
    .ka               = 1,
    .kd               = 1,
    .ks               = 1,
};

/* pipeline state */
static struct gl_pipeline g_pipe = {
    .fbuf             = &g_fbuf,
    .uniform          = (void*)(&g_uniform),
    .vs               = 0,
    .fs               = 0,
    .pts_in           = 0,
    .n_pts            = 0,
    .n_attr_in        = 0,
    .n_attr_out       = 0,
    .winding          = GL_WINDING_ORDER_CCW,
};

/***************************************************************
 *                                                             *
 *                      render interface                       *
 *                                                             *
 ***************************************************************/

/**************
 * gl_renderl *
 **************/

/* builds the mvp and renders the global state */
extern void
gl_renderl(int *indices, int n_indices, enum gl_primitive prim_type)
{
    /* create mvp */
    mvp = identity;
    matmul(&mvp, &proj);
    matmul(&mvp, &view);
    matmul(&mvp, &model);

    /* create normal transform matrix */
    normal = model;
    upper_3x3(&normal);
    transpose(&normal);
    invert(&normal);

    /* camera position */
    float origin[4] = {0, 0, 0, 1};
    float tmp[4];
    struct mat4 view_inverse = view;
    invert(&view_inverse);
    vec4_matmul(tmp, &view_inverse, origin);
    memcpy(g_uniform.cam_pos, tmp, 3 * sizeof(float));

    /* send down the pipeline */
    gl_render(&g_pipe, indices, n_indices, prim_type);
}

/***************************************************************
 *                                                             *
 *               pipeline and uniform bindings                 *
 *                                                             *
 ***************************************************************/

/***************
 * gl_bind_pts *
 ***************/

/* sets points */
extern void
gl_bind_vertices(float *pts, int n_pts, int n_attr)
{
    g_pipe.pts_in    = pts;
    g_pipe.n_pts     = n_pts;
    g_pipe.n_attr_in = n_attr;
}

/***********************
 * gl_bind_framebuffer *
 ***********************/

/* attaches color and depth buffers to global framebuffer */
extern void
gl_bind_framebuffer(int width, int height, uint32_t *colors, float *depths)
{
    g_fbuf.width  = width;
    g_fbuf.height = height;
    g_fbuf.colors = colors;
    g_fbuf.depths = depths;
}

/*******************
 * gl_bind_uniform *
 *******************/

/* loads a custom uniform */
extern void
gl_bind_uniform(void *uniform)
{
    g_pipe.uniform = uniform;
}

/**********************
 * gl_restore_uniform *
 **********************/

/* loads default uniform */
extern void
gl_restore_uniform()
{
    g_pipe.uniform = &g_uniform;
}

/**************
 * gl_bind_custom_vs *
 **************/

/* sets the vertex shader */
extern void
gl_bind_custom_vs(vs_f vs, int n_attr_out)
{
    g_pipe.vs = vs;
    g_pipe.n_attr_out = n_attr_out;
}

/**************
 * gl_bind_custom_fs *
 **************/

/* sets the fragment shader */
extern void
gl_bind_custom_fs(fs_f fs)
{
    g_pipe.fs = fs;
}

/*******************
 * gl_bind_texture *
 *******************/

/* binds a texture to pipeline */
extern void
gl_bind_texture(uint32_t *colors, int width, int height)
{
    g_uniform.has_texture = 1;
    g_texture.colors      = colors;
    g_texture.width       = width;
    g_texture.height      = height;
}

/***************************************************************
 *                                                             *
 *                          light slot                         *
 *                                                             *
 ***************************************************************/

/***************
 * split_light *
 ***************/

/* TODO: bounds check and add error handling */

static int
split_light(enum gl_light_slot slot)
{
    return (int)slot;
} 

/************
 * gl_light_slot *
 ************/

/* binds a light to pipeline */

extern void 
gl_light(enum gl_light_slot slot, enum gl_light_attr attr, float *data)
{
    /* split attribute data */
    switch(attr) {
        case GL_POSITION:
            memcpy(g_lights[slot].pos, data, 3 * sizeof(float));
            break;
        case GL_DIRECTION:
            memcpy(g_lights[slot].dir, data, 3 * sizeof(float));
            break;
        case GL_COLOR:
            memcpy(g_lights[slot].color, data, 4 * sizeof(float));
            break;
        case GL_SPOT_ANGLE:
            g_lights[slot].spot_angle = *data;
            break;
        case GL_SPOT_PENUMBRA:
            g_lights[slot].spot_penumbra = *data;
            break;
        case GL_CONSTANT_ATTENUATION:
            g_lights[slot].attn_const = *data;
            break;
        case GL_LINEAR_ATTENUATION:
            g_lights[slot].attn_lin = *data;
            break;
        case GL_QUADRATIC_ATTENUATION:
            g_lights[slot].attn_quad = *data;
            break;
        default:
            return;
    }
}

/*************
 * gl_glight *
 *************/

/* binds global light data to uniform */
extern void 
gl_glight(enum gl_light_attr attr, float *data)
{
    /* split attribute data */
    switch(attr) {
        case GL_AMBIENT:
            g_uniform.ka = *data;
            break;
        case GL_DIFFUSE:
            g_uniform.kd = *data;
            break;
        case GL_SPECULAR:
            g_uniform.ks = *data;
            break;
        default:
            return;
    }
}

/*****************
 * gl_light_type *
 *****************/

/* binds light type to slot */

extern void 
gl_light_type(enum gl_light_slot slot, enum gl_light_type type)
{
    int idx;
    
    idx = split_light(slot);
    switch (type) {
        case GL_DIRECTIONAL:
            g_lights[idx].type = 1 << 0;
            break;
        case GL_POINT:
            g_lights[idx].type = 1 << 1;
            break;
        case GL_SPOT:
            g_lights[idx].type = 1 << 2;
            break;
    }
}

/*******************
 * gl_light_enable *
 *******************/

/* enables light at specified slot */

extern void 
gl_light_enable(enum gl_light_slot slot)
{
    g_uniform.light_state |= 1 << slot;
}

/********************
 * gl_light_disable *
 ********************/

/* disables light at specified slot */
extern void 
gl_light_disable(enum gl_light_slot slot)
{
    g_uniform.light_state &= ~(1 << slot);
}

/***************
 * gl_material *
 ***************/

/* binds a material to pipeline */
extern void
gl_material(enum gl_light_attr attr, float *data)
{
    switch(attr) {
        case GL_AMBIENT:
            memcpy(g_material.ambient, data, 4 * sizeof(float));
            break;
       case GL_DIFFUSE:
            memcpy(g_material.diffuse, data, 4 * sizeof(float));
            break;
        case GL_SPECULAR:
            memcpy(g_material.specular, data, 4 * sizeof(float));
            break;
        case GL_BLEND:
            g_material.blend = *data;
            break;
        case GL_SHININESS:
            g_material.shininess = *data;
            break;
        default:
            return;
    }
}

/***************************************************************
 *                                                             *
 *                    matrix stack operations                  *
 *                                                             *
 ***************************************************************/

/******************
 * gl_matrix_mode *
 ******************/

/* sets current matrix stack */
extern void
gl_matrix_mode(enum gl_matrix_mode mode)
{
    switch (mode) {
        case GL_MODEL_MATRIX:
            cur_mat = &model;
            break;
        case GL_VIEW_MATRIX:
            cur_mat = &view;
            break;
        case GL_PROJECTION_MATRIX:
            cur_mat = &proj;
            break;
        case GL_MVP_MATRIX:
            cur_mat = &mvp;
            break;
    }
}

/***************
 * gl_dump_mat *
 ***************/

/* dumps contents of current matrix to given buffer in row major order */
extern void
gl_dump_matrix(float *dest)
{
    dest[0]  = cur_mat->e00;
    dest[1]  = cur_mat->e01;
    dest[2]  = cur_mat->e02;
    dest[3]  = cur_mat->e03;
    dest[4]  = cur_mat->e10;
    dest[5]  = cur_mat->e11;
    dest[6]  = cur_mat->e12;
    dest[7]  = cur_mat->e13;
    dest[8]  = cur_mat->e20;
    dest[9]  = cur_mat->e21;
    dest[10] = cur_mat->e22;
    dest[11] = cur_mat->e23;
    dest[12] = cur_mat->e30;
    dest[13] = cur_mat->e31;
    dest[14] = cur_mat->e32;
    dest[15] = cur_mat->e33;
}

/******************
 * gl_load_matrix *
 ******************/

/* loads the entries in a 16 length row major float array to current matrix */
extern void
gl_load_matrix(float *glc)
{
    cur_mat->e00 = glc[0];
    cur_mat->e01 = glc[1];
    cur_mat->e02 = glc[2];
    cur_mat->e03 = glc[3];
    cur_mat->e10 = glc[4];
    cur_mat->e11 = glc[5];
    cur_mat->e12 = glc[6];
    cur_mat->e13 = glc[7];
    cur_mat->e20 = glc[8];
    cur_mat->e21 = glc[9];
    cur_mat->e22 = glc[10];
    cur_mat->e23 = glc[11];
    cur_mat->e30 = glc[12];
    cur_mat->e31 = glc[13];
    cur_mat->e32 = glc[14];
    cur_mat->e33 = glc[15];
}

/********************
 * gl_load_identity *
 ********************/

/* sets current matrix to the identity */
extern void
gl_load_identity()
{
   *cur_mat = identity;
}

/***************************************************************
 *                                                             *
 *                            model                            *
 *                                                             *
 ***************************************************************/

/****************
 * gl_translate *
 ****************/

/* pushes an affine transformation with translation of x, y, z */
extern void
gl_translate(float x, float y, float z)
{
    struct mat4 t = {
        1, 0, 0, x,
        0, 1, 0, y,
        0, 0, 1, z,
        0, 0, 0, 1,
    };

    matmul(cur_mat, &t);
}

/***************
 * gl_rotate_x *
 ***************/

/* pushes a matrix rotating about the x axis by t radians */
extern void
gl_rotate_x(float t)
{
    float c = cos(t);
    float s = sin(t);

    struct mat4 x = {
        1,  0,  0,  0,
        0,  c,  -s, 0,
        0,  s,  c,  0,
        0,  0,  0,  1,
    };

    matmul(cur_mat, &x);
}

/***************
 * gl_rotate_y *
 ***************/

/* pushes a matrix rotating about the y axis by t radians */

void
gl_rotate_y(float t)
{
    float c = cos(t);
    float s = sin(t);

    struct mat4 y = {
        c,  0,  s,  0,
        0,  1,  0,  0,
        -s, 0,  c,  0,
        0,  0,  0,  1,
    };

    matmul(cur_mat, &y);
}

/***************
 * gl_rotate_z *
 ***************/

/* pushes a matrix rotating about the z axis by t radians */

void
gl_rotate_z(float t)
{
    float c = cos(t);
    float s = sin(t);
    
    struct mat4 z = {
        c,  -s, 0,  0,
        s,  c,  0,  0,
        0,  0,  1,  0,
        0,  0,  0,  1,
    };

    matmul(cur_mat, &z);
}

/************
 * gl_scale *
 ************/

/* pushes a scale matrix by sx, sy, sz */
void
gl_scale(float sx, float sy, float sz)
{
    struct mat4 s = {
        sx, 0,  0,  0,
        0,  sy, 0,  0,
        0,  0,  sz, 0,
        0,  0,  0,  1,
    };

    matmul(cur_mat, &s);
}

/***************************************************************
 *                                                             *
 *                             view                            *
 *                                                             *
 ***************************************************************/

/**************
 * gl_look_at *
 **************/

/** 
 * constructs a view matrix from three vectors :
 * the eye vector, camera position in world space
 * the target vector, the position of view target in world space
 * the up vector, pointing to whats generally above the camera
 */

void
gl_look_at(float ex, float ey, float ez, 
           float lx, float ly, float lz, 
           float ux, float uy, float uz)
{
    /* eye vector */
    float eye[3] = {
        ex, ey, ez,
    };

    /* look vector */
    float look[3] = {
        lx, ly, lz,
    };

    /* up vector */
    float up[3] = {
        ux, uy, uz,
    };

    /* backward vector, w */
    float w[3];
    vec3_sub(w, eye, look);
    normalize(w);

    /* side vector, u */
    float u[3];
    cross(u, up, w);   
    normalize(u);

    /* new up vector, v */
    float v[3];
    float up_proj_w[3]; /* up projected onto w */
    vec3_scale(up_proj_w, w, dot(up, w));
    vec3_sub(v, up, up_proj_w);
    normalize(v);
    
    struct mat4 m = {
        u[0], u[1], u[2], 0,
        v[0], v[1], v[2], 0,
        w[0], w[1], w[2], 0,
        0,    0,    0,    1,
    };

    matmul(cur_mat, &m);
    gl_translate(-ex, -ey, -ez);
}

/***************************************************************
 *                                                             *
 *                          projection                         *
 *                                                             *
 ***************************************************************/

/******************
 * gl_perspective *
 ******************/

/* pushes a perspective matrix specified by fov */

void
gl_perspective(float fovy, float aspect, float near, float far)
{
    float f, e22, e23, a;

    f   = 1                / (tan(fovy / 2));
    e22 = (far + near)     / (near - far);
    e23 = (2 * far * near) / (near - far);
    a   = aspect;

    struct mat4 p = {
        f/a, 0,   0,    0,
        0,   f,   0,    0,
        0,   0,   e22,  e23,
        0,   0,   -1,   0,
    };

    matmul(cur_mat, &p);
}

/**************
 * gl_frustum *
 **************/

/* pushes a projection matrix based on frustum */

void
gl_frustum(float left, float right, float bottom, 
           float top, float near, float far)
{
    float e00, e11, e02, e12, e22, e23;

    e00 = (2 * near)        / (right - left);
    e11 = (2 * near)        / (top - bottom);
    e02 = (right + left)    / (right - left);
    e12 = (top + bottom)    / (top - bottom);
    e22 = -(far + near)     / (near - far);
    e23 = -(2 * far * near) / (near - far);

    struct mat4 p = {
        e00, 0,   e02,  0,
        0,   e11, e12,  0,
        0,   0,   e22,  e23,
        0,   0,   -1,   0,
    };

    matmul(cur_mat, &p);
}

/***************************************************************
 *                                                             *
 *                      color & blending                       *
 *                                                             *
 ***************************************************************/

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

/***************************************************************
 *                                                             *
 *                        shader helpers                       *
 *                                                             *
 ***************************************************************/

/**************
 * clip_space *
 **************/

/**
 * sends the vector 'in' to clip space by applying an mvp 
 * sets the first four indices of 'out' to these coordinates
 */

static void
clip_space(float *out, float *in, struct uniform *uniform)
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
world_space(float *out, float *in, struct uniform *uniform)
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
world_space_normal(float *out, float *in, struct uniform *uniform)
{
    /* homogenize vector */
    float tmp_in[4] = { in[0], in[1], in[2], 1 };
    float tmp_out[4];
    vec4_matmul(tmp_out, uniform->normal, tmp_in);
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
sample_texture(struct texture *texture, float *color, float u, float v)
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
    struct uniform *uniform)
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

    for (int i = 0; i < GL_MAX_LIGHT_COUNT; i++) {
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

/***************************************************************
 *                                                             *
 *                            color                            *
 *                                                             *
 ***************************************************************/

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

/***************************************************************
 *                                                             *
 *                           texture                           *
 *                                                             *
 ***************************************************************/

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
    struct uniform *std_uniform;
    std_uniform = (struct uniform *)uniform;
    float color[4];
    sample_texture(std_uniform->texture, color, in[4], in[5]); 
    *out = rgb_int(color);  /* frag color */
}

/***************************************************************
 *                                                             *
 *                           phong                             *
 *                                                             *
 ***************************************************************/

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
    struct uniform *std_uniform;
    
    std_uniform = (struct uniform *)uniform;

    /* x y z w */
    clip_space(out, in, std_uniform);  /* position */

    /* wx, wy, wz */
    world_space(out + 4, in, std_uniform);

    /* u v */
    memcpy(out + 7, in + 3, 2 * sizeof(float));

    /* nx ny nz */
    world_space_normal(out + 9, in + 5, std_uniform);

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
    struct uniform *std_uniform;
    std_uniform = (struct uniform *)uniform;

    float color[4];
    normalize(in + 9);
    phong(color, in + 4, in + 7, in + 9, std_uniform);
    *out = rgb_int(color);
}

/***************************************************************
 *                                                             *
 *                         bindings                            *
 *                                                             *
 ***************************************************************/

/*********
 * color *
 *********/

void
gl_bind_color_vs()
{
    gl_bind_custom_vs(color_vs, 7);
}

void
gl_bind_color_fs()
{
    gl_bind_custom_fs(color_fs);
}

/***********
 * texture *
 ***********/

void
gl_bind_texture_vs()
{
    gl_bind_custom_vs(texture_vs, 6);
}

void
gl_bind_texture_fs()
{
    gl_bind_custom_fs(texture_fs);
}

/*********
 * phong *
 *********/

void
gl_bind_std_vs()
{
    gl_bind_custom_vs(std_vs, 12);
}

void
gl_bind_phong_fs()
{
    gl_bind_custom_fs(phong_fs);
}