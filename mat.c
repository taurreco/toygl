#include "mat.h"
#include <math.h>

/**********
 * matmul *
 **********/

/* multiplies two 4x4 matrices, 'a' and 'b', stores the result in 'a' */

void 
matmul(struct mat4* a, struct mat4* b)
{
    struct mat4 tmp;
    tmp.e00 = a->e00 * b->e00 + a->e01 * b->e10 + a->e02 * b->e20 + a->e03 * b->e30;
    tmp.e01 = a->e00 * b->e01 + a->e01 * b->e11 + a->e02 * b->e21 + a->e03 * b->e31; 
    tmp.e02 = a->e00 * b->e02 + a->e01 * b->e12 + a->e02 * b->e22 + a->e03 * b->e32; 
    tmp.e03 = a->e00 * b->e03 + a->e01 * b->e13 + a->e02 * b->e23 + a->e03 * b->e33; 
    tmp.e10 = a->e10 * b->e00 + a->e11 * b->e10 + a->e12 * b->e20 + a->e13 * b->e30; 
    tmp.e11 = a->e10 * b->e01 + a->e11 * b->e11 + a->e12 * b->e21 + a->e13 * b->e31; 
    tmp.e12 = a->e10 * b->e02 + a->e11 * b->e12 + a->e12 * b->e22 + a->e13 * b->e32; 
    tmp.e13 = a->e10 * b->e03 + a->e11 * b->e13 + a->e12 * b->e23 + a->e13 * b->e33; 
    tmp.e20 = a->e20 * b->e00 + a->e21 * b->e10 + a->e22 * b->e20 + a->e23 * b->e30; 
    tmp.e21 = a->e20 * b->e01 + a->e21 * b->e11 + a->e22 * b->e21 + a->e23 * b->e31; 
    tmp.e22 = a->e20 * b->e02 + a->e21 * b->e12 + a->e22 * b->e22 + a->e23 * b->e32; 
    tmp.e23 = a->e20 * b->e03 + a->e21 * b->e13 + a->e22 * b->e23 + a->e23 * b->e33; 
    tmp.e30 = a->e30 * b->e00 + a->e31 * b->e10 + a->e32 * b->e20 + a->e33 * b->e30; 
    tmp.e31 = a->e30 * b->e01 + a->e31 * b->e11 + a->e32 * b->e21 + a->e33 * b->e31; 
    tmp.e32 = a->e30 * b->e02 + a->e31 * b->e12 + a->e32 * b->e22 + a->e33 * b->e32; 
    tmp.e33 = a->e30 * b->e03 + a->e31 * b->e13 + a->e32 * b->e23 + a->e33 * b->e33;
    *a = tmp;
};

/**********
 * invert *
 **********/

/* inverts a 4x4 matrix, returns 0 if non invertible */

