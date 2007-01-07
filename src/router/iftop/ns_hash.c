/* hash table */

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in_systm.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "ns_hash.h"
#include "hash.h"
#include "iftop.h"

#define hash_table_size 256

int ns_hash_compare(void* a, void* b) {
    struct in_addr* aa = (struct in_addr*)a;
    struct in_addr* bb = (struct in_addr*)b;
    return (aa->s_addr == bb->s_addr);
}

int ns_hash_hash(void* key) {
    int hash;
    long addr;
        
    addr = (long)((struct in_addr*)key)->s_addr;

    hash = ((addr & 0x000000FF)
            + (addr & 0x0000FF00 >> 8)
            + (addr & 0x00FF0000 >> 16)
            + (addr & 0xFF000000 >> 24)) % 0xFF;

    return hash;
}

void* ns_hash_copy_key(void* orig) {
    struct in_addr* copy;
    copy = xmalloc(sizeof *copy);
    *copy = *(struct in_addr*)orig;
    return copy;
}

void ns_hash_delete_key(void* key) {
    free(key);
}

/*
 * Allocate and return a hash
 */
hash_type* ns_hash_create() {
    hash_type* hash_table;
    hash_table = xcalloc(hash_table_size, sizeof *hash_table);
    hash_table->size = hash_table_size;
    hash_table->compare = &ns_hash_compare;
    hash_table->hash = &ns_hash_hash;
    hash_table->delete_key = &ns_hash_delete_key;
    hash_table->copy_key = &ns_hash_copy_key;
    hash_initialise(hash_table);
    return hash_table;
}

