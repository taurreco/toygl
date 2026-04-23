/***************
 * struct edge *
 ***************/

/* holds the steps sizes / data for barycentric weight for an edge */

struct edge {
    float step_x;      /* steps to increment to get the det at new pt */
    float step_y;    
    int   is_tl;       /* tracks if the edge is top left */
};

/***************
 * struct bbox *
 ***************/

/* holds the upper and lower bounds of the triangle */

struct bbox {
    float min_x; 
    float min_y;
    float max_x;
    float max_y;
};

struct raster {
    struct sr_framebuffer *fbuf;
    void* uniform;
    fs_f fs;
    int n_attr;
    int winding;
};

void draw_pt(struct raster *rast, float *pt);
void draw_ln(struct raster *rast, float *v0, float *v1);
void draw_tr(struct raster *rast, float *v0, float *v1, float *v2);
