/* Portions of this file are subject to the following copyright(s).  See
 * the Net-SNMP's COPYING file for more details and other copyrights
 * that may apply:
 */
/*
 * Portions of this file are copyrighted by:
 * Copyright Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * Use is subject to license terms specified in the COPYING file
 * distributed with the Net-SNMP package.
 */

#include <net-snmp/net-snmp-config.h>

#include <stdio.h>
#include <sys/types.h>
#include <ctype.h>
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

#if HAVE_WINSOCK_H
#include <winsock2.h>
#include <ws2tcpip.h>
#endif

#if HAVE_DMALLOC_H
#include <dmalloc.h>
#endif

#include <net-snmp/types.h>
#include <net-snmp/output_api.h>
#include <net-snmp/config_api.h>

#include <net-snmp/library/snmp_transport.h>
#include <net-snmp/library/snmpUDPDomain.h>
#include <net-snmp/library/system.h>
#include <net-snmp/library/tools.h>

#ifndef INADDR_NONE
#define INADDR_NONE	-1
#endif

static netsnmp_tdomain udpDomain;

typedef struct netsnmp_udp_addr_pair_s {
    struct sockaddr_in remote_addr;
    struct in_addr local_addr;
} netsnmp_udp_addr_pair;

/*
 * not static, since snmpUDPIPv6Domain needs it, but not public, either.
 * (ie don't put it in a public header.)
 */
void _netsnmp_udp_sockopt_set(int fd, int server);
int
netsnmp_sockaddr_in2(struct sockaddr_in *addr,
                     const char *inpeername, const char *default_target);

/*
 * Return a string representing the address in data, or else the "far end"
 * address if data is NULL.  
 */

static char *
netsnmp_udp_fmtaddr(netsnmp_transport *t, void *data, int len)
{
    netsnmp_udp_addr_pair *addr_pair = NULL;

    if (data != NULL && len == sizeof(netsnmp_udp_addr_pair)) {
	addr_pair = (netsnmp_udp_addr_pair *) data;
    } else if (t != NULL && t->data != NULL) {
	addr_pair = (netsnmp_udp_addr_pair *) t->data;
    }

    if (addr_pair == NULL) {
        return strdup("UDP: unknown");
    } else {
        struct sockaddr_in *to = NULL;
	char tmp[64];
        to = (struct sockaddr_in *) &(addr_pair->remote_addr);
        if (to == NULL) {
            return strdup("UDP: unknown");
        }

        sprintf(tmp, "UDP: [%s]:%hu",
                inet_ntoa(to->sin_addr), ntohs(to->sin_port));
        return strdup(tmp);
    }
}



#if defined(linux) && defined(IP_PKTINFO)

# define netsnmp_dstaddr(x) (&(((struct in_pktinfo *)(CMSG_DATA(x)))->ipi_addr))

static int netsnmp_udp_recvfrom(int s, char *buf, int len, struct sockaddr *from, socklen_t *fromlen, struct in_addr *dstip)
{
    int r;
    struct iovec iov[1];
    char cmsg[CMSG_SPACE(sizeof(struct in_pktinfo))];
    struct cmsghdr *cmsgptr;
    struct msghdr msg;

    iov[0].iov_base = buf;
    iov[0].iov_len = len;

    memset(&msg, 0, sizeof msg);
    msg.msg_name = from;
    msg.msg_namelen = *fromlen;
    msg.msg_iov = iov;
    msg.msg_iovlen = 1;
    msg.msg_control = &cmsg;
    msg.msg_controllen = sizeof(cmsg);

    r = recvmsg(s, &msg, 0);

    if (r == -1) {
        return -1;
    }
    
    DEBUGMSGTL(("netsnmp_udp", "got source addr: %s\n", inet_ntoa(((struct sockaddr_in *)from)->sin_addr)));
    for (cmsgptr = CMSG_FIRSTHDR(&msg); cmsgptr != NULL; cmsgptr = CMSG_NXTHDR(&msg, cmsgptr)) {
        if (cmsgptr->cmsg_level == SOL_IP && cmsgptr->cmsg_type == IP_PKTINFO) {
            memcpy((void *) dstip, netsnmp_dstaddr(cmsgptr), sizeof(struct in_addr));
            DEBUGMSGTL(("netsnmp_udp", "got destination (local) addr %s\n",
                    inet_ntoa(*dstip)));
        }
    }
    return r;
}

static int netsnmp_udp_sendto(int fd, struct in_addr *srcip, struct sockaddr *remote,
			char *data, int len)
{
    struct iovec iov = { data, len };
    struct {
        struct cmsghdr cm;
        struct in_pktinfo ipi;
    } cmsg = {
        .cm = {
            .cmsg_len	= sizeof(struct cmsghdr) + sizeof(struct in_pktinfo),
            .cmsg_level	= SOL_IP,
            .cmsg_type	= IP_PKTINFO,
        },
        .ipi = {
            .ipi_ifindex	= 0,
            .ipi_spec_dst	= srcip ? srcip->s_addr : 0,
        },
    };
    struct msghdr m = {
        .msg_name	= remote,
        .msg_namelen	= sizeof(struct sockaddr_in),
        .msg_iov	= &iov,
        .msg_iovlen	= 1,
        .msg_control	= &cmsg,
        .msg_controllen	= sizeof(cmsg),
        .msg_flags	= 0,
    };
    return sendmsg(fd, &m, MSG_NOSIGNAL|MSG_DONTWAIT);
}
#endif /* linux && IP_PKTINFO */

