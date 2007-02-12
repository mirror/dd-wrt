#include <net-snmp/net-snmp-config.h>

#include <stdio.h>
#if HAVE_STRING_H
#include <string.h>
#else
#include <strings.h>
#endif
#include <sys/types.h>

#if HAVE_STDLIB_H
#include <stdlib.h>
#endif

#if HAVE_DMALLOC_H
#include <dmalloc.h>
#endif

#include <net-snmp/output_api.h>
#include <net-snmp/utilities.h>

#include <net-snmp/library/snmp_transport.h>
#include <net-snmp/library/snmpUDPDomain.h>
#ifdef SNMP_TRANSPORT_TCP_DOMAIN
#include <net-snmp/library/snmpTCPDomain.h>
#endif
#ifdef SNMP_TRANSPORT_IPX_DOMAIN
#include <net-snmp/library/snmpIPXDomain.h>
#endif
#ifdef SNMP_TRANSPORT_UNIX_DOMAIN
#include <net-snmp/library/snmpUnixDomain.h>
#endif
#ifdef SNMP_TRANSPORT_AAL5PVC_DOMAIN
#include <net-snmp/library/snmpAAL5PVCDomain.h>
#endif
#ifdef SNMP_TRANSPORT_UDPIPV6_DOMAIN
#include <net-snmp/library/snmpUDPIPv6Domain.h>
#endif
#ifdef SNMP_TRANSPORT_TCPIPV6_DOMAIN
#include <net-snmp/library/snmpTCPIPv6Domain.h>
#endif
#include <net-snmp/library/snmp_api.h>


/*
 * Our list of supported transport domains.  
 */

static netsnmp_tdomain *domain_list = NULL;



/*
 * The standard SNMP domains.  
 */

oid             netsnmpUDPDomain[] = { 1, 3, 6, 1, 6, 1, 1 };
size_t          netsnmpUDPDomain_len = OID_LENGTH(netsnmpUDPDomain);
oid             netsnmpCLNSDomain[] = { 1, 3, 6, 1, 6, 1, 2 };
size_t          netsnmpCLNSDomain_len = OID_LENGTH(netsnmpCLNSDomain);
oid             netsnmpCONSDomain[] = { 1, 3, 6, 1, 6, 1, 3 };
size_t          netsnmpCONSDomain_len = OID_LENGTH(netsnmpCONSDomain);
oid             netsnmpDDPDomain[] = { 1, 3, 6, 1, 6, 1, 4 };
size_t          netsnmpDDPDomain_len = OID_LENGTH(netsnmpDDPDomain);
oid             netsnmpIPXDomain[] = { 1, 3, 6, 1, 6, 1, 5 };
size_t          netsnmpIPXDomain_len = OID_LENGTH(netsnmpIPXDomain);



static void     netsnmp_tdomain_dump(void);


/*
 * Make a deep copy of an netsnmp_transport.  
 */

netsnmp_transport *
netsnmp_transport_copy(netsnmp_transport *t)
{
    netsnmp_transport *n = NULL;

    n = (netsnmp_transport *) malloc(sizeof(netsnmp_transport));
    if (n == NULL) {
        return NULL;
    }
    memset(n, 0, sizeof(netsnmp_transport));

    if (t->domain != NULL) {
        n->domain = t->domain;
        n->domain_length = t->domain_length;
    } else {
        n->domain = NULL;
        n->domain_length = 0;
    }

    if (t->local != NULL) {
        n->local = (u_char *) malloc(t->local_length);
        if (n->local == NULL) {
            netsnmp_transport_free(n);
            return NULL;
        }
        n->local_length = t->local_length;
        memcpy(n->local, t->local, t->local_length);
    } else {
        n->local = NULL;
        n->local_length = 0;
    }

    if (t->remote != NULL) {
        n->remote = (u_char *) malloc(t->remote_length);
        if (n->remote == NULL) {
            netsnmp_transport_free(n);
            return NULL;
        }
        n->remote_length = t->remote_length;
        memcpy(n->remote, t->remote, t->remote_length);
    } else {
        n->remote = NULL;
        n->remote_length = 0;
    }

    if (t->data != NULL && t->data_length > 0) {
        n->data = malloc(t->data_length);
        if (n->data == NULL) {
            netsnmp_transport_free(n);
            return NULL;
        }
        n->data_length = t->data_length;
        memcpy(n->data, t->data, t->data_length);
    } else {
        n->data = NULL;
        n->data_length = 0;
    }

    n->msgMaxSize = t->msgMaxSize;
    n->f_accept = t->f_accept;
    n->f_recv = t->f_recv;
    n->f_send = t->f_send;
    n->f_close = t->f_close;
    n->f_fmtaddr = t->f_fmtaddr;
    n->sock = t->sock;
    n->flags = t->flags;

    return n;
}



