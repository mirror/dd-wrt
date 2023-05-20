/*
 * container_binary_array.c
 *
 * see comments in header file.
 *
 * Portions of this file are subject to the following copyright(s).  See
 * the Net-SNMP's COPYING file for more details and other copyrights
 * that may apply:
 *
 * Portions of this file are copyrighted by:
 * Copyright (c) 2016 VMware, Inc. All rights reserved.
 * Use is subject to license terms specified in the COPYING file
 * distributed with the Net-SNMP package.
 */

#include <net-snmp/net-snmp-config.h>

#ifdef HAVE_IO_H
#include <io.h>
#endif
#include <stdio.h>
#if HAVE_STDLIB_H
#include <stdlib.h>
#endif
#if HAVE_MALLOC_H
#include <malloc.h>
#endif
#include <sys/types.h>
#if HAVE_STRING_H
#include <string.h>
#else
#include <strings.h>
#endif

#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/types.h>
#include <net-snmp/library/snmp_api.h>
#include <net-snmp/library/container.h>
#include <net-snmp/library/container_binary_array.h>
#include <net-snmp/library/tools.h>
#include <net-snmp/library/snmp_assert.h>
#include "factory.h"

typedef struct binary_array_table_s {
    size_t                     max_size;   /* Size of the current data table */
    size_t                     count;      /* Index of the next free entry */
    int                        dirty;
    void                     **data;       /* The table itself */
} binary_array_table;

typedef struct binary_array_iterator_s {
    netsnmp_iterator base;

    size_t           pos;
} binary_array_iterator;

static netsnmp_iterator *_ba_iterator_get(netsnmp_container *c);

static int
Sort_Array(netsnmp_container *c)
{
    binary_array_table *t = (binary_array_table*)c->container_data;
    netsnmp_assert(t!=NULL);
    netsnmp_assert(c->compare!=NULL);

    if (c->flags & CONTAINER_KEY_UNSORTED)
        return 0;

    if (t->dirty) {
        /*
         * Sort the table 
         */
        qsort(t->data, t->count, sizeof(void *), c->compare);
        t->dirty = 0;

        /*
         * no way to know if it actually changed... just assume so.
         */
        ++c->sync;
    }

    return 1;
}

static int
linear_search(const void *val, netsnmp_container *c)
{
    binary_array_table *t = (binary_array_table*)c->container_data;
    size_t             pos = 0;

    if (!t->count)
        return -1;

    if (! (c->flags & CONTAINER_KEY_UNSORTED)) {
        snmp_log(LOG_ERR, "linear search on sorted container %s?!?\n",
                 c->container_name);
        return -1;
    }

    for (; pos < t->count; ++pos) {
        if (c->compare(t->data[pos], val) == 0)
            return pos;
    }

    return -1;
}

static int
binary_search(const void *val, netsnmp_container *c, int exact, size_t *next)
{
    binary_array_table *t = (binary_array_table*)c->container_data;
    size_t             len = t->count;
    size_t             half;
    size_t             first = 0;
    size_t             middle = 0; /* init not needed; keeps compiler happy */
    int                result = 0; /* init not needed; keeps compiler happy */

    if (!len) {
        if (NULL != next)
            *next = 0;
        return -1;
    }

    if (c->flags & CONTAINER_KEY_UNSORTED) {
        if (!exact) {
            snmp_log(LOG_ERR, "non-exact search on unsorted container %s?!?\n",
                     c->container_name);
            return -1;
        }
        return linear_search(val, c);
    }

    if (t->dirty)
        Sort_Array(c);

    while (len > 0) {
        half = len >> 1;
        middle = first + half;
        if ((result = c->compare(t->data[middle], val)) < 0) {
            first = middle + 1;
            len = len - half - 1;
        } else if (result == 0) {
            first = middle;
            break;
        } else {
            len = half;
        }
    }

    if (first >= t->count) {
        if (exact && NULL != next)
            *next = t->count;
        return -1;
    }

    if (first != middle) {
        /* last compare wasn't against first, so get actual result */
        result = c->compare(t->data[first], val);
    }

    if(result == 0) {
        if (exact && NULL != next)
            *next = first+1;
        else if (!exact && ++first == t->count) {
            if (NULL != next)
                *next = first;
            first = -1;
        }
    } else if(exact) {
        if (NULL != next) {
            if (result > 0)
                *next = first;
            else
                *next = t->count;
        }
        first = -1;
    }

    return first;
}