/*
 * You can write something into opaque that will subsequently get passed back 
 * to your send function if you like.  For instance, you might want to
 * remember where a PDU came from, so that you can send a reply there...  
 */

static int
netsnmp_udp_recv(netsnmp_transport *t, void *buf, int size,
		 void **opaque, int *olength)
{
    int             rc = -1;
    socklen_t       fromlen = sizeof(struct sockaddr);
    netsnmp_udp_addr_pair *addr_pair = NULL;
    struct sockaddr *from;

    if (t != NULL && t->sock >= 0) {
        addr_pair = (netsnmp_udp_addr_pair *) malloc(sizeof(netsnmp_udp_addr_pair));
        if (addr_pair == NULL) {
            *opaque = NULL;
            *olength = 0;
            return -1;
        } else {
            memset(addr_pair, 0, sizeof(netsnmp_udp_addr_pair));
            from = (struct sockaddr *) &(addr_pair->remote_addr);
        }

	while (rc < 0) {
#if defined(linux) && defined(IP_PKTINFO)
            rc = netsnmp_udp_recvfrom(t->sock, buf, size, from, &fromlen, &(addr_pair->local_addr));
#else
            rc = recvfrom(t->sock, buf, size, 0, from, &fromlen);
#endif /* linux && IP_PKTINFO */
	    if (rc < 0 && errno != EINTR) {
		break;
	    }
	}

        if (rc >= 0) {
            char *str = netsnmp_udp_fmtaddr(NULL, addr_pair, sizeof(netsnmp_udp_addr_pair));
            DEBUGMSGTL(("netsnmp_udp",
			"recvfrom fd %d got %d bytes (from %s)\n",
			t->sock, rc, str));
            free(str);
        } else {
            DEBUGMSGTL(("netsnmp_udp", "recvfrom fd %d err %d (\"%s\")\n",
                        t->sock, errno, strerror(errno)));
        }
        *opaque = (void *)addr_pair;
        *olength = sizeof(netsnmp_udp_addr_pair);
    }
    return rc;
}



static int
netsnmp_udp_send(netsnmp_transport *t, void *buf, int size,
		 void **opaque, int *olength)
{
    int rc = -1;
    netsnmp_udp_addr_pair *addr_pair = NULL;
    struct sockaddr *to = NULL;

    if (opaque != NULL && *opaque != NULL &&
        *olength == sizeof(netsnmp_udp_addr_pair)) {
        addr_pair = (netsnmp_udp_addr_pair *) (*opaque);
    } else if (t != NULL && t->data != NULL &&
                t->data_length == sizeof(netsnmp_udp_addr_pair)) {
        addr_pair = (netsnmp_udp_addr_pair *) (t->data);
    }

    to = (struct sockaddr *) &(addr_pair->remote_addr);

    if (to != NULL && t != NULL && t->sock >= 0) {
        char *str = netsnmp_udp_fmtaddr(NULL, (void *) addr_pair,
                                        sizeof(netsnmp_udp_addr_pair));
        DEBUGMSGTL(("netsnmp_udp", "send %d bytes from %p to %s on fd %d\n",
                    size, buf, str, t->sock));
        free(str);
	while (rc < 0) {
#if defined(linux) && defined(IP_PKTINFO)
            rc = netsnmp_udp_sendto(t->sock, addr_pair ? &(addr_pair->local_addr) : NULL, to, buf, size);
#else
            rc = sendto(t->sock, buf, size, 0, to, sizeof(struct sockaddr));
#endif /* linux && IP_PKTINFO */
	    if (rc < 0 && errno != EINTR) {
                DEBUGMSGTL(("netsnmp_udp", "sendto error, rc %d (errno %d)\n",
                            rc, errno));
		break;
	    }
	}
    }
    return rc;
}



static int
netsnmp_udp_close(netsnmp_transport *t)
{
    int rc = -1;
    if (t->sock >= 0) {
#ifndef HAVE_CLOSESOCKET
        rc = close(t->sock);
#else
        rc = closesocket(t->sock);
#endif
        t->sock = -1;
    }
    return rc;
}

/*
 * find largest possible buffer between current size and specified size.
 *
 * Try to maximize the current buffer of type "optname"
 * to the maximum allowable size by the OS (as close to
 * size as possible)
 */
