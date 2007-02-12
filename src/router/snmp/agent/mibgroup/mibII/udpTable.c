/*
 *  UDP MIB group Table implementation - udpTable.c
 *
 */

#include <net-snmp/net-snmp-config.h>

#if HAVE_STRING_H
#include <string.h>
#else
#include <strings.h>
#endif
#include <sys/types.h>
#if HAVE_WINSOCK_H
#include <winsock.h>
#endif

#if HAVE_SYS_PARAM_H
#include <sys/param.h>
#endif

#if HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif
#if HAVE_SYS_SYSMP_H
#include <sys/sysmp.h>
#endif
#if HAVE_SYS_TCPIPSTATS_H
#include <sys/tcpipstats.h>
#endif
#if defined(IFNET_NEEDS_KERNEL) && !defined(_KERNEL)
#define _KERNEL 1
#define _I_DEFINED_KERNEL
#endif
#if HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif
#if HAVE_NET_IF_H
#include <net/if.h>
#endif
#if HAVE_NET_IF_VAR_H
#include <net/if_var.h>
#endif
#ifdef _I_DEFINED_KERNEL
#undef _KERNEL
#endif

#if HAVE_SYS_STREAM_H
#include <sys/stream.h>
#endif
#if HAVE_NET_ROUTE_H
#include <net/route.h>
#endif
#if HAVE_NETINET_IN_SYSTM_H
#include <netinet/in_systm.h>
#endif
#if HAVE_NETINET_IP_H
#include <netinet/ip.h>
#endif
#if HAVE_SYS_QUEUE_H
#include <sys/queue.h>
#endif
#if HAVE_SYS_SOCKETVAR_H
#include <sys/socketvar.h>
#endif
#if HAVE_NETINET_IP_VAR_H
#include <netinet/ip_var.h>
#endif
#ifdef INET6
#if HAVE_NETINET6_IP6_VAR_H
#include <netinet6/ip6_var.h>
#endif
#endif
#if HAVE_NETINET_IN_PCB_H
#include <netinet/in_pcb.h>
#endif
#if HAVE_NETINET_UDP_H
#include <netinet/udp.h>
#endif
#if HAVE_NETINET_UDP_VAR_H
#include <netinet/udp_var.h>
#endif
#if HAVE_INET_MIB2_H
#include <inet/mib2.h>
#endif

#ifdef solaris2
#include "kernel_sunos5.h"
#else
#include "kernel.h"
#endif

#ifdef cygwin
#define WIN32
#include <windows.h>
#endif

#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>
#include <net-snmp/agent/auto_nlist.h>

#ifdef hpux
#include <sys/mib.h>
#include <netinet/mib_kern.h>
#endif                          /* hpux */

#ifdef linux
#include "tcpTable.h"
#endif
#include "udp.h"
#include "udpTable.h"
#include "sysORTable.h"

#ifdef CAN_USE_SYSCTL
#include <sys/sysctl.h>
#endif

#if HAVE_DMALLOC_H
#include <dmalloc.h>
#endif

        /*********************
	 *
	 *  Kernel & interface information,
	 *   and internal forward declarations
	 *
	 *********************/

#ifndef solaris2
static void     UDP_Scan_Init(void);
#ifdef hpux11
static int      UDP_Scan_Next(mib_udpLsnEnt *);
#else
static int      UDP_Scan_Next(struct inpcb *);
#endif
#endif

        /*********************
	 *
	 *  Initialisation & common implementation functions
	 *
	 *********************/


        /*********************
	 *
	 *  System specific implementation functions
	 *
	 *********************/

#ifndef WIN32
#ifndef solaris2