NETSNMP_STATIC_INLINE binary_array_table *
netsnmp_binary_array_initialize(void)
{
    binary_array_table *t;

    t = SNMP_MALLOC_TYPEDEF(binary_array_table);
    if (t == NULL)
        return NULL;

    t->max_size = 0;
    t->count = 0;
    t->dirty = 0;
    t->data = NULL;

    return t;
}

void
netsnmp_binary_array_release(netsnmp_container *c)
{
    binary_array_table *t = (binary_array_table*)c->container_data;
    SNMP_FREE(t->data);
    SNMP_FREE(t);
    SNMP_FREE(c);
}

/**
 * Set or test the options of a binary array container.
 * @param c: Container.
 * @param set: Set (1) or test (0).
 * @param flags: Zero or more CONTAINER_KEY_* flags.
 */
int
netsnmp_binary_array_options_set(netsnmp_container *c, int set, u_int flags)
{
#define BA_FLAGS (CONTAINER_KEY_ALLOW_DUPLICATES|CONTAINER_KEY_UNSORTED)

    if (set) {
        if ((flags & BA_FLAGS) == flags) {
            /** if turning off unsorted, do sort */
            int sort = ((c->flags & CONTAINER_KEY_UNSORTED) &&
                        ! (flags & CONTAINER_KEY_UNSORTED));
            c->flags = flags;
            if (sort) {
                binary_array_table *t = (binary_array_table*)c->container_data;
                t->dirty = 1; /* force sort */
                Sort_Array(c);
            }
            return flags;
        } else {
            return -1; /* unsupported flag */
        }
    } else {
        return ((c->flags & flags) == flags);
    }
}

NETSNMP_STATIC_INLINE size_t
netsnmp_binary_array_count(netsnmp_container *c)
{
    binary_array_table *t = (binary_array_table*)c->container_data;
    /*
     * return count
     */
    return t ? t->count : 0;
}

NETSNMP_STATIC_INLINE void           *
netsnmp_binary_array_get(netsnmp_container *c, const void *key, int exact)
{
    binary_array_table *t = (binary_array_table*)c->container_data;
    int             index = 0;

    /*
     * if there is no data, return NULL;
     */
    if (!t->count)
        return NULL;

    /*
     * if the table is dirty, sort it.
     */
    if (t->dirty)
        Sort_Array(c);

    /*
     * if there is a key, search. Otherwise default is 0;
     */
    if (key) {
        if ((index = binary_search(key, c, exact, NULL)) == -1)
            return NULL;
        if (!exact &&
            c->flags & CONTAINER_KEY_ALLOW_DUPLICATES) {
            int result;

            /*
             * If duplicates are allowed, we have to be extra
             * sure that we didn't just increment to a duplicate,
             * thus causing a getnext loop.
             */
            result = c->compare(t->data[index], key);
            while (result == 0) {
		DEBUGMSGTL(("container","skipping duplicate key in %s\n",
					    c->container_name));
                if (++index == t->count)
                   return NULL;
                result = c->compare(t->data[index], key);
            }
        }
    }

    return t->data[index];
}

static int
netsnmp_binary_array_get_at(netsnmp_container *c, size_t pos, void **entry)
{
    binary_array_table *t = (binary_array_table*)c->container_data;

    /*
     * if there is no data, return NULL;
     */
    if (!t->count || pos >= t->count || NULL == entry)
        return -1;

    *entry = t->data[pos];

    return 0;
}

/**
 * Returns 1 if and only if the elements in @c are sorted in ascending order.
 *
 * To do: stop calling this function after
 * https://github.com/net-snmp/net-snmp/issues/107 and
 * https://github.com/net-snmp/net-snmp/issues/293 have been fixed.
 */
static int _ba_is_sorted(const netsnmp_container *c)
{
    /*
     * The code below has been commented out because it negatively affects
     * performance.
     */
#if 0
    const binary_array_table *t = c->container_data;
    int i;

    for (i = 0; i + 1 < t->count; ++i)
        if (c->compare(t->data[i], t->data[i + 1]) > 0)
            return 0;
#endif

    return 1;
}