int
invert(struct mat4* a)
{
    struct mat4 tmp;

    tmp.e00 = a->e11 * a->e22 * a->e33 - 
              a->e11 * a->e23 * a->e32 - 
              a->e21 * a->e12 * a->e33 + 
              a->e21 * a->e13 * a->e32 +
              a->e31 * a->e12 * a->e23 - 
              a->e31 * a->e13 * a->e22;

    tmp.e10 = -a->e10 * a->e22 * a->e33 + 
               a->e10 * a->e23 * a->e32 + 
               a->e20 * a->e12 * a->e33 - 
               a->e20 * a->e13 * a->e32 - 
               a->e30 * a->e12 * a->e23 + 
               a->e30 * a->e13 * a->e22;

    tmp.e20 = a->e10 * a->e21 * a->e33 - 
              a->e10 * a->e23 * a->e31 - 
              a->e20 * a->e11 * a->e33 + 
              a->e20 * a->e13 * a->e31 + 
              a->e30 * a->e11 * a->e23 - 
              a->e30 * a->e13 * a->e21;

    tmp.e30 = -a->e10 * a->e21 * a->e32 + 
               a->e10 * a->e22 * a->e31 +
               a->e20 * a->e11 * a->e32 - 
               a->e20 * a->e12 * a->e31 - 
               a->e30 * a->e11 * a->e22 + 
               a->e30 * a->e12 * a->e21;

    tmp.e01 = -a->e01 * a->e22 * a->e33 + 
               a->e01 * a->e23 * a->e32 + 
               a->e21 * a->e02 * a->e33 - 
               a->e21 * a->e03 * a->e32 - 
               a->e31 * a->e02 * a->e23 + 
               a->e31 * a->e03 * a->e22;

    tmp.e11 = a->e00 * a->e22 * a->e33 - 
              a->e00 * a->e23 * a->e32 - 
              a->e20 * a->e02 * a->e33 + 
              a->e20 * a->e03 * a->e32 + 
              a->e30 * a->e02 * a->e23 - 
              a->e30 * a->e03 * a->e22;

    tmp.e21 = -a->e00 * a->e21 * a->e33 + 
               a->e00 * a->e23 * a->e31 + 
               a->e20 * a->e01 * a->e33 - 
               a->e20 * a->e03 * a->e31 - 
               a->e30 * a->e01 * a->e23 + 
               a->e30 * a->e03 * a->e21;

    tmp.e31 = a->e00 * a->e21 * a->e32 - 
              a->e00 * a->e22 * a->e31 - 
              a->e20 * a->e01 * a->e32 + 
              a->e20 * a->e02 * a->e31 + 
              a->e30 * a->e01 * a->e22 - 
              a->e30 * a->e02 * a->e21;

    tmp.e02 = a->e01 * a->e12 * a->e33 - 
              a->e01 * a->e13 * a->e32 - 
              a->e11 * a->e02 * a->e33 + 
              a->e11 * a->e03 * a->e32 + 
              a->e31 * a->e02 * a->e13 - 
              a->e31 * a->e03 * a->e12;

    tmp.e12 = -a->e00 * a->e12 * a->e33 + 
               a->e00 * a->e13 * a->e32 + 
               a->e10 * a->e02 * a->e33 - 
               a->e10 * a->e03 * a->e32 - 
               a->e30 * a->e02 * a->e13 + 
               a->e30 * a->e03 * a->e12;

    tmp.e22 = a->e00 * a->e11 * a->e33 - 
              a->e00 * a->e13 * a->e31 - 
              a->e10 * a->e01 * a->e33 + 
              a->e10 * a->e03 * a->e31 + 
              a->e30 * a->e01 * a->e13 - 
              a->e30 * a->e03 * a->e11;

    tmp.e32 = -a->e00 * a->e11 * a->e32 + 
               a->e00 * a->e12 * a->e31 + 
               a->e10 * a->e01 * a->e32 - 
               a->e10 * a->e02 * a->e31 - 
               a->e30 * a->e01 * a->e12 + 
               a->e30 * a->e02 * a->e11;

    tmp.e03 = -a->e01 * a->e12 * a->e23 + 
               a->e01 * a->e13 * a->e22 + 
               a->e11 * a->e02 * a->e23 - 
               a->e11 * a->e03 * a->e22 - 
               a->e21 * a->e02 * a->e13 + 
               a->e21 * a->e03 * a->e12;

    tmp.e13 = a->e00 * a->e12 * a->e23 - 
              a->e00 * a->e13 * a->e22 - 
              a->e10 * a->e02 * a->e23 + 
              a->e10 * a->e03 * a->e22 + 
              a->e20 * a->e02 * a->e13 - 
              a->e20 * a->e03 * a->e12;

    tmp.e23 = -a->e00 * a->e11 * a->e23 + 
               a->e00 * a->e13 * a->e21 + 
               a->e10 * a->e01 * a->e23 - 
               a->e10 * a->e03 * a->e21 - 
               a->e20 * a->e01 * a->e13 + 
               a->e20 * a->e03 * a->e11;

    tmp.e33 = a->e00 * a->e11 * a->e22 - 
              a->e00 * a->e12 * a->e21 - 
              a->e10 * a->e01 * a->e22 + 
              a->e10 * a->e02 * a->e21 + 
              a->e20 * a->e01 * a->e12 - 
              a->e20 * a->e02 * a->e11;

    float det = a->e00 * tmp.e00 + a->e01 * tmp.e10 + 
                a->e02 * tmp.e20 + a->e03 * tmp.e30;

    if (det == 0)
        return 0;

    det = 1.0 / det;

    tmp.e00 *= det;
    tmp.e01 *= det;
    tmp.e02 *= det;
    tmp.e03 *= det;
    tmp.e10 *= det;
    tmp.e11 *= det;
    tmp.e12 *= det;
    tmp.e13 *= det;
    tmp.e20 *= det;
    tmp.e21 *= det;
    tmp.e22 *= det;
    tmp.e23 *= det;
    tmp.e30 *= det;
    tmp.e31 *= det;
    tmp.e32 *= det;
    tmp.e33 *= det;

    *a = tmp;

    return 1;
}

/*************
 * transpose *
 *************/

/* transpose of a 4x4 matrix */

void
transpose(struct mat4 *a) {
    struct mat4 tmp;
    tmp.e00 = a->e00;
    tmp.e01 = a->e10;
    tmp.e02 = a->e20;
    tmp.e03 = a->e30;
    tmp.e10 = a->e01;
    tmp.e11 = a->e11;
    tmp.e12 = a->e21;
    tmp.e13 = a->e31;
    tmp.e20 = a->e02;
    tmp.e21 = a->e12;
    tmp.e22 = a->e22;
    tmp.e23 = a->e32;
    tmp.e30 = a->e03;
    tmp.e31 = a->e13;
    tmp.e32 = a->e23;
    tmp.e33 = a->e33;
    *a = tmp;
}

/*************
 * upper_3x3 *
 *************/

