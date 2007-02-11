/*
 * container_list_sl.c
 * $Id: container_list_ssll.c,v 1.7 2004/09/09 10:43:40 slif Exp $
 *
 */
#include <net-snmp/net-snmp-config.h>

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
#include <net-snmp/library/tools.h>
#include <net-snmp/library/snmp_assert.h>

#include <net-snmp/library/container_list_ssll.h>

typedef struct sl_node {
   void           *data;
   struct sl_node *next;
} sl_node;

typedef struct sl_container_s {
   netsnmp_container          c;
   
   size_t                     count;      /* Index of the next free entry */
   sl_node                   *head;       /* head of list */

   int                        unsorted;   /* unsorted list? */
   int                        fifo;       /* lifo or fifo? */

} sl_container;


static void *
_get(netsnmp_container *c, const void *key, int exact)
{
    sl_container *sl = (sl_container*)c;
    sl_node  *curr = sl->head;
    int rc = 0;
    
    /*
     * note: get-next on unsorted list is meaningless. we
     * don't try to search whole array, looking for the next highest.
     */
    if( (NULL != curr) && (NULL != key)) {
        while (curr) {
            rc = sl->c.compare(curr->data, key);
            if (rc == 0)
                break;
            else if (rc > 0) {
                if (0 == sl->unsorted) {
                    /*
                     * if sorted, we can stop.
                     */
                    break;
                }
            }
            curr = curr->next;
        }
        
        if((curr) && (!exact) && (rc == 0)) {
            curr = curr->next;
        }
    }
    
    return curr ? curr->data : NULL;
}

/**********************************************************************
 *
 *
 *
 **********************************************************************/
static void
_ssll_free(netsnmp_container *c)
{
    if(c) {
        free(c);
    }
}

static void *
_ssll_find(netsnmp_container *c, const void *data)
{
    if((NULL == c) || (NULL == data))
        return NULL;

    return _get(c, data, 1);
}

static void *
_ssll_find_next(netsnmp_container *c, const void *data)
{
    if(NULL == c)
        return NULL;

    return _get(c, data, 0);
}

static int
_ssll_insert(netsnmp_container *c, const void *data)
{
    sl_container *sl = (sl_container*)c;
    sl_node  *new_node, *curr = sl->head;
    
    if(NULL == c)
        return -1;
    
    new_node = SNMP_MALLOC_TYPEDEF(sl_node);
    if(NULL == new_node)
        return -1;
    new_node->data = (void *)data;
    ++sl->count;

    /*
     * first node?
     */
    if(NULL == sl->head) {
        sl->head = new_node;
        return 0;
    }

    /*
     * sorted or unsorted insert?
     */
    if (1 == sl->unsorted) {
        /*
         * unsorted: fifo, or lifo?
         */
        if (1 == sl->fifo) {
            /*
             * fifo: insert at tail
             */
            while(NULL != curr->next)
                curr = curr->next;
            curr->next = new_node;
        }
        else {
            /*
             * lifo: insert at head
             */
            new_node->next = sl->head;
            sl->head = new_node;
        }
    }
    else {
        /*
         * sorted
         */
        sl_node *last = NULL;
        for( ; curr; last = curr, curr = curr->next) {
            if(sl->c.compare(curr->data, data) > 0)
                break;
        }
        if(NULL == last) {
            new_node->next = sl->head;
            sl->head = new_node;
        }
        else {
            new_node->next = last->next;
            last->next = new_node;
        }
    }
    
    return 0;
}

static int
_ssll_remove(netsnmp_container *c, const void *data)
{
    sl_container *sl = (sl_container*)c;
    sl_node  *curr = sl->head;
    
    if((NULL == c) || (NULL == curr))
        return -1;
    
    /*
     * special case for NULL data, act like stack
     */
    if ((NULL == data) ||
        (sl->c.compare(sl->head->data, data) == 0)) {
        curr = sl->head;
        sl->head = sl->head->next;
    }
    else {
        sl_node *last = sl->head;
        int rc;
        for(curr = sl->head->next ; curr; last = curr, curr = curr->next) {
            rc = sl->c.compare(curr->data, data);
            if (rc == 0) {
                last->next = curr->next;
                break;
            }
            else if ((rc > 0) && (0 == sl->unsorted)) {
                /*
                 * if sorted and rc > 0, didn't find entry
                 */
                curr = NULL;
                break;
            }
        }
    }

    if(NULL == curr)
        return -2;
    
    /*
     * free our node structure, but not the data
     */
    free(curr);
    --sl->count;
    
    return 0;
}

