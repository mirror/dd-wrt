#ifndef AGENT_HANDLER_H
#define AGENT_HANDLER_H

#ifdef __cplusplus
extern          "C" {
#endif

/** @addgroup handler */

    struct netsnmp_handler_registration_s;

    typedef struct netsnmp_mib_handler_s {
        char           *handler_name;
        void           *myvoid; /* for handler's internal use */

        int             (*access_method) (struct netsnmp_mib_handler_s *,
                                          struct
                                          netsnmp_handler_registration_s *,
                                          struct
                                          netsnmp_agent_request_info_s *,
                                          struct netsnmp_request_info_s *);

        struct netsnmp_mib_handler_s *next;
        struct netsnmp_mib_handler_s *prev;
    } netsnmp_mib_handler;

#define HANDLER_CAN_GETANDGETNEXT     0x1       /* must be able to do both */
#define HANDLER_CAN_SET               0x2
#define HANDLER_CAN_GETBULK           0x4

#define HANDLER_CAN_RONLY   (HANDLER_CAN_GETANDGETNEXT)
#define HANDLER_CAN_RWRITE  (HANDLER_CAN_GETANDGETNEXT | HANDLER_CAN_SET)
#define HANDLER_CAN_DEFAULT HANDLER_CAN_RONLY

    /*
     * root registration info 
     */
    typedef struct netsnmp_handler_registration_s {

        char           *handlerName;    /* for mrTable listings, and other uses */
        char           *contextName;    /* NULL = default context */

        /*
         * where are we registered at? 
         */
        oid            *rootoid;
        size_t          rootoid_len;

        /*
         * handler details 
         */
        netsnmp_mib_handler *handler;
        int             modes;

        /*
         * more optional stuff 
         */
        int             priority;
        int             range_subid;
        oid             range_ubound;
        int             timeout;
        int             global_cacheid;

    } netsnmp_handler_registration;

    /*
     * function handler definitions 
     */
    typedef int     (Netsnmp_Node_Handler) (netsnmp_mib_handler *handler, netsnmp_handler_registration *reginfo,        /* pointer to registration struct */
                                            netsnmp_agent_request_info *reqinfo,        /* pointer to current transaction */
                                            netsnmp_request_info
                                            *requests);

    typedef struct netsnmp_delegated_cache_s {
        int             transaction_id;
        netsnmp_mib_handler *handler;
        netsnmp_handler_registration *reginfo;
        netsnmp_agent_request_info *reqinfo;
        netsnmp_request_info *requests;
        void           *localinfo;
    } netsnmp_delegated_cache;

    /*
     * handler API functions 
     */
    void            netsnmp_init_handler_conf(void);
    int             netsnmp_register_handler(netsnmp_handler_registration
                                             *reginfo);
    int            
        netsnmp_register_handler_nocallback(netsnmp_handler_registration
                                            *reginfo);
    int             netsnmp_inject_handler(netsnmp_handler_registration
                                           *reginfo,
                                           netsnmp_mib_handler *handler);
    netsnmp_mib_handler
        *netsnmp_find_handler_by_name(netsnmp_handler_registration
                                      *reginfo, const char *name);
    void          
        *netsnmp_find_handler_data_by_name(netsnmp_handler_registration
                                           *reginfo, const char *name);
    int             netsnmp_call_handlers(netsnmp_handler_registration
                                          *reginfo,
                                          netsnmp_agent_request_info
                                          *reqinfo,
                                          netsnmp_request_info *requests);
    int             netsnmp_call_handler(netsnmp_mib_handler *next_handler,
                                         netsnmp_handler_registration
                                         *reginfo,
                                         netsnmp_agent_request_info
                                         *reqinfo,
                                         netsnmp_request_info *requests);
    int             netsnmp_call_next_handler(netsnmp_mib_handler *current,
                                              netsnmp_handler_registration
                                              *reginfo,
                                              netsnmp_agent_request_info
                                              *reqinfo,
                                              netsnmp_request_info
                                              *requests);
    int             netsnmp_call_next_handler_one_request(netsnmp_mib_handler *current,
                                                          netsnmp_handler_registration *reginfo,
                                                          netsnmp_agent_request_info *reqinfo,
                                                          netsnmp_request_info *requests);
    
    netsnmp_mib_handler *netsnmp_create_handler(const char *name,
                                                Netsnmp_Node_Handler *
                                                handler_access_method);
    netsnmp_handler_registration *
        netsnmp_create_handler_registration(const char *name,
                                            Netsnmp_Node_Handler *
                                               handler_access_method,
                                            oid * reg_oid,
                                            size_t reg_oid_len,
                                            int modes);

    NETSNMP_INLINE netsnmp_delegated_cache
        *netsnmp_create_delegated_cache(netsnmp_mib_handler *,
                                        netsnmp_handler_registration *,
                                        netsnmp_agent_request_info *,
                                        netsnmp_request_info *, void *);
    NETSNMP_INLINE void     netsnmp_free_delegated_cache(netsnmp_delegated_cache
                                                 *dcache);
    NETSNMP_INLINE netsnmp_delegated_cache
        *netsnmp_handler_check_cache(netsnmp_delegated_cache *dcache);
    void            netsnmp_register_handler_by_name(const char *,
                                                     netsnmp_mib_handler
                                                     *);

    NETSNMP_INLINE void
        netsnmp_request_add_list_data(netsnmp_request_info *request,
                                      netsnmp_data_list *node);

    NETSNMP_INLINE void    *netsnmp_request_get_list_data(netsnmp_request_info
                                                  *request,
                                                  const char *name);

    NETSNMP_INLINE void
              netsnmp_free_request_data_set(netsnmp_request_info *request);

    NETSNMP_INLINE void
             netsnmp_free_request_data_sets(netsnmp_request_info *request);

    void            netsnmp_handler_free(netsnmp_mib_handler *);
    netsnmp_mib_handler *netsnmp_handler_dup(netsnmp_mib_handler *);
    netsnmp_handler_registration
        *netsnmp_handler_registration_dup(netsnmp_handler_registration *);
    void           
        netsnmp_handler_registration_free(netsnmp_handler_registration *);

#define REQUEST_IS_DELEGATED     1
#define REQUEST_IS_NOT_DELEGATED 0
    void           
        netsnmp_handler_mark_requests_as_delegated(netsnmp_request_info *,
                                                   int);
    void           *netsnmp_handler_get_parent_data(netsnmp_request_info *,
                                                    const char *);

#ifdef __cplusplus
};
#endif

#endif                          /* AGENT_HANDLER_H */