static int
_sock_buffer_maximize(int s, int optname, const char *buftype, int size)
{
    int            curbuf = 0;
    size_t         curbuflen = sizeof(int);
    int            lo, mid, hi;

    /*
     * First we need to determine our current buffer
     */
    if ((getsockopt(s, SOL_SOCKET, optname, (void *) &curbuf,
                    &curbuflen) == 0) 
            && (curbuflen == sizeof(int))) {

        DEBUGMSGTL(("verbose:socket:buffer:max", "Current %s is %d\n",
                    buftype, curbuf));

        /*
         * Let's not be stupid ... if we were asked for less than what we
         * already have, then forget about it
         */
        if (size <= curbuf) {
            DEBUGMSGTL(("verbose:socket:buffer:max",
                        "Requested %s <= current buffer\n", buftype));
            return curbuf;
        }

        /*
         * Do a binary search the optimal buffer within 1k of the point of
         * failure. This is rather bruteforce, but simple
         */
        hi = size;
        lo = curbuf;

        while (hi - lo > 1024) {
            mid = (lo + hi) / 2;
            if (setsockopt(s, SOL_SOCKET, optname, (void *) &mid,
                        sizeof(int)) == 0) {
                lo = mid; /* Success: search between mid and hi */
            } else {
                hi = mid; /* Failed: search between lo and mid */
            }
        }

        /*
         * Now print if this optimization helped or not
         */
        if (getsockopt(s,SOL_SOCKET, optname, (void *) &curbuf,
                    &curbuflen) == 0) {
            DEBUGMSGTL(("socket:buffer:max", 
                        "Maximized %s: %d\n",buftype, curbuf));
        } 
    } else {
        /*
         * There is really not a lot we can do anymore.
         * If the OS doesn't give us the current buffer, then what's the 
         * point in trying to make it better
         */
        DEBUGMSGTL(("socket:buffer:max", "Get %s failed ... giving up!\n",
                    buftype));
        curbuf = -1;
    }

    return curbuf;
}


static const char *
_sock_buf_type_get(int optname, int local)
{
    if (optname == SO_SNDBUF) {
        if (local)
            return "server send buffer";
        else
            return "client send buffer";
    } else if (optname == SO_RCVBUF) {
        if (local)
            return "server receive buffer";
        else
            return "client receive buffer";
    }

    return "unknown buffer";
}

/*
 *
 * Get the requested buffersize, based on
 * - sockettype : client (local = 0) or server (local = 1) 
 * - buffertype : send (optname = SO_SNDBUF) or recv (SO_RCVBUF)
 *
 * In case a compile time buffer was specified, then use that one
 * if there was no runtime configuration override
 */
static int
_sock_buffer_size_get(int optname, int local, const char **buftype)
{
    int size;

    if (NULL != buftype)
        *buftype = _sock_buf_type_get(optname, local);

    if (optname == SO_SNDBUF) {
        if (local) {
            size = netsnmp_ds_get_int(NETSNMP_DS_LIBRARY_ID, 
                    NETSNMP_DS_LIB_SERVERSENDBUF);
#ifdef NETSNMP_DEFAULT_SERVER_SEND_BUF
            if (size <= 0)
               size = NETSNMP_DEFAULT_SERVER_SEND_BUF;
#endif
        } else {
            size = netsnmp_ds_get_int(NETSNMP_DS_LIBRARY_ID, 
                    NETSNMP_DS_LIB_CLIENTSENDBUF);
#ifdef NETSNMP_DEFAULT_CLIENT_SEND_BUF
            if (size <= 0)
               size = NETSNMP_DEFAULT_CLIENT_SEND_BUF;
#endif
        }
    } else if (optname == SO_RCVBUF) {
        if (local) {
            size = netsnmp_ds_get_int(NETSNMP_DS_LIBRARY_ID, 
                    NETSNMP_DS_LIB_SERVERRECVBUF);
#ifdef NETSNMP_DEFAULT_SERVER_RECV_BUF
            if (size <= 0)
               size = NETSNMP_DEFAULT_SERVER_RECV_BUF;
#endif
        } else {
            size = netsnmp_ds_get_int(NETSNMP_DS_LIBRARY_ID, 
                    NETSNMP_DS_LIB_CLIENTRECVBUF);
#ifdef NETSNMP_DEFAULT_CLIENT_RECV_BUF
            if (size <= 0)
               size = NETSNMP_DEFAULT_CLIENT_RECV_BUF;
#endif
        }
    } else {
        size = 0;
    }

    DEBUGMSGTL(("socket:buffer", "Requested %s is %d\n",
                (buftype) ? *buftype : "unknown buffer", size));

    return(size);
}

/*
 * set socket buffer size
 *
 * @param ss     : socket
 * @param optname: SO_SNDBUF or SO_RCVBUF
 * @param local  : 1 for server, 0 for client
 * @param reqbuf : requested size, or 0 for default
 *
 * @retval    -1 : error
 * @retval    >0 : new buffer size
 */