static size_t
_ssll_size(netsnmp_container *c)
{
    sl_container *sl = (sl_container*)c;
    
    if(NULL == c)
        return 0;

    return sl->count;
}

static void
_ssll_for_each(netsnmp_container *c, netsnmp_container_obj_func *f,
             void *context)
{
    sl_container *sl = (sl_container*)c;
    sl_node  *curr;
    
    if(NULL == c)
        return;
    
    for(curr = sl->head; curr; curr = curr->next)
        (*f) ((void *)curr->data, context);
}

static void
_ssll_clear(netsnmp_container *c, netsnmp_container_obj_func *f,
             void *context)
{
    sl_container *sl = (sl_container*)c;
    sl_node  *curr, *next;
    
    if(NULL == c)
        return;
    
    for(curr = sl->head; curr; curr = next) {

        next = curr->next;

        if( NULL != f ) {
            curr->next = NULL;
            (*f) ((void *)curr->data, context);
        }

        /*
         * free our node structure, but not the data
         */
        free(curr);
    }
    sl->head = NULL;
    sl->count = 0;
}

/**********************************************************************
 *
 *
 *
 **********************************************************************/
netsnmp_container *
netsnmp_container_get_ssll(void)
{
    /*
     * allocate memory
     */
    sl_container *sl = SNMP_MALLOC_TYPEDEF(sl_container);
    if (NULL==sl) {
        snmp_log(LOG_ERR, "couldn't allocate memory\n");
        return NULL;
    }

    sl->c.cfree = (netsnmp_container_rc*)_ssll_free;
        
    sl->c.get_size = _ssll_size;
    sl->c.init = NULL;
    sl->c.insert = _ssll_insert;
    sl->c.remove = _ssll_remove;
    sl->c.find = _ssll_find;
    sl->c.find_next = _ssll_find_next;
    sl->c.get_subset = NULL;
    sl->c.get_iterator = NULL;
    sl->c.for_each = _ssll_for_each;
    sl->c.clear = _ssll_clear;

       
    return (netsnmp_container*)sl;
}

netsnmp_factory *
netsnmp_container_get_ssll_factory(void)
{
    static netsnmp_factory f = {"sorted_singly_linked_list",
                                (netsnmp_factory_produce_f*)
                                netsnmp_container_get_ssll };
    
    return &f;
}


netsnmp_container *
netsnmp_container_get_usll(void)
{
    /*
     * allocate memory
     */
    sl_container *sl = (sl_container *)netsnmp_container_get_ssll();
    if (NULL==sl)
        return NULL; /* msg already logged */

    sl->unsorted = 1;

    return (netsnmp_container*)sl;
}

netsnmp_container *
netsnmp_container_get_singly_linked_list(int fifo)
{
    sl_container *sl = (sl_container *)netsnmp_container_get_usll();
    if (NULL == sl)
        return NULL; /* error already logged */

    sl->fifo = fifo;

    return (netsnmp_container *)sl;
}

netsnmp_container *
netsnmp_container_get_fifo(void)
{
    return netsnmp_container_get_singly_linked_list(1);
}

netsnmp_factory *
netsnmp_container_get_usll_factory(void)
{
    static netsnmp_factory f = {"unsorted_singly_linked_list-lifo",
                                (netsnmp_factory_produce_f*)
                                netsnmp_container_get_usll };
    
    return &f;
}

netsnmp_factory *
netsnmp_container_get_fifo_factory(void)
{
    static netsnmp_factory f = {"unsorted_singly_linked_list-fifo",
                                (netsnmp_factory_produce_f*)
                                netsnmp_container_get_fifo };
    
    return &f;
}

void
netsnmp_container_ssll_init(void)
{
    netsnmp_container_register("sorted_singly_linked_list",
                               netsnmp_container_get_ssll_factory());
    netsnmp_container_register("unsorted_singly_linked_list",
                               netsnmp_container_get_usll_factory());
    netsnmp_container_register("lifo",
                               netsnmp_container_get_usll_factory());
    netsnmp_container_register("fifo",
                               netsnmp_container_get_fifo_factory());
}

