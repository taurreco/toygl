#ifndef GL_H
#define GL_H

#include <stdint.h>
#include <stddef.h>

#define GL_MAX_ATTRIBUTE_COUNT 32
#define GL_MAX_LIGHT_COUNT 8
#define GL_WINDING_ORDER_CCW 1
#define GL_WINDING_ORDER_CW -1

enum gl_clip_plane {
    GL_CLIP_LEFT_PLANE = 1 << 0,
    GL_CLIP_BOTTOM_PLANE = 1 << 1,
    GL_CLIP_NEAR_PLANE = 1 << 2,
    GL_CLIP_RIGHT_PLANE = 1 << 3,
    GL_CLIP_TOP_PLANE = 1 << 4
};

enum gl_primitive {
    GL_POINT_LIST,
    GL_LINE_LIST,
    GL_LINE_STRIP,
    GL_TRIANGLE_LIST,
    GL_TRIANGLE_STRIP
};

enum gl_matrix_mode {
    GL_MODEL_MATRIX,
    GL_VIEW_MATRIX,
    GL_PROJECTION_MATRIX,
    GL_MVP_MATRIX
};

enum gl_light {
    GL_LIGHT_1 = 0,
    GL_LIGHT_2 = 1,
    GL_LIGHT_3 = 2,
    GL_LIGHT_4 = 3,
    GL_LIGHT_5 = 4,
    GL_LIGHT_6 = 5,
    GL_LIGHT_7 = 6,
    GL_LIGHT_8 = 7,
};

enum gl_light_attr {
    GL_TYPE,
    GL_POSITION,
    GL_COLOR,
    GL_AMBIENT,
    GL_DIFFUSE,
    GL_SPECULAR,
    GL_BLEND,
    GL_SHININESS,
    GL_DIRECTION,
    GL_SPOT_ANGLE,
    GL_SPOT_PENUMBRA,
    GL_CONSTANT_ATTENUATION,
    GL_LINEAR_ATTENUATION,
    GL_QUADRATIC_ATTENUATION
};

enum gl_light_type {
    GL_DIRECTIONAL,
    GL_POINT,
    GL_SPOT
};

typedef void (*fs_f)(uint32_t *out, float *in, void *uniform);

struct gl_framebuffer {
    uint32_t *colors;
    float *depths;           
    int width; 
    int height;    
};

struct gl_pipeline {
    struct gl_framebuffer *fbuf;
    void *uniform;        
    vs_f vs;
    fs_f fs;
    float *pts_in;
    int n_pts;
    int n_attr_in;
    int n_attr_out;
    int winding;
};

void gl_bind_vertices(float *pts, int n_pts, int n_attr);
void gl_bind_framebuffer(int width, int height, uint32_t *colors, float *depths);
void gl_bind_uniform(void *uniform);
void gl_restore_uniform();
void gl_bind_texture(uint32_t *colors, int width, int height);
void gl_bind_base_color(float r, float g, float b);
void gl_renderl(int *indices, int n_indices, enum gl_primitive prim_type);
void gl_render(struct gl_pipeline *pipe, int *indices, int n_indices, enum gl_primitive prim_type);

void gl_light(enum gl_light slot, enum gl_light_attr attr, float *data);
void gl_glight(enum gl_light_attr attr, float *data);
void gl_light_type(enum gl_light slot, enum gl_light_type type);
void gl_light_enable(enum gl_light slot);
void gl_light_disable(enum gl_light slot);
void gl_material(enum gl_light_attr attr, float *data);

void gl_matrix_mode(enum gl_matrix_mode mode);
void gl_dump_matrix(float *dest);
void gl_load_matrix(float *glc);
void gl_load_identity();
void gl_perspective(float fovy, float aspect, float near, float far);
void gl_frustum(float left, float right, float bottom, float top, float near, float far);
void gl_translate(float x, float y, float z);
void gl_rotate_x(float t);
void gl_rotate_y(float t);
void gl_rotate_z(float t);
void gl_scale(float sx, float sy, float sz);
void gl_look_at(float ex, float ey, float ez, float cx, float cy, float cz, float ux, float uy, float uz);

void gl_bind_custom_vs(vs_f vs, int n_attr_out);
void gl_bind_custom_fs(fs_f fs);
void gl_bind_color_vs();
void gl_bind_color_fs();
void gl_bind_texture_vs();
void gl_bind_texture_fs();
void gl_bind_std_vs();
void gl_bind_phong_fs();

#endif    /* GL_H */