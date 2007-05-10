#ifndef __UTIL_H__
#define __UTIL_H__
 typedef struct _set_t
{
  void *next;
 } set_t;
void list_add (set_t **, set_t *);
set_t * list_remove (set_t **);
set_t * list_get_head (set_t **);
set_t * list_get_next (set_t *);
int list_remove_node (set_t **, set_t *);
void list_cat (set_t **, set_t **);
unsigned short in_cksum (unsigned short *, int);

#endif	/*  */