int
netsnmp_binary_array_remove_at(netsnmp_container *c, size_t index, void **save)
{
    binary_array_table *t = (binary_array_table*)c->container_data;

    if (save)
        *save = NULL;
    
    /*
     * if there is no data, return NULL;
     */
    if (!t->count)
        return -1;

    /*
     * find old data and save it, if ptr provided
     */
    if (save)
        *save = t->data[index];

    /*
     * if entry was last item, just decrement count
     */
    --t->count;
    if (index != t->count) {
        /*
         * otherwise, shift array down
         */
        memmove(&t->data[index], &t->data[index+1],
                sizeof(void*) * (t->count - index));

        ++c->sync;
    }

    netsnmp_assert(t->dirty || _ba_is_sorted(c));

    return 0;
}

int
netsnmp_binary_array_remove(netsnmp_container *c, const void *key, void **save)
{
    binary_array_table *t = (binary_array_table*)c->container_data;
    int                index = 0;

    if (save)
        *save = NULL;
    
    /*
     * if there is no data, return NULL;
     */
    if (!t->count)
        return 0;

    /*
     * if the table is dirty, sort it.
     */
    if (t->dirty)
        Sort_Array(c);

    /*
     * search
     */
    if ((index = binary_search(key, c, 1, NULL)) == -1)
        return -1;

    return netsnmp_binary_array_remove_at(c, (size_t)index, save);
}

NETSNMP_STATIC_INLINE void
netsnmp_binary_array_for_each(netsnmp_container *c,
                              netsnmp_container_obj_func *fe,
                              void *context, int sort)
{
    binary_array_table *t = (binary_array_table*)c->container_data;
    size_t             i;

    if (sort && t->dirty)
        Sort_Array(c);

    for (i = 0; i < t->count; ++i)
        (*fe) (t->data[i], context);
}

NETSNMP_STATIC_INLINE void
netsnmp_binary_array_clear(netsnmp_container *c,
                           netsnmp_container_obj_func *fe,
                           void *context)
{
    binary_array_table *t = (binary_array_table*)c->container_data;

    if( NULL != fe ) {
        size_t             i;

        for (i = 0; i < t->count; ++i)
            (*fe) (t->data[i], context);
    }

    t->count = 0;
    t->dirty = 0;
    ++c->sync;
}

static int
_ba_resize_check(binary_array_table *t)
{
    size_t new_max;
    void ** new_data;
    if (t->max_size > t->count)
        return 0; /* resize not needed */

    /*
     * Table is full, so extend it to double the size, or use 10 elements
     * if it is empty.
     */
    new_max = t->max_size > 0 ? 2 * t->max_size : 10;
    new_data = (void**) realloc(t->data, new_max * sizeof(void*));
    if (new_data == NULL) {
        snmp_log(LOG_ERR, "malloc failed in _ba_resize_check\n");
        return -1; /* error */
    }

    memset(new_data + t->max_size, 0x0,
           (new_max - t->max_size) * sizeof(void*));

    t->data = new_data;
    t->max_size = new_max;

    return 1; /* resized */
}

static int
netsnmp_binary_array_insert_before(netsnmp_container *c, size_t index,
                                   const void *entry, int dirty)
{
    binary_array_table *t = (binary_array_table*)c->container_data;

    if (NULL == entry)
        return -1;

    if (index > t->count) {
        DEBUGMSGTL(("container:insert:before", "index out of range\n"));
        return -1;
    }

     /*
      * check if we need to resize the array
      */
    _ba_resize_check(t);

    netsnmp_assert(t->count < t->max_size);

    /*
     * shift array
     */
    memmove(&t->data[index+1], &t->data[index],
            sizeof(void*) * (t->count - index));

    /*
     * Insert the new entry into the data array
     */
    t->data[index] = NETSNMP_REMOVE_CONST(void *, entry);
    ++t->count;

    netsnmp_assert(index < t->count);
    netsnmp_assert(t->count <= t->max_size);

    if (dirty)
        t->dirty = 1;

    netsnmp_assert(t->dirty || _ba_is_sorted(c));

    ++c->sync;

    return 0;
}

