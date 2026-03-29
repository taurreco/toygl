struct raster {
    struct sr_framebuffer *fbuf;
    void* uniform;
    fs_f fs;
    int n_attr;
    int winding;
};

void draw_pt(struct raster *rast, float *pt);
void draw_ln(struct raster *rast, float *v0, float *v1);
void draw_tr(struct raster *rast, float *v0, float* v1, float *v2);