u_char         *
var_udpEntry(struct variable *vp,
             oid * name,
             size_t * length,
             int exact, size_t * var_len, WriteMethod ** write_method)
{
    int             i;
    oid             newname[MAX_OID_LEN], lowest[MAX_OID_LEN], *op;
    u_char         *cp;
    int             LowState;
#ifdef hpux11
    static mib_udpLsnEnt udp, Lowudp;
#else
    static struct inpcb inpcb, Lowinpcb;
#endif

    memcpy((char *) newname, (char *) vp->name,
           (int) vp->namelen * sizeof(oid));
    /*
     * find the "next" pseudo-connection 
     */
#ifndef hpux11
  Again:
#endif
    LowState = -1;              /* UDP doesn't have 'State', but it's a useful flag */
    UDP_Scan_Init();
    for (;;) {
#ifdef hpux11
        if ((i = UDP_Scan_Next(&udp)) == 0)
            break;              /* Done */
        cp = (u_char *) & udp.LocalAddress;
#else                           /* hpux11 */
        if ((i = UDP_Scan_Next(&inpcb)) < 0)
            goto Again;
        if (i == 0)
            break;              /* Done */
        cp = (u_char *) & inpcb.inp_laddr.s_addr;
#endif                          /* hpux11 */
        op = newname + 10;
        *op++ = *cp++;
        *op++ = *cp++;
        *op++ = *cp++;
        *op++ = *cp++;

#ifdef hpux11
        newname[14] = (unsigned short) udp.LocalPort;
#else
        newname[14] = ntohs(inpcb.inp_lport);
#endif

        if (exact) {
            if (snmp_oid_compare(newname, 15, name, *length) == 0) {
                memcpy((char *) lowest, (char *) newname,
                       15 * sizeof(oid));
                LowState = 0;
#ifdef hpux11
                Lowudp = udp;
#else
                Lowinpcb = inpcb;
#endif
                break;          /* no need to search further */
            }
        } else {
            if ((snmp_oid_compare(newname, 15, name, *length) > 0) &&
                ((LowState < 0)
                 || (snmp_oid_compare(newname, 15, lowest, 15) < 0))) {
                /*
                 * if new one is greater than input and closer to input than
                 * previous lowest, save this one as the "next" one.
                 */
                memcpy((char *) lowest, (char *) newname,
                       15 * sizeof(oid));
                LowState = 0;
#ifdef hpux11
                Lowudp = udp;
#else
                Lowinpcb = inpcb;
#endif
            }
        }
    }
    if (LowState < 0)
        return (NULL);
    memcpy((char *) name, (char *) lowest,
           ((int) vp->namelen + 10) * sizeof(oid));
    *length = vp->namelen + 5;
    *write_method = 0;
    *var_len = sizeof(long);
    switch (vp->magic) {
    case UDPLOCALADDRESS:
#ifdef hpux11
        return (u_char *) & Lowudp.LocalAddress;
#else
        return (u_char *) & Lowinpcb.inp_laddr.s_addr;
#endif
    case UDPLOCALPORT:
#ifdef hpux11
        long_return = (unsigned short) Lowudp.LocalPort;
#else
        long_return = ntohs(Lowinpcb.inp_lport);
#endif
        return (u_char *) & long_return;
    default:
        DEBUGMSGTL(("snmpd", "unknown sub-id %d in var_udpEntry\n",
                    vp->magic));
    }
    return NULL;
}

#else                           /* solaris2 - udp */

static int
UDP_Cmp(void *addr, void *ep)
{
    if (memcmp((mib2_udpEntry_t *) ep, (mib2_udpEntry_t *) addr,
               sizeof(mib2_udpEntry_t)) == 0)
        return (0);
    else
        return (1);
}