NETSNMP_STATIC_INLINE int
netsnmp_binary_array_insert(netsnmp_container *c, const void *const_entry)
{
    binary_array_table *t = (binary_array_table*)c->container_data;
    const int duplicates_allowed = c->flags & CONTAINER_KEY_ALLOW_DUPLICATES;
    const int sorted = !(c->flags & CONTAINER_KEY_UNSORTED);
    int             i = -2;
    size_t          next, pos;
    void           *entry = NETSNMP_REMOVE_CONST(void *, const_entry);

    if (NULL == entry)
        return -1;

    /*
     * check key if we have at least 1 item and duplicates aren't allowed
     */
    if (!duplicates_allowed && t->count) {
        i = binary_search(entry, c, 1, &next);
        if (i >= 0) {
            DEBUGMSGTL(("container","not inserting duplicate key\n"));
            return -1;
        }
    }
 
    /*
     * if unsorted, just add at the end
     */
    if (!sorted) {
        pos = t->count;
    } else {
        /** if we haven't searched for key yet, do it now */
        if (-2 == i) {
            if (0 == t->count) {
                next = 0;
                i = -1;
            } else {
                i = binary_search(entry, c, 1, &next);
            }
        }

        pos = next;
        /* if key found, advance past any duplicates */
        if (duplicates_allowed && i >= 0)
            while (pos < t->count && c->compare(t->data[pos], entry) == 0)
                ++pos;
    }

    return netsnmp_binary_array_insert_before(c, pos, entry, !sorted);
}

/**********************************************************************
 *
 * Special case support for subsets
 *
 */
static int
binary_search_for_start(netsnmp_index *val, netsnmp_container *c)
{
    binary_array_table *t = (binary_array_table*)c->container_data;
    size_t             len = t->count;
    size_t             half;
    size_t             middle;
    size_t             first = 0;
    int                result = 0;

    if (!len)
        return -1;

    if (t->dirty)
        Sort_Array(c);

    while (len > 0) {
        half = len >> 1;
        middle = first + half;
        if ((result = c->ncompare(t->data[middle], val)) < 0) {
            first = middle + 1;
            len = len - half - 1;
        } else
            len = half;
    }

    if ((first >= t->count) ||
        c->ncompare(t->data[first], val) != 0)
        return -1;

    return first;
}

void          **
netsnmp_binary_array_get_subset(netsnmp_container *c, void *key, int *len)
{
    binary_array_table *t;
    void          **subset;
    int             start, end;
    size_t          i;

    /*
     * if there is no data, return NULL;
     */
    if (!c || !key || !len)
        return NULL;

    t = (binary_array_table*)c->container_data;
    netsnmp_assert(c->ncompare);
    if (!t->count || !c->ncompare)
        return NULL;

    /*
     * if the table is dirty, sort it.
     */
    if (t->dirty)
        Sort_Array(c);

    /*
     * find matching items
     */
    start = end = binary_search_for_start((netsnmp_index *)key, c);
    if (start == -1)
        return NULL;

    for (i = start + 1; i < t->count; ++i) {
        if (0 != c->ncompare(t->data[i], key))
            break;
        ++end;
    }

    *len = end - start + 1;
    if (*len <= 0)
        return NULL;

    subset = (void **)malloc((*len) * sizeof(void*));
    if (subset)
        memcpy(subset, &t->data[start], sizeof(void*) * (*len));

    return subset;
}

/**********************************************************************
 *
 * container
 *
 */
static void *
_ba_find(netsnmp_container *container, const void *data)
{
    return netsnmp_binary_array_get(container, data, 1);
}

static void *
_ba_find_next(netsnmp_container *container, const void *data)
{
    return netsnmp_binary_array_get(container, data, 0);
}

static int
_ba_insert(netsnmp_container *container, const void *data)
{
    return netsnmp_binary_array_insert(container, data);
}

static int
_ba_insert_before(netsnmp_container *container, size_t index, void *data)
{
    /** don't trust users direct-acces inserts, mark array dirty */
    return netsnmp_binary_array_insert_before(container, index, data, 1);
}

