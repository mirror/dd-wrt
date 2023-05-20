#include <net-snmp/net-snmp-config.h>

#include <net-snmp/library/snmpSTDDomain.h>

#include <stdio.h>
#include <sys/types.h>
#include <signal.h>
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

#include <net-snmp/types.h>
#include <net-snmp/output_api.h>

#include <net-snmp/library/snmp_transport.h>
#include <net-snmp/library/tools.h>

oid netsnmp_snmpSTDDomain[] = { TRANSPORT_DOMAIN_STD_IP };
static netsnmp_tdomain stdDomain;

/*
 * Return a string representing the address in data, or else the "far end"
 * address if data is NULL.  
 */

static char *
netsnmp_std_fmtaddr(netsnmp_transport *t, const void *data, int len)
{
    DEBUGMSGTL(("domain:std","formatting addr.  data=%p\n",t->data));
    if (t->data) {
        netsnmp_std_data *data = (netsnmp_std_data*)t->data;
	char *buf;

        if (asprintf(&buf, "STD:%s", data->prog) < 0)
            buf = NULL;
        DEBUGMSGTL(("domain:std","  formatted:=%s\n",buf));
        return buf;
    }
    return strdup("STDInOut");
}

static void
netsnmp_std_get_taddr(netsnmp_transport *t, void **addr, size_t *addr_len)
{
    *addr_len = t->remote_length;
    *addr = netsnmp_memdup(t->remote, *addr_len);
}

/*
 * You can write something into opaque that will subsequently get passed back 
 * to your send function if you like.  For instance, you might want to
 * remember where a PDU came from, so that you can send a reply there...  
 */

static int
netsnmp_std_recv(netsnmp_transport *t, void *buf, int size,
		 void **opaque, int *olength)
{
    int rc = -1;

    DEBUGMSGTL(("domain:std","recv on sock %d.  data=%p\n",t->sock, t->data));
    while (rc < 0) {
        rc = read(t->sock, buf, size);
        DEBUGMSGTL(("domain:std","  bytes: %d.\n", rc));
        if (rc < 0 && errno != EINTR) {
            DEBUGMSGTL(("netsnmp_std", " read on fd %d failed: %d (\"%s\")\n",
                        t->sock, errno, strerror(errno)));
            break;
        }
        if (rc == 0) {
            /* 0 input is probably bad since we selected on it */
            return -1;
        }
        DEBUGMSGTL(("netsnmp_std", "read on stdin got %d bytes\n", rc));
    }

    return rc;
}



static int
netsnmp_std_send(netsnmp_transport *t, const void *buf, int size,
		 void **opaque, int *olength)
{
    int rc = -1;

    DEBUGMSGTL(("domain:std","send on sock.  data=%p\n", t->data));
    while (rc < 0) {
        if (t->data) {
            netsnmp_std_data *data = (netsnmp_std_data*)t->data;
            rc = write(data->outfd, buf, size);
        } else {
            /* straight to stdout */
            rc = write(1, buf, size);
        }
        if (rc < 0 && errno != EINTR) {
            break;
        }
    }
    return rc;
}

static int
netsnmp_std_close(netsnmp_transport *t)
{
    DEBUGMSGTL(("domain:std","close.  data=%p\n", t->data));
    if (t->data) {
        netsnmp_std_data *data = (netsnmp_std_data*)t->data;
        close(data->outfd);
        close(t->sock);

        /* kill the child too */
        DEBUGMSGTL(("domain:std"," killing %d\n", data->childpid));
        kill(data->childpid, SIGTERM);
        sleep(1);
        kill(data->childpid, SIGKILL);
        /* XXX: set an alarm to kill harder the child */
    } else {
        /* close stdout/in */
        close(STDOUT_FILENO);
        close(STDIN_FILENO);
    }
    return 0;
}



static int
netsnmp_std_accept(netsnmp_transport *t)
{
    DEBUGMSGTL(("domain:std"," accept data=%p\n", t->data));
    /* nothing to do here */
    return 0;
}

/*
 * Open a STDIN/STDOUT -based transport for SNMP.
 */

