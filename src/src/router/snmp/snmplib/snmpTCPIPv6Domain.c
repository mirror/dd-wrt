#include <net-snmp/net-snmp-config.h>

#include <stdio.h>
#include <sys/types.h>
#include <errno.h>

#if HAVE_STRING_H
#include <string.h>
#else
#include <strings.h>
#endif
#if HAVE_STDLIB_H
#include <stdlib.h>
#endif
#if HAVE_UNISTD_H
#include <unistd.h>
#endif
#if HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif
#if HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif
#if HAVE_ARPA_INET_H
#include <arpa/inet.h>
#endif
#if HAVE_NETDB_H
#include <netdb.h>
#endif
#if HAVE_FCNTL_H
#include <fcntl.h>
#endif

#if HAVE_DMALLOC_H
#include <dmalloc.h>
#endif

#include <net-snmp/types.h>
#include <net-snmp/output_api.h>
#include <net-snmp/config_api.h>

#include <net-snmp/library/snmp_transport.h>
#include <net-snmp/library/snmpUDPIPv6Domain.h>
#include <net-snmp/library/snmpTCPIPv6Domain.h>

oid netsnmp_TCPIPv6Domain[] = { ENTERPRISE_MIB, 3, 3, 5 };
static netsnmp_tdomain tcp6Domain;

/*
 * Return a string representing the address in data, or else the "far end"
 * address if data is NULL.  
 */

static char *
netsnmp_tcp6_fmtaddr(netsnmp_transport *t, void *data, int len)
{
    struct sockaddr_in6 *to = NULL;

    DEBUGMSGTL(("netsnmp_tcp6", "fmtaddr: t = %p, data = %p, len = %d\n", t,
                data, len));
    if (data != NULL && len == sizeof(struct sockaddr_in6)) {
        to = (struct sockaddr_in6 *) data;
    } else if (t != NULL && t->data != NULL) {
        to = (struct sockaddr_in6 *) t->data;
    }
    if (to == NULL) {
        return strdup("TCP/IPv6: unknown");
    } else {
        char addr[INET6_ADDRSTRLEN];
        char tmp[INET6_ADDRSTRLEN + 8];

        sprintf(tmp, "[%s]:%hd",
                inet_ntop(AF_INET6, (void *) &(to->sin6_addr), addr,
                          INET6_ADDRSTRLEN), ntohs(to->sin6_port));
        return strdup(tmp);
    }
}

/*
 * You can write something into opaque that will subsequently get passed back 
 * to your send function if you like.  For instance, you might want to
 * remember where a PDU came from, so that you can send a reply there...  
 */

static int
netsnmp_tcp6_recv(netsnmp_transport *t, void *buf, int size,
		  void **opaque, int *olength)
{
    int rc = -1;

    if (t != NULL && t->sock >= 0) {
	while (rc < 0) {
	    rc = recv(t->sock, buf, size, 0);
	    if (rc < 0 && errno != EINTR) {
		DEBUGMSGTL(("netsnmp_tcp6", "recv fd %d err %d (\"%s\")\n",
			    t->sock, errno, strerror(errno)));
		return -1;
	    }
	}
        DEBUGMSGTL(("netsnmp_tcp6", "recv fd %d got %d bytes\n", t->sock, rc));
    } else {
        return -1;
    }

    if (opaque != NULL && olength != NULL) {
        if (t->data_length > 0) {
            if ((*opaque = malloc(t->data_length)) != NULL) {
                memcpy(*opaque, t->data, t->data_length);
                *olength = t->data_length;
            } else {
                *olength = 0;
            }
        } else {
            *opaque = NULL;
            *olength = 0;
        }
    }

    return rc;
}

static int
netsnmp_tcp6_send(netsnmp_transport *t, void *buf, int size,
		  void **opaque, int *olength)
{
    int rc = -1;

    if (t != NULL && t->sock >= 0) {
	while (rc < 0) {
	    rc = send(t->sock, buf, size, 0);
	    if (rc < 0 && errno != EINTR) {
		break;
	    }
	}
    }
    return rc;
}

static int
netsnmp_tcp6_close(netsnmp_transport *t)
{
    int rc = -1;
    if (t != NULL && t->sock >= 0) {
        DEBUGMSGTL(("netsnmp_tcp6", "close fd %d\n", t->sock));
#ifndef HAVE_CLOSESOCKET
        rc = close(t->sock);
#else
        rc = closesocket(t->sock);
#endif
        t->sock = -1;
    }
    return rc;
}