int
netsnmp_sock_buffer_set(int s, int optname, int local, int size)
{
#if ! defined(SO_SNDBUF) && ! defined(SO_RCVBUF)
    DEBUGMSGTL(("socket:buffer", "Changing socket buffer is not supported\n"));
    return -1;
#else
    const char     *buftype;
    int            curbuf = 0;
    size_t         curbuflen = sizeof(int);

#   ifndef  SO_SNDBUF
    if (SO_SNDBUF == optname) {
        DEBUGMSGTL(("socket:buffer",
                    "Changing socket send buffer is not supported\n"));
        return -1;
    }
#   endif                          /*SO_SNDBUF */
#   ifndef  SO_RCVBUF
    if (SO_RCVBUF == optname) {
        DEBUGMSGTL(("socket:buffer",
                    "Changing socket receive buffer is not supported\n"));
        return -1;
    }
#   endif                          /*SO_RCVBUF */

    /*
     * What is the requested buffer size ?
     */
    if (0 == size)
        size = _sock_buffer_size_get(optname, local, &buftype);
    else {
        buftype = _sock_buf_type_get(optname, local);
        DEBUGMSGT(("verbose:socket:buffer", "Requested %s is %d\n",
                   buftype, size));
    }

    if ((getsockopt(s, SOL_SOCKET, optname, (void *) &curbuf,
                    &curbuflen) == 0) 
        && (curbuflen == sizeof(int))) {
        
        DEBUGMSGT(("verbose:socket:buffer", "Original %s is %d\n",
                   buftype, curbuf));
        if (curbuf >= size) {
            DEBUGMSGT(("verbose:socket:buffer",
                      "New %s size is smaller than original!\n", buftype));
        }
    }

    /*
     * If the buffersize was not specified or it was a negative value
     * then don't change the OS buffers at all
     */
    if (size <= 0) {
       DEBUGMSGT(("socket:buffer",
                    "%s not valid or not specified; using OS default(%d)\n",
                    buftype,curbuf));
       return curbuf;
    }

    /*
     * Try to set the requested send buffer
     */
    if (setsockopt(s, SOL_SOCKET, optname, (void *) &size, sizeof(int)) == 0) {
        /*
         * Because some platforms lie about the actual buffer that has been 
         * set (Linux will always say it worked ...), we print some 
         * diagnostic output for debugging
         */
        DEBUGIF("socket:buffer") {
            DEBUGMSGT(("socket:buffer", "Set %s to %d\n",
                       buftype, size));
            if ((getsockopt(s, SOL_SOCKET, optname, (void *) &curbuf,
                            &curbuflen) == 0) 
                    && (curbuflen == sizeof(int))) {

                DEBUGMSGT(("verbose:socket:buffer",
                           "Now %s is %d\n", buftype, curbuf));
            }
        }
        /*
         * If the new buffer is smaller than the size we requested, we will
         * try to increment the new buffer with 1k increments 
         * (this will sometime allow us to reach a more optimal buffer.)
         *   For example : On Solaris, if the max OS buffer is 100k and you
         *   request 110k, you end up with the default 8k :-(
         */
        if (curbuf < size) {
            curbuf = _sock_buffer_maximize(s, optname, buftype, size);
            if(-1 != curbuf)
                size = curbuf;
        }

    } else {
        /*
         * Obviously changing the buffer failed, most like like because we 
         * requested a buffer greater than the OS limit.
         * Therefore we need to search for an optimal buffer that is close
         * enough to the point of failure.
         * This will allow us to reach a more optimal buffer.
         *   For example : On Solaris, if the max OS buffer is 100k and you 
         *   request 110k, you end up with the default 8k :-(
         *   After this quick seach we would get 1k close to 100k (the max)
         */
        DEBUGMSGTL(("socket:buffer", "couldn't set %s to %d\n",
                    buftype, size));

        curbuf = _sock_buffer_maximize(s, optname, buftype, size);
        if(-1 != curbuf)
            size = curbuf;
    }

    return size;
#endif
}

/*
 * Open a UDP-based transport for SNMP.  Local is TRUE if addr is the local
 * address to bind to (i.e. this is a server-type session); otherwise addr is 
 * the remote address to send things to.  
 */

