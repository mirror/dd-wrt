/*
 * addr_hash.h:
 *
 */

#ifndef __HASH_H_ /* include guard */
#define __HASH_H_

/* implementation independent declarations */
typedef enum {
    HASH_STATUS_OK,
    HASH_STATUS_MEM_EXHAUSTED,
    HASH_STATUS_KEY_NOT_FOUND
} hash_status_enum;

typedef struct node_tag {
    struct node_tag *next;       /* next node */
    void* key;                /* key */
    void* rec;                /* user data */
} hash_node_type;

typedef struct {
    int (*compare) (void*, void*);
    int (*hash) (void*);
    void* (*copy_key) (void*);
    void (*delete_key) (void*);
    hash_node_type** table;
    int size;
} hash_type;


hash_status_enum hash_initialise(hash_type*);
hash_status_enum hash_destroy(hash_type*);
hash_status_enum hash_insert(hash_type*, void* key, void *rec);
hash_status_enum hash_delete(hash_type* hash_table, void* key);
hash_status_enum hash_find(hash_type* hash_table, void* key, void** rec);
hash_status_enum hash_next_item(hash_type* hash_table, hash_node_type** ppnode);
void hash_delete_all(hash_type* hash_table);

#endif /* __HASH_H_ */
