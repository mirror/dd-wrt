/* hash table */

#include <stdio.h>
#include <stdlib.h>
#include "addr_hash.h"
#include "hash.h"
#include "iftop.h"

#define hash_table_size 256

int compare(void* a, void* b) {
    addr_pair* aa = (addr_pair*)a;
    addr_pair* bb = (addr_pair*)b;
    return (aa->src.s_addr == bb->src.s_addr 
            && aa->src_port == bb->src_port
            && aa->dst.s_addr == bb->dst.s_addr
            && aa->dst_port == bb->dst_port
            && aa->protocol == bb->protocol);
}

int hash(void* key) {
    int hash;
    long addr;
    addr_pair* ap = (addr_pair*)key;
        
    addr = (long)ap->src.s_addr;

    hash = ((addr & 0x000000FF)
            + (addr & 0x0000FF00 >> 8)
            + (addr & 0x00FF0000 >> 16)
            + (addr & 0xFF000000 >> 24)
            + ap->src_port) % 0xFF;

    addr = (long)ap->dst.s_addr;
    hash = ( hash + (addr & 0x000000FF)
            + (addr & 0x0000FF00 >> 8)
            + (addr & 0x00FF0000 >> 16)
            + (addr & 0xFF000000 >> 24)
            + ap->dst_port) % 0xFF;

    return hash;
}

void* copy_key(void* orig) {
    addr_pair* copy;
    copy = xmalloc(sizeof *copy);
    *copy = *(addr_pair*)orig;
    return copy;
}

void delete_key(void* key) {
    free(key);
}

/*
 * Allocate and return a hash
 */
hash_type* addr_hash_create() {
    hash_type* hash_table;
    hash_table = xcalloc(hash_table_size, sizeof *hash_table);
    hash_table->size = hash_table_size;
    hash_table->compare = &compare;
    hash_table->hash = &hash;
    hash_table->delete_key = &delete_key;
    hash_table->copy_key = &copy_key;
    hash_initialise(hash_table);
    return hash_table;
}

