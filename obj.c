#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "gl.h"
#include "mat.h"
#include "map.h"

#define VT 1 << 0
#define VN 1 << 1

#define MAX_LINE_SIZE 1000

/*********************************************************************
 *                                                                   *
 *                              structs                              *
 *                                                                   *
 *********************************************************************/

struct obj_desc {                  
    int n_v; 
    int n_vt; 
    int n_vn;
    int n_pts;
    int n_tr;
};

/*********************************************************************
 *                                                                   *
 *                   (private) obj parsing helpers                   *
 *                                                                   *
 *********************************************************************/

/************
 * strtok_r *
 ************/

/* implementation of strtok_r for non POSIX compliant C compilers */
char*
strtok_r(char *str, const char *delim, char **nextp)
{
    char *ret;

    if (str == NULL)
        str = *nextp;
    str += strspn(str, delim);

    if (*str == '\0')
        return NULL;
    ret = str;
    str += strcspn(str, delim);

    if (*str)
        *str++ = '\0';
    *nextp = str;
    return ret;
}

/***************
 * push_stream *
 ***************/

/**
 * takes a stream of whitespace seperated strings, converts 
 * them to floats, and copies it to the dest buffer
 */
static void
push_stream(float* dest, char* stream, int n_str) 
{
    char* start = stream;
    char* end;
    for (int i = 0; i < n_str; i++) {
        float val = strtof(start, &end);
        dest[i] = val;
        char* tmp = start;
        start = end;
        end = tmp;
    }
}

/************
 * to_index *
 ************/

/* converts number to index in respective buffer */
static int
to_index(int buf_len, int num) {
    if (num > 0)
        return num - 1;
    return buf_len + num;
}

/*****************
 * split_indices *
 *****************/

/* converts raw obj point indices into readable indices */
static void
split_indices(struct obj_desc* obj_desc, int* indices, char* raw, int8_t flags)
{
    char* save = "";
    
    int tmp = atoi(strtok_r(raw, "/", &save));
    indices[0] = to_index(obj_desc->n_v, tmp);
    
    if (flags & VT) {
        tmp  = atoi(strtok_r(NULL, "/", &save));
        indices[1] = to_index(obj_desc->n_vt, tmp);
    }

    if (flags & VN) {
        tmp  = atoi(strtok_r(NULL, "/", &save));
        indices[2] = to_index(obj_desc->n_vn, tmp);
    }
}

/**************
 * get_normal *
 **************/

/* calculates face normal from three points */
static void
get_normal(float* normal, float* v0, float* v1, float* v2) {
    float e10[3];
    float e20[3];
    vec3_sub(e10, v1, v0);
    vec3_sub(e20, v2, v0);
    cross(normal, e10, e20);
}

/*********************************************************************
 *                                                                   *
 *                  (private) obj memory & parsing                   *
 *                                                                   *
 *********************************************************************/

/**************
 * first_pass *
 **************/

/* reads over the file to find buffer sizes */
static void
first_pass(struct obj_desc* obj_desc, FILE* fp)
{
    char line[MAX_LINE_SIZE];
    char* token;
    
    while (fgets(line, MAX_LINE_SIZE, fp) != NULL) {

        char* r = strchr(line, '\r');
        if (r)
            *r = 0;
        token = strtok(line, "\n");
        token = strtok(line, " ");

        if (!token) continue;

        if (strcmp(token, "v") == 0)
            obj_desc->n_v++;

        if (strcmp(token, "vt") == 0)
            obj_desc->n_vt++;

        if (strcmp(token, "vn") == 0)
            obj_desc->n_vn++;

        if (strcmp(token, "f") == 0) {
            int i = 0;
            token = strtok(NULL, " ");
            while (token != NULL) {
                obj_desc->n_pts++;
                i++;
                token = strtok(NULL, " ");
            }
            obj_desc->n_tr += 1 + (i - 3);
        }
    }
    fseek(fp, 0, SEEK_SET);
}

/***************
 * second_pass *
 ***************/

/** 
 * moves data from file into its respective buffers, 
 * returns precise length of vertex buffer 'pts' 
 */