u_char         *
var_udpEntry(struct variable * vp,
             oid * name,
             size_t * length,
             int exact, size_t * var_len, WriteMethod ** write_method)
{
    oid             newname[MAX_OID_LEN], lowest[MAX_OID_LEN], *op;
    u_char         *cp;

#define UDP_LISTEN_LENGTH 15
#define UDP_LOCADDR_OFF   10
#define UDP_LOCPORT_OFF   14
    mib2_udpEntry_t Lowentry, Nextentry, entry;
    req_e           req_type;
    int             Found = 0;

    memset(&Lowentry, 0, sizeof(Lowentry));
    memcpy((char *) newname, (char *) vp->name, vp->namelen * sizeof(oid));
    if (*length == UDP_LISTEN_LENGTH)   /* Assume that the input name is the lowest */
        memcpy((char *) lowest, (char *) name,
               UDP_LISTEN_LENGTH * sizeof(oid));
    for (Nextentry.udpLocalAddress = (u_long) - 1, req_type = GET_FIRST;;
         req_type = GET_NEXT) {
        if (getMibstat
            (MIB_UDP_LISTEN, &entry, sizeof(mib2_udpEntry_t), req_type,
             &UDP_Cmp, &entry) != 0)
            break;
        if (entry.udpEntryInfo.ue_state != MIB2_UDP_idle)
            continue;           /* we only want to get listen ports */
        COPY_IPADDR(cp, (u_char *) & entry.udpLocalAddress, op,
                    newname + UDP_LOCADDR_OFF);
        newname[UDP_LOCPORT_OFF] = entry.udpLocalPort;

        if (exact) {
            if (snmp_oid_compare(newname, UDP_LISTEN_LENGTH, name, *length)
                == 0) {
                memcpy((char *) lowest, (char *) newname,
                       UDP_LISTEN_LENGTH * sizeof(oid));
                Lowentry = entry;
                Found++;
                break;          /* no need to search further */
            }
        } else {
            if ((snmp_oid_compare
                 (newname, UDP_LISTEN_LENGTH, name, *length) > 0)
                && ((Nextentry.udpLocalAddress == (u_long) - 1)
                    ||
                    (snmp_oid_compare
                     (newname, UDP_LISTEN_LENGTH, lowest,
                      UDP_LISTEN_LENGTH) < 0)
                    ||
                    (snmp_oid_compare
                     (name, *length, lowest, UDP_LISTEN_LENGTH) == 0))) {
                /*
                 * if new one is greater than input and closer to input than
                 * * previous lowest, and is not equal to it, save this one as
                 * * the "next" one.
                 */
                memcpy((char *) lowest, (char *) newname,
                       UDP_LISTEN_LENGTH * sizeof(oid));
                Lowentry = entry;
                Found++;
            }
        }
        Nextentry = entry;
    }
    if (Found == 0)
        return (NULL);
    memcpy((char *) name, (char *) lowest,
           (vp->namelen + UDP_LISTEN_LENGTH -
            UDP_LOCADDR_OFF) * sizeof(oid));
    *length = vp->namelen + UDP_LISTEN_LENGTH - UDP_LOCADDR_OFF;
    *write_method = 0;
    *var_len = sizeof(long);
    switch (vp->magic) {
    case UDPLOCALADDRESS:
        long_return = Lowentry.udpLocalAddress;
        return (u_char *) & long_return;
    case UDPLOCALPORT:
        long_return = Lowentry.udpLocalPort;
        return (u_char *) & long_return;
    default:
        DEBUGMSGTL(("snmpd", "unknown sub-id %d in var_udpEntry\n",
                    vp->magic));
    }
    return NULL;
}
#endif                          /* solaris2 - udp */


        /*********************
	 *
	 *  Internal implementation functions
	 *
	 *********************/

#ifdef linux
static struct inpcb *udp_inpcb_list;
#endif

#ifndef solaris2

#ifdef hpux11
static int      udptab_size, udptab_current;
static mib_udpLsnEnt *udp = (mib_udpLsnEnt *) 0;
#else                           /* hpux11 */
static struct inpcb udp_inpcb, *udp_prev;
#ifdef PCB_TABLE
static struct inpcb *udp_head, *udp_next;
#endif
#if defined(CAN_USE_SYSCTL) && defined(UDPCTL_PCBLIST)
static char    *udpcb_buf = NULL;
static struct xinpgen *xig = NULL;
#endif                          /* !defined(CAN_USE_SYSCTL) || !define(UDPCTL_PCBLIST) */
#endif                          /* hpux11 */