static int
_ba_remove(netsnmp_container *container, const void *data)
{
    return netsnmp_binary_array_remove(container,data, NULL);
}

static int
_ba_free(netsnmp_container *container)
{
    netsnmp_binary_array_release(container);
    return 0;
}

static size_t
_ba_size(netsnmp_container *container)
{
    return netsnmp_binary_array_count(container);
}

static void
_ba_for_each(netsnmp_container *container, netsnmp_container_obj_func *f,
             void *context)
{
    netsnmp_binary_array_for_each(container, f, context, 1);
}

static void
_ba_clear(netsnmp_container *container, netsnmp_container_obj_func *f,
             void *context)
{
    netsnmp_binary_array_clear(container, f, context);
}

static netsnmp_void_array *
_ba_get_subset(netsnmp_container *container, void *data)
{
    netsnmp_void_array * va;
    void ** rtn;
    int len;

    rtn = netsnmp_binary_array_get_subset(container, data, &len);
    if (NULL==rtn)
        return NULL;
    
    va = SNMP_MALLOC_TYPEDEF(netsnmp_void_array);
    if (va == NULL) {
        free(rtn);
        return NULL;
    }
    
    va->size = len;
    va->array = rtn;

    return va;
}

static int _ba_options(netsnmp_container *c, int set, u_int flags)
{
    return netsnmp_binary_array_options_set(c, set, flags);
}

static netsnmp_container *
_ba_duplicate(netsnmp_container *c, void *ctx, u_int flags)
{
    netsnmp_container *dup;
    binary_array_table *dupt, *t;

    if (flags) {
        snmp_log(LOG_ERR, "binary arry duplicate does not supprt flags yet\n");
        return NULL;
    }

    dup = netsnmp_container_get_binary_array();
    if (NULL == dup) {
        snmp_log(LOG_ERR," no memory for binary array duplicate\n");
        return NULL;
    }
    /*
     * deal with container stuff
     */
    if (netsnmp_container_data_dup(dup, c) != 0) {
        netsnmp_binary_array_release(dup);
        return NULL;
    }

    /*
     * deal with data
     */
    dupt = (binary_array_table*)dup->container_data;
    t = (binary_array_table*)c->container_data;

    dupt->max_size = t->max_size;
    dupt->count = t->count;
    dupt->dirty = t->dirty;

    /*
     * shallow copy
     */
    dupt->data = (void**) malloc(dupt->max_size * sizeof(void*));
    if (NULL == dupt->data) {
        snmp_log(LOG_ERR, "no memory for binary array duplicate\n");
        netsnmp_binary_array_release(dup);
        return NULL;
    }

    memcpy(dupt->data, t->data, dupt->max_size * sizeof(void*));

    return dup;
}

netsnmp_container *
netsnmp_container_get_binary_array(void)
{
    /*
     * allocate memory
     */
    netsnmp_container *c = SNMP_MALLOC_TYPEDEF(netsnmp_container);
    if (NULL==c) {
        snmp_log(LOG_ERR, "couldn't allocate memory\n");
        return NULL;
    }

    c->container_data = netsnmp_binary_array_initialize();

    /*
     * NOTE: CHANGES HERE MUST BE DUPLICATED IN duplicate AS WELL!!
     */
    netsnmp_init_container(c, NULL, _ba_free, _ba_size, NULL, _ba_insert,
                           _ba_remove, _ba_find);
    c->find_next = _ba_find_next;
    c->get_subset = _ba_get_subset;
    c->get_iterator = _ba_iterator_get;
    c->for_each = _ba_for_each;
    c->clear = _ba_clear;
    c->options = _ba_options;
    c->duplicate = _ba_duplicate;
    c->get_at = netsnmp_binary_array_get_at;
    c->remove_at = netsnmp_binary_array_remove_at;
    c->insert_before = _ba_insert_before;

    return c;
}

netsnmp_factory *
netsnmp_container_get_binary_array_factory(void)
{
    static netsnmp_factory f = { "binary_array",
                                 netsnmp_container_get_binary_array };
    
    return &f;
}

void
netsnmp_container_binary_array_init(void)
{
    netsnmp_container_register("binary_array",
                               netsnmp_container_get_binary_array_factory());
}