static int
netsnmp_tcp6_accept(netsnmp_transport *t)
{
    struct sockaddr_in6 *farend = NULL;
    int             newsock = -1, sockflags = 0;
    socklen_t       farendlen = sizeof(struct sockaddr_in6);
    char           *string = NULL;

    farend = (struct sockaddr_in6 *) malloc(sizeof(struct sockaddr_in6));

    if (farend == NULL) {
        /*
         * Indicate that the acceptance of this socket failed.  
         */
        DEBUGMSGTL(("netsnmp_tcp6", "accept: malloc failed\n"));
        return -1;
    }

    if (t != NULL && t->sock >= 0) {
        newsock = accept(t->sock, (struct sockaddr *) farend, &farendlen);

        if (newsock < 0) {
            DEBUGMSGTL(("netsnmp_tcp6","accept failed rc %d errno %d \"%s\"\n",
			newsock, errno, strerror(errno)));
            free(farend);
            return newsock;
        }

        if (t->data != NULL) {
            free(t->data);
        }

        t->data = farend;
        t->data_length = farendlen;
        string = netsnmp_tcp6_fmtaddr(NULL, farend, farendlen);
        DEBUGMSGTL(("netsnmp_tcp6", "accept succeeded (from %s)\n", string));
        free(string);

        /*
         * Try to make the new socket blocking.  
         */

#ifdef WIN32
        ioctlsocket(newsock, FIONBIO, &sockflags);
#else
        if ((sockflags = fcntl(newsock, F_GETFL, 0)) >= 0) {
            fcntl(newsock, F_SETFL, (sockflags & ~O_NONBLOCK));
        } else {
            DEBUGMSGTL(("netsnmp_tcp6", "accept: couldn't f_getfl of fd %d\n",
                        newsock));
        }
#endif
        return newsock;
    } else {
        free(farend);
        return -1;
    }
}



/*
 * Open a TCP/IPv6-based transport for SNMP.  Local is TRUE if addr is the
 * local address to bind to (i.e. this is a server-type session); otherwise
 * addr is the remote address to send things to.  
 */

