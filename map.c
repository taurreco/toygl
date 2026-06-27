#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "map.h"

const int SIZES[23] = {
    53, 107, 223, 449, 907, 1823, 3659, 
    7321, 14653, 29311, 58631, 117269, 
    234539, 469099, 938207, 1876417, 
    3752839, 7505681, 15011389, 30022781, 
    60045577, 120091177, 240182359
};

int SIZES_INDEX = 0;  /* current position in the array above */

/*********************************************************************
 *                                                                   *
 *                              structs                              *
 *                                                                   *
 *********************************************************************/

struct kv_pair {
    char* key;
    int val;
};

struct hash_table {
    struct kv_pair** pairs;         /* list of obj indices */
    int n_pairs;                    /* running count of keys */
    int size;                       /* number of buckets in ht */
};

/*********************************************************************
 *                                                                   *
 *                (private) kv_pair memory management                *
 *                                                                   *
 *********************************************************************/

/*****************
 * kv_pair_alloc *
 *****************/

/* allocates a key value pair */
static struct kv_pair* 
kv_pair_alloc(char* key, int val)
{
    struct kv_pair* pair = malloc(sizeof(struct kv_pair));
    pair->key = strdup(key);
    pair->val = val;
    return pair;
}

/****************
 * kv_pair_free *
 ****************/

/* frees kv_pair */
static void
kv_pair_free(struct kv_pair* pair) 
{
    free(pair->key);
    free(pair);
}

/*********************************************************************
 *                                                                   *
 *             (private) hash table memory management                *
 *                                                                   *
 *********************************************************************/

/********************
 * hash_table_alloc *
 ********************/

/* allocates empty hashtable of current size */
struct hash_table* 
hash_table_alloc()
{
    struct hash_table* ht = malloc(sizeof(struct hash_table));
    ht->size = SIZES[SIZES_INDEX];
    ht->n_pairs = 0;
    ht->pairs = calloc(ht->size, sizeof(struct kv_pair*));
    return ht;
}

/*******************
 * hash_table_free *
 *******************/

/* frees hashtable */
void
hash_table_free(struct hash_table* ht) 
{
    for (int i = 0; i < ht->size; i++) {
        struct kv_pair* pair= ht->pairs[i];
        if (pair != NULL) {
            kv_pair_free(pair);
        }
    }
    free(ht->pairs);
    free(ht);
}

/*********************************************************************
 *                                                                   *
 *             (private) open addressed string hashing               *
 *                                                                   *
 *********************************************************************/

/**********
 * hash_1 *
 **********/

static int
hash_1(char* key)
{
    int64_t hash = 5381;
    int c;

    while ((c = *key++))
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

    return (int)hash;
}

/**********
 * hash_2 *
 **********/

static int
hash_2(char* key)
{
    int hash, i;
    int key_len = strlen(key);

    for (hash = i = 0; i < key_len; i++) {
        hash += key[i];
        hash += (hash << 10);
        hash ^= (hash >> 6);
    }
    hash += (hash << 3);
    hash ^= (hash >> 11);
    hash += (hash << 15);

    return hash;
}

/***************
 * double_hash *
 ***************/

/**
 * sends a string through two hash functions, useses them with an 
 * attempt counter to find a determenistic index into the hash table
 */
static int
double_hash(char* string, int size, int attempt)
{
    int hash_a = hash_1(string);
    int hash_b = hash_2(string);
    return abs(hash_a + (attempt * (hash_b + 1))) % size;
}

/*********************************************************************
 *                                                                   *
 *             (private) hash table interface functions              *
 *                                                                   *
 *********************************************************************/
/*********************************************************************
 *                                                                   *
 *                    (private) hash table helpers                   *
 *                                                                   *
 *********************************************************************/

/********
 * grow *
 ********/

/** 
 * nearly doubles the capacity of hashtable
 * it finds new size by indexing into a global sizes array
 */
static void 
grow(struct hash_table* ht)
{
    SIZES_INDEX++;
    struct hash_table* new_ht = hash_table_alloc();
    for (int i = 0; i < ht->size; i++) {
        struct kv_pair* pair = ht->pairs[i];
        if (pair != NULL) {
            insert(new_ht, pair->key, pair->val);
        }
    }
    ht->size = new_ht->size;
    ht->n_pairs = new_ht->n_pairs;

    ht->size = SIZES[SIZES_INDEX];
    new_ht->size = SIZES[SIZES_INDEX - 1];

    /* swap list of kv_pairs */

    struct kv_pair** tmp_pairs = ht->pairs;
    ht->pairs = new_ht->pairs;
    new_ht->pairs = tmp_pairs;

    hash_table_free(new_ht);
}

/**********
 * insert *
 **********/

/* inserts key into hash table */
void
insert(struct hash_table* ht, char* key, int val) 
{
    int load = ht->n_pairs * 100 / ht->size;
    if (load > 70) {
        grow(ht);
    }
    struct kv_pair* pair = kv_pair_alloc(key, val);
    int index = double_hash(pair->key, ht->size, 0);
    struct kv_pair* cur_pair = ht->pairs[index];
    int i = 1;
    while (cur_pair != NULL) {
        index = double_hash(pair->key, ht->size, i);
        cur_pair = ht->pairs[index]; /* collided item */
        i++;
    }
    ht->pairs[index] = pair;
    ht->n_pairs++;
}

/**********
 * search *
 **********/

/* returns val of key or -1 if it isn't in the hash table */
int
search(struct hash_table* ht, char* key)
{
    int index = double_hash(key, ht->size, 0);
    struct kv_pair* cur_pair = ht->pairs[index];
    int i = 1;
    while (cur_pair != NULL) {
        if (strcmp(cur_pair->key, key) == 0) {
            return cur_pair->val;
        }
        index = double_hash(key, ht->size, i);
        cur_pair = ht->pairs[index];
        i++;
    }
    return -1;
}

