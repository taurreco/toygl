struct mat4 {
    float e00, e01, e02, e03;
    float e10, e11, e12, e13;
    float e20, e21, e22, e23;
    float e30, e31, e32, e33;
};

void matmul(struct mat4 *a, struct mat4 *b);
int invert(struct mat4 *a);
void transpose(struct mat4 *a);
void upper_3x3(struct mat4 *a);
void vec4_matmul(float *a, struct mat4 *b, float *c);
void vec4_mul(float *a, float *b, float *c);
void vec4_add(float *a, float *b, float *c);
void vec4_scale(float *a, float *b, float c);
void lerp(float *a, float *b, float *c, float alpha);
void vec3_sub(float *a, float *b, float *c);
void vec3_add(float *a, float *b, float *c);
void vec3_scale(float *a, float *b, float c);
void reflect(float *r, float *n, float *v);
float dot(float *a, float *b);
void cross(float *a, float *b, float *c);
float magnitude(float *a);
void normalize(float *a);
float radians(float deg);