static int
second_pass(struct obj_desc* obj_desc, float* pts, int* indices, FILE* fp) 
{

    float* v = malloc(obj_desc->n_v * 3 * sizeof(float));
    float* vt = malloc(obj_desc->n_vt * 2 * sizeof(float));
    float* vn = malloc(obj_desc->n_vn * 3 * sizeof(float));

    int v_idx = 0;
    int vt_idx = 0;
    int vn_idx = 0;
    int tr_idx = 0;
    int n_pts = 0;

    char line[MAX_LINE_SIZE];
    char* token;

    int8_t flags = 0;

    struct hash_table* ht = hash_table_alloc();

    while (fgets(line, MAX_LINE_SIZE, fp) != NULL) {
        char* r = strchr(line, '\r');
        if (r)
            *r = 0;

        token = strtok(line, "\n");
        token = strtok(line, " ");

        if (!token) continue;

        if (strcmp(token, "v") == 0) {
            push_stream(v + v_idx * 3, strtok(NULL, "\n"), 3);
            v_idx++;
            continue;
        }
        if (strcmp(token, "vt") == 0) {
            flags |= VT;
            push_stream(vt + vt_idx * 2, strtok(NULL, "\n"), 2);
            vt_idx++;
            continue;
        }
        if (strcmp(token, "vn") == 0) {
            flags |= VN;
            push_stream(vn + vn_idx * 3, strtok(NULL, "\n"), 3);
            vn_idx++;
            continue;
        }
        if (strcmp(token, "f") == 0) {
            char* tokens[3];    /* the "v/vt/vn" form of triangle vertices */

            tokens[0] = strtok(NULL, " ");
            tokens[1] = strtok(NULL, " ");
            tokens[2] = strtok(NULL, " ");

            int token_indices[3];    /* stores indexes into v, vt, vn */
            char tmp[255];

            while (tokens[2] != NULL) {
                
                float normal[3];    /* stores face normal */
                float* v012[3];    /* stores locations of v0, v1, v2 */
                float* normals_dest[3] = {0, 0, 0};     /* stores where normal goes */

                for (int i = 0; i < 3; i++) {    /* per triangle */
                    float* cur = pts + n_pts * 8;    /* current point */

                    if (search(ht, tokens[i]) == -1) {    /* haven't seen this point */

                        strcpy(tmp, tokens[i]);
                        split_indices(obj_desc, token_indices, tmp, flags);  /* convert to indices */
                        
                        v012[i] = v + token_indices[0] * 3;
                        normals_dest[i] = cur + 5;
                        
                        /* fill pts buffer with the vertex attributes */
                        
                        memcpy(cur, v + token_indices[0] * 3, 
                               3 * sizeof(float));

                        if (flags & VT) {
                            memcpy(cur + 3, vt + token_indices[1] * 2, 
                                   2 * sizeof(float));
                        }

                        if (flags & VN) {
                            memcpy(cur + 5, vn + token_indices[2] * 3, 
                                   3 * sizeof(float));
                        }

                        insert(ht, tokens[i], n_pts);
                        n_pts++;
                    } else {
                        v012[i] = pts + search(ht, tokens[i]) * 8;  /* potential for horrible error */
                    }
                }

                if (!(flags & VN)) {    /* if no explicit normals, calculate new ones */
                    get_normal(normal, v012[0], v012[1], v012[2]);
                    for (int i = 0; i < 3; i++) {
                        if (normals_dest[i])
                            memcpy(normals_dest[i], normal, 3 * sizeof(float));
                    }
                }

                for (int i = 0; i < 3; i++)    /* index buffer fill */
                    indices[tr_idx * 3 + i] = search(ht, tokens[i]);

                tokens[1] = tokens[2];
                tokens[2] = strtok(NULL, " ");
                tr_idx++;
            }
        }
    }

    hash_table_free(ht);
    free(v);
    free(vt);
    free(vn);
    return n_pts;
}

/*********************************************************************
 *                                                                   *
 *                       (public) obj loading                        *
 *                                                                   *
 *********************************************************************/

/***************
 * sr_load_obj *
 ***************/

/* reads obj file data from path into an indexed triangle list */
extern struct gl_obj*
gl_load_obj(char* file)
{
    struct gl_obj* obj = malloc(sizeof(struct gl_obj));

    /* open file and pre allocate buffers in obj context */
    FILE* fp = fopen(file, "r");
    if (fp == NULL) {
        return 0;
    }

    struct obj_desc obj_desc;
    memset(&obj_desc, 0, sizeof(struct obj_desc));

    first_pass(&obj_desc, fp);

    int n_pts = obj_desc.n_pts;
    int n_indices = obj_desc.n_tr * 3;
    
    float* pts = malloc(n_pts * 8 * sizeof(float));
    int* indices = malloc(n_indices * sizeof(int));

    /* fill triangle list buffers */
    n_pts = second_pass(&obj_desc, pts, indices, fp);
    fclose(fp);

    float* tmp;
    if ((tmp = realloc(pts, n_pts * 8 * sizeof(float))))
        pts = tmp;

    /* fill return obj */

    obj->pts = pts;
    obj->indices = indices;
    obj->n_pts = n_pts;
    obj->n_attr = 8;
    obj->n_indices = n_indices;

    return obj;
}

/***************
 * gl_obj_free *
 ***************/

/* takes a heap allocated gl_obj struct and frees it and contents */
extern void
gl_destroy_obj(struct gl_obj* obj)
{
    free(obj->pts);
    free(obj->indices);
    free(obj);
}