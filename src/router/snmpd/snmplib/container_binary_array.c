/*
 * container_binary_array.c
 * $Id: container_binary_array.c,v 1.24 2006/08/23 15:53:22 tanders Exp $
 *
 * see comments in header file.
 *
 */

#include <net-snmp/net-snmp-config.h>

#if HAVE_IO_H
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

typedef struct binary_array_table_s {
    size_t                     max_size;   /* Size of the current data table */
    size_t                     count;      /* Index of the next free entry */
    u_int                      flags;      /* flags */
    int                        dirty;
    int                        data_size;  /* Size of an individual entry */
    void                     **data;       /* The table itself */
} binary_array_table;

typedef struct binary_array_iterator_s {
    netsnmp_iterator base;

    size_t           pos;
} binary_array_iterator;

static netsnmp_iterator *_ba_iterator_get(netsnmp_container *c);

/**********************************************************************
 *
 * 
 *
 */
static void
array_qsort(void **data, int first, int last, netsnmp_container_compare *f)
{
    int i, j;
    void *mid, *tmp;
    
    i = first;
    j = last;
    mid = data[(first+last)/2];
    
    do {
        while ( ((*f)(data[i], mid) < 0) && (i < last))
            ++i;
        while ( ((*f)(mid, data[j]) < 0) && (j > first))
            --j;

        if(i < j) {
            tmp = data[i];
            data[i] = data[j];
            data[j] = tmp;
            ++i;
            --j;
        }
        else if (i == j) {
            ++i;
            --j;
            break;
        }
    } while(i <= j);

    if (j > first)
        array_qsort(data, first, j, f);
    
    if (i < last)
        array_qsort(data, i, last, f);
}

static int
Sort_Array(netsnmp_container *c)
{
    binary_array_table *t = (binary_array_table*)c->container_data;
    netsnmp_assert(t!=NULL);
    netsnmp_assert(c->compare!=NULL);

    if (t->flags & CONTAINER_KEY_UNSORTED)
        return 0;

    if (t->dirty) {
        /*
         * Sort the table 
         */
        if (t->count > 1)
            array_qsort(t->data, 0, t->count - 1, c->compare);
        t->dirty = 0;

        ++c->sync;
    }

    return 1;
}