static void
UDP_Scan_Init(void)
{
#ifdef hpux11

    int             fd;
    struct nmparms  p;
    int             val;
    unsigned int    ulen;
    int             ret;

    if (udp)
        free(udp);
    udp = (mib_udpLsnEnt *) 0;
    udptab_size = 0;

    if ((fd = open_mib("/dev/ip", O_RDONLY, 0, NM_ASYNC_OFF)) >= 0) {
        p.objid = ID_udpLsnNumEnt;
        p.buffer = (void *) &val;
        ulen = sizeof(int);
        p.len = &ulen;
        if ((ret = get_mib_info(fd, &p)) == 0)
            udptab_size = val;

        if (udptab_size > 0) {
            ulen = (unsigned) udptab_size *sizeof(mib_udpLsnEnt);
            udp = (mib_udpLsnEnt *) malloc(ulen);
            p.objid = ID_udpLsnTable;
            p.buffer = (void *) udp;
            p.len = &ulen;
            if ((ret = get_mib_info(fd, &p)) < 0)
                udptab_size = 0;
        }

        close_mib(fd);
    }

    udptab_current = 0;

#else                           /* hpux11 */

#if !defined(CAN_USE_SYSCTL) || !defined(UDPCTL_PCBLIST)
#ifdef PCB_TABLE
    struct inpcbtable table;
#endif
#ifndef linux
#ifdef PCB_TABLE
    auto_nlist(UDB_SYMBOL, (char *) &table, sizeof(table));
    udp_next = table.inpt_queue.cqh_first;
    udp_head = udp_prev =
        (struct inpcb *) &((struct inpcbtable *)
                           auto_nlist_value(UDB_SYMBOL))->inpt_queue.
        cqh_first;
#else
    auto_nlist(UDB_SYMBOL, (char *) &udp_inpcb, sizeof(udp_inpcb));
#if !(defined(freebsd2) || defined(netbsd1) || defined(openbsd2))
    udp_prev = (struct inpcb *) auto_nlist_value(UDB_SYMBOL);
#endif
#endif
#else                           /* linux */
    FILE           *in;
    char            line[256];
    struct inpcb  **pp;
    struct timeval  now;
    static unsigned long Time_Of_Last_Reload = 0;

    /*
     * save some cpu-cycles, and reload after 5 secs...
     */
    gettimeofday(&now, (struct timezone *) 0);
    if (Time_Of_Last_Reload + 5 > now.tv_sec) {
        udp_prev = udp_inpcb_list;
        return;
    }
    Time_Of_Last_Reload = now.tv_sec;


    if (!(in = fopen("/proc/net/udp", "r"))) {
        snmp_log(LOG_ERR, "snmpd: cannot open /proc/net/udp ...\n");
        udp_prev = 0;
        return;
    }

    /*
     * free old chain: 
     */
    while (udp_inpcb_list) {
        struct inpcb   *p = udp_inpcb_list;
        udp_inpcb_list = udp_inpcb_list->inp_next;
        free(p);
    }

    /*
     * scan proc-file and append: 
     */

    pp = &udp_inpcb_list;

    while (line == fgets(line, sizeof(line), in)) {
        struct inpcb    pcb, *nnew;
        unsigned int    state, lport;

        if (3 != sscanf(line, "%*d: %x:%x %*x:%*x %x",
                        &pcb.inp_laddr.s_addr, &lport, &state))
            continue;

        if (state != 7)         /* fix me:  UDP_LISTEN ??? */
            continue;

        pcb.inp_lport = htons((unsigned short) (lport));
        pcb.inp_fport = htons(pcb.inp_fport);

        nnew = (struct inpcb *) malloc(sizeof(struct inpcb));
        if (nnew == NULL)
            break;
        *nnew = pcb;
        nnew->inp_next = 0;

        *pp = nnew;
        pp = &nnew->inp_next;
    }

    fclose(in);

    /*
     * first entry to go: 
     */
    udp_prev = udp_inpcb_list;
#endif                          /*linux */
#else                           /*  !defined(CAN_USE_SYSCTL) || !defined(UDPCTL_PCBLIST) */
    {
        size_t          len;
        int             sname[] =
            { CTL_NET, PF_INET, IPPROTO_UDP, UDPCTL_PCBLIST };

        if (udpcb_buf) {
            free(udpcb_buf);
            udpcb_buf = NULL;
        }
        xig = NULL;

        len = 0;
        if (sysctl(sname, 4, 0, &len, 0, 0) < 0) {
            return;
        }
        if ((udpcb_buf = malloc(len)) == NULL) {
            return;
        }
        if (sysctl(sname, 4, udpcb_buf, &len, 0, 0) < 0) {
            free(udpcb_buf);
            udpcb_buf = NULL;
            return;
        }

        xig = (struct xinpgen *) udpcb_buf;
        xig = (struct xinpgen *) ((char *) xig + xig->xig_len);
        return;
    }
#endif                          /*  !defined(CAN_USE_SYSCTL) || !defined(UDPCTL_PCBLIST) */

#endif                          /* hpux11 */
}

