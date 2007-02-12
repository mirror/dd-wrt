#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/library/container.h>
#include <net-snmp/library/container_binary_array.h>

/*------------------------------------------------------------------
 */
static netsnmp_container *containers = NULL;

typedef struct container_type_s {
   const char                 *name;
   netsnmp_factory            *factory;
} container_type;


/*------------------------------------------------------------------
 */
void
netsnmp_container_init_list(void)
{
    container_type *ct;
    
    if (NULL != containers)
        return;

    containers = netsnmp_container_get_binary_array();
    containers->compare = netsnmp_compare_cstring;

    /*
     */
    ct = SNMP_MALLOC_TYPEDEF(container_type);
    if (NULL == ct)
        return;
    ct->name = "binary_array";
    ct->factory = netsnmp_container_get_binary_array_factory();
    CONTAINER_INSERT(containers, ct);

    netsnmp_container_register("table_container", ct->factory);
}

int
netsnmp_container_register(const char* name, netsnmp_factory *f)
{
    container_type *ct, tmp;

    tmp.name = name;
    ct = CONTAINER_FIND(containers, &tmp);
    if (NULL!=ct) {
        DEBUGMSGT(("container_registry",
                   "replacing previous container factory\n"));
        ct->factory = f;
    }
    else {
        ct = SNMP_MALLOC_TYPEDEF(container_type);
        if (NULL == ct)
            return -1;
        ct->name = strdup(name);
        ct->factory = f;
        CONTAINER_INSERT(containers, ct);
    }
    DEBUGMSGT(("container_registry", "registered container factory %s (%s)\n",
               ct->name, f->product));

    return 0;
}

/*------------------------------------------------------------------
 */
netsnmp_factory *
netsnmp_container_get_factory(const char *type)
{
    container_type ct, *found;
    
    ct.name = type;
    found = CONTAINER_FIND(containers, &ct);

    return found ? found->factory : NULL;
}

netsnmp_factory *
netsnmp_container_find_factory(const char *type_list)
{
    netsnmp_factory   *f = NULL;
    char              *list, *entry;

    if (NULL==type_list)
        return NULL;

    list = strdup(type_list);
    entry = strtok(list, ":");
    while(entry) {
        f = netsnmp_container_get_factory(entry);
        if (NULL != f)
            break;
        entry = strtok(NULL, ":");
    }

    free(list);
    return f;
}

/*------------------------------------------------------------------
 */
netsnmp_container *
netsnmp_container_get(const char *type)
{
    netsnmp_factory *f = netsnmp_container_get_factory(type);
    if (f)
        return f->produce();

    return NULL;
}

int
netsnmp_container_get_noalloc(const char *type, netsnmp_container *mem)
{
    netsnmp_factory *f = netsnmp_container_get_factory(type);
    if (f)
        return f->produce_noalloc(mem);

    return FACTORY_NOTFOUND;
}

/*------------------------------------------------------------------
 */
netsnmp_container *
netsnmp_container_find(const char *type)
{
    netsnmp_factory *f = netsnmp_container_find_factory(type);
    if (f)
        return f->produce();

    return NULL;
}

int
netsnmp_container_find_noalloc(const char *type, netsnmp_container *mem)
{
    netsnmp_factory *f = netsnmp_container_find_factory(type);
    if (f)
        return f->produce_noalloc(mem);

    return FACTORY_NOTFOUND;
}

/*------------------------------------------------------------------
 */
void
netsnmp_container_add_index(netsnmp_container *primary,
                            netsnmp_container *new_index)
{
    while(primary->next)
        primary = primary->next;

    primary->next = new_index;
}

#ifdef NETSNMP_NO_INLINE /* default is to inline */

/*------------------------------------------------------------------
 * These functions should EXACTLY match the inline version in
 * container.h. If you chance one, change them both.
 */
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
 * These functions should EXACTLY match the inline version in
 * container.h. If you chance one, change them both.
 */
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
 * These functions should EXACTLY match the inline version in
 * container.h. If you chance one, change them both.
 */
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
                snmp_log(LOG_ERR,"error on subcontainer free (%d)\n", rc);
            tmp = prev;
        }
    }
    return x->cfree(x);
}
#endif


/*------------------------------------------------------------------
 */
void
netsnmp_init_container(netsnmp_container         *c,
                       netsnmp_container_rc      *init,
                       netsnmp_container_rc      *cfree,
                       netsnmp_container_size    *size,
                       netsnmp_container_compare *cmp,
                       netsnmp_container_op      *ins,
                       netsnmp_container_op      *rem,
                       netsnmp_container_rtn     *fnd)
{
    if (c == NULL)
        return;

    c->init = init;
    c->cfree = cfree;
    c->get_size = size;
    c->compare = cmp;
    c->insert = ins;
    c->remove = rem;
    c->find = fnd;
}

/*------------------------------------------------------------------
 *
 * simple comparison routines
 *
 */
int
netsnmp_compare_netsnmp_index(const void *lhs, const void *rhs)
{
    int rc;
#ifndef NDEBUG
    DEBUGIF("compare:index") {
        DEBUGMSGT(("compare:index", "compare "));
        DEBUGMSGOID(("compare:index", ((const netsnmp_index *) lhs)->oids,
                     ((const netsnmp_index *) lhs)->len));
        DEBUGMSG(("compare:index", " to "));
        DEBUGMSGOID(("compare:index", ((const netsnmp_index *) rhs)->oids,
                     ((const netsnmp_index *) rhs)->len));
        DEBUGMSG(("compare:index", "\n"));
    }
#endif
    rc = snmp_oid_compare(((const netsnmp_index *) lhs)->oids,
                          ((const netsnmp_index *) lhs)->len,
                          ((const netsnmp_index *) rhs)->oids,
                          ((const netsnmp_index *) rhs)->len);
    return rc;
}

int
netsnmp_ncompare_netsnmp_index(const void *lhs, const void *rhs)
{
    int rc;
#ifndef NDEBUG
    DEBUGIF("compare:index") {
        DEBUGMSGT(("compare:index", "compare "));
        DEBUGMSGOID(("compare:index", ((const netsnmp_index *) lhs)->oids,
                     ((const netsnmp_index *) lhs)->len));
        DEBUGMSG(("compare:index", " to "));
        DEBUGMSGOID(("compare:index", ((const netsnmp_index *) rhs)->oids,
                     ((const netsnmp_index *) rhs)->len));
        DEBUGMSG(("compare:index", "\n"));
    }
#endif
    rc = snmp_oid_ncompare(((const netsnmp_index *) lhs)->oids,
                           ((const netsnmp_index *) lhs)->len,
                           ((const netsnmp_index *) rhs)->oids,
                           ((const netsnmp_index *) rhs)->len,
                           ((const netsnmp_index *) rhs)->len);
    return rc;
}

int
netsnmp_compare_cstring(const void * lhs, const void * rhs)
{
    return strcmp(((const container_type*)lhs)->name,
                  ((const container_type*)rhs)->name);
}

int
netsnmp_ncompare_cstring(const void * lhs, const void * rhs)
{
    return strncmp(((const container_type*)lhs)->name,
                   ((const container_type*)rhs)->name,
                   strlen(((const container_type*)rhs)->name));
}

int
netsnmp_compare_mem(const char * lhs, size_t lhs_len,
                    const char * rhs, size_t rhs_len)
{
    int rc, min = SNMP_MIN(lhs_len, rhs_len);

    rc = memcmp(lhs, rhs, min);
    if((rc==0) && (lhs_len != rhs_len)) {
        if(lhs_len < rhs_len)
            rc = -1;
        else
            rc = 1;
    }

    return rc;
}
