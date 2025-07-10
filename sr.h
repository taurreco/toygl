
#ifndef SR_H
#define SR_H

#include <stdint.h>
#include <stddef.h>

#define SR_MAX_ATTRIBUTE_COUNT 32
#define SR_MAX_LIGHT_COUNT 8

#define SR_WINDING_ORDER_CCW 1
#define SR_WINDING_ORDER_CW -1

enum sr_clip_plane {
    SR_CLIP_LEFT_PLANE = 1 << 0,
    SR_CLIP_BOTTOM_PLANE = 1 << 1,
    SR_CLIP_NEAR_PLANE = 1 << 2,
    SR_CLIP_RIGHT_PLANE = 1 << 3,
    SR_CLIP_TOP_PLANE = 1 << 4
};

enum sr_primitive {
    SR_POINT_LIST,
    SR_LINE_LIST,
    SR_LINE_STRIP,
    SR_TRIANGLE_LIST,
    SR_TRIANGLE_STRIP
};

enum sr_matrix_mode {
    SR_MODEL_MATRIX,
    SR_VIEW_MATRIX,
    SR_PROJECTION_MATRIX,
    SR_MVP_MATRIX
};

enum sr_light {
    SR_LIGHT_1 = 0,
    SR_LIGHT_2 = 1,
    SR_LIGHT_3 = 2,
    SR_LIGHT_4 = 3,
    SR_LIGHT_5 = 4,
    SR_LIGHT_6 = 5,
    SR_LIGHT_7 = 6,
    SR_LIGHT_8 = 7,
};

enum sr_light_attr {
    SR_TYPE,
    SR_POSITION,
    SR_COLOR,
    SR_AMBIENT,
    SR_DIFFUSE,
    SR_SPECULAR,
    SR_BLEND,
    SR_SHININESS,
    SR_DIRECTION,
    SR_SPOT_ANGLE,
    SR_SPOT_PENUMBRA,
    SR_CONSTANT_ATTENUATION,
    SR_LINEAR_ATTENUATION,
    SR_QUADRATIC_ATTENUATION
};

enum sr_light_type {
    SR_DIRECTIONAL,
    SR_POINT,
    SR_SPOT
};

/*********************************************************************
 *                                                                   *
 *                     shader function pointers                      *
 *                                                                   *
 *********************************************************************/

typedef void (*fs_f)(uint32_t* out, float* in, void* uniform);

/******************
 * sr_framebuffer *
 ******************/

/* interface to whatever writes to the screen */

struct sr_framebuffer {
    uint32_t* colors;
    float* depths;           
    int width; 
    int height;    
};

/***************
 * sr_pipeline *
 ***************/

/**
 * stores all data required to pre-allocate
 * memory required to render an indexed list of
 * arbitrary length
 */

struct sr_pipeline {
    struct sr_framebuffer* fbuf;
    void* uniform;        
    vs_f vs;
    fs_f fs;
    float* pts_in;
    int n_pts;
    int n_attr_in;
    int n_attr_out;
    int winding;
};

/*********************************************************************
 *                                                                   *
 *                           render pipeline                         *
 *                                                                   *
 *********************************************************************/

/* render interface */

void sr_bind_vertices(float* pts, int n_pts, int n_attr);
void sr_bind_framebuffer(int width, int height, uint32_t* colors, float* depths);
void sr_bind_uniform(void* uniform);
void sr_restore_uniform();
void sr_bind_texture(uint32_t* colors, int width, int height);
void sr_bind_base_color(float r, float g, float b);
void sr_renderl(int* indices, int n_indices, enum sr_primitive prim_type);
void sr_render(struct sr_pipeline* pipe, int* indices, 
               int n_indices, enum sr_primitive prim_type);

/*********************************************************************
 *                                                                   *
 *                         light interface                           *
 *                                                                   *
 *********************************************************************/

void sr_light(enum sr_light slot, enum sr_light_attr attr, float* data);
void sr_glight(enum sr_light_attr attr, float* data);
void sr_light_type(enum sr_light slot, enum sr_light_type type);
void sr_light_enable(enum sr_light slot);
void sr_light_disable(enum sr_light slot);
void sr_material(enum sr_light_attr attr, float* data);

/*********************************************************************
 *                                                                   *
 *                      matrix stack interface                       *
 *                                                                   *
 *********************************************************************/

void sr_matrix_mode(enum sr_matrix_mode mode);
void sr_dump_matrix(float* dest);
void sr_load_matrix(float* src);
void sr_load_identity();
void sr_perspective(float fovy, float aspect, float near, float far);
void sr_frustum(float left, float right, float bottom, 
                float top, float near, float far);
void sr_translate(float x, float y, float z);
void sr_rotate_x(float t);
void sr_rotate_y(float t);
void sr_rotate_z(float t);
void sr_scale(float sx, float sy, float sz);
void sr_look_at(float ex, float ey, float ez, 
                float cx, float cy, float cz, 
                float ux, float uy, float uz);

/*********************************************************************
 *                                                                   *
 *                     prebuilt shader bindings                      *
 *                                                                   *
 *********************************************************************/

void sr_bind_custom_vs(vs_f vs, int n_attr_out);
void sr_bind_custom_fs(fs_f fs);
void sr_bind_color_vs();
void sr_bind_color_fs();
void sr_bind_texture_vs();
void sr_bind_texture_fs();
void sr_bind_std_vs();
void sr_bind_phong_fs();

#endif /* SR_H */