netsnmp_transport *
netsnmp_std_transport(const char *instring, size_t instring_len,
		      const char *default_target)
{
    netsnmp_transport *t;

    t = SNMP_MALLOC_TYPEDEF(netsnmp_transport);
    if (t == NULL) {
        return NULL;
    }

    t->domain = netsnmp_snmpSTDDomain;
    t->domain_length =
        sizeof(netsnmp_snmpSTDDomain) / sizeof(netsnmp_snmpSTDDomain[0]);

    t->sock = -1;
    t->flags = NETSNMP_TRANSPORT_FLAG_STREAM | NETSNMP_TRANSPORT_FLAG_TUNNELED;

    /*
     * Message size is not limited by this transport (hence msgMaxSize
     * is equal to the maximum legal size of an SNMP message).  
     */

    t->msgMaxSize = SNMP_MAX_PACKET_LEN;
    t->f_recv     = netsnmp_std_recv;
    t->f_send     = netsnmp_std_send;
    t->f_close    = netsnmp_std_close;
    t->f_accept   = netsnmp_std_accept;
    t->f_fmtaddr  = netsnmp_std_fmtaddr;
    t->f_get_taddr = netsnmp_std_get_taddr;

    /*
     * if instring is not null length, it specifies a path to a prog
     * XXX: plus args
     */
    if (instring_len == 0 && default_target != NULL) {
	instring = default_target;
	instring_len = strlen(default_target);
    }

    if (instring_len != 0) {
        int infd[2], outfd[2];  /* sockets to and from the client */
        int childpid;

        if (pipe(infd) || pipe(outfd)) {
            snmp_log(LOG_ERR,
                     "Failed to create needed pipes for a STD transport");
            netsnmp_transport_free(t);
            return NULL;
        }

        childpid = fork();
        /* parentpid => childpid */
        /* infd[1]   => infd[0] */
        /* outfd[0]  <= outfd[1] */

        if (childpid) {
            netsnmp_std_data *data;
            
            /* we're in the parent */
            close(infd[0]);
            close(outfd[1]);

            data = SNMP_MALLOC_TYPEDEF(netsnmp_std_data);
            if (!data) {
                snmp_log(LOG_ERR, "snmpSTDDomain: malloc failed");
                netsnmp_transport_free(t);
                return NULL;
            }
            t->data = data;
            t->data_length = sizeof(netsnmp_std_data);
            t->sock = outfd[0];
            data->prog = strdup(instring);
            data->outfd = infd[1];
            data->childpid = childpid;
            DEBUGMSGTL(("domain:std","parent.  data=%p\n", t->data));
        } else {
            /* we're in the child */

            dup2(infd[0], STDIN_FILENO);
            dup2(outfd[1], STDOUT_FILENO);

            /* close all the pipes themselves */
            close(infd[0]);
            close(infd[1]);
            close(outfd[0]);
            close(outfd[1]);

            NETSNMP_IGNORE_RESULT(system(instring));
            /* XXX: TODO: use exec form instead; needs args */
            /* execv(instring, NULL); */
        }
    }            

    return t;
}

netsnmp_transport *
netsnmp_std_create_tstring(const char *instring, int local,
			   const char *default_target)
{
    return netsnmp_std_transport(instring, strlen(instring), default_target);
}

netsnmp_transport *
netsnmp_std_create_ostring(const void *o, size_t o_len, int local)
{
    return netsnmp_std_transport(o, o_len, NULL);
}

void
netsnmp_std_ctor(void)
{
    stdDomain.name = netsnmp_snmpSTDDomain;
    stdDomain.name_length = sizeof(netsnmp_snmpSTDDomain) / sizeof(oid);
    stdDomain.prefix = (const char **)calloc(2, sizeof(char *));
    stdDomain.prefix[0] = "std";

    stdDomain.f_create_from_tstring_new = netsnmp_std_create_tstring;
    stdDomain.f_create_from_ostring     = netsnmp_std_create_ostring;

    netsnmp_tdomain_register(&stdDomain);
}
