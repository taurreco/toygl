#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "obj.h"
#include "map.h"

#define LINE_SIZE 100


/********
 * desc *
 ********/

/* telemetry data about an obj file  */

struct desc {                  
    int n_v;        /* number of position attributes */
    int n_vt;       /* number of texture attributes */
    int n_vn;       /* number of normal attributes */
    int n_pts;      /* number of points */
    int n_tr;       /* number of triangle faces */
};

/***************************************************************
 *                                                             *
 *                    obj parsing helpers                      *
 *                                                             *
 ***************************************************************/

/***********
 * strtofs *
 ***********/

/* converts whitespace seperated string into list of floats */

void
strtofs(char* src, float* dst, int len) 
{
    char* start;
    char* end;

    start = src

    for (int i = 0; i < len; i++) {
	char* tmp;

        dst[i] = strtof(start, &end);
        tmp = start;
        start = end;
        end = tmp;
    }
}

/*********
 * index *
 *********/

/* converts number to index in respective buffer */

int
index(int len, int num) {
    if (num > 0) 
        return num - 1;
    return len + num;
}

/*********
 * slash *
 *********/

/**
 * converts raw obj point indices into readable indices 
 *
 *  src       dst
 * "1/2/3" -> [1, 2, 3]
 * "1//3"  -> [1, _, 3]
 */

void
slash(struct desc *desc, char *src, int *dst, int8_t flags)
{
    char* save;
    int tmp;
    
    tmp = atoi(strtok_r(src, "/", &save));
    dst[0] = index(desc->n_v, tmp);
    
    if (flags & VT) {
        tmp = atoi(strtok_r(0, "/", &save));
        dst[1] = index(desc->n_vt, tmp);
    }

    if (flags & VN) {
        tmp = atoi(strtok_r(0, "/", &save));
        dst[2] = index(desc->n_vn, tmp);
    }
}

/***************************************************************
 *                                                             *
 *                   obj memory & parsing                      *
 *                                                             *
 ***************************************************************/

/**************
 * first_pass *
 **************/

/* reads over the file to find buffer sizes */

void
first_pass(struct desc *desc, FILE *fp)
{
    char line[LINE_SIZE];
    char *tok;

    memset(desc, 0, sizeof(struct desc));
    
    while (fgets(line, LINE_SIZE, fp)) {
	
	char *r;
        
	/* this sequence removes windows style line endings */

	r = strchr(line, '\r');
        if (r) 
	    *r = 0;
        
	tok = strtok(line, "\n");
        tok = strtok(line, " ");

        if (tok == 0) 
	    continue;

        if (strcmp(tok, "v") == 0)
            desc->n_v++;

        if (strcmp(tok, "vt") == 0)
            desc->n_vt++;

        if (strcmp(token, "vn") == 0)
            desc->n_vn++;

        if (strcmp(token, "f") == 0) {
            int i;

	    i = 0;
        tok = strtok(0, " ");

	    /* count triangles in a fan */
            
	    while (token) {
                desc->n_pts++;
                i++;
                tok = strtok(0, " ");
            }
            desc->n_tr += 1 + (i - 3);
        }
    }

    fseek(fp, 0, SEEK_SET);
}

/***************
 * second_pass *
 ***************/

/** 
 * moves data from file into its respective buffers, returns precise
 * length of vertex buffer 'pts' 
 */

int
second_pass(struct desc *desc, float *pts, int *indices, FILE *fp) 
{
    struct hashmap *map;
    char line[LINE_SIZE];
    char *tok;
    float *v, *vt, *vn;
    int v_idx, vt_idx, vn_idx, tr_idx, n_pts;
    uint8_t flags;
	
    v  = malloc(desc->n_v  * 3 * sizeof(float));
    vt = malloc(desc->n_vt * 2 * sizeof(float));
    vn = malloc(desc->n_vn * 3 * sizeof(float));

    v_idx  = 0;
    vt_idx = 0;
    vn_idx = 0;
    tr_idx = 0;
    n_pts  = 0;

    /* loop through every line in the file */

    while (fgets(line, MAX_LINE_SIZE, fp)) {

        char* r;
        
	    r = strchr(line, '\r');
        if (r)
            *r = 0;

        tok = strtok(line, "\n");
        tok = strtok(line, " ");

        if (tok == 0) 
	        continue;

        if (strcmp(token, "v") == 0) {
            strtofs(v + v_idx * 3, strtok(0, "\n"), 3);
            v_idx++;
            continue;
        }

        if (strcmp(token, "vt") == 0) {
            flags |= VT;
            strtofs(vt + vt_idx * 2, strtok(0, "\n"), 2);
            vt_idx++;
            continue;
        }

        if (strcmp(token, "vn") == 0) {
            flags |= VN;
            strtofs(vn + vn_idx * 3, strtok(0, "\n"), 3);
            vn_idx++;
            continue;
        }

        if (strcmp(token, "f") == 0) {
            /* what to do here */    
	}
    }

    map_free(map);
    free(v);
    free(vt);
    free(vn);
    return n_pts;
}

/***************************************************************
 *                                                             *
 *                        obj loading                          *
 *                                                             *
 ***************************************************************/

/************
 * obj_load *
 ************/

/* reads obj file data from path into an indexed triangle list */

struct obj*
obj_load(char *file)
{
    FILE *fp;
    struct obj *obj;
    struct desc desc;
    float *pts, *tmp;
    int *indices;
    char *src;
    int n_pts, n_indices;

    obj = malloc(sizeof(struct obj));

    /* open file and pre allocate buffers in obj context */
    
    fp = fopen(file, "r");
    
    if (fp == 0) {
        return 0;
    }

    /* TODO set src to all of contents */

    fclose(fp);

    first_pass(&desc, src);

    n_pts = obj_desc.n_pts;
    n_indices = obj_desc.n_tr * 3;
    
    pts = malloc(n_pts * 8 * sizeof(float));
    indices = malloc(n_indices * sizeof(int));

    /* fill triangle list buffers */
    
    n_pts = second_pass(&desc, pts, indices, fp);

    if ((tmp = realloc(pts, n_pts * 8 * sizeof(float))))
        pts = tmp;

    /* fill return obj */

    obj->pts = pts;
    obj->indices = indices;
    obj->n_pts = n_pts;
    obj->n_indices = n_indices;

    return obj;
}

/************
 * obj_free *
 ************/

/* takes a heap allocated obj struct and frees it and contents */

void
obj_destroy(struct obj *obj)
{
    free(obj->pts);
    free(obj->indices);
    free(obj);
}