netsnmp_transport *
netsnmp_tcp6_transport(struct sockaddr_in6 *addr, int local)
{
    netsnmp_transport *t = NULL;
    int             rc = 0;
    char           *string = NULL;

    if (addr == NULL || addr->sin6_family != AF_INET6) {
        return NULL;
    }

    t = (netsnmp_transport *) malloc(sizeof(netsnmp_transport));
    if (t == NULL) {
        return NULL;
    }
    memset(t, 0, sizeof(netsnmp_transport));

    string = netsnmp_tcp6_fmtaddr(NULL, (void *)addr,
				  sizeof(struct sockaddr_in6));
    DEBUGMSGTL(("netsnmp_tcp6", "open %s %s\n", local ? "local" : "remote",
                string));
    free(string);

    memset(t, 0, sizeof(netsnmp_transport));

    t->data = malloc(sizeof(struct sockaddr_in6));
    if (t->data == NULL) {
        netsnmp_transport_free(t);
        return NULL;
    }
    t->data_length = sizeof(struct sockaddr_in6);
    memcpy(t->data, addr, sizeof(struct sockaddr_in6));

    t->domain = netsnmp_TCPIPv6Domain;
    t->domain_length = sizeof(netsnmp_TCPIPv6Domain) / sizeof(oid);

    t->sock = socket(PF_INET6, SOCK_STREAM, 0);
    if (t->sock < 0) {
        netsnmp_transport_free(t);
        return NULL;
    }

    t->flags = NETSNMP_TRANSPORT_FLAG_STREAM;

    if (local) {
        int sockflags = 0, opt = 1;

        /*
         * This session is inteneded as a server, so we must bind on to the
         * given IP address, which may include an interface address, or could
         * be INADDR_ANY, but certainly includes a port number.
         */

        t->flags |= NETSNMP_TRANSPORT_FLAG_LISTEN;
        t->local = malloc(18);
        if (t->local == NULL) {
            netsnmp_tcp6_close(t);
            netsnmp_transport_free(t);
            return NULL;
        }
        memcpy(t->local, addr->sin6_addr.s6_addr, 16);
        t->local[16] = (addr->sin6_port & 0xff00) >> 8;
        t->local[17] = (addr->sin6_port & 0x00ff) >> 0;
        t->local_length = 18;

        /*
         * We should set SO_REUSEADDR too.  
         */

        setsockopt(t->sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

        rc = bind(t->sock, (struct sockaddr *) addr,
		  sizeof(struct sockaddr_in6));
        if (rc != 0) {
            netsnmp_tcp6_close(t);
            netsnmp_transport_free(t);
            return NULL;
        }

        /*
         * Since we are going to be letting select() tell us when connections
         * are ready to be accept()ed, we need to make the socket n0n-blocking
         * to avoid the race condition described in W. R. Stevens, ``Unix
         * Network Programming Volume I Second Edition'', pp. 422--4, which
         * could otherwise wedge the agent.
         */

#ifdef WIN32
        opt = 1;
        ioctlsocket(t->sock, FIONBIO, &opt);
#else
        sockflags = fcntl(t->sock, F_GETFL, 0);
        fcntl(t->sock, F_SETFL, sockflags | O_NONBLOCK);
#endif

        /*
         * Now sit here and wait for connections to arrive.  
         */

        rc = listen(t->sock, NETSNMP_STREAM_QUEUE_LEN);
        if (rc != 0) {
            netsnmp_tcp6_close(t);
            netsnmp_transport_free(t);
            return NULL;
        }
    } else {
        t->remote = malloc(18);
        if (t->remote == NULL) {
            netsnmp_tcp6_close(t);
            netsnmp_transport_free(t);
            return NULL;
        }
        memcpy(t->remote, addr->sin6_addr.s6_addr, 16);
        t->remote[16] = (addr->sin6_port & 0xff00) >> 8;
        t->remote[17] = (addr->sin6_port & 0x00ff) >> 0;
        t->remote_length = 18;

        /*
         * This is a client-type session, so attempt to connect to the far
         * end.  We don't go non-blocking here because it's not obvious what
         * you'd then do if you tried to do snmp_sends before the connection
         * had completed.  So this can block.
         */

        rc = connect(t->sock, (struct sockaddr *) addr,
                     sizeof(struct sockaddr_in6));

        DEBUGMSGTL(("netsnmp_tcp6", "connect returns %d\n", rc));

        if (rc < 0) {
            netsnmp_tcp6_close(t);
            netsnmp_transport_free(t);
            return NULL;
        }
    }

    /*
     * Message size is not limited by this transport (hence msgMaxSize
     * is equal to the maximum legal size of an SNMP message).  
     */

    t->msgMaxSize = 0x7fffffff;
    t->f_recv     = netsnmp_tcp6_recv;
    t->f_send     = netsnmp_tcp6_send;
    t->f_close    = netsnmp_tcp6_close;
    t->f_accept   = netsnmp_tcp6_accept;
    t->f_fmtaddr  = netsnmp_tcp6_fmtaddr;

    return t;
}



netsnmp_transport *
netsnmp_tcp6_create_tstring(const char *string, int local)
{
    struct sockaddr_in6 addr;

    if (netsnmp_sockaddr_in6(&addr, string, 0)) {
        return netsnmp_tcp6_transport(&addr, local);
    } else {
        return NULL;
    }
}


/*
 * See:
 * 
 * http://www.ietf.org/internet-drafts/draft-ietf-ops-taddress-mib-01.txt
 * 
 * (or newer equivalent) for details of the TC which we are using for
 * the mapping here.  
 */

netsnmp_transport *
netsnmp_tcp6_create_ostring(const u_char * o, size_t o_len, int local)
{
    struct sockaddr_in6 addr;

    if (o_len == 18) {
        memset((u_char *) & addr, 0, sizeof(struct sockaddr_in6));
        addr.sin6_family = AF_INET6;
        memcpy((u_char *) & (addr.sin6_addr.s6_addr), o, 16);
        addr.sin6_port = (o[16] << 8) + o[17];
        return netsnmp_tcp6_transport(&addr, local);
    }
    return NULL;
}


void
netsnmp_tcp6_ctor(void)
{
    tcp6Domain.name = netsnmp_TCPIPv6Domain;
    tcp6Domain.name_length = sizeof(netsnmp_TCPIPv6Domain) / sizeof(oid);
    tcp6Domain.f_create_from_tstring = netsnmp_tcp6_create_tstring;
    tcp6Domain.f_create_from_ostring = netsnmp_tcp6_create_ostring;
    tcp6Domain.prefix = calloc(4, sizeof(char *));
    tcp6Domain.prefix[0] = "tcp6";
    tcp6Domain.prefix[1] = "tcpv6";
    tcp6Domain.prefix[2] = "tcpipv6";

    netsnmp_tdomain_register(&tcp6Domain);
}
