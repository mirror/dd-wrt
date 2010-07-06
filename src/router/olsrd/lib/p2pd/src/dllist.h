/* 
 * File:   dllist.h
 * Author: Caspar
 *
 * Created on February 28, 2010, 1:59 PM
 */

#ifndef _DLLIST_H
#define	_DLLIST_H

struct node {
  void * data;
  struct node * next;
  struct node * prev;
};

struct node * append_node(struct node ** head, struct node ** tail, void * data);
void remove_node(struct node ** head, struct node **tail, struct node * node, bool free_data);
void clear_list(struct node **head, struct node **tail, bool free_data);

#endif	/* _DLLIST_H */

