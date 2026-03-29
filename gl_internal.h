struct texture {
    uint32_t *data;
    int n_pixels;
};

struct light {
    enum sr_light_type type;
    float pos[3];
    float color[4];
    float dir[3];
    float spot_angle;
    float spot_penumbra;
    float attn_const;
    float attn_lin;
    float attn_quad;
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
    uint8_t light_state;
    struct light *lights;
    float ka;
    float kd;
    float ks;
};