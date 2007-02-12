/*
 * snmp_agent.h
 *
 * External definitions for functions and variables in snmp_agent.c.
 */

#ifndef SNMP_AGENT_H
#define SNMP_AGENT_H

#ifdef __cplusplus
extern          "C" {
#endif

#include <net-snmp/library/snmp_impl.h>
#include <net-snmp/library/tools.h>
#include <net-snmp/library/data_list.h>

#define SNMP_MAX_PDU_SIZE 64000 /* local constraint on PDU size sent by agent
                                 * (see also SNMP_MAX_MSG_SIZE in snmp_api.h) */

    /*
     * If non-zero, causes the addresses of peers to be logged when receptions
     * occur.  
     */

    extern int      log_addresses;

    /*
     * How many ticks since we last aged the address cache entries.  
     */

    extern int      lastAddrAge;

    typedef struct netsnmp_request_info_s {
        netsnmp_variable_list *requestvb;

        /*
         * can be used to pass information on a per-request basis from a
         * helper to the later handlers 
         */
        netsnmp_data_list *parent_data;

        oid            *range_end;      /* don't free, reference to (struct tree)->end */
        size_t          range_end_len;
        int             delegated;
        int             processed;
        int             inclusive;
        int             status;
        int             index; /* index in original pdu */
 
       /* get-bulk */
        int             repeat;
        int             orig_repeat;
        netsnmp_variable_list *requestvb_start;

        struct netsnmp_request_info_s *next;
        struct netsnmp_request_info_s *prev;
        struct netsnmp_subtree_s      *subtree;
    } netsnmp_request_info;

    typedef struct netsnmp_set_info_s {
        int             action;
        void           *stateRef;

        /*
         * don't use yet: 
         */
        void          **oldData;
        int             setCleanupFlags;
#define AUTO_FREE_STATEREF 0x01 /* calls free(stateRef) */
#define AUTO_FREE_OLDDATA  0x02 /* calls free(*oldData) */
#define AUTO_UNDO          0x03 /* ... */
    } netsnmp_set_info;

    typedef struct netsnmp_tree_cache_s {
        struct netsnmp_subtree_s *subtree;
        netsnmp_request_info *requests_begin;
        netsnmp_request_info *requests_end;
    } netsnmp_tree_cache;

#define MODE_GET              SNMP_MSG_GET
#define MODE_GETNEXT          SNMP_MSG_GETNEXT
#define MODE_GETBULK          SNMP_MSG_GETBULK
#define MODE_IS_GET(x)        (x == SNMP_MSG_GET || x == SNMP_MSG_GETNEXT || x == SNMP_MSG_GETBULK)

#define MODE_SET_BEGIN        SNMP_MSG_INTERNAL_SET_BEGIN
#define MODE_SET_RESERVE1     SNMP_MSG_INTERNAL_SET_RESERVE1
#define MODE_SET_RESERVE2     SNMP_MSG_INTERNAL_SET_RESERVE2
#define MODE_SET_ACTION       SNMP_MSG_INTERNAL_SET_ACTION
#define MODE_SET_COMMIT       SNMP_MSG_INTERNAL_SET_COMMIT
#define MODE_SET_FREE         SNMP_MSG_INTERNAL_SET_FREE
#define MODE_SET_UNDO         SNMP_MSG_INTERNAL_SET_UNDO
#define MODE_IS_SET(x)         (!MODE_IS_GET(x))

    typedef struct netsnmp_agent_request_info_s {
        int             mode;
        netsnmp_pdu    *pdu;    /* pdu contains authinfo, eg */
        struct netsnmp_agent_session_s *asp;    /* may not be needed */
        /*
         * can be used to pass information on a per-pdu basis from a
         * helper to the later handlers 
         */
        netsnmp_data_list *agent_data;
        /*
         * ... 
         */
    } netsnmp_agent_request_info;

    typedef struct netsnmp_cachemap_s {
        int             globalid;
        int             cacheid;
        struct netsnmp_cachemap_s *next;
    } netsnmp_cachemap;

    typedef struct netsnmp_agent_session_s {
        int             mode;
        netsnmp_session *session;
        netsnmp_pdu    *pdu;
        netsnmp_pdu    *orig_pdu;
        int             rw;
        int             exact;
        int             status;
        int             index;
        int             oldmode;

        struct netsnmp_agent_session_s *next;

        /*
         * new API pointers 
         */
        netsnmp_agent_request_info *reqinfo;
        netsnmp_request_info *requests;
        netsnmp_tree_cache *treecache;
        netsnmp_variable_list **bulkcache;
        int             treecache_len;  /* length of cache array */
        int             treecache_num;  /* number of current cache entries */
        netsnmp_cachemap *cache_store;
        int             vbcount;
    } netsnmp_agent_session;

    /*
     * Address cache handling functions.  
     */

    void            netsnmp_addrcache_initialise(void);
    void            netsnmp_addrcache_age(void);


    /*
     * config file parsing routines 
     */
    int             handle_snmp_packet(int, netsnmp_session *, int,
                                       netsnmp_pdu *, void *);
    void            snmp_agent_parse_config(char *, char *);
    netsnmp_agent_session *init_agent_snmp_session(netsnmp_session *,
                                                   netsnmp_pdu *);
    void            free_agent_snmp_session(netsnmp_agent_session *);
    void           
        netsnmp_remove_and_free_agent_snmp_session(netsnmp_agent_session
                                                   *asp);
#ifdef SNMP_NEED_REQUEST_LIST
    void           
        netsnmp_free_agent_snmp_session_by_session(netsnmp_session * sess,
                                                   void (*free_request)
                                                   (netsnmp_request_list
                                                    *));
#endif
    int             getNextSessID(void);
    void            dump_sess_list(void);
    int             init_master_agent(void);
    int             agent_check_and_process(int block);
    void            netsnmp_check_outstanding_agent_requests(void);
    int             netsnmp_set_mode_request_error(int mode,
                                                   netsnmp_request_info
                                                   *request,
                                                   int error_value);
    int             netsnmp_set_request_error(netsnmp_agent_request_info
                                              *reqinfo,
                                              netsnmp_request_info
                                              *request, int error_value);
    int            
        netsnmp_set_all_requests_error(netsnmp_agent_request_info *reqinfo,
                                       netsnmp_request_info *requests,
                                       int error_value);
    u_long          netsnmp_marker_uptime(marker_t pm);
    u_long          netsnmp_timeval_uptime(struct timeval *tv);
    u_long          netsnmp_get_agent_uptime(void);
    int             netsnmp_check_transaction_id(int transaction_id);
    int             netsnmp_agent_check_packet(netsnmp_session *,
                                               struct netsnmp_transport_s
                                               *, void *, int);
    int             netsnmp_agent_check_parse(netsnmp_session *,
                                              netsnmp_pdu *, int);
    int             netsnmp_allocate_globalcacheid(void);

    int netsnmp_remove_delegated_requests_for_session(netsnmp_session *sess);

    /*
     * Register and de-register agent NSAPs.  
     */

    struct netsnmp_transport_s;

    int             netsnmp_register_agent_nsap(struct netsnmp_transport_s
                                                *t);
    void            netsnmp_deregister_agent_nsap(int handle);

    NETSNMP_INLINE void
        netsnmp_agent_add_list_data(netsnmp_agent_request_info *agent,
                                    netsnmp_data_list *node);

    NETSNMP_INLINE void    *netsnmp_agent_get_list_data(netsnmp_agent_request_info
                                                *agent, const char *name);

    NETSNMP_INLINE void
            netsnmp_free_agent_data_set(netsnmp_agent_request_info *agent);

    NETSNMP_INLINE void
           netsnmp_free_agent_data_sets(netsnmp_agent_request_info *agent);
    NETSNMP_INLINE void    
        netsnmp_free_agent_request_info(netsnmp_agent_request_info *ari);

#ifdef __cplusplus
}
#endif
#endif
