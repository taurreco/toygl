#ifndef OBJ_H
#define OBJ_H

struct obj {
    float *pts;
    int *indices;
    int n_pts;
    int n_indices;
    int n_attr;
};

struct obj *obj_load(char *file);
void obj_destroy(struct obj *obj);

#endif    /* OBJ_H */
