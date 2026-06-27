#ifndef MAP_H
#define MAP_H

struct hash_table;

struct hash_table* 
hash_table_alloc();

void
hash_table_free(struct hash_table* ht);

int
search(struct hash_table* ht, char* key);

/* inserts key into hash table */
void
insert(struct hash_table* ht, char* key, int val);

#endif    /* MAP_H */