#ifdef hpux11
static int
UDP_Scan_Next(mib_udpLsnEnt * RetUdp)
{
    if (udptab_current < udptab_size) {
        /*
         * copy values 
         */
        *RetUdp = udp[udptab_current];
        /*
         * increment to point to next entry 
         */
        udptab_current++;
        /*
         * return success 
         */
        return (1);
    }

    /*
     * return done 
     */
    return (0);
}
#else                           /* hpux11 */
static int
UDP_Scan_Next(struct inpcb *RetInPcb)
{
#if !defined(CAN_USE_SYSCTL) || !defined(UDPCTL_PCBLIST)
    register struct inpcb *next;

#ifndef linux
#ifdef PCB_TABLE
    if (udp_next == udp_head)
        return 0;
#else
    if ((udp_inpcb.INP_NEXT_SYMBOL == NULL) ||
        (udp_inpcb.INP_NEXT_SYMBOL ==
         (struct inpcb *) auto_nlist_value(UDB_SYMBOL))) {
        return (0);             /* "EOF" */
    }
#endif

#ifdef PCB_TABLE
    klookup((unsigned long) udp_next, (char *) &udp_inpcb,
            sizeof(udp_inpcb));
    udp_next = udp_inpcb.inp_queue.cqe_next;
#else
    next = udp_inpcb.INP_NEXT_SYMBOL;

    klookup((unsigned long) next, (char *) &udp_inpcb, sizeof(udp_inpcb));
#if !(defined(netbsd1) || defined(freebsd2) || defined(linux) || defined(openbsd2))
    if (udp_inpcb.INP_PREV_SYMBOL != udp_prev)  /* ??? */
        return (-1);            /* "FAILURE" */
#endif
#endif
    *RetInPcb = udp_inpcb;
#if !(defined(netbsd1) || defined(freebsd2) || defined(openbsd2))
    udp_prev = next;
#endif
#else                           /* linux */
    if (!udp_prev)
        return 0;

    udp_inpcb = *udp_prev;
    next = udp_inpcb.inp_next;
    *RetInPcb = udp_inpcb;
    udp_prev = next;
#endif                          /* linux */
#else                           /*  !defined(CAN_USE_SYSCTL) || !defined(UDPCTL_PCBLIST) */
    /*
     * Are we done? 
     */
    if ((xig == NULL) || (xig->xig_len <= sizeof(struct xinpgen)))
        return (0);

    *RetInPcb = ((struct xinpcb *) xig)->xi_inp;

    /*
     * Prepare for Next read 
     */
    xig = (struct xinpgen *) ((char *) xig + xig->xig_len);
#endif                          /*  !defined(CAN_USE_SYSCTL) || !defined(UDPCTL_PCBLIST) */
    return (1);                 /* "OK" */
}
#endif                          /* hpux11 */
#endif                          /* solaris2 */