void
netsnmp_transport_free(netsnmp_transport *t)
{
    if (t->local != NULL) {
        free(t->local);
    }
    if (t->remote != NULL) {
        free(t->remote);
    }
    if (t->data != NULL) {
        free(t->data);
    }
    free(t);
}



int
netsnmp_tdomain_support(const oid * in_oid,
                        size_t in_len,
                        const oid ** out_oid, size_t * out_len)
{
    netsnmp_tdomain *d = NULL;

    for (d = domain_list; d != NULL; d = d->next) {
        if (netsnmp_oid_equals(in_oid, in_len, d->name, d->name_length) == 0) {
            if (out_oid != NULL && out_len != NULL) {
                *out_oid = d->name;
                *out_len = d->name_length;
            }
            return 1;
        }
    }
    return 0;
}



void
netsnmp_tdomain_init(void)
{
    DEBUGMSGTL(("tdomain", "netsnmp_tdomain_init() called\n"));
    netsnmp_udp_ctor();
#ifdef SNMP_TRANSPORT_TCP_DOMAIN
    netsnmp_tcp_ctor();
#endif
#ifdef SNMP_TRANSPORT_IPX_DOMAIN
    netsnmp_ipx_ctor();
#endif
#ifdef SNMP_TRANSPORT_UNIX_DOMAIN
    netsnmp_unix_ctor();
#endif
#ifdef SNMP_TRANSPORT_AAL5PVC_DOMAIN
    netsnmp_aal5pvc_ctor();
#endif
#ifdef SNMP_TRANSPORT_UDPIPV6_DOMAIN
    netsnmp_udp6_ctor();
#endif
#ifdef SNMP_TRANSPORT_TCPIPV6_DOMAIN
    netsnmp_tcp6_ctor();
#endif
    netsnmp_tdomain_dump();
}


static void
netsnmp_tdomain_dump(void)
{
    netsnmp_tdomain *d;
    int i = 0;

    DEBUGMSGTL(("tdomain", "domain_list -> "));
    for (d = domain_list; d != NULL; d = d->next) {
        DEBUGMSG(("tdomain", "{ "));
        DEBUGMSGOID(("tdomain", d->name, d->name_length));
        DEBUGMSG(("tdomain", ", \""));
        for (i = 0; d->prefix[i] != NULL; i++) {
            DEBUGMSG(("tdomain", "%s%s", d->prefix[i],
		      (d->prefix[i + 1]) ? "/" : ""));
        }
        DEBUGMSG(("tdomain", "\" } -> "));
    }
    DEBUGMSG(("tdomain", "[NIL]\n"));
}



int
netsnmp_tdomain_register(netsnmp_tdomain *n)
{
    netsnmp_tdomain **prevNext = &domain_list, *d;

    if (n != NULL) {
        for (d = domain_list; d != NULL; d = d->next) {
            if (netsnmp_oid_equals(n->name, n->name_length,
                                d->name, d->name_length) == 0) {
                /*
                 * Already registered.  
                 */
                return 0;
            }
            prevNext = &(d->next);
        }
        n->next = NULL;
        *prevNext = n;
        return 1;
    } else {
        return 0;
    }
}



int
netsnmp_tdomain_unregister(netsnmp_tdomain *n)
{
    netsnmp_tdomain **prevNext = &domain_list, *d;

    if (n != NULL) {
        for (d = domain_list; d != NULL; d = d->next) {
            if (netsnmp_oid_equals(n->name, n->name_length,
                                d->name, d->name_length) == 0) {
                *prevNext = n->next;
                return 1;
            }
            prevNext = &(d->next);
        }
        return 0;
    } else {
        return 0;
    }
}



