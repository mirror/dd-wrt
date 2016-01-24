#define _GNU_SOURCE 1

#include "bag.h"

/* need to use vlg etc. */
#define EX_UTILS_NO_FUNCS 1
#include "ex_utils.h"

#ifdef __GNUC__
# define BAG__ATTR_UNUSED(x) vstr__UNUSED_ ## x __attribute__((unused))
#elif defined(__LCLINT__)
# define BAG__ATTR_UNUSED(x) /*@unused@*/ vstr__UNUSED_ ## x
#else
# define BAG__ATTR_UNUSED(x) vstr__UNUSED_ ## x
#endif

#ifndef __GLIBC__ /* hack */
# define strverscmp(x, y) strcmp(x, y)
#endif

void bag_del_all(Bag *bag)
{
  while (bag->num--)
  {
    bag->free_key_func((void *)bag->data[bag->num].key);
    bag->free_val_func(bag->data[bag->num].val);
  }
  bag->num = 0;
}

Bag *bag_make(size_t sz,
              void (*free_key_func)(void *), void (*free_val_func)(void *))
{
  Bag *bag = malloc(sizeof(Bag) + (sizeof(Bag_obj) * sz));

  if (!bag)
    return (NULL);

  bag->num           = 0;
  bag->sz            = sz;
  bag->free_key_func = free_key_func;
  bag->free_val_func = free_val_func;

  bag->can_resize    = FALSE;
  
  return (bag);
}

void bag_free(Bag *bag)
{
  if (!bag)
    return;
  
  bag_del_all(bag);
  
  free(bag);
}

Bag *bag_add_obj(Bag *bag, const char *key, void *val)
{
  ASSERT(bag);

  ASSERT(bag->num <= bag->sz);
  
  if ((bag->num >= bag->sz))
  {
    Bag *tmp = NULL;
    size_t sz = bag->sz;
    
    if (!bag->can_resize)
      return (NULL);

    sz <<= 1;
    if (!(tmp = realloc(bag, sizeof(Bag) + (sizeof(Bag_obj) * sz))))
      return (NULL);
    
    bag = tmp;
    bag->sz = sz;
  }
  
  bag->data[bag->num].key = key;
  bag->data[bag->num].val = val;
  ++bag->num;

  return (bag);
}

Bag *bag_add_cstr(Bag *bag, const char *key, char *val)
{
  return (bag_add_obj(bag, key, val));
}

void bag_sort(Bag *bag, int (*cmp)(const void *, const void *))
{
  qsort(bag->data, bag->num, sizeof(Bag_obj), cmp);
}

int bag_cb_sort_key_cmp(const void *passed_one, const void *passed_two)
{
  const Bag_obj *const one = passed_one;
  const Bag_obj *const two = passed_two;

  return (strcmp(one->key, two->key));
}

int bag_cb_sort_key_case(const void *passed_one, const void *passed_two)
{
  const Bag_obj *const one = passed_one;
  const Bag_obj *const two = passed_two;

  return (strcasecmp(one->key, two->key));
}

int bag_cb_sort_key_vers(const void *passed_one, const void *passed_two)
{
  const Bag_obj *const one = passed_one;
  const Bag_obj *const two = passed_two;

  return (strverscmp(one->key, two->key));
}

int bag_cb_sort_key_coll(const void *passed_one, const void *passed_two)
{
  const Bag_obj *const one = passed_one;
  const Bag_obj *const two = passed_two;

  return (strcoll(one->key, two->key));
}

const Bag_obj *bag_iter_nxt(Bag_iter *iter)
{
  ASSERT(iter);
  
  if (iter->num >= iter->bag->num)
    return (NULL);

  return (iter->bag->data + iter->num++);
}

const Bag_obj *bag_iter_beg(Bag *bag, Bag_iter *iter)
{
  ASSERT(bag && iter);
  
  iter->bag = bag;
  iter->num = 0;
  
  return (bag_iter_nxt(iter));
}

const Bag_obj *bag_srch_eq(Bag *bag,
                           int (*cmp_func)(const Bag_obj *, const void *),
                           const void *val)
{
  Bag_iter iter[1];
  const Bag_obj *obj = bag_iter_beg(bag, iter);

  while (obj)
  {
    if ((*cmp_func)(obj, val))
      return (obj);
    
    obj = bag_iter_nxt(iter);
  }

  return (NULL);
}

int bag_cb_srch_eq_key_ptr(const Bag_obj *obj, const void *val)
{
  if (obj->key == val)
    return (TRUE);
    
  return (FALSE);
}

int bag_cb_srch_eq_val_ptr(const Bag_obj *obj, const void *val)
{
  if (obj->val == val)
    return (TRUE);
    
  return (FALSE);
}

void bag_cb_free_nothing(void *BAG__ATTR_UNUSED(val))
{
}

void bag_cb_free_ref(void *val)
{
  vstr_ref_del(val);
}

void bag_cb_free_malloc(void *val)
{
  free(val);
}