netsnmp_transport *
netsnmp_udp_transport(struct sockaddr_in *addr, int local)
{
    netsnmp_transport *t = NULL;
    int             rc = 0;
    char           *str = NULL;
    char           *client_socket = NULL;
    netsnmp_udp_addr_pair *addr_pair = NULL;

    if (addr == NULL || addr->sin_family != AF_INET) {
        return NULL;
    }

    addr_pair = (netsnmp_udp_addr_pair *) malloc(sizeof(netsnmp_udp_addr_pair));
    if (addr_pair == NULL) {
        return NULL;
    }
    memset(addr_pair, 0, sizeof(netsnmp_udp_addr_pair));
    memcpy(&(addr_pair->remote_addr), addr, sizeof(struct sockaddr_in));

    t = (netsnmp_transport *) malloc(sizeof(netsnmp_transport));
    if (t == NULL) {
        return NULL;
    }

    str = netsnmp_udp_fmtaddr(NULL, (void *)addr_pair, 
                                 sizeof(netsnmp_udp_addr_pair));
    DEBUGMSGTL(("netsnmp_udp", "open %s %s:%d\n", local ? "local" : "remote",
                str,addr->sin_port));
    free(str);

    memset(t, 0, sizeof(netsnmp_transport));

    t->domain = netsnmpUDPDomain;
    t->domain_length = netsnmpUDPDomain_len;

    t->sock = socket(PF_INET, SOCK_DGRAM, 0);
    if (t->sock < 0) {
        netsnmp_transport_free(t);
        return NULL;
    }

    _netsnmp_udp_sockopt_set(t->sock, local);

    if (local) {
        /*
         * This session is inteneded as a server, so we must bind on to the
         * given IP address, which may include an interface address, or could
         * be INADDR_ANY, but certainly includes a port number.
         */

      t->local = (u_char *) malloc(6);
        if (t->local == NULL) {
            netsnmp_transport_free(t);
            return NULL;
        }
        memcpy(t->local, (u_char *) & (addr->sin_addr.s_addr), 4);
        t->local[4] = (htons(addr->sin_port) & 0xff00) >> 8;
        t->local[5] = (htons(addr->sin_port) & 0x00ff) >> 0;
        t->local_length = 6;

#if defined(linux) && defined(IP_PKTINFO)
        { 
            int sockopt = 1;
            if (setsockopt(t->sock, SOL_IP, IP_PKTINFO, &sockopt, sizeof sockopt) == -1) {
                DEBUGMSGTL(("netsnmp_udp", "couldn't set IP_PKTINFO: %s\n",
                    strerror(errno)));
                return NULL;
            }
            DEBUGMSGTL(("netsnmp_udp", "set IP_PKTINFO\n"));
        }
#endif
        rc = bind(t->sock, (struct sockaddr *) addr,
                  sizeof(struct sockaddr));
        if (rc != 0) {
            netsnmp_udp_close(t);
            netsnmp_transport_free(t);
            return NULL;
        }
        t->data = NULL;
        t->data_length = 0;
    } else {
        /*
         * This is a client session.  If we've been given a
         * client address to send from, then bind to that.
         * Otherwise the send will use "something sensible".
         */
        client_socket = netsnmp_ds_get_string(NETSNMP_DS_LIBRARY_ID,
                                              NETSNMP_DS_LIB_CLIENT_ADDR);
        if (client_socket) {
            struct sockaddr_in client_addr;
            netsnmp_sockaddr_in2(&client_addr, client_socket, NULL);
            client_addr.sin_port = 0;
            bind(t->sock, (struct sockaddr *)&client_addr,
                  sizeof(struct sockaddr));
        }
        /*
         * Save the (remote) address in the
         * transport-specific data pointer for later use by netsnmp_udp_send.
         */

        t->data = malloc(sizeof(netsnmp_udp_addr_pair));
        t->remote = (u_char *)malloc(6);
        if (t->data == NULL || t->remote == NULL) {
            netsnmp_transport_free(t);
            return NULL;
        }
        memcpy(t->remote, (u_char *) & (addr->sin_addr.s_addr), 4);
        t->remote[4] = (htons(addr->sin_port) & 0xff00) >> 8;
        t->remote[5] = (htons(addr->sin_port) & 0x00ff) >> 0;
        t->remote_length = 6;
        memcpy(t->data, addr_pair, sizeof(netsnmp_udp_addr_pair));
        t->data_length = sizeof(netsnmp_udp_addr_pair);
    }

    /*
     * 16-bit length field, 8 byte UDP header, 20 byte IPv4 header  
     */

    t->msgMaxSize = 0xffff - 8 - 20;
    t->f_recv     = netsnmp_udp_recv;
    t->f_send     = netsnmp_udp_send;
    t->f_close    = netsnmp_udp_close;
    t->f_accept   = NULL;
    t->f_fmtaddr  = netsnmp_udp_fmtaddr;

    return t;
}


void
_netsnmp_udp_sockopt_set(int fd, int local)
{
#ifdef  SO_BSDCOMPAT
    /*
     * Patch for Linux.  Without this, UDP packets that fail get an ICMP
     * response.  Linux turns the failed ICMP response into an error message
     * and return value, unlike all other OS's.  
     */
    if (0 == netsnmp_os_prematch("Linux","2.4"))
    {
        int             one = 1;
        DEBUGMSGTL(("socket:option", "setting socket option SO_BSDCOMPAT\n"));
        setsockopt(fd, SOL_SOCKET, SO_BSDCOMPAT, (void *) &one,
                   sizeof(one));
    }
#endif                          /*SO_BSDCOMPAT */
    /*
     * SO_REUSEADDR will allow multiple apps to open the same port at
     * the same time. Only the last one to open the socket will get
     * data. Obviously, for an agent, this is a bad thing. There should
     * only be one listener.
     */
#ifdef ALLOW_PORT_HIJACKING
#ifdef  SO_REUSEADDR
    /*
     * Allow the same port to be specified multiple times without failing.
     *    (useful for a listener)
     */
    {
        int             one = 1;
        DEBUGMSGTL(("socket:option", "setting socket option SO_REUSEADDR\n"));
        setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (void *) &one,
                   sizeof(one));
    }
#endif                          /*SO_REUSEADDR */
#endif

    /*
     * Try to set the send and receive buffers to a reasonably large value, so
     * that we can send and receive big PDUs (defaults to 8192 bytes (!) on
     * Solaris, for instance).  Don't worry too much about errors -- just
     * plough on regardless.  
     */
    netsnmp_sock_buffer_set(fd, SO_SNDBUF, local, 0);
    netsnmp_sock_buffer_set(fd, SO_RCVBUF, local, 0);
}