/**********************************************************************
 *
 * iterator
 *
 */
NETSNMP_STATIC_INLINE binary_array_table *
_ba_it2cont(binary_array_iterator *it)
{
    if(NULL == it) {
        netsnmp_assert(NULL != it);
        return NULL;
    }
    if(NULL == it->base.container) {
        netsnmp_assert(NULL != it->base.container);
        return NULL;
    }
    if(NULL == it->base.container->container_data) {
        netsnmp_assert(NULL != it->base.container->container_data);
        return NULL;
    }

    return (binary_array_table*)(it->base.container->container_data);
}

NETSNMP_STATIC_INLINE void *
_ba_iterator_position(binary_array_iterator *it, size_t pos)
{
    binary_array_table *t = _ba_it2cont(it);
    if (NULL == t)
        return t; /* msg already logged */

    if(it->base.container->sync != it->base.sync) {
        DEBUGMSGTL(("container:iterator", "out of sync\n"));
        return NULL;
    }
    
    if(0 == t->count) {
        DEBUGMSGTL(("container:iterator", "empty\n"));
        return NULL;
    }
    else if(pos >= t->count) {
        DEBUGMSGTL(("container:iterator", "end of container\n"));
        return NULL;
    }

    return t->data[ pos ];
}

static void *
_ba_iterator_curr(binary_array_iterator *it)
{
    if(NULL == it) {
        netsnmp_assert(NULL != it);
        return NULL;
    }

    return _ba_iterator_position(it, it->pos);
}

static void *
_ba_iterator_first(binary_array_iterator *it)
{
    return _ba_iterator_position(it, 0);
}

static void *
_ba_iterator_next(binary_array_iterator *it)
{
    if(NULL == it) {
        netsnmp_assert(NULL != it);
        return NULL;
    }

    ++it->pos;

    return _ba_iterator_position(it, it->pos);
}

static void *
_ba_iterator_last(binary_array_iterator *it)
{
    binary_array_table* t = _ba_it2cont(it);
    if(NULL == t) {
        netsnmp_assert(NULL != t);
        return NULL;
    }
    
    return _ba_iterator_position(it, t->count - 1 );
}

static int
_ba_iterator_remove(binary_array_iterator *it)
{
    binary_array_table* t = _ba_it2cont(it);
    if(NULL == t) {
        netsnmp_assert(NULL != t);
        return -1;
    }

    /*
     * since this iterator was used for the remove, keep it in sync with
     * the container. Also, back up one so that next will be the position
     * that was just removed.
     */
    ++it->base.sync;
    return netsnmp_binary_array_remove_at(it->base.container, it->pos--, NULL);

}

static int
_ba_iterator_reset(binary_array_iterator *it)
{
    binary_array_table* t = _ba_it2cont(it);
    if(NULL == t) {
        netsnmp_assert(NULL != t);
        return -1;
    }

    if (t->dirty)
        Sort_Array(it->base.container);

    /*
     * save sync count, to make sure container doesn't change while
     * iterator is in use.
     */
    it->base.sync = it->base.container->sync;

    it->pos = 0;

    return 0;
}

static int
_ba_iterator_release(netsnmp_iterator *it)
{
    free(it);

    return 0;
}

static netsnmp_iterator *
_ba_iterator_get(netsnmp_container *c)
{
    binary_array_iterator* it;

    if(NULL == c)
        return NULL;

    it = SNMP_MALLOC_TYPEDEF(binary_array_iterator);
    if(NULL == it)
        return NULL;

    it->base.container = c;
    
    it->base.first = (netsnmp_iterator_rtn*)_ba_iterator_first;
    it->base.next = (netsnmp_iterator_rtn*)_ba_iterator_next;
    it->base.curr = (netsnmp_iterator_rtn*)_ba_iterator_curr;
    it->base.last = (netsnmp_iterator_rtn*)_ba_iterator_last;
    it->base.remove = (netsnmp_iterator_rc*)_ba_iterator_remove;
    it->base.reset = (netsnmp_iterator_rc*)_ba_iterator_reset;
    it->base.release = (netsnmp_iterator_rc*)_ba_iterator_release;

    (void)_ba_iterator_reset(it);

    return (netsnmp_iterator *)it;
}
