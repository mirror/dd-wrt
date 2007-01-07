/*
 * sorted_list.h:
 *
 */

#ifndef __SORTED_LIST_H_ /* include guard */
#define __SORTED_LIST_H_

typedef struct sorted_list_node_tag {
    struct sorted_list_node_tag* next;
    void* data;
} sorted_list_node;

typedef struct {
    sorted_list_node root;
    int (*compare)(void*, void*);
} sorted_list_type;

void sorted_list_initialise(sorted_list_type* list);
void sorted_list_insert(sorted_list_type* list, void* item);
sorted_list_node* sorted_list_next_item(sorted_list_type* list, sorted_list_node* prev);
void sorted_list_destroy(sorted_list_type* list);


#endif /* __SORTED_LIST_H_ */