netsnmp_transport *
netsnmp_tdomain_transport(const char *string, int local,
                          const char *default_domain)
{
    netsnmp_tdomain *d;
    netsnmp_transport *t = NULL;
    const char     *spec, *addr;
    char           *cp, *mystring;
    int             i;

    if (string == NULL) {
        return NULL;
    }

    if ((mystring = strdup(string)) == NULL) {
        DEBUGMSGTL(("tdomain", "can't strdup(\"%s\")\n", string));
        return NULL;
    }

    if ((cp = strchr(mystring, ':')) == NULL) {
        /*
         * There doesn't appear to be a transport specifier.  
         */
        DEBUGMSGTL(("tdomain", "no specifier in \"%s\"\n", mystring));
        if (*mystring == '/') {
            spec = "unix";
            addr = mystring;
        } else {
            if (default_domain) {
                spec = default_domain;
            } else {
                spec = "udp";
            }
            addr = mystring;
        }
    } else {
        *cp = '\0';
        spec = mystring;
        addr = cp + 1;
    }
    DEBUGMSGTL(("tdomain", "specifier \"%s\" address \"%s\"\n", spec,
                addr));

    for (d = domain_list; d != NULL; d = d->next) {
        for (i = 0; d->prefix[i] != NULL; i++) {
            if (strcasecmp(d->prefix[i], spec) == 0) {
                DEBUGMSGTL(("tdomain", "specifier \"%s\" matched\n",
                            spec));
                t = d->f_create_from_tstring(addr, local);
                free(mystring);
                return t;
            }
        }
    }

    /*
     * Okay no match so far.  Consider the possibility that we have something
     * like hostname.domain.com:port which will have confused the parser above.
     * Try and match again with the appropriate default domain.  
     */

    if (default_domain) {
        spec = default_domain;
    } else {
        spec = "udp";
    }
    if (cp) {
        *cp = ':';
    }

    addr = mystring;
    DEBUGMSGTL(("tdomain",
                "try again with specifier \"%s\" address \"%s\"\n", spec,
                addr));

    for (d = domain_list; d != NULL; d = d->next) {
        for (i = 0; d->prefix[i] != NULL; i++) {
            if (strcmp(d->prefix[i], spec) == 0) {
                DEBUGMSGTL(("tdomain", "specifier \"%s\" matched\n",
                            spec));
                t = d->f_create_from_tstring(addr, local);
                free(mystring);
                return t;
            }
        }
    }

    snmp_log(LOG_ERR, "No support for requested transport domain \"%s\"\n",
             spec);
    free(mystring);
    return NULL;
}


netsnmp_transport *
netsnmp_tdomain_transport_oid(const oid * dom,
                              size_t dom_len,
                              const u_char * o, size_t o_len, int local)
{
    netsnmp_tdomain *d;
    int             i;

    DEBUGMSGTL(("tdomain", "domain \""));
    DEBUGMSGOID(("tdomain", dom, dom_len));
    DEBUGMSG(("tdomain", "\"\n"));

    for (d = domain_list; d != NULL; d = d->next) {
        for (i = 0; d->prefix[i] != NULL; i++) {
            if (netsnmp_oid_equals(dom, dom_len, d->name, d->name_length) ==
                0) {
                return d->f_create_from_ostring(o, o_len, local);
            }
        }
    }

    snmp_log(LOG_ERR, "No support for requested transport domain\n");
    return NULL;
}


/** adds a transport to a linked list of transports.
    Returns 1 on failure, 0 on success */
int
netsnmp_transport_add_to_list(netsnmp_transport_list **transport_list,
                              netsnmp_transport *transport)
{
    netsnmp_transport_list *newptr =
        SNMP_MALLOC_TYPEDEF(netsnmp_transport_list);

    if (!newptr)
        return 1;

    newptr->next = *transport_list;
    newptr->transport = transport;

    *transport_list = newptr;

    return 0;
}


/**  removes a transport from a linked list of transports.
     Returns 1 on failure, 0 on success */
int
netsnmp_transport_remove_from_list(netsnmp_transport_list **transport_list,
                                   netsnmp_transport *transport)
{
    netsnmp_transport_list *ptr = *transport_list, *lastptr = NULL;

    while (ptr && ptr->transport != transport) {
        lastptr = ptr;
        ptr = ptr->next;
    }

    if (!ptr)
        return 1;

    if (lastptr)
        lastptr->next = ptr->next;
    else
        *transport_list = ptr->next;

    free(ptr);

    return 0;
}
