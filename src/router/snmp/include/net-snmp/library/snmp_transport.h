#ifndef _SNMP_TRANSPORT_H
#define _SNMP_TRANSPORT_H

/*
 * Portions of this file are copyrighted by:
 * Copyright (c) 2016 VMware, Inc. All rights reserved.
 * Use is subject to license terms specified in the COPYING file
 * distributed with the Net-SNMP package.
 */
#include <sys/types.h>

#ifdef HAVE_SYS_SOCKET_H
#ifdef solaris2
#define _XPG4_2
#endif

#if defined(aix4) || defined(aix5) || defined(aix6) || defined(aix7)
#define _LINUX_SOURCE_COMPAT
#endif

#include <sys/socket.h>

#ifdef solaris2
# ifndef CMSG_SPACE
#  define CMSG_SPACE(l) \
            ((unsigned int)_CMSG_HDR_ALIGN(sizeof (struct cmsghdr) + (l)))
#  define CMSG_LEN(l)   (_CMSG_HDR_ALIGN(sizeof(struct cmsghdr)) + (l))
# endif
#endif
#endif /* HAVE_SYS_SOCKET_H */

#ifdef HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif

#include <net-snmp/library/asn1.h>

#ifdef __cplusplus
extern          "C" {
#endif

/*  Some transport-type constants.  */

#ifndef NETSNMP_STREAM_QUEUE_LEN
#define	NETSNMP_STREAM_QUEUE_LEN	5
#endif

/*  Some transport-type flags.  */

#define		NETSNMP_TRANSPORT_FLAG_STREAM	 0x01
#define		NETSNMP_TRANSPORT_FLAG_LISTEN	 0x02
#define		NETSNMP_TRANSPORT_FLAG_TUNNELED	 0x04
#define         NETSNMP_TRANSPORT_FLAG_TMSTATE   0x08  /* indicates opaque is a
                                                          TSM tmStateReference */
#define		NETSNMP_TRANSPORT_FLAG_EMPTY_PKT 0x10
#define		NETSNMP_TRANSPORT_FLAG_OPENED	 0x20  /* f_open called */
#define		NETSNMP_TRANSPORT_FLAG_SHARED	 0x40
#define		NETSNMP_TRANSPORT_FLAG_HOSTNAME	 0x80  /* for fmtaddr hook */

/*  The standard SNMP domains.  */

NETSNMP_IMPORT oid      netsnmpUDPDomain[]; 	/*      = { 1, 3, 6, 1, 6, 1, 1 };  */
NETSNMP_IMPORT oid      netsnmpCLNSDomain[];    /*      = { 1, 3, 6, 1, 6, 1, 2 };  */
NETSNMP_IMPORT oid      netsnmpCONSDomain[];    /*      = { 1, 3, 6, 1, 6, 1, 3 };  */
NETSNMP_IMPORT oid      netsnmpDDPDomain[]; 	/*      = { 1, 3, 6, 1, 6, 1, 4 };  */
NETSNMP_IMPORT oid      netsnmpIPXDomain[]; 	/*      = { 1, 3, 6, 1, 6, 1, 5 };  */
NETSNMP_IMPORT size_t   netsnmpUDPDomain_len;
NETSNMP_IMPORT size_t   netsnmpCLNSDomain_len;
NETSNMP_IMPORT size_t   netsnmpCONSDomain_len;
NETSNMP_IMPORT size_t   netsnmpDDPDomain_len;
NETSNMP_IMPORT size_t   netsnmpIPXDomain_len;

/* Structure which stores transport security model specific parameters */
/* isms-secshell-11 section 4.1 */

/* contents documented in draft-ietf-isms-transport-security-model
   Section 3.2 */
/* note: VACM only allows <= 32 so this is overkill till another ACM comes */
#define NETSNMP_TM_MAX_SECNAME 256

typedef union netsnmp_sockaddr_storage_u {
    struct sockaddr     sa;
    struct sockaddr_in  sin;
#ifdef NETSNMP_ENABLE_IPV6
    struct sockaddr_in6 sin6;
#endif
} netsnmp_sockaddr_storage;

typedef struct netsnmp_addr_pair_s {
   netsnmp_sockaddr_storage remote_addr;
   netsnmp_sockaddr_storage local_addr;
} netsnmp_addr_pair;

typedef struct netsnmp_indexed_addr_pair_s {
   netsnmp_sockaddr_storage remote_addr;
   netsnmp_sockaddr_storage local_addr;
    int if_index;
} netsnmp_indexed_addr_pair;

typedef struct netsnmp_tmStateReference_s {
   oid    transportDomain[MAX_OID_LEN];
   size_t transportDomainLen;
   char   securityName[NETSNMP_TM_MAX_SECNAME];
   size_t securityNameLen;
   int    requestedSecurityLevel;
   int    transportSecurityLevel;
   char   sameSecurity;
   char   sessionID[8];
   
   char   have_addresses;
   netsnmp_indexed_addr_pair addresses;

   void *otherTransportOpaque; /* XXX: May have mem leak issues */
} netsnmp_tmStateReference;

#define NETSNMP_TSPEC_LOCAL                     0x01 /* 1=server, 0=client */
#define NETSNMP_TSPEC_PREBOUND                  0x02 /* 1=bound by systemd, 0=needs bind in the library */

struct netsnmp_container_s; /* forward decl */
typedef struct netsnmp_tdomain_spec_s {
    const char *application;             /* application name */
    const char *target;                  /* target as string */
    u_int       flags;
    const char *default_domain;          /* default domain */
    const char *default_target;          /* default target */
    const char *source;                  /* source as string iff remote */
    struct netsnmp_container_s *transport_config; /* extra config */
} netsnmp_tdomain_spec;

/*  Structure which defines the transport-independent API.  */

struct snmp_session;

typedef struct netsnmp_transport_s {
    /*  The transport domain object identifier.  */

    const oid      *domain;
    int             domain_length;  /*  In sub-IDs, not octets.  */

    /*  Local transport address (in relevant SNMP-style encoding).  */
    
    void           *local;
    int             local_length;   /*  In octets.  */

    /*  Remote transport address (in relevant SNMP-style encoding).  */

    void           *remote;
    int             remote_length;  /*  In octets.  */

    /*  The actual socket.  */
    
    int             sock;

    /*  Flags (see #definitions above).  */

    unsigned int    flags;

    /*  Protocol-specific opaque data pointer.  */

    void           *data;
    int             data_length;

    /*  Maximum size of PDU that can be sent/received by this transport.  */

    size_t          msgMaxSize;

#ifdef FOR_STANDARDS_COMPLIANCE_OR_FUTURE_USE
    /* TM state reference per ISMS WG solution */
    netsnmp_tmStateReference *tmStateRef;
#endif

    /* tunneled transports */
    struct netsnmp_transport_s * base_transport;

    /*  Callbacks.  Arguments are:
     *		
     *              "this" pointer, fd, buf, size, *opaque, *opaque_length  
     */

    int             (*f_recv)   (struct netsnmp_transport_s *, void *,
				 int, void **, int *);
    int             (*f_send)   (struct netsnmp_transport_s *, const void *,
				 int, void **, int *);
    int             (*f_close)  (struct netsnmp_transport_s *);

    /* Optional: opening can occur during creation if more appropriate */
   struct netsnmp_transport_s * (*f_open)   (struct netsnmp_transport_s *);

    /*  This callback is only necessary for stream-oriented transports.  */

    int             (*f_accept) (struct netsnmp_transport_s *);

    /*  Optional callback to format a transport address.  */

    char           *(*f_fmtaddr)(struct netsnmp_transport_s *, const void *,
                                 int);

    /*  Optional callback to support extra configuration token/value pairs */
    /*  return non-zero on error */
    int            (*f_config)(struct netsnmp_transport_s *, const char *,
                               const char *);

    /*  Optional callback that is called after the first transport is
        cloned to the second */
    int            (*f_copy)(const struct netsnmp_transport_s *,
                             struct netsnmp_transport_s *);

    /*  Setup initial session config if special things are needed */
   int             (*f_setup_session)(struct netsnmp_transport_s *,
                                      struct snmp_session *);
  
    /* allocated host name identifier; used by configuration system
       to load localhost.conf for host-specific configuration */
    u_char         *identifier; /* udp:localhost:161 -> "localhost" */

    /* Duplicate the remote address in the format required by SNMP-TARGET-MIB */
    void           (*f_get_taddr)(struct netsnmp_transport_s *t,
                                  void **addr, size_t *addr_len);

} netsnmp_transport;

typedef struct netsnmp_transport_list_s {
    netsnmp_transport *transport;
    struct netsnmp_transport_list_s *next;
} netsnmp_transport_list;

typedef struct netsnmp_tdomain_s {
    const oid      *name;
    size_t          name_length;
    const char    **prefix;

    /*
     * The f_create_from_tstring and f_create_from_tstring_new fields are
     * deprecated, please do not use them for new code and try to migrate
     * old code away from using them.
     */
    netsnmp_transport *(*f_create_from_tstring) (const char *, int);

    /* @o and @o_len define an address in the format used by SNMP-TARGET-MIB */
    netsnmp_transport *(*f_create_from_ostring) (const void *o, size_t o_len,
                                                 int local);

    struct netsnmp_tdomain_s *next;

    /** deprecated, please do not use it */
    netsnmp_transport *(*f_create_from_tstring_new) (const char *, int,
						     const char*);
    netsnmp_transport *(*f_create_from_tspec) (netsnmp_tdomain_spec *);

} netsnmp_tdomain;

void init_snmp_transport(void);
void shutdown_snmp_transport(void);

/*  Some utility functions.  */

NETSNMP_IMPORT
char *netsnmp_transport_peer_string(netsnmp_transport *t, const void *data,
                                    int len);

int netsnmp_transport_send(netsnmp_transport *t, const void *data, int len,
                           void **opaque, int *olength);
int netsnmp_transport_recv(netsnmp_transport *t, void *data, int len,
                           void **opaque, int *olength);

int netsnmp_transport_add_to_list(netsnmp_transport_list **transport_list,
				  netsnmp_transport *transport);
int netsnmp_transport_remove_from_list(netsnmp_transport_list **transport_list,
				       netsnmp_transport *transport);
NETSNMP_IMPORT
int netsnmp_sockaddr_size(const struct sockaddr *sa);


/*
 * Return an exact (deep) copy of t, or NULL if there is a memory allocation
 * problem (for instance).
 */

netsnmp_transport *netsnmp_transport_copy(const netsnmp_transport *t);


/*  Free an netsnmp_transport.  */

NETSNMP_IMPORT
void            netsnmp_transport_free(netsnmp_transport *t);

#ifndef FEATURE_REMOVE_TRANSPORT_CACHE

/**  transport cache support */

NETSNMP_IMPORT
netsnmp_transport *netsnmp_transport_cache_get(int af, int type, int local,
                                               const netsnmp_sockaddr_storage *bind_addr);

NETSNMP_IMPORT
int                netsnmp_transport_cache_save(int af, int type, int local,
                                                const netsnmp_sockaddr_storage *addr,
                                                netsnmp_transport *t);

NETSNMP_IMPORT
int                netsnmp_transport_cache_remove(netsnmp_transport *t);

#endif /* FEATURE_REMOVE_TRANSPORT_CACHE */

/*
 * If the passed oid (in_oid, in_len) corresponds to a supported transport
 * domain, return 1; if not return 0.  If out_oid is not NULL and out_len is
 * not NULL, then the "internal" oid which should be used to identify this
 * domain (e.g. in pdu->tDomain etc.) is written to *out_oid and its length to
 * *out_len.
 */

NETSNMP_IMPORT
int             netsnmp_tdomain_support(const oid *in_oid, size_t in_len,
					const oid **out_oid, size_t *out_len);

int             netsnmp_tdomain_register(netsnmp_tdomain *domain);
    
int             netsnmp_tdomain_unregister(netsnmp_tdomain *domain);

NETSNMP_IMPORT
void            netsnmp_clear_tdomain_list(void);

void            netsnmp_tdomain_init(void);

NETSNMP_IMPORT
netsnmp_transport *netsnmp_tdomain_transport(const char *str,
					     int local,
					     const char *default_domain);

NETSNMP_IMPORT
netsnmp_transport *netsnmp_tdomain_transport_full(const char *application,
						  const char *str,
						  int local,
						  const char *default_domain,
						  const char *default_target);

NETSNMP_IMPORT
netsnmp_transport *netsnmp_tdomain_transport_oid(const oid * dom,
						 size_t dom_len,
						 const u_char * o,
						 size_t o_len,
						 int local);

NETSNMP_IMPORT
netsnmp_transport *netsnmp_tdomain_transport_tspec(netsnmp_tdomain_spec *tspec);

NETSNMP_IMPORT
netsnmp_transport*
netsnmp_transport_open_client(const char* application, const char* str);

NETSNMP_IMPORT
netsnmp_transport*
netsnmp_transport_open_server(const char* application, const char* str);

netsnmp_transport*
netsnmp_transport_open(const char* application, const char* str, int local);

typedef struct netsnmp_transport_config_s {
   char *key;
   char *value;
} netsnmp_transport_config;
    
NETSNMP_IMPORT    
int netsnmp_transport_config_compare(netsnmp_transport_config *left,
                                     netsnmp_transport_config *right);
NETSNMP_IMPORT
netsnmp_transport_config *netsnmp_transport_create_config(const char *key,
                                                          const char *value);

#ifndef NETSNMP_FEATURE_REMOVE_FILTER_SOURCE
NETSNMP_IMPORT
void netsnmp_transport_parse_filterType(const char *word, char *cptr);

NETSNMP_IMPORT
int netsnmp_transport_filter_add(const char *addrtxt);

NETSNMP_IMPORT
int netsnmp_transport_filter_remove(const char *addrtxt);

NETSNMP_IMPORT
int netsnmp_transport_filter_check(const char *addrtxt);

NETSNMP_IMPORT
void netsnmp_transport_filter_cleanup(void);

#endif /* NETSNMP_FEATURE_REMOVE_FILTER_SOURCE */

#ifdef __cplusplus
}
#endif
#endif/*_SNMP_TRANSPORT_H*/