int
netsnmp_sockaddr_in2(struct sockaddr_in *addr,
                     const char *inpeername, const char *default_target)
{
#if HAVE_GETADDRINFO
    struct addrinfo *addrs = NULL;
    struct addrinfo hint;
    int             err;
#elif HAVE_GETIPNODEBYNAME
    struct hostent *hp = NULL;
    int             err;
#elif HAVE_GETHOSTBYNAME
    struct hostent *hp = NULL;
#endif

    if (addr == NULL) {
        return 0;
    }

    DEBUGMSGTL(("netsnmp_sockaddr_in",
                "addr %p, inpeername \"%s\", default_target \"%s\"\n",
                addr, inpeername ? inpeername : "[NIL]",
                default_target ? default_target : "[NIL]"));

    if (default_target == NULL ||
	netsnmp_sockaddr_in2(addr, default_target, NULL) == 0) {
	int port;

	memset(addr, 0, sizeof(struct sockaddr_in));
	addr->sin_addr.s_addr = htonl(INADDR_ANY);
	addr->sin_family = AF_INET;
	port = netsnmp_ds_get_int(NETSNMP_DS_LIBRARY_ID,
				  NETSNMP_DS_LIB_DEFAULT_PORT);
	if (port == 0)
	    port = SNMP_PORT;
	addr->sin_port = htons((u_short)port);
    }

    if (inpeername != NULL && *inpeername != '\0') {
	const char     *host, *port;
	char           *peername = NULL;
        char           *cp;
        /*
         * Duplicate the peername because we might want to mank around with
         * it.  
         */

        peername = strdup(inpeername);
        if (peername == NULL) {
            return 0;
        }

        /*
         * Try and extract an appended port number.  
         */
        cp = strchr(peername, ':');
        if (cp != NULL) {
            *cp = '\0';
            port = cp + 1;
            host = peername;
        } else {
            host = NULL;
            port = peername;
        }

        /*
         * Try to convert the user port specifier
         */
        if (port && *port == '\0')
            port = NULL;

        if (port != NULL) {
            long int l;
            char* ep;

            DEBUGMSGTL(("netsnmp_sockaddr_in", "check user service %s\n",
                        port));

            l = strtol(port, &ep, 10);
            if (ep != port && *ep == '\0' && 0 <= l && l <= 0x0ffff)
                addr->sin_port = htons((u_short)l);
            else {
                if (host == NULL) {
                    DEBUGMSGTL(("netsnmp_sockaddr_in",
                                "servname not numeric, "
				"check if it really is a destination)"));
                    host = port;
                    port = NULL;
                } else {
                    DEBUGMSGTL(("netsnmp_sockaddr_in",
                                "servname not numeric"));
                    free(peername);
                    return 0;
                }
            }
        }

        /*
         * Try to convert the user host specifier
         */
        if (host && *host == '\0')
            host = NULL;

        if (host != NULL) {
            DEBUGMSGTL(("netsnmp_sockaddr_in",
                        "check destination %s\n", host));

#if HAVE_GETADDRINFO
            memset(&hint, 0, sizeof hint);
            hint.ai_flags = 0;
            hint.ai_family = PF_INET;
            hint.ai_socktype = SOCK_DGRAM;
            hint.ai_protocol = 0;

            err = getaddrinfo(peername, NULL, &hint, &addrs);
            if (err != 0) {
                snmp_log(LOG_ERR, "getaddrinfo: %s %s\n", peername,
                         gai_strerror(err));
                free(peername);
                return 0;
            }
            if (addrs != NULL) {
                DEBUGMSGTL(("netsnmp_sockaddr_in",
                            "hostname (resolved okay)\n"));
                memcpy(&addr->sin_addr,
                       &((struct sockaddr_in *) addrs->ai_addr)->sin_addr,
                       sizeof(struct in_addr));
                freeaddrinfo(addrs);
            }
            else {
                DEBUGMSGTL(("netsnmp_sockaddr_in",
                            "Failed to resolve IPv4 hostname\n"));
            }
#elif HAVE_GETHOSTBYNAME
            hp = gethostbyname(host);
            if (hp == NULL) {
                DEBUGMSGTL(("netsnmp_sockaddr_in",
                            "hostname (couldn't resolve)\n"));
                free(peername);
                return 0;
            } else if (hp->h_addrtype != AF_INET) {
                DEBUGMSGTL(("netsnmp_sockaddr_in",
                            "hostname (not AF_INET!)\n"));
                free(peername);
                return 0;
            } else {
                DEBUGMSGTL(("netsnmp_sockaddr_in",
                            "hostname (resolved okay)\n"));
                memcpy(&addr->sin_addr, hp->h_addr, hp->h_length);
            }
#elif HAVE_GETIPNODEBYNAME
            hp = getipnodebyname(peername, AF_INET, 0, &err);
            if (hp == NULL) {
                DEBUGMSGTL(("netsnmp_sockaddr_in",
                            "hostname (couldn't resolve = %d)\n", err));
                free(peername);
                return 0;
            }
            DEBUGMSGTL(("netsnmp_sockaddr_in",
                        "hostname (resolved okay)\n"));
            memcpy(&(addr->sin_addr), hp->h_addr, hp->h_length);
#else /* HAVE_GETIPNODEBYNAME */
            /*
             * There is no name resolving function available.
             */
            DEBUGMSGTL(("netsnmp_sockaddr_in",
                        "no getaddrinfo()/getipnodebyname()/gethostbyname()\n"));
            free(peername);
            return 0;
#endif /* HAVE_GETHOSTBYNAME */
        }
	free(peername);
    }

    /*
     * Finished
     */

    DEBUGMSGTL(("netsnmp_sockaddr_in", "return { AF_INET, %s:%hu }\n",
                inet_ntoa(addr->sin_addr), ntohs(addr->sin_port)));
    return 1;
}


int
netsnmp_sockaddr_in(struct sockaddr_in *addr,
                    const char *inpeername, int remote_port)
{
    char buf[sizeof(int) * 3 + 2];
    sprintf(buf, ":%u", remote_port);
    return netsnmp_sockaddr_in2(addr, inpeername, remote_port ? buf : NULL);
}

#if !defined(NETSNMP_DISABLE_SNMPV1) || !defined(NETSNMP_DISABLE_SNMPV2C)
/*
 * The following functions provide the "com2sec" configuration token
 * functionality for compatibility.  
 */

