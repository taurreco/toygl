
#include <stdio.h>
#include "driver.h"
#include <stdlib.h>
#include "sr.h"

/**
 * bunny.c
 * --------
 * this is the minimal code needed to load an .obj
 * file and render it with a single directional light
 * 
 */

struct sr_obj* obj;

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

    float material_ambient[4] = {1, 0.5, 0.5, 0.5};
    float material_diffuse[4] = {1, 1, 1, 1};
    float material_specular[4] = {1, 1, 1, 1};
    float material_shininess = 50;

    float ka = 0.5;
    float kd = 0.5;
    float ks = 0.5;

    /* light bindings */

    sr_glight(SR_AMBIENT, &ka);
    sr_glight(SR_DIFFUSE, &kd);
    sr_glight(SR_SPECULAR, &ks);

    sr_light_type(SR_LIGHT_1, SR_DIRECTIONAL);
    sr_light(SR_LIGHT_1, SR_DIRECTION, light_dir);
    sr_light(SR_LIGHT_1, SR_COLOR, light_color);
    sr_light(SR_LIGHT_1, SR_CONSTANT_ATTENUATION, &light_attn_const);
    sr_light(SR_LIGHT_1, SR_LINEAR_ATTENUATION, &light_attn_lin);
    sr_light(SR_LIGHT_1, SR_QUADRATIC_ATTENUATION, &light_attn_quad);
    sr_light_enable(SR_LIGHT_1);


    /* material bindings */

    sr_material(SR_AMBIENT, material_ambient);
    sr_material(SR_DIFFUSE, material_diffuse);
    sr_material(SR_SPECULAR, material_specular);
    sr_material(SR_SHININESS, &material_shininess);

    /* load model and texture into RAM */

    obj = sr_load_obj("./assets/bunny.obj");

    /* prepare gl context */

    sr_bind_pts(obj->pts, obj->n_pts, obj->n_attr);
    sr_bind_framebuffer(SCREEN_WIDTH, SCREEN_HEIGHT, colors, depths);
    sr_bind_std_vs();
    sr_bind_phong_fs();
    sr_matrix_mode(SR_PROJECTION_MATRIX);
    sr_perspective(1, (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT, 2, 1000);
    sr_matrix_mode(SR_VIEW_MATRIX);
    sr_look_at(0, 1, 3, 0, 0, 0, 0, 1, 0);
    sr_matrix_mode(SR_MODEL_MATRIX);
    sr_scale(0.3, 0.3, 0.3);
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
    sr_renderl(obj->indices, obj->n_indices, SR_TRIANGLE_LIST);
    sr_rotate_y(dt);
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
    sr_obj_free(obj);
}