#else                           /* WIN32 */
#include <iphlpapi.h>

u_char         *
var_udpEntry(struct variable *vp,
             oid * name,
             size_t * length,
             int exact, size_t * var_len, WriteMethod ** write_method)
{
    oid             newname[MAX_OID_LEN], lowest[MAX_OID_LEN], *op;
    u_char         *cp;
    int             LowState = -1;
    static PMIB_UDPTABLE pUdpTable = NULL;
    DWORD           status = NO_ERROR;
    DWORD           dwActualSize = 0;
    UINT            i;
    struct timeval  now;
    static long     Time_Of_Last_Reload = 0;
    struct in_addr  inadLocal;
    memcpy((char *) newname, (char *) vp->name,
           (int) vp->namelen * sizeof(oid));

    /*
     * save some cpu-cycles, and reload after 5 secs...
     */
    gettimeofday(&now, (struct timezone *) 0);
    if ((Time_Of_Last_Reload + 5 <= now.tv_sec) || (pUdpTable == NULL)) {
        if (pUdpTable != NULL)
            free(pUdpTable);
        Time_Of_Last_Reload = now.tv_sec;
        /*
         * query for the buffer size needed 
         */
        status = GetUdpTable(pUdpTable, &dwActualSize, TRUE);
        if (status == ERROR_INSUFFICIENT_BUFFER) {
            pUdpTable = (PMIB_UDPTABLE) malloc(dwActualSize);
            if (pUdpTable != NULL) {
                /*
                 * Get the sorted UDP table 
                 */
                status = GetUdpTable(pUdpTable, &dwActualSize, TRUE);

            }
        }
    }
    if (status == NO_ERROR) {
        for (i = 0; i < pUdpTable->dwNumEntries; ++i) {
            inadLocal.s_addr = pUdpTable->table[i].dwLocalAddr;
            cp = (u_char *) & pUdpTable->table[i].dwLocalAddr;

            op = newname + 10;
            *op++ = *cp++;
            *op++ = *cp++;
            *op++ = *cp++;
            *op++ = *cp++;

            newname[14] =
                ntohs((unsigned short) (0x0000FFFF & pUdpTable->table[i].
                                        dwLocalPort));

            if (exact) {
                if (snmp_oid_compare(newname, 15, name, *length) == 0) {
                    memcpy((char *) lowest, (char *) newname,
                           15 * sizeof(oid));
                    LowState = 0;
                    break;      /* no need to search further */
                }
            } else {
                if (snmp_oid_compare(newname, 15, name, *length) > 0) {
                    memcpy((char *) lowest, (char *) newname,
                           15 * sizeof(oid));
                    LowState = 0;
                    inadLocal.s_addr = pUdpTable->table[i].dwLocalAddr;
                    break;      /* As the table is sorted, no need to search further */
                }
            }
        }
    }

    if (LowState < 0) {
        free(pUdpTable);
        pUdpTable = NULL;
        return (NULL);
    }
    memcpy((char *) name, (char *) lowest,
           ((int) vp->namelen + 10) * sizeof(oid));
    *length = vp->namelen + 5;
    *write_method = 0;
    *var_len = sizeof(long);
    switch (vp->magic) {
    case UDPLOCALADDRESS:
        return (u_char *) & pUdpTable->table[i].dwLocalAddr;
    case UDPLOCALPORT:
        long_return =
            ntohs((unsigned short) (0x0000FFFF & pUdpTable->table[i].
                                    dwLocalPort));
        return (u_char *) & long_return;
    default:
        DEBUGMSGTL(("snmpd", "unknown sub-id %d in var_udpEntry\n",
                    vp->magic));
    }
    return NULL;
}

#endif                          /* WIN32 */