#define EXAMPLE_NETWORK		"NETWORK"
#define EXAMPLE_COMMUNITY	"COMMUNITY"

typedef struct _com2SecEntry {
    char            community[VACMSTRINGLEN];
    unsigned long   network;
    unsigned long   mask;
    char            secName[VACMSTRINGLEN];
    char            contextName[VACMSTRINGLEN];
    struct _com2SecEntry *next;
} com2SecEntry;

com2SecEntry   *com2SecList = NULL, *com2SecListLast = NULL;

void
netsnmp_udp_parse_security(const char *token, char *param)
{
    char            secName[VACMSTRINGLEN];
    char            contextName[VACMSTRINGLEN];
    char            community[VACMSTRINGLEN];
    char            source[SNMP_MAXBUF_SMALL];
    char           *cp = NULL;
    const char     *strmask = NULL;
    com2SecEntry   *e = NULL;
    in_addr_t   network = 0, mask = 0;

    /*
     * Get security, source address/netmask and community strings.  
     */

    cp = copy_nword( param, secName, sizeof(secName));
    if (strcmp(secName, "-Cn") == 0) {
        if (!cp) {
            config_perror("missing CONTEXT_NAME parameter");
            return;
        }
        cp = copy_nword( cp, contextName, sizeof(contextName));
        cp = copy_nword( cp, secName, sizeof(secName));
    } else {
        contextName[0] = '\0';
    }
    if (secName[0] == '\0') {
        config_perror("missing NAME parameter");
        return;
    } else if (strlen(secName) > (VACMSTRINGLEN - 1)) {
        config_perror("security name too long");
        return;
    }
    cp = copy_nword( cp, source, sizeof(source));
    if (source[0] == '\0') {
        config_perror("missing SOURCE parameter");
        return;
    } else if (strncmp(source, EXAMPLE_NETWORK, strlen(EXAMPLE_NETWORK)) ==
               0) {
        config_perror("example config NETWORK not properly configured");
        return;
    }
    cp = copy_nword( cp, community, sizeof(community));
    if (community[0] == '\0') {
        config_perror("missing COMMUNITY parameter\n");
        return;
    } else
        if (strncmp
            (community, EXAMPLE_COMMUNITY, strlen(EXAMPLE_COMMUNITY))
            == 0) {
        config_perror("example config COMMUNITY not properly configured");
        return;
    } else if (strlen(community) > (VACMSTRINGLEN - 1)) {
        config_perror("community name too long");
        return;
    }

    /*
     * Process the source address/netmask string.  
     */

    cp = strchr(source, '/');
    if (cp != NULL) {
        /*
         * Mask given.  
         */
        *cp = '\0';
        strmask = cp + 1;
    }

    /*
     * Deal with the network part first.  
     */

    if ((strcmp(source, "default") == 0)
        || (strcmp(source, "0.0.0.0") == 0)) {
        network = 0;
        strmask = "0.0.0.0";
    } else {
        /*
         * Try interpreting as a dotted quad.  
         */
        network = inet_addr(source);

        if (network == (in_addr_t) -1) {
            /*
             * Nope, wasn't a dotted quad.  Must be a hostname.  
             */
#ifdef  HAVE_GETHOSTBYNAME
            struct hostent *hp = gethostbyname(source);
            if (hp == NULL) {
                config_perror("bad source address");
                return;
            } else {
                if (hp->h_addrtype != AF_INET) {
                    config_perror("no IP address for source hostname");
                    return;
                }
                network = *((in_addr_t *) hp->h_addr);
            }
#else                           /*HAVE_GETHOSTBYNAME */
            /*
             * Oh dear.  
             */
            config_perror("cannot resolve source hostname");
            return;
#endif                          /*HAVE_GETHOSTBYNAME */
        }
    }

    /*
     * Now work out the mask.  
     */

    if (strmask == NULL || *strmask == '\0') {
        /*
         * No mask was given.  Use 255.255.255.255.  
         */
        mask = 0xffffffffL;
    } else {
        if (strchr(strmask, '.')) {
            /*
             * Try to interpret mask as a dotted quad.  
             */
            mask = inet_addr(strmask);
            if (mask == (in_addr_t) -1 &&
                strncmp(strmask, "255.255.255.255", 15) != 0) {
                config_perror("bad mask");
                return;
            }
        } else {
            /*
             * Try to interpret mask as a "number of 1 bits".  
             */
            int             maskLen = atoi(strmask), maskBit = 0x80000000L;
            if (maskLen <= 0 || maskLen > 32) {
                config_perror("bad mask length");
                return;
            }
            while (maskLen--) {
                mask |= maskBit;
                maskBit >>= 1;
            }
            mask = htonl(mask);
        }
    }

    /*
     * Check that the network and mask are consistent.  
     */

    if (network & ~mask) {
        config_perror("source/mask mismatch");
        return;
    }

    e = (com2SecEntry *) malloc(sizeof(com2SecEntry));
    if (e == NULL) {
        config_perror("memory error");
        return;
    }

    /*
     * Everything is okay.  Copy the parameters to the structure allocated
     * above and add it to END of the list.  
     */

    DEBUGMSGTL(("netsnmp_udp_parse_security",
                "<\"%s\", 0x%08x/0x%08x> => \"%s\"\n", community, network,
                mask, secName));

    strcpy(e->contextName, contextName);
    strcpy(e->secName, secName);
    strcpy(e->community, community);
    e->network = network;
    e->mask = mask;
    e->next = NULL;

    if (com2SecListLast != NULL) {
        com2SecListLast->next = e;
        com2SecListLast = e;
    } else {
        com2SecListLast = com2SecList = e;
    }
}


