/*
 * testhandler.h 
 */
#ifndef _TABLE_HANDLER_H_
#define _TABLE_HANDLER_H_

#ifdef __cplusplus
extern          "C" {
#endif

    /*
     * The table helper is designed to simplify the task of writing a
     * * table handler for the net-snmp agent.  You should create a normal
     * * handler and register it using the netsnmp_register_table() function instead
     * * of the netsnmp_register_handler() function.
     */

    /*
     * Notes:
     *
     *   1) illegal indexes automatically get handled for get/set cases.
     *      Simply check to make sure the value is type ASN_NULL before
     *      you answer a request.
     */

    /*
     * used as an index to parent_data lookups 
     */
#define TABLE_HANDLER_NAME "table"

    /*
     * column info struct.  OVERLAPPING RANGES ARE NOT SUPPORTED.
     */
    typedef struct netsnmp_column_info_t {
        char            isRange;
        char            list_count;     /* only useful if isRange == 0 */

        union {
            unsigned int    range[2];
            unsigned int   *list;
        } details;

        struct netsnmp_column_info_t *next;

    } netsnmp_column_info;

    typedef struct netsnmp_table_registration_info_s {
        netsnmp_variable_list *indexes; /* list of varbinds with only 'type' set */
        unsigned int    number_indexes; /* calculated automatically */

        /*
         * the minimum and maximum columns numbers. If there are columns
         * * in-between which are not valid, use valid_columns to get
         * * automatic column range checking.
         */
        unsigned int    min_column;
        unsigned int    max_column;

        netsnmp_column_info *valid_columns;     /* more details on columns */

    } netsnmp_table_registration_info;

    typedef struct netsnmp_table_request_info_s {
        unsigned int    colnum; /* 0 if OID not long enough */
        unsigned int    number_indexes; /* 0 if failure to parse any */
        netsnmp_variable_list *indexes; /* contents freed by helper upon exit */
        oid             index_oid[MAX_OID_LEN];
        size_t          index_oid_len;
        netsnmp_table_registration_info *reg_info;
    } netsnmp_table_request_info;

    netsnmp_mib_handler
        *netsnmp_get_table_handler(netsnmp_table_registration_info
                                   *tabreq);
    int             netsnmp_register_table(netsnmp_handler_registration
                                           *reginfo,
                                           netsnmp_table_registration_info
                                           *tabreq);
    int             netsnmp_table_build_oid(netsnmp_handler_registration
                                            *reginfo,
                                            netsnmp_request_info *reqinfo,
                                            netsnmp_table_request_info
                                            *table_info);
    int            
        netsnmp_table_build_oid_from_index(netsnmp_handler_registration
                                           *reginfo,
                                           netsnmp_request_info *reqinfo,
                                           netsnmp_table_request_info
                                           *table_info);
    int             netsnmp_table_build_result(netsnmp_handler_registration
                                               *reginfo,
                                               netsnmp_request_info
                                               *reqinfo,
                                               netsnmp_table_request_info
                                               *table_info, u_char type,
                                               u_char * result,
                                               size_t result_len);
    int            
        netsnmp_update_variable_list_from_index(netsnmp_table_request_info
                                                *);
    int            
        netsnmp_update_indexes_from_variable_list
        (netsnmp_table_request_info *tri);
    netsnmp_table_registration_info
        *netsnmp_find_table_registration_info(netsnmp_handler_registration
                                              *reginfo);

    unsigned int    netsnmp_closest_column(unsigned int current,
                                           netsnmp_column_info
                                           *valid_columns);

    Netsnmp_Node_Handler table_helper_handler;

#define netsnmp_table_helper_add_index(tinfo, type) snmp_varlist_add_variable(&tinfo->indexes, NULL, 0, (u_char)type, NULL, 0);

#if HAVE_STDARG_H
    void           
        netsnmp_table_helper_add_indexes(netsnmp_table_registration_info
                                         *tinfo, ...);
#else
    void            netsnmp_table_helper_add_indexes(va_alist);
#endif

    int
     
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        netsnmp_check_getnext_reply(netsnmp_request_info *request,
                                    oid * prefix, size_t prefix_len,
                                    netsnmp_variable_list * newvar,
                                    netsnmp_variable_list ** outvar);

    netsnmp_table_request_info
        *netsnmp_extract_table_info(netsnmp_request_info *);
    netsnmp_oid_stash_node
        **netsnmp_table_get_or_create_row_stash(netsnmp_agent_request_info
                                                *reqinfo,
                                                const u_char *
                                                storage_name);

#ifdef __cplusplus
};
#endif

#endif                          /* _TABLE_HANDLER_H_ */
