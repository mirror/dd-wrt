#ifndef BAG_H
#define BAG_H
/* general utilities for C */


#include <vstr.h>

typedef struct Bag_obj
{
 const char *key;
 void *val;
} Bag_obj;

typedef struct Bag
{
 size_t num;
 size_t sz;
 void (*free_key_func)(void *);
 void (*free_val_func)(void *);

 unsigned int can_resize : 1;

 Bag_obj VSTR__STRUCT_HACK_ARRAY(data);
} Bag;

typedef struct Bag_iter
{
 Bag *bag;
 size_t num;
} Bag_iter;

extern Bag *bag_make(size_t, void (*)(void *), void (*)(void *));
extern void bag_free(Bag *);

extern Bag *bag_add_obj(Bag *, const char *, void *);
extern Bag *bag_add_cstr(Bag *, const char *, char *);

extern void bag_del_all(Bag *);

extern void bag_sort(Bag *, int (*)(const void *, const void *));
extern int bag_cb_sort_key_coll(const void *, const void *);
extern int bag_cb_sort_key_case(const void *, const void *);
extern int bag_cb_sort_key_vers(const void *, const void *);
extern int bag_cb_sort_key_cmp(const void *, const void *);

extern const Bag_obj *bag_iter_beg(Bag *, Bag_iter *);
extern const Bag_obj *bag_iter_nxt(Bag_iter *);

extern const Bag_obj *bag_srch_eq(Bag *,
                                  int (*)(const Bag_obj *, const void *),
                                  const void *);
extern int bag_cb_srch_eq_key_ptr(const Bag_obj *, const void *);
extern int bag_cb_srch_eq_val_ptr(const Bag_obj *, const void *);

extern void bag_cb_free_nothing(void *);
extern void bag_cb_free_ref(void *);
extern void bag_cb_free_malloc(void *);

#endif