void
netsnmp_udp_com2SecList_free(void)
{
    com2SecEntry   *e = com2SecList;
    while (e != NULL) {
        com2SecEntry   *tmp = e;
        e = e->next;
        free(tmp);
    }
    com2SecList = com2SecListLast = NULL;
}
#endif /* support for community based SNMP */

void
netsnmp_udp_agent_config_tokens_register(void)
{
#if !defined(NETSNMP_DISABLE_SNMPV1) || !defined(NETSNMP_DISABLE_SNMPV2C)
    register_app_config_handler("com2sec", netsnmp_udp_parse_security,
                                netsnmp_udp_com2SecList_free,
                                "[-Cn CONTEXT] secName IPv4-network-address[/netmask] community");
#endif /* support for community based SNMP */
}



/*
 * Return 0 if there are no com2sec entries, or return 1 if there ARE com2sec 
 * entries.  On return, if a com2sec entry matched the passed parameters,
 * then *secName points at the appropriate security name, or is NULL if the
 * parameters did not match any com2sec entry.  
 */

#if !defined(NETSNMP_DISABLE_SNMPV1) || !defined(NETSNMP_DISABLE_SNMPV2C)
int
netsnmp_udp_getSecName(void *opaque, int olength,
                       const char *community,
                       size_t community_len, char **secName,
                       char **contextName)
{
    com2SecEntry   *c;
    netsnmp_udp_addr_pair *addr_pair = (netsnmp_udp_addr_pair *) opaque;
    struct sockaddr_in *from = (struct sockaddr_in *) &(addr_pair->remote_addr);
    char           *ztcommunity = NULL;

    if (secName != NULL) {
        *secName = NULL;  /* Haven't found anything yet */
    }

    /*
     * Special case if there are NO entries (as opposed to no MATCHING
     * entries).  
     */

    if (com2SecList == NULL) {
        DEBUGMSGTL(("netsnmp_udp_getSecName", "no com2sec entries\n"));
        return 0;
    }

    /*
     * If there is no IPv4 source address, then there can be no valid security
     * name.  
     */

    if (opaque == NULL || olength != sizeof(netsnmp_udp_addr_pair) ||
        from->sin_family != AF_INET) {
        DEBUGMSGTL(("netsnmp_udp_getSecName",
		    "no IPv4 source address in PDU?\n"));
        return 1;
    }

    DEBUGIF("netsnmp_udp_getSecName") {
	ztcommunity = (char *)malloc(community_len + 1);
	if (ztcommunity != NULL) {
	    memcpy(ztcommunity, community, community_len);
	    ztcommunity[community_len] = '\0';
	}

	DEBUGMSGTL(("netsnmp_udp_getSecName", "resolve <\"%s\", 0x%08x>\n",
		    ztcommunity ? ztcommunity : "<malloc error>",
		    from->sin_addr.s_addr));
    }

    for (c = com2SecList; c != NULL; c = c->next) {
        DEBUGMSGTL(("netsnmp_udp_getSecName","compare <\"%s\", 0x%08x/0x%08x>",
		    c->community, c->network, c->mask));
        if ((community_len == strlen(c->community)) &&
	    (memcmp(community, c->community, community_len) == 0) &&
            ((from->sin_addr.s_addr & c->mask) == c->network)) {
            DEBUGMSG(("netsnmp_udp_getSecName", "... SUCCESS\n"));
            if (secName != NULL) {
                *secName = c->secName;
                *contextName = c->contextName;
            }
            break;
        }
        DEBUGMSG(("netsnmp_udp_getSecName", "... nope\n"));
    }
    if (ztcommunity != NULL) {
        free(ztcommunity);
    }
    return 1;
}
#endif /* support for community based SNMP */


netsnmp_transport *
netsnmp_udp_create_tstring(const char *str, int local,
			   const char *default_target)
{
    struct sockaddr_in addr;

    if (netsnmp_sockaddr_in2(&addr, str, default_target)) {
        return netsnmp_udp_transport(&addr, local);
    } else {
        return NULL;
    }
}


netsnmp_transport *
netsnmp_udp_create_ostring(const u_char * o, size_t o_len, int local)
{
    struct sockaddr_in addr;

    if (o_len == 6) {
        unsigned short porttmp = (o[4] << 8) + o[5];
        addr.sin_family = AF_INET;
        memcpy((u_char *) & (addr.sin_addr.s_addr), o, 4);
        addr.sin_port = htons(porttmp);
        return netsnmp_udp_transport(&addr, local);
    }
    return NULL;
}


void
netsnmp_udp_ctor(void)
{
    udpDomain.name = netsnmpUDPDomain;
    udpDomain.name_length = netsnmpUDPDomain_len;
    udpDomain.prefix = (const char**)calloc(2, sizeof(char *));
    udpDomain.prefix[0] = "udp";

    udpDomain.f_create_from_tstring_new = netsnmp_udp_create_tstring;
    udpDomain.f_create_from_ostring = netsnmp_udp_create_ostring;

    netsnmp_tdomain_register(&udpDomain);
}
