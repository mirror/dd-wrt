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
#if HAVE_SYS_UN_H
#include <sys/un.h>
#endif

#if HAVE_DMALLOC_H
#include <dmalloc.h>
#endif

#include <net-snmp/types.h>
#include <net-snmp/output_api.h>
#include <net-snmp/config_api.h>

#include <net-snmp/library/snmp_transport.h>
#include <net-snmp/library/snmpUDPDomain.h>
#include <net-snmp/library/snmpUnixDomain.h>


#ifndef NETSNMP_STREAM_QUEUE_LEN
#define NETSNMP_STREAM_QUEUE_LEN  5
#endif

#ifndef SUN_LEN
/*
 * Evaluate to actual length of the `sockaddr_un' structure.
 */
#define SUN_LEN(ptr) ((size_t) (((struct sockaddr_un *) 0)->sun_path)         \
                      + strlen ((ptr)->sun_path))
#endif

oid netsnmp_UnixDomain[] = { TRANSPORT_DOMAIN_LOCAL };
static netsnmp_tdomain unixDomain;


/*
 * This is the structure we use to hold transport-specific data.
 */

typedef struct _sockaddr_un_pair {
    int             local;
    struct sockaddr_un server;
    struct sockaddr_un client;
} sockaddr_un_pair;


/*
 * Return a string representing the address in data, or else the "far end"
 * address if data is NULL.
 */

static char *
netsnmp_unix_fmtaddr(netsnmp_transport *t, void *data, int len)
{
    struct sockaddr_un *to = NULL;

    if (data != NULL) {
        to = (struct sockaddr_un *) data;
    } else if (t != NULL && t->data != NULL) {
        to = &(((sockaddr_un_pair *) t->data)->server);
        len = SUN_LEN(to);
    }
    if (to == NULL) {
        /*
         * "Local IPC" is the Posix.1g term for Unix domain protocols,
         * according to W. R. Stevens, ``Unix Network Programming Volume I
         * Second Edition'', p. 374.
         */
        return strdup("Local IPC: unknown");
    } else if (to->sun_path[0] == 0) {
        /*
         * This is an abstract name.  We could render it as hex or something
         * but let's not worry about that for now.
         */
        return strdup("Local IPC: abstract");
    } else {
        char           *tmp = (char *) malloc(16 + len);
        if (tmp != NULL) {
            sprintf(tmp, "Local IPC: %s", to->sun_path);
        }
        return tmp;
    }
}



/*
 * You can write something into opaque that will subsequently get passed back
 * to your send function if you like.  For instance, you might want to
 * remember where a PDU came from, so that you can send a reply there...
 */

static int
netsnmp_unix_recv(netsnmp_transport *t, void *buf, int size,
                  void **opaque, int *olength)
{
    int rc = -1;
    socklen_t       tolen = sizeof(struct sockaddr_un);
    struct sockaddr *to;


    if (t != NULL && t->sock >= 0) {
        to = (struct sockaddr *) malloc(sizeof(struct sockaddr_un));
        if (to == NULL) {
            *opaque = NULL;
            *olength = 0;
            return -1;
        } else {
            memset(to, 0, tolen);
        }
        if(getsockname(t->sock, to, &tolen) != 0){
            free(to);
            *opaque = NULL;
            *olength = 0;
            return -1;
        };
        while (rc < 0) {
            rc = recv(t->sock, buf, size, 0);
            if (rc < 0 && errno != EINTR) {
                DEBUGMSGTL(("netsnmp_unix", "recv fd %d err %d (\"%s\")\n",
                            t->sock, errno, strerror(errno)));
                return rc;
            }
            *opaque = (void*)to;
            *olength = sizeof(struct sockaddr_un);
        }
        DEBUGMSGTL(("netsnmp_unix", "recv fd %d got %d bytes\n", t->sock, rc));
    }
    return rc;
}



