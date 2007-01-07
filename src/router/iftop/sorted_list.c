/*
 * sorted_list.c:
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include "sorted_list.h"
#include "iftop.h"


void sorted_list_insert(sorted_list_type* list, void* item) {
    sorted_list_node *node, *p;

    p = &(list->root);

    while(p->next != NULL && list->compare(item, p->next->data) > 0) {
        p = p->next;
    } 

    node = xmalloc(sizeof *node);

    node->next = p->next;
    node->data = item;
    p->next = node;
}


sorted_list_node* sorted_list_next_item(sorted_list_type* list, sorted_list_node* prev) {
    if(prev == NULL) {
        return list->root.next;
    }
    else {
        return prev->next;
    }
}

void sorted_list_destroy(sorted_list_type* list) {
    sorted_list_node *p, *n;
    p = list->root.next;

    while(p != NULL) {
        n = p->next;
        free(p);
        p = n;
    }

    list->root.next = NULL;
}

void sorted_list_initialise(sorted_list_type* list) {
    list->root.next = NULL;
}



