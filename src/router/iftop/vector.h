/*
 * vector.h:
 * simple vectors
 *
 * Copyright (c) 2001 Chris Lightfoot. All rights reserved.
 *
 * $Id: vector.h,v 1.1 2003/10/19 06:44:33 pdw Exp $
 *
 */

#ifndef __VECTOR_H_ /* include guard */
#define __VECTOR_H_

typedef union _item {
    void *v;
    long l;
} item;

#define _inline inline

static _inline item item_long(const long l) { item u; u.l = l; return u; }
static _inline item item_ptr(void *const v) { item u; u.v = v; return u; }

typedef struct _vector{
    item *ary;
    size_t n, n_used;
} *vector;

vector vector_new(void);
void vector_delete(vector);
void vector_delete_free(vector);

void  vector_push_back(vector, const item);
void  vector_pop_back(vector);
item vector_back(const vector);

item *vector_remove(vector, item *t);

void  vector_reallocate(vector, const size_t n);

/* A macro to iterate over a vector */
#define vector_iterate(_v, _t)  for ((_t) = (_v)->ary; (_t) < (_v)->ary + (_v)->n_used; ++(_t))

#endif /* __VECTOR_H_ */
