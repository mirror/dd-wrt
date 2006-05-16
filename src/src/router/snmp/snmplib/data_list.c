/*
 * netsnmp_data_list.c
 *
 * $Id: data_list.c,v 1.1.2.1 2004/06/20 21:55:00 nikki Exp $
 */
#include <net-snmp/net-snmp-config.h>
#include <sys/types.h>
#include <stdlib.h>

#if HAVE_STRING_H
#include <string.h>
#else
#include <strings.h>
#endif

#if HAVE_DMALLOC_H
#include <dmalloc.h>
#endif

#include <net-snmp/library/data_list.h>

/***********************************************************************/
/*
 * New Handler based API 
 */
/***********************************************************************/

NETSNMP_INLINE void
netsnmp_free_list_data(netsnmp_data_list *node)
{
    Netsnmp_Free_List_Data *beer;
    if (!node)
        return;

    beer = node->free_func;
    if (beer)
        (beer) (node->data);
    SNMP_FREE(node->name);
}

NETSNMP_INLINE void
netsnmp_free_all_list_data(netsnmp_data_list *head)
{
    netsnmp_data_list *tmpptr;
    for (; head;) {
        netsnmp_free_list_data(head);
        tmpptr = head;
        head = head->next;
        SNMP_FREE(tmpptr);
    }
}

NETSNMP_INLINE netsnmp_data_list *
netsnmp_create_data_list(const char *name, void *data,
                         Netsnmp_Free_List_Data * beer)
{
    netsnmp_data_list *node = SNMP_MALLOC_TYPEDEF(netsnmp_data_list);
    if (!node)
        return NULL;
    node->name = strdup(name);
    node->data = data;
    node->free_func = beer;
    return node;
}


NETSNMP_INLINE void
netsnmp_add_list_data(netsnmp_data_list **head, netsnmp_data_list *node)
{
    netsnmp_data_list *ptr;
    if (!*head) {
        *head = node;
        return;
    }

    /*
     * xxx-rks: check for duplicate names? 
     */
    for (ptr = *head; ptr->next != NULL; ptr = ptr->next) {
        /*
         * noop 
         */
    }

    if (ptr)                    /* should always be true */
        ptr->next = node;
}

NETSNMP_INLINE void    *
netsnmp_get_list_data(netsnmp_data_list *head, const char *name)
{
    for (; head; head = head->next)
        if (head->name && strcmp(head->name, name) == 0)
            break;
    if (head)
        return head->data;
    return NULL;
}

NETSNMP_INLINE void    *
netsnmp_get_list_node(netsnmp_data_list *head, const char *name)
{
    for (; head; head = head->next)
        if (head->name && strcmp(head->name, name) == 0)
            break;
    if (head)
        return head;
    return NULL;
}

int
netsnmp_remove_list_node(netsnmp_data_list **realhead, const char *name)
{
    netsnmp_data_list *head, *prev;
    for (head = *realhead, prev = NULL; head;
         prev = head, head = head->next) {
        if (head->name && strcmp(head->name, name) == 0) {
            if (prev)
                prev->next = head->next;
            else
                *realhead = head->next;
            netsnmp_free_list_data(head);
            free(head);
            return 0;
        }
    }
    return 1;
}