/* converts 4x4 matrix to its upper 3x3 matrix by filling 0s */

void
upper_3x3(struct mat4 *a) {
    a->e03 = 0;
    a->e13 = 0;
    a->e23 = 0;
    a->e30 = 0;
    a->e31 = 0;
    a->e32 = 0;
    a->e33 = 1;
}

/***************
 * vec4_matmul *
 ***************/

/* applys the matrix 'b' to vector 'c', stores result in vector 'a' */ 

void
vec4_matmul(float *a, struct mat4 *b, float *c)
{
    a[0] = c[0] * b->e00 + c[1] * b->e01 + c[2] * b->e02 + c[3] * b->e03;
    a[1] = c[0] * b->e10 + c[1] * b->e11 + c[2] * b->e12 + c[3] * b->e13;
    a[2] = c[0] * b->e20 + c[1] * b->e21 + c[2] * b->e22 + c[3] * b->e23;
    a[3] = c[0] * b->e30 + c[1] * b->e31 + c[2] * b->e32 + c[3] * b->e33;
}

/************
 * vec4_mul *
 ************/

/* multiplies vec4 'b' and 'c' componentwise, result in 'a' */

void
vec4_mul(float *a, float *b, float *c)
{
    a[0] = b[0] * c[0];
    a[1] = b[1] * c[1];
    a[2] = b[2] * c[2];
    a[3] = b[3] * c[3];
}

/************
 * vec4_add *
 ***********/

/* adds vec4 'b' to 'c' and stores result in 'a' */

void
vec4_add(float *a, float *b, float *c)
{
    a[0] = b[0] + c[0];
    a[1] = b[1] + c[1];
    a[2] = b[2] + c[2];
    a[3] = b[3] + c[3];
}

/**************
 * vec4_scale *
 **************/

/* multiplies vec4 'b' by scalar 'c' and stores result in 'a' */

void
vec4_scale(float *a, float *b, float c)
{
    a[0] = b[0] * c;
    a[1] = b[1] * c;
    a[2] = b[2] * c;
    a[3] = b[3] * c;
}

/*************
 * vec4_lerp *
 *************/

/* linear interpolation for vec4s, result in 'a' */

void
lerp(float *a, float *b, float *c, float alpha)
{
    a[0] = b[0] + (c[0] - b[0]) * alpha;
    a[1] = b[1] + (c[1] - b[1]) * alpha;
    a[2] = b[2] + (c[2] - b[2]) * alpha;
    a[3] = b[3] + (c[3] - b[3]) * alpha;
}

/************
 * vec3_sub *
 ************/

/* subtracts vec3 'c' from 'b' and stores result in 'a' */

void
vec3_sub(float *a, float *b, float *c)
{
    a[0] = b[0] - c[0];
    a[1] = b[1] - c[1];
    a[2] = b[2] - c[2];
}

/************
 * vec3_add *
 ***********/

/* adds vec3 'b' to 'c' and stores result in 'a' */

void
vec3_add(float *a, float *b, float *c)
{
    a[0] = b[0] + c[0];
    a[1] = b[1] + c[1];
    a[2] = b[2] + c[2];
}

/**************
 * vec3_scale *
 **************/

/* multiplies vec3 'b' by scalar 'c' and stores result in 'a' */

void
vec3_scale(float *a, float *b, float c)
{
    a[0] = b[0] * c;
    a[1] = b[1] * c;
    a[2] = b[2] * c;
}

/***********
 * reflect *
 ***********/

/* reflects a vec3 'l' across a normal 'n', result in 'r' */

void
reflect(float *r, float *l, float *n)
{
    float tmp[3];
    vec3_scale(tmp, n, 2 * dot(l, n));
    vec3_sub(r, tmp, l);
}

/*******
 * dot *
 *******/

/* dot product of two vec3s */

float
dot(float *a, float *b)
{
    return a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
}

/*********
 * cross *
 *********/

/* applys the cross product 'b' x 'c', stores result in 'a' */

void
cross(float *a, float *b, float *c)
{
    a[0] = (b[1] * c[2]) - (b[2] * c[1]);
    a[1] = (b[2] * c[0]) - (b[0] * c[2]);
    a[2] = (b[0] * c[1]) - (b[1] * c[0]);
}

/*************
 * magnitude *
 *************/

/* returns the magnitude of a vec3 */

float
magnitude(float *a)
{
    return sqrt(pow(a[0], 2) + pow(a[1], 2) + pow(a[2], 2));
}

/*************
 * normalize *
 *************/

/* normalizes vec3 in place */

void
normalize(float *a)
{
    float m = magnitude(a);
    a[0] = a[0] / m;
    a[1] = a[1] / m;
    a[2] = a[2] / m;
}

/***********
 * radians *
 ***********/

/* converts degrees to radians */

float 
radians(float deg) 
{
    return deg * (M_PI / 180);
}