static int
binary_search(const void *val, netsnmp_container *c, int exact)
{
    binary_array_table *t = (binary_array_table*)c->container_data;
    size_t             len = t->count;
    size_t             half;
    size_t             middle = 0;
    size_t             first = 0;
    int                result = 0;

    if (!len)
        return -1;

    if (t->dirty)
        Sort_Array(c);

    while (len > 0) {
        half = len >> 1;
        middle = first;
        middle += half;
        if ((result =
             c->compare(t->data[middle], val)) < 0) {
            first = middle;
            ++first;
            len = len - half - 1;
        } else {
            if(result == 0) {
                first = middle;
                break;
            }
            len = half;
        }
    }

    if (first >= t->count)
        return -1;

    if(first != middle) {
        /* last compare wasn't against first, so get actual result */
        result = c->compare(t->data[first], val);
    }

    if(result == 0) {
        if (!exact) {
            if (++first == t->count)
               first = -1;
        }
    }
    else {
        if(exact)
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
    t->data_size = sizeof(void*);
    t->data = NULL;

    return t;
}

void
netsnmp_binary_array_release(netsnmp_container *c)
{
    binary_array_table *t = (binary_array_table*)c->container_data;
    if (t->data != NULL) {
	SNMP_FREE(t->data);
    }
    SNMP_FREE(t);
    SNMP_FREE(c);
}

int
netsnmp_binary_array_options_set(netsnmp_container *c, int set, u_int flags)
{
    binary_array_table *t = (binary_array_table*)c->container_data;
    if (set)
        t->flags = flags;
    else
        return ((t->flags & flags) == flags);
    return flags;
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
        return 0;

    /*
     * if the table is dirty, sort it.
     */
    if (t->dirty)
        Sort_Array(c);

    /*
     * if there is a key, search. Otherwise default is 0;
     */
    if (key) {
        if ((index = binary_search(key, c, exact)) == -1)
            return 0;
    }

    return t->data[index];
}

NETSNMP_STATIC_INLINE int
netsnmp_binary_array_replace(netsnmp_container *c, void *entry)
{
    binary_array_table *t = (binary_array_table*)c->container_data;
    int             index = 0;

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
    if ((index = binary_search(entry, c, 1)) == -1)
        return 0;

    t->data[index] = entry;

    return 0;
}

int
netsnmp_binary_array_remove(netsnmp_container *c, const void *key, void **save)
{
    binary_array_table *t = (binary_array_table*)c->container_data;
    size_t             index = 0;

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
    if ((index = binary_search(key, c, 1)) == -1)
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
        memmove(&t->data[index], &t->data[index+1], t->data_size * (t->count - index));
    }

    return 0;
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

NETSNMP_STATIC_INLINE int
netsnmp_binary_array_insert(netsnmp_container *c, const void *entry)
{
    binary_array_table *t = (binary_array_table*)c->container_data;
    int             new_max;
    void           *new_data;   /* Used for * a) extending the data table
                                 * * b) the next entry to use */
    /*
     * check for duplicates
     */
    if (! (t->flags & CONTAINER_KEY_ALLOW_DUPLICATES)) {
        new_data = netsnmp_binary_array_get(c, entry, 1);
        if (NULL != new_data) {
            DEBUGMSGTL(("container","not inserting duplicate key\n"));
            return -1;
        }
    }
    
    /*
     * check if we need to resize the array
     */
    if (t->max_size <= t->count) {
        /*
         * Table is full, so extend it to double the size
         */
        new_max = 2 * t->max_size;
        if (new_max == 0)
            new_max = 10;       /* Start with 10 entries */

        new_data = (void *) calloc(new_max, t->data_size);
        if (new_data == NULL)
            return -1;

        if (t->data) {
            memcpy(new_data, t->data, t->max_size * t->data_size);
            SNMP_FREE(t->data);
        }
        t->data = (void**)new_data;
        t->max_size = new_max;
    }

    /*
     * Insert the new entry into the data array
     */
    t->data[t->count++] = (void *)entry;
    t->dirty = 1;
    return 0;
}

NETSNMP_STATIC_INLINE void           *
netsnmp_binary_array_retrieve(netsnmp_container *c, int *max_oids, int sort)
{
    binary_array_table *t = (binary_array_table*)c->container_data;
    if (sort && t->dirty)
        Sort_Array(c);

    *max_oids = t->count;
    return t->data;
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
        middle = first;
        middle += half;
        if ((result = c->ncompare(t->data[middle], val)) < 0) {
            first = middle;
            ++first;
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
    binary_array_table *t = (binary_array_table*)c->container_data;
    void          **subset;
    int             start, end;
    size_t          i;

    /*
     * if there is no data, return NULL;
     */
    if (!t->count || !key)
        return 0;

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
        return 0;

    for (i = start + 1; i < t->count; ++i) {
        if (0 != c->ncompare(t->data[i], key))
            break;
        ++end;
    }

    *len = end - start + 1;
    subset = (void **)malloc((*len) * t->data_size);
    if (subset)
        memcpy(subset, &t->data[start], t->data_size * (*len));

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
    if ((NULL==rtn) || (len <=0))
        return NULL;
    
    va = SNMP_MALLOC_TYPEDEF(netsnmp_void_array);
    if (NULL==va)
        return NULL;

    va->size = len;
    va->array = rtn;

    return va;
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
        
    c->get_size = _ba_size;
    c->init = NULL;
    c->cfree = _ba_free;
    c->insert = _ba_insert;
    c->remove = _ba_remove;
    c->find = _ba_find;
    c->find_next = _ba_find_next;
    c->get_subset = _ba_get_subset;
    c->get_iterator = _ba_iterator_get;
    c->for_each = _ba_for_each;
    c->clear = _ba_clear;
        
    return c;
}

netsnmp_factory *
netsnmp_container_get_binary_array_factory(void)
{
    static netsnmp_factory f = { "binary_array",
                                 (netsnmp_factory_produce_f*)
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
        DEBUGMSGTL(("container:iterator", "end of containter\n"));
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
    it->base.reset = (netsnmp_iterator_rc*)_ba_iterator_reset;
    it->base.release = (netsnmp_iterator_rc*)_ba_iterator_release;

    (void)_ba_iterator_reset(it);

    return (netsnmp_iterator *)it;
}
