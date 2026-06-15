
#include <stdio.h>
#include "driver.h"
#include <stdlib.h>
#include "gl.h"



struct gl_obj *obj;
struct gl_texture *texture;

/*********
 * start *
 *********/

/* runs only once in the begining */

extern void
start()
{

    /* light data */

    float light_dir[3] = {0, -1, 0};
    float light_color[4] = {1, 1, 1, 1};
    float light_attn_const = 0.8;
    float light_attn_lin = 0.2;
    float light_attn_quad = 0.2;

    float material_ambient[4] = {1, 0.5, 0, 0};
    float material_diffuse[4] = {1, 1, 1, 1};
    float material_specular[4] = {1, 1, 1, 1};
    float material_blend = 1;
    float material_shininess = 50;

    float ka = 0.5;
    float kd = 0.5;
    float ks = 0.5;

    uint32_t *texture_colors;
    int texture_width;
    int texture_height;

    /* light bindings */

    gl_glight(GL_AMBIENT, &ka);
    gl_glight(GL_DIFFUSE, &kd);
    gl_glight(GL_SPECULAR, &ks);

    gl_light_type(GL_LIGHT_1, GL_DIRECTIONAL);
    gl_light(GL_LIGHT_1, GL_DIRECTION, light_dir);
    gl_light(GL_LIGHT_1, GL_COLOR, light_color);
    gl_light(GL_LIGHT_1, GL_CONSTANT_ATTENUATION, &light_attn_const);
    gl_light(GL_LIGHT_1, GL_LINEAR_ATTENUATION, &light_attn_lin);
    gl_light(GL_LIGHT_1, GL_QUADRATIC_ATTENUATION, &light_attn_quad);
    gl_light_enable(GL_LIGHT_1);

    /* material bindings */

    gl_material(GL_AMBIENT, material_ambient);
    gl_material(GL_DIFFUSE, material_diffuse);
    gl_material(GL_SPECULAR, material_specular);
    gl_material(GL_BLEND, &material_blend);
    gl_material(GL_SHININESS, &material_shininess);

    /* load model and texture into RAM */

    obj = gl_load_obj("./examples/assets/link.obj");
    gl_load_tga("./examples/assets/link.tga", &texture_colors, &texture_width, &texture_height);


    /* prepare gl context */

    gl_bind_vertices(obj->pts, obj->n_pts, obj->n_attr);
    gl_bind_texture(texture_colors, texture_width, texture_height);

    gl_bind_framebuffer(SCREEN_WIDTH, SCREEN_HEIGHT, colors, depths);
    gl_bind_std_vs();
    gl_bind_phong_fs();

    gl_matrix_mode(GL_PROJECTION_MATRIX);
    gl_perspective(1, (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT, 2, 1000);
    gl_matrix_mode(GL_VIEW_MATRIX);
    gl_look_at(0, 3, 5, 0, 2, 0, 0, 1, 0);
    gl_matrix_mode(GL_MODEL_MATRIX);
    gl_scale(0.02, 0.02, 0.02);
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
    gl_renderl(obj->indices, obj->n_indices, GL_TRIANGLE_LIST);
    gl_rotate_y(dt);
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
end()
{
    gl_destroy_obj(obj);
}