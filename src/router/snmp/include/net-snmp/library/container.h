#ifndef NETSNMP_CONTAINER_H
#define NETSNMP_CONTAINER_H

/*
 * WARNING: This is a recently created file, and all of it's contents are
 *          subject to change at any time.
 *
 * A basic container template. A generic way for code to store and
 * retrieve data. Allows for interchangable storage algorithms.
 */
#ifndef NET_SNMP_CONFIG_H
#error "Please include <net-snmp/net-snmp-config.h> before this file"
#endif

#include <net-snmp/types.h>
#include <net-snmp/library/factory.h>

#ifdef  __cplusplus
extern "C" {
#endif

    /*************************************************************************
     *
     * function pointer definitions
     *
     *************************************************************************/
    struct netsnmp_iterator_s; /** forward declare */
    struct netsnmp_container_s; /** forward declare */

    /*
     * function returning an int for an operation on a container
     */
    typedef int (netsnmp_container_rc)(struct netsnmp_container_s *);

    /*
     * function returning an int for an operation on a container
     */
    typedef struct netsnmp_iterator_s * (netsnmp_container_it)
        (struct netsnmp_container_s *);

    /*
     * function returning a size_t for an operation on a container
     */
    typedef size_t (netsnmp_container_size)(struct netsnmp_container_s *);

    /*
     * function returning an int for an operation on an object and
     * a container
     */
    typedef int (netsnmp_container_op)(struct netsnmp_container_s *,
                                       const void *data);

    /*
     * function returning an oject for an operation on an object and a
     * container
     */
    typedef void * (netsnmp_container_rtn)(struct netsnmp_container_s *,
                                           const void *data);

    /*
     * function with no return which acts on an object
     */
    typedef void (netsnmp_container_obj_func)(void *data, void *context);

    /*
     * function with no return which acts on an object
     */
    typedef void (netsnmp_container_func)(struct netsnmp_container_s *,
                                          netsnmp_container_obj_func *,
                                          void *context);

    /*
     * function returning an array of objects for an operation on an
     * ojbect and a container
     */
    typedef netsnmp_void_array * (netsnmp_container_set)
        (struct netsnmp_container_s *, void *data);

    /*
     * function returning an int for a comparison between two objects
     */
    typedef int (netsnmp_container_compare)(const void *lhs,
                                            const void *rhs);

    /*************************************************************************
     *
     * Basic container
     *
     *************************************************************************/
    typedef struct netsnmp_container_s {
       
       /*
        * pointer for container
        */
       void *         private;

       /*
        * returns the number of items in a container
        */
       netsnmp_container_size  *get_size;
       
       /*
        * initialize a container
        */
       netsnmp_container_rc    *init;

       /*
        * release memory used by a container.
        *
        * Note: if your data structures contained allcoated
        * memory, you are responsible for releasing that
        * memory before calling this function!
        */
       netsnmp_container_rc    *cfree;

       /*
        * add an entry to the container
        */
       netsnmp_container_op    *insert;

       /*
        * remove an entry from the container
        */
       netsnmp_container_op    *remove;

       /*
        * Note: do not change the key!  If you need to
        * change a key, remove the entry, change the key,
        * and the re-add the entry.
        */

       /*
        * find the entry in the container with the same key
        *
        */
       netsnmp_container_rtn   *find;

       /*
        * find the first entry in the container with a key greater than
        * the specified key.
        *
        * If the key is NULL, return the first item in the container.
        */
       netsnmp_container_rtn   *find_next;

       /*
        * find all entries iin the container which match the partial key
        */
       netsnmp_container_set            *get_subset;

       /*
        * function to return an iterator for the container
        */
       struct netsnmp_container_it    *get_iterator;

       /*
        * function to call another function for each object in the container
        */
       netsnmp_container_func         *for_each;

       /*
        * function to compare two object stored in the container.
        *
        * Returns:
        *
        *   -1  LHS < RHS
        *    0  LHS = RHS
        *    1  LHS > RHS
        */
       netsnmp_container_compare        *compare;

       /*
        * same as compare, but RHS will be a partial key
        */
       netsnmp_container_compare        *ncompare;

       /*
        * containers can contain other containers (additional indexes)
        */
       struct netsnmp_container_s *next, *prev;

    } netsnmp_container;

    /*
     * initialize a container of container factories. used by
     * netsnmp_container_find* functions.
     */
    void netsnmp_container_init_list(void);

    /*
     * register a new container factory
     */
    int netsnmp_container_register(const char* name, netsnmp_factory *f);

    /*
     * search for and create a container from a list of types or a
     * specific type.
     */
    netsnmp_container * netsnmp_container_find(const char *type_list);
    netsnmp_container * netsnmp_container_get(const char *type);

    /*
     * search for and create a container from a list of types or a
     * specific type, using the provided container structure
     */
    int netsnmp_container_find_noalloc(const char *type_list,
                                       netsnmp_container* c);
    int netsnmp_container_get_noalloc(const char *type, netsnmp_container* c);

    /*
     * utility routines
     */
    void netsnmp_container_add_index(netsnmp_container *primary,
                                     netsnmp_container *new_index);


    /*
     * commone comparison routines
     */
    /** first data element is a 'netsnmp_index' */
    int netsnmp_compare_netsnmp_index(const void *lhs, const void *rhs);
    int netsnmp_ncompare_netsnmp_index(const void *lhs, const void *rhs);

    /** first data element is a 'char *' */
    int netsnmp_compare_cstring(const void * lhs, const void * rhs);
    int netsnmp_ncompare_cstring(const void * lhs, const void * rhs);

    /** useful for octet strings */
    int netsnmp_compare_mem(const char * lhs, size_t lhs_len,
                            const char * rhs, size_t rhs_len);


    /*
     * useful macros
     */
#define CONTAINER_FIRST(x)          (x)->find(x,NULL)
#define CONTAINER_FIND(x,k)         (x)->find(x,k)
#define CONTAINER_NEXT(x,k)         (x)->find_next(x,k)
#define CONTAINER_GET_SUBSET(x,k)   (x)->get_subset(x,k)
#define CONTAINER_SIZE(x)           (x)->get_size(x)
#define CONTAINER_ITERATOR(x)       (x)->get_iterator(x)
#define CONTAINER_COMPARE(x,l,r)    (x)->compare(l,r)
#define CONTAINER_FOR_EACH(x,f,c)   (x)->for_each(x,f,c)

    /*
     * if you are getting multiple definitions of these three
     * inline functions, you most likely have optimizations turned off for your
     * compiler (-O flag). Either turn them back on, or make sure that
     * NETSNMP_INLINE is not defined in net-snmp-config.h.
     */
#ifdef NETSNMP_NO_INLINE
    int CONTAINER_INSERT(netsnmp_container *x, const void *k);
    int CONTAINER_REMOVE(netsnmp_container *x, const void *k);
    int CONTAINER_FREE(netsnmp_container *x);
#else
    /*------------------------------------------------------------------
     * These functions should EXACTLY match the function version in
     * container.c. If you change one, change them both.
     */
    static NETSNMP_INLINE /* gcc docs recommend static w/inline */
    int CONTAINER_INSERT(netsnmp_container *x, const void *k)
    {
        int rc;
        
        rc = x->insert(x,k);
        if (NULL != x->next) {
            netsnmp_container *tmp = x->next;
            int                rc2;
            while(tmp) {
                rc2 = tmp->insert(tmp,k);
                if (rc2)
                    snmp_log(LOG_ERR,"error on subcontainer insert (%d)\n", rc2);
                tmp = tmp->next;
            }
        }
        return rc;
    }
    
    /*------------------------------------------------------------------
     * These functions should EXACTLY match the function version in
     * container.c. If you change one, change them both.
     */
    static NETSNMP_INLINE /* gcc docs recommend static w/inline */
    int CONTAINER_REMOVE(netsnmp_container *x, const void *k)
    {
        if (NULL != x->next) {
            netsnmp_container *tmp = x->next;
            int                rc;
            while(tmp->next)
                tmp = tmp->next;
            while(tmp) {
                rc = tmp->remove(tmp,k);
                if (rc)
                    snmp_log(LOG_ERR,"error on subcontainer remove (%d)\n", rc);
                tmp = tmp->prev;
            }
        }
        return x->remove(x,k);
    }
    
    /*------------------------------------------------------------------
     * These functions should EXACTLY match the function version in
     * container.c. If you change one, change them both.
     */
    static NETSNMP_INLINE /* gcc docs recommend static w/inline */
    int CONTAINER_FREE(netsnmp_container *x)
    {
        if (NULL != x->next) {
            netsnmp_container *prev, *tmp = x->next;
            int                rc;
            while(tmp->next)
                tmp = tmp->next;
            while(tmp) {
                prev = tmp->prev;
                rc = tmp->cfree(tmp);
                if (rc)
                    snmp_log(LOG_ERR,"error on subcontainer cfree (%d)\n", rc);
                tmp = prev;
            }
        }
        return x->cfree(x);
    }
#endif

    /*************************************************************************
     *
     * container iterator
     *
     *************************************************************************/
    /*
     * function returning an int for an operation on an iterator
     */
    typedef int (netsnmp_iterator_rc)(struct netsnmp_iterator_s *);

    /*
     * function returning an int for an operation on an iterator and
     * an object in the container.
     */
    typedef int (netsnmp_iterator_rc_op)(struct netsnmp_iterator_s *,
                                         void *data);

    /*
     * function returning an oject for an operation on an iterator
     */
    typedef void * (netsnmp_iterator_rtn)(struct netsnmp_iterator_s *);

    typedef struct netsnmp_iterator_s {

       netsnmp_container              *container;

       void                           *context;

       netsnmp_iterator_rc           *init;
       netsnmp_iterator_rc_op        *position;
       netsnmp_iterator_rtn          *first;
       netsnmp_iterator_rtn          *next;
       netsnmp_iterator_rtn          *last;

    } netsnmp_iterator;


#define ITERATOR_FIRST(x)  x->first(x)
#define ITERATOR_NEXT(x)   x->next(x)
#define ITERATOR_LAST(x)   x->last(x)


    /*************************************************************************
     *
     * Sorted container
     *
     *************************************************************************/
    typedef struct netsnmp_sorted_container_s {
       
       netsnmp_container                bc;
       
       /*
        * methods to manipulate container
        */

       netsnmp_container_rtn            *first;
       netsnmp_container_rtn            *next;
       netsnmp_container_set            *subset;
       
    } netsnmp_sorted_container;
    

    void
    netsnmp_init_sorted_container(netsnmp_sorted_container  *sc,
                                  netsnmp_container_rtn     *first,
                                  netsnmp_container_rtn     *next,
                                  netsnmp_container_set     *subset);
    
    
    
#ifdef  __cplusplus
};
#endif

#endif /** NETSNMP_CONTAINER_H */