static int
netsnmp_unix_send(netsnmp_transport *t, void *buf, int size,
                  void **opaque, int *olength)
{
    int rc = -1;

    if (t != NULL && t->sock >= 0) {
        DEBUGMSGTL(("netsnmp_unix", "send %d bytes to %p on fd %d\n",
                    size, buf, t->sock));
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
netsnmp_unix_close(netsnmp_transport *t)
{
    int rc = 0;
    sockaddr_un_pair *sup = (sockaddr_un_pair *) t->data;

    if (t->sock >= 0) {
#ifndef HAVE_CLOSESOCKET
        rc = close(t->sock);
#else
        rc = closesocket(t->sock);
#endif
        t->sock = -1;
        if (sup != NULL) {
            if (sup->local) {
                if (sup->server.sun_path[0] != 0) {
                  DEBUGMSGTL(("netsnmp_unix", "close: server unlink(\"%s\")\n",
                              sup->server.sun_path));
                  unlink(sup->server.sun_path);
                }
            } else {
                if (sup->client.sun_path[0] != 0) {
                  DEBUGMSGTL(("netsnmp_unix", "close: client unlink(\"%s\")\n",
                              sup->client.sun_path));
                  unlink(sup->client.sun_path);
                }
            }
        }
        return rc;
    } else {
        return -1;
    }
}



static int
netsnmp_unix_accept(netsnmp_transport *t)
{
    struct sockaddr *farend = NULL;
    int             newsock = -1;
    socklen_t       farendlen = sizeof(struct sockaddr_un);

    farend = (struct sockaddr *) malloc(farendlen);

    if (farend == NULL) {
        /*
         * Indicate that the acceptance of this socket failed.
         */
        DEBUGMSGTL(("netsnmp_unix", "accept: malloc failed\n"));
        return -1;
    }
    memset(farend, 0, farendlen);

    if (t != NULL && t->sock >= 0) {
        newsock = accept(t->sock, farend, &farendlen);

        if (newsock < 0) {
            DEBUGMSGTL(("netsnmp_unix","accept failed rc %d errno %d \"%s\"\n",
                        newsock, errno, strerror(errno)));
            free(farend);
            return newsock;
        }

        if (t->data != NULL) {
            free(t->data);
        }

        DEBUGMSGTL(("netsnmp_unix", "accept succeeded (farend %p len %d)\n",
                    farend, farendlen));
        t->data = farend;
        t->data_length = sizeof(struct sockaddr_un);
       netsnmp_sock_buffer_set(newsock, SO_SNDBUF, 1, 0);
       netsnmp_sock_buffer_set(newsock, SO_RCVBUF, 1, 0);
        return newsock;
    } else {
        free(farend);
        return -1;
    }
}



/*
 * Open a Unix-domain transport for SNMP.  Local is TRUE if addr is the local
 * address to bind to (i.e. this is a server-type session); otherwise addr is
 * the remote address to send things to (and we make up a temporary name for
 * the local end of the connection).
 */

netsnmp_transport *
netsnmp_unix_transport(struct sockaddr_un *addr, int local)
{
    netsnmp_transport *t = NULL;
    sockaddr_un_pair *sup = NULL;
    int             rc = 0;
    char           *string = NULL;

    if (addr == NULL || addr->sun_family != AF_UNIX) {
        return NULL;
    }

    t = (netsnmp_transport *) malloc(sizeof(netsnmp_transport));
    if (t == NULL) {
        return NULL;
    }

    string = netsnmp_unix_fmtaddr(NULL, (void *)addr,
                                  sizeof(struct sockaddr_un));
    DEBUGMSGTL(("netsnmp_unix", "open %s %s\n", local ? "local" : "remote",
                string));
    free(string);

    memset(t, 0, sizeof(netsnmp_transport));

    t->domain = netsnmp_UnixDomain;
    t->domain_length =
        sizeof(netsnmp_UnixDomain) / sizeof(netsnmp_UnixDomain[0]);

    t->data = malloc(sizeof(sockaddr_un_pair));
    if (t->data == NULL) {
        netsnmp_transport_free(t);
        return NULL;
    }
    memset(t->data, 0, sizeof(sockaddr_un_pair));
    t->data_length = sizeof(sockaddr_un_pair);
    sup = (sockaddr_un_pair *) t->data;

    t->sock = socket(PF_UNIX, SOCK_STREAM, 0);
    if (t->sock < 0) {
        netsnmp_transport_free(t);
        return NULL;
    }

    t->flags = NETSNMP_TRANSPORT_FLAG_STREAM;

    if (local) {
      t->local = (u_char *)malloc(strlen(addr->sun_path));
        if (t->local == NULL) {
            netsnmp_transport_free(t);
            return NULL;
        }
        memcpy(t->local, addr->sun_path, strlen(addr->sun_path));
        t->local_length = strlen(addr->sun_path);

        /*
         * This session is inteneded as a server, so we must bind to the given
         * path (unlinking it first, to avoid errors).
         */

        t->flags |= NETSNMP_TRANSPORT_FLAG_LISTEN;

        unlink(addr->sun_path);
        rc = bind(t->sock, (struct sockaddr *) addr, SUN_LEN(addr));
        if (rc != 0) {
            DEBUGMSGTL(("netsnmp_unix_transport",
                        "couldn't bind \"%s\", errno %d (%s)\n",
                        addr->sun_path, errno, strerror(errno)));
            netsnmp_unix_close(t);
            netsnmp_transport_free(t);
            return NULL;
        }

        /*
         * Save the address in the transport-specific data pointer for later
         * use by netsnmp_unix_close.
         */

        sup->server.sun_family = AF_UNIX;
        strcpy(sup->server.sun_path, addr->sun_path);
        sup->local = 1;

        /*
         * Now sit here and listen for connections to arrive.
         */

        rc = listen(t->sock, NETSNMP_STREAM_QUEUE_LEN);
        if (rc != 0) {
            DEBUGMSGTL(("netsnmp_unix_transport",
                        "couldn't listen to \"%s\", errno %d (%s)\n",
                        addr->sun_path, errno, strerror(errno)));
            netsnmp_unix_close(t);
            netsnmp_transport_free(t);
            return NULL;
        }

    } else {
      t->remote = (u_char *)malloc(strlen(addr->sun_path));
        if (t->remote == NULL) {
            netsnmp_transport_free(t);
            return NULL;
        }
        memcpy(t->remote, addr->sun_path, strlen(addr->sun_path));
        t->remote_length = strlen(addr->sun_path);

        rc = connect(t->sock, (struct sockaddr *) addr,
                     sizeof(struct sockaddr_un));
        if (rc != 0) {
            DEBUGMSGTL(("netsnmp_unix_transport",
                        "couldn't connect to \"%s\", errno %d (%s)\n",
                        addr->sun_path, errno, strerror(errno)));
            netsnmp_unix_close(t);
            netsnmp_transport_free(t);
            return NULL;
        }

        /*
         * Save the remote address in the transport-specific data pointer for
         * later use by netsnmp_unix_send.
         */

        sup->server.sun_family = AF_UNIX;
        strcpy(sup->server.sun_path, addr->sun_path);
        sup->local = 0;
       netsnmp_sock_buffer_set(t->sock, SO_SNDBUF, local, 0);
       netsnmp_sock_buffer_set(t->sock, SO_RCVBUF, local, 0);
    }

    /*
     * Message size is not limited by this transport (hence msgMaxSize
     * is equal to the maximum legal size of an SNMP message).
     */

    t->msgMaxSize = 0x7fffffff;
    t->f_recv     = netsnmp_unix_recv;
    t->f_send     = netsnmp_unix_send;
    t->f_close    = netsnmp_unix_close;
    t->f_accept   = netsnmp_unix_accept;
    t->f_fmtaddr  = netsnmp_unix_fmtaddr;

    return t;
}

netsnmp_transport *
netsnmp_unix_create_tstring(const char *string, int local,
			    const char *default_target)
{
    struct sockaddr_un addr;

    if (string && *string != '\0') {
    } else if (default_target && *default_target != '\0') {
      string = default_target;
    }

    if ((string != NULL && *string != '\0') &&
	(strlen(string) < sizeof(addr.sun_path))) {
        addr.sun_family = AF_UNIX;
        memset(addr.sun_path, 0, sizeof(addr.sun_path));
        strncpy(addr.sun_path, string, sizeof(addr.sun_path) - 1);
        return netsnmp_unix_transport(&addr, local);
    } else {
        if (string != NULL && *string != '\0') {
            snmp_log(LOG_ERR, "Path too long for Unix domain transport\n");
        }
        return NULL;
    }
}



netsnmp_transport *
netsnmp_unix_create_ostring(const u_char * o, size_t o_len, int local)
{
    struct sockaddr_un addr;

    if (o_len > 0 && o_len < (sizeof(addr.sun_path) - 1)) {
        addr.sun_family = AF_UNIX;
        memset(addr.sun_path, 0, sizeof(addr.sun_path));
        strncpy(addr.sun_path, (char *)o, o_len);
        return netsnmp_unix_transport(&addr, local);
    } else {
        if (o_len > 0) {
            snmp_log(LOG_ERR, "Path too long for Unix domain transport\n");
        }
    }
    return NULL;
}



void
netsnmp_unix_ctor(void)
{
    unixDomain.name = netsnmp_UnixDomain;
    unixDomain.name_length = sizeof(netsnmp_UnixDomain) / sizeof(oid);
    unixDomain.prefix = (const char**)calloc(2, sizeof(char *));
    unixDomain.prefix[0] = "unix";

    unixDomain.f_create_from_tstring_new = netsnmp_unix_create_tstring;
    unixDomain.f_create_from_ostring = netsnmp_unix_create_ostring;

    netsnmp_tdomain_register(&unixDomain);
}

#if !defined(NETSNMP_DISABLE_SNMPV1) || !defined(NETSNMP_DISABLE_SNMPV2C)
/* support for SNMPv1 and SNMPv2c on unix domain*/

#define EXAMPLE_COMMUNITY "COMMUNITY"
typedef struct _com2SecUnixEntry {
    char            community[VACMSTRINGLEN];
    char            sockpath[sizeof(struct sockaddr_un)];
    unsigned long   pathlen;
    char            secName[VACMSTRINGLEN];
    char            contextName[VACMSTRINGLEN];
    struct _com2SecUnixEntry *next;
} com2SecUnixEntry;

com2SecUnixEntry   *com2SecUnixList = NULL, *com2SecUnixListLast = NULL;


int
netsnmp_unix_getSecName(void *opaque, int olength,
                        const char *community,
                        size_t community_len,
                        char **secName, char **contextName)
{
    com2SecUnixEntry   *c;
    struct sockaddr_un *to = (struct sockaddr_un *) opaque;
    char           *ztcommunity = NULL;

    if (secName != NULL) {
        *secName = NULL;  /* Haven't found anything yet */
    }

    /*
     * Special case if there are NO entries (as opposed to no MATCHING
     * entries).
     */

    if (com2SecUnixList == NULL) {
        DEBUGMSGTL(("netsnmp_unix_getSecName", "no com2sec entries\n"));
        return 0;
    }

    /*
     * If there is no unix socket path, then there can be no valid security
     * name.
     */

    if (opaque == NULL || olength != sizeof(struct sockaddr_un) ||
        to->sun_family != AF_UNIX) {
        DEBUGMSGTL(("netsnmp_unix_getSecName",
                    "no unix destine address in PDU?\n"));
        return 1;
    }

    DEBUGIF("netsnmp_unix_getSecName") {
        ztcommunity = (char *)malloc(community_len + 1);
        if (ztcommunity != NULL) {
            memcpy(ztcommunity, community, community_len);
            ztcommunity[community_len] = '\0';
        }

        DEBUGMSGTL(("netsnmp_unix_getSecName", "resolve <\"%s\">\n",
                    ztcommunity ? ztcommunity : "<malloc error>"));
    }

    for (c = com2SecUnixList; c != NULL; c = c->next) {
        DEBUGMSGTL(("netsnmp_unix_getSecName","compare <\"%s\",to socket %s>",
                    c->community, c->sockpath ));
        if ((community_len == strlen(c->community)) &&
            (memcmp(community, c->community, community_len) == 0) &&
            /* compare sockpath, if pathlen == 0, always match */
            (strlen(to->sun_path) == c->pathlen || c->pathlen == 0) &&
            (memcmp(to->sun_path, c->sockpath, c->pathlen) == 0)
            ) {
            DEBUGMSG(("netsnmp_unix_getSecName", "... SUCCESS\n"));
            if (secName != NULL) {
                *secName = c->secName;
                *contextName = c->contextName;
            }
            break;
        }
        DEBUGMSG(("netsnmp_unix_getSecName", "... nope\n"));
    }
    if (ztcommunity != NULL) {
        free(ztcommunity);
    }
    return 1;
}

void
netsnmp_unix_parse_security(const char *token, char *param)
{
    char              secName[VACMSTRINGLEN + 1], community[VACMSTRINGLEN + 1];
    char              contextName[VACMSTRINGLEN + 1];
    char              sockpath[sizeof(struct sockaddr_un) + 1];
    com2SecUnixEntry *e = NULL;


    param = copy_nword(param, secName, VACMSTRINGLEN);
    if (strcmp(secName, "-Cn") == 0) {
        if (!secName) {
            config_perror("missing CONTEXT_NAME parameter");
            return;
        }
        param = copy_nword( param, contextName, sizeof(contextName));
        param = copy_nword( param, secName, sizeof(secName));
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

        param = copy_nword(param, sockpath, sizeof(struct sockaddr_un) - 1);
    if (sockpath[0] == '\0') {
        config_perror("missing SOCKPATH parameter");
        return;
    } else if (strlen(sockpath) > (sizeof(struct sockaddr_un) - 1)) {
        config_perror("sockpath too long");
        return;
    }
    /* if sockpath == "default", set pathlen=0*/
    if(strcmp(sockpath, "default") == 0){
        sockpath[0] = 0;
    }

    param = copy_nword(param, community, VACMSTRINGLEN);
    if (community[0] == '\0') {
        config_perror("missing COMMUNITY parameter\n");
        return;
    } else if (strncmp
               (community, EXAMPLE_COMMUNITY, strlen(EXAMPLE_COMMUNITY))
               == 0) {
        config_perror("example config COMMUNITY not properly configured");
        return;
    } else if (strlen(community) > (VACMSTRINGLEN - 1)) {
        config_perror("community name too long");
        return;
    }

    e = (com2SecUnixEntry *) malloc(sizeof(com2SecUnixEntry));
    if (e == NULL) {
        config_perror("memory error");
        return;
    }

    DEBUGMSGTL(("netsnmp_unix_parse_security",
                "<\"%s\"> => \"%s\"\n", community, secName));

    strcpy(e->secName, secName);
    strcpy(e->contextName, contextName);
    strcpy(e->community, community);
    strcpy(e->sockpath, sockpath);
    e->pathlen = strlen(sockpath);
    e->next = NULL;

    if (com2SecUnixListLast != NULL) {
        com2SecUnixListLast->next = e;
        com2SecUnixListLast = e;
    } else {
        com2SecUnixListLast = com2SecUnixList = e;
    }
}

void
netsnmp_unix_com2SecList_free(void)
{
    com2SecUnixEntry   *e = com2SecUnixList;
    while (e != NULL) {
        com2SecUnixEntry   *tmp = e;
        e = e->next;
        free(tmp);
    }
    com2SecUnixList = com2SecUnixListLast = NULL;
}
#endif /* support for community based SNMP */

void
netsnmp_unix_agent_config_tokens_register(void)
{
#if !defined(NETSNMP_DISABLE_SNMPV1) || !defined(NETSNMP_DISABLE_SNMPV2C)
    register_app_config_handler("com2secunix", netsnmp_unix_parse_security,
                                netsnmp_unix_com2SecList_free,
                                "[-Cn CONTEXT] secName sockpath community");
#endif /* support for community based SNMP */
}
