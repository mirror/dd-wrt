/*
 * netsnmp_data_list.h
 *
 * $Id: data_list.h,v 1.1.2.1 2004/06/20 21:54:38 nikki Exp $
 *
 * External definitions for functions and variables in netsnmp_data_list.c.
 */

#ifndef DATA_LIST_H
#define DATA_LIST_H

#ifdef __cplusplus
extern          "C" {
#endif

#include <net-snmp/library/snmp_impl.h>
#include <net-snmp/library/tools.h>

    typedef void    (Netsnmp_Free_List_Data) (void *);

    typedef struct netsnmp_data_list_s {
        struct netsnmp_data_list_s *next;
        char           *name;
        void           *data;   /* The pointer to the data passed on. */
        Netsnmp_Free_List_Data *free_func;      /* must know how to free netsnmp_data_list->data */
    } netsnmp_data_list;


    NETSNMP_INLINE netsnmp_data_list *netsnmp_create_data_list(const char *,
                                                       void *,
                                                       Netsnmp_Free_List_Data
                                                       *);
    void            netsnmp_add_list_data(netsnmp_data_list **head,
                                          netsnmp_data_list *node);
    void           *netsnmp_get_list_data(netsnmp_data_list *head,
                                          const char *node);
    void            netsnmp_free_list_data(netsnmp_data_list *head);    /* single */
    void            netsnmp_free_all_list_data(netsnmp_data_list *head);        /* multiple */
    int             netsnmp_remove_list_node(netsnmp_data_list **realhead,
                                             const char *name);
    NETSNMP_INLINE void    *netsnmp_get_list_node(netsnmp_data_list *head,
                                          const char *name);


#ifdef __cplusplus
}
#endif
#endif
