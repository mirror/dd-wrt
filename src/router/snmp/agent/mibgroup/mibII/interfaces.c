/*
 *  Interfaces MIB group implementation - interfaces.c
 *
 */

#include <net-snmp/net-snmp-config.h>

#if defined(IFNET_NEEDS_KERNEL) && !defined(_KERNEL) && !defined(IFNET_NEEDS_KERNEL_LATE)
#define _KERNEL 1
#define _I_DEFINED_KERNEL
#endif

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
#if HAVE_SYS_PARAM_H
#include <sys/param.h>
#endif
#include <sys/types.h>
#if HAVE_WINSOCK_H
#include <winsock.h>
#endif
#if defined(IFNET_NEEDS_KERNEL) && !defined(_KERNEL) && defined(IFNET_NEEDS_KERNEL_LATE)
#define _KERNEL 1
#define _I_DEFINED_KERNEL
#endif
#if HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif
#ifndef STREAM_NEEDS_KERNEL_ISLANDS
#if HAVE_SYS_STREAM_H
#include <sys/stream.h>
#endif
#endif
#if HAVE_SYS_SOCKETVAR_H
#include <sys/socketvar.h>
#endif

#if TIME_WITH_SYS_TIME
# ifdef WIN32
#  include <sys/timeb.h>
# else
# include <sys/time.h>
# endif
# include <time.h>
#else
# if HAVE_SYS_TIME_H
#  include <sys/time.h>
# else
#  include <time.h>
# endif
#endif
#if HAVE_SYS_SOCKIO_H
#include <sys/sockio.h>
#endif
#if HAVE_FCNTL_H
#include <fcntl.h>
#endif
#if HAVE_SYS_IOCTL_H
#include <sys/ioctl.h>
#endif
#if HAVE_NETINET_IN_H
#include <netinet/in.h>
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
#ifdef STREAM_NEEDS_KERNEL_ISLANDS
#if HAVE_SYS_STREAM_H
#include <sys/stream.h>
#endif
#endif
#if HAVE_NET_ROUTE_H
#include <net/route.h>
#endif
#if HAVE_NETINET_IN_SYSTM_H
#include <netinet/in_systm.h>
#endif
#if HAVE_SYS_HASHING_H
#include <sys/hashing.h>
#endif
#if HAVE_NETINET_IN_VAR_H
#include <netinet/in_var.h>
#endif
#if HAVE_NETINET_IP_H
#include <netinet/ip.h>
#endif
#ifdef INET6
#if HAVE_NETINET_IP6_H
#include <netinet/ip6.h>
#endif
#endif
#if HAVE_SYS_QUEUE_H
#include <sys/queue.h>
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
#if HAVE_NETINET_IF_ETHER_H
#include <netinet/if_ether.h>
#endif
#if HAVE_NET_IF_TYPES_H
#include <net/if_types.h>
#endif
#if HAVE_NET_IF_DL_H
#ifndef dynix
#include <net/if_dl.h>
#else
#include <sys/net/if_dl.h>
#endif
#endif
#if HAVE_INET_MIB2_H
#include <inet/mib2.h>
#endif
#if HAVE_IOCTLS_H
#include <ioctls.h>
#endif

#if HAVE_DMALLOC_H
#include <dmalloc.h>
#endif

#ifdef solaris2
# include <errno.h>
#include "kernel_sunos5.h"
#else
#include "kernel.h"
#endif

#ifdef hpux
#include <sys/mib.h>
#include <netinet/mib_kern.h>
#endif                          /* hpux */

#ifdef cygwin
#define WIN32
#include <windows.h>
#endif

#if HAVE_SYS_SYSCTL_H
#include <sys/sysctl.h>

#if defined(freebsd3) || defined(freebsd4) || defined(freebsd5)
#    define USE_SYSCTL_IFLIST
#else
# if defined(CTL_NET) && !defined(freebsd2) && !defined(netbsd1)
#  ifdef PF_ROUTE
#   ifdef NET_RT_IFLIST
#    ifndef netbsd1
#     define USE_SYSCTL_IFLIST
#    endif
#   endif
#  endif
# endif
#endif                          /* defined(freebsd3) */
#endif                          /* HAVE_SYS_SYSCTL_H */

#if HAVE_OSRELDATE_H
#include <osreldate.h>
#endif
#if HAVE_SYS_SYSCTL_H
#ifdef CAN_USE_SYSCTL
#include <sys/sysctl.h>
#endif
#endif

#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>
#include <net-snmp/agent/auto_nlist.h>

#include "interfaces.h"
#include "struct.h"
#include "util_funcs.h"
#include "sysORTable.h"

extern struct timeval starttime;

static void     parse_interface_config(const char *, char *);
static void     free_interface_config(void);

struct variable3 interfaces_variables[] = {
    {IFNUMBER, ASN_INTEGER, RONLY, var_interfaces, 1, {1}},
    {IFINDEX, ASN_INTEGER, RONLY, var_ifEntry, 3, {2, 1, 1}},
    {IFDESCR, ASN_OCTET_STR, RONLY, var_ifEntry, 3, {2, 1, 2}},
    {IFTYPE, ASN_INTEGER, RONLY, var_ifEntry, 3, {2, 1, 3}},
    {IFMTU, ASN_INTEGER, RONLY, var_ifEntry, 3, {2, 1, 4}},
    {IFSPEED, ASN_GAUGE, RONLY, var_ifEntry, 3, {2, 1, 5}},
    {IFPHYSADDRESS, ASN_OCTET_STR, RONLY, var_ifEntry, 3, {2, 1, 6}},
#ifdef WIN32
    {IFADMINSTATUS, ASN_INTEGER, RWRITE, var_ifEntry, 3, {2, 1, 7}},
#else
    {IFADMINSTATUS, ASN_INTEGER, RONLY, var_ifEntry, 3, {2, 1, 7}},
#endif
    {IFOPERSTATUS, ASN_INTEGER, RONLY, var_ifEntry, 3, {2, 1, 8}},
    {IFLASTCHANGE, ASN_TIMETICKS, RONLY, var_ifEntry, 3, {2, 1, 9}},
    {IFINOCTETS, ASN_COUNTER, RONLY, var_ifEntry, 3, {2, 1, 10}},
    {IFINUCASTPKTS, ASN_COUNTER, RONLY, var_ifEntry, 3, {2, 1, 11}},
    {IFINNUCASTPKTS, ASN_COUNTER, RONLY, var_ifEntry, 3, {2, 1, 12}},
    {IFINDISCARDS, ASN_COUNTER, RONLY, var_ifEntry, 3, {2, 1, 13}},
    {IFINERRORS, ASN_COUNTER, RONLY, var_ifEntry, 3, {2, 1, 14}},
    {IFINUNKNOWNPROTOS, ASN_COUNTER, RONLY, var_ifEntry, 3, {2, 1, 15}},
    {IFOUTOCTETS, ASN_COUNTER, RONLY, var_ifEntry, 3, {2, 1, 16}},
    {IFOUTUCASTPKTS, ASN_COUNTER, RONLY, var_ifEntry, 3, {2, 1, 17}},
    {IFOUTNUCASTPKTS, ASN_COUNTER, RONLY, var_ifEntry, 3, {2, 1, 18}},
    {IFOUTDISCARDS, ASN_COUNTER, RONLY, var_ifEntry, 3, {2, 1, 19}},
    {IFOUTERRORS, ASN_COUNTER, RONLY, var_ifEntry, 3, {2, 1, 20}},
    {IFOUTQLEN, ASN_GAUGE, RONLY, var_ifEntry, 3, {2, 1, 21}},
    {IFSPECIFIC, ASN_OBJECT_ID, RONLY, var_ifEntry, 3, {2, 1, 22}}
};

/*
 * Define the OID pointer to the top of the mib tree that we're
 * registering underneath, and the OID of the MIB module 
 */
oid             interfaces_variables_oid[] = { SNMP_OID_MIB2, 2 };
oid             interfaces_module_oid[] = { SNMP_OID_MIB2, 31 };

void
init_interfaces(void)
{
    /*
     * register ourselves with the agent to handle our mib tree 
     */
    REGISTER_MIB("mibII/interfaces", interfaces_variables, variable3,
                 interfaces_variables_oid);
    REGISTER_SYSOR_ENTRY(interfaces_module_oid,
                         "The MIB module to describe generic objects for network interface sub-layers");

    snmpd_register_config_handler("interface", parse_interface_config,
                                  free_interface_config,
                                  "name type speed");

#ifndef USE_SYSCTL_IFLIST
#if HAVE_NET_IF_MIB_H
    init_interfaces_setup();
#endif
#endif
#ifdef solaris2
    init_kernel_sunos5();
#endif
}

#ifdef linux
/*
 * if_type_from_name
 * Return interface type using the interface name as a clue.
 * Returns 1 to imply "other" type if name not recognized. 
 */
static int
if_type_from_name(const char *pcch)
{
    typedef struct _match_if {
        int             mi_type;
        const char     *mi_name;
    }              *pmatch_if, match_if;

    static match_if lmatch_if[] = {
        {24, "lo"},
        {6, "eth"},
        {9, "tr"},
        {23, "ppp"},
        {28, "sl"},
        {0, 0}                  /* end of list */
    };

    int             ii, len;
    register pmatch_if pm;

    for (ii = 0, pm = lmatch_if; pm->mi_name; pm++) {
        len = strlen(pm->mi_name);
        if (0 == strncmp(pcch, pm->mi_name, len)) {
            return (pm->mi_type);
        }
    }
    return (1);                 /* in case search fails */
}
#endif


typedef struct _conf_if_list {
    char           *name;
    int             type;
    u_long          speed;
    struct _conf_if_list *next;
} conf_if_list;

static conf_if_list *conf_list;
#ifdef linux
static struct ifnet *ifnetaddr_list;
#endif

static void
parse_interface_config(const char *token, char *cptr)
{
    conf_if_list   *if_ptr, *if_new;
    char           *name, *type, *speed, *ecp;

    name = strtok(cptr, " \t");
    if (!name) {
        config_perror("Missing NAME parameter");
        return;
    }
    type = strtok(NULL, " \t");
    if (!type) {
        config_perror("Missing TYPE parameter");
        return;
    }
    speed = strtok(NULL, " \t");
    if (!speed) {
        config_perror("Missing SPEED parameter");
        return;
    }
    if_ptr = conf_list;
    while (if_ptr)
        if (strcmp(if_ptr->name, name))
            if_ptr = if_ptr->next;
        else
            break;
    if (if_ptr)
        config_pwarn("Duplicate interface specification");
    if_new = (conf_if_list *) malloc(sizeof(conf_if_list));
    if (!if_new) {
        config_perror("Out of memory");
        return;
    }
    if_new->speed = strtoul(speed, &ecp, 0);
    if (*ecp) {
        config_perror("Bad SPEED value");
        free(if_new);
        return;
    }
    if_new->type = strtol(type, &ecp, 0);
    if (*ecp || if_new->type < 0) {
        config_perror("Bad TYPE");
        free(if_new);
        return;
    }
    if_new->name = strdup(name);
    if (!if_new->name) {
        config_perror("Out of memory");
        free(if_new);
        return;
    }
    if_new->next = conf_list;
    conf_list = if_new;
}

static void
free_interface_config(void)
{
    conf_if_list   *if_ptr = conf_list, *if_next;
    while (if_ptr) {
        if_next = if_ptr->next;
        free(if_ptr->name);
        free(if_ptr);
        if_ptr = if_next;
    }
    conf_list = NULL;
}



/*
 * header_ifEntry(...
 * Arguments:
 * vp     IN      - pointer to variable entry that points here
 * name    IN/OUT  - IN/name requested, OUT/name found
 * length  IN/OUT  - length of IN/OUT oid's 
 * exact   IN      - TRUE if an exact match was requested
 * var_len OUT     - length of variable or 0 if function returned
 * write_method
 * 
 */
#ifndef WIN32
static int
header_ifEntry(struct variable *vp,
               oid * name,
               size_t * length,
               int exact, size_t * var_len, WriteMethod ** write_method)
{
#define IFENTRY_NAME_LENGTH	10
    oid             newname[MAX_OID_LEN];
    register int    interface;
    int             result, count;

    DEBUGMSGTL(("mibII/interfaces", "var_ifEntry: "));
    DEBUGMSGOID(("mibII/interfaces", name, *length));
    DEBUGMSG(("mibII/interfaces", " %d\n", exact));

    memcpy((char *) newname, (char *) vp->name,
           (int) vp->namelen * sizeof(oid));
    /*
     * find "next" interface 
     */
    count = Interface_Scan_Get_Count();
    for (interface = 1; interface <= count; interface++) {
        newname[IFENTRY_NAME_LENGTH] = (oid) interface;
        result =
            snmp_oid_compare(name, *length, newname,
                             (int) vp->namelen + 1);
        if ((exact && (result == 0)) || (!exact && (result < 0)))
            break;
    }
    if (interface > count) {
        DEBUGMSGTL(("mibII/interfaces", "... index out of range\n"));
        return MATCH_FAILED;
    }


    memcpy((char *) name, (char *) newname,
           ((int) vp->namelen + 1) * sizeof(oid));
    *length = vp->namelen + 1;
    *write_method = 0;
    *var_len = sizeof(long);    /* default to 'long' results */

    DEBUGMSGTL(("mibII/interfaces", "... get I/F stats "));
    DEBUGMSGOID(("mibII/interfaces", name, *length));
    DEBUGMSG(("mibII/interfaces", "\n"));

    return interface;
}



u_char         *
var_interfaces(struct variable * vp,
               oid * name,
               size_t * length,
               int exact, size_t * var_len, WriteMethod ** write_method)
{
    if (header_generic(vp, name, length, exact, var_len, write_method) ==
        MATCH_FAILED)
        return NULL;

    switch (vp->magic) {
    case IFNUMBER:
        long_return = Interface_Scan_Get_Count();
        return (u_char *) & long_return;
    default:
        DEBUGMSGTL(("snmpd", "unknown sub-id %d in var_interfaces\n",
                    vp->magic));
    }
    return NULL;
}

#ifdef USE_SYSCTL_IFLIST

static u_char  *if_list = 0;
static const u_char *if_list_end;
static size_t   if_list_size = 0;

struct small_ifaddr {
    struct in_addr  sifa_addr;
    struct in_addr  sifa_netmask;
    struct in_addr  sifa_broadcast;
};

extern const struct sockaddr *get_address(const void *, int, int);
extern const struct in_addr *get_in_address(const void *, int, int);
static int      Interface_Scan_By_Index(int, struct if_msghdr *, char *,
                                        struct small_ifaddr *);
static int      Interface_Get_Ether_By_Index(int, u_char *);

static int
Interface_Scan_By_Index(int iindex,
                        struct if_msghdr *if_msg,
                        char *if_name, struct small_ifaddr *sifa)
{
    u_char         *cp;
    struct if_msghdr *ifp;
    int             have_ifinfo = 0, have_addr = 0;

    memset(sifa, 0, sizeof(*sifa));
    for (cp = if_list; cp < if_list_end; cp += ifp->ifm_msglen) {
        ifp = (struct if_msghdr *) cp;
        DEBUGMSGTL(("mibII/interfaces", "ifm_type = %d, ifm_index = %d\n",
                    ifp->ifm_type, ifp->ifm_index));

        switch (ifp->ifm_type) {
        case RTM_IFINFO:
            {
                const struct sockaddr *a;

                if (ifp->ifm_index == iindex) {
                    a = get_address(ifp + 1, ifp->ifm_addrs, RTA_IFP);
                    if (a == NULL)
                        return 0;
                    strncpy(if_name,
                            ((const struct sockaddr_in *) a)->sin_zero,
                            ((const u_char *) a)[5]);
                    if_name[((const u_char *) a)[5]] = 0;
                    *if_msg = *ifp;
                    ++have_ifinfo;
                }
            }
            break;
        case RTM_NEWADDR:
            {
                struct ifa_msghdr *ifap = (struct ifa_msghdr *) cp;

                if (ifap->ifam_index == iindex) {
                    const struct in_addr *ia;

                    /*
                     * I don't know why the normal get_address() doesn't
                     * work on IRIX 6.2.  Maybe this has to do with the
                     * existence of struct sockaddr_new.  Hopefully, on
                     * other systems we can simply use get_in_address
                     * three times, with (ifap+1) as the starting
                     * address. 
                     */

                    sifa->sifa_netmask =
                        *((struct in_addr *) ((char *) (ifap + 1) + 4));
                    ia = get_in_address((char *) (ifap + 1) + 8,
                                        ifap->ifam_addrs &=
                                        ~RTA_NETMASK, RTA_IFA);
                    if (ia == NULL)
                        return 0;

                    sifa->sifa_addr = *ia;
                    ia = get_in_address((char *) (ifap + 1) + 8,
                                        ifap->ifam_addrs &= ~RTA_NETMASK,
                                        RTA_BRD);
                    if (ia == NULL)
                        return 0;

                    sifa->sifa_broadcast = *ia;
                    ++have_addr;
                }
            }
            break;
        default:
            DEBUGMSGTL(("mibII/interfaces",
                        "routing socket: unknown message type %d\n",
                        ifp->ifm_type));
        }
    }
    if (have_ifinfo && have_addr) {
        return 0;
    } else if (have_ifinfo && !(if_msg->ifm_flags & IFF_UP))
        return 0;
    else {
        return -1;
    }
}

int
Interface_Scan_Get_Count(void)
{
    u_char         *cp;
    struct if_msghdr *ifp;
    long            n = 0;

    Interface_Scan_Init();

    if (if_list_size) {
        for (cp = if_list, n = 0; cp < if_list_end; cp += ifp->ifm_msglen) {
            ifp = (struct if_msghdr *) cp;
    
            if (ifp->ifm_type == RTM_IFINFO) {
                ++n;
            }
        }
    }
    return n;
}

void
Interface_Scan_Init(void)
{
    int             name[] = { CTL_NET, PF_ROUTE, 0, 0, NET_RT_IFLIST, 0 };
    size_t          size;

    if (sysctl(name, sizeof(name) / sizeof(int), 0, &size, 0, 0) == -1) {
        snmp_log(LOG_ERR, "sysctl size fail\n");
    } else {
        if (if_list == 0 || if_list_size < size) {
            if (if_list != 0) {
                free(if_list);
            }
            if_list      = NULL;
            if_list_size = 0;
            if_list_end  = 0;
            if ((if_list = malloc(size)) == NULL) {
                snmp_log(LOG_ERR,
                         "out of memory allocating route table (size = %d)\n", size);
                return;
            }
            if_list_size = size;
        } else {
            size = if_list_size;
        }
        if (sysctl(name, sizeof(name) / sizeof(int),
                   if_list, &size, 0, 0) == -1) {
            snmp_log(LOG_ERR, "sysctl get fail\n");
        }
        if_list_end = if_list + size;
    }
}

u_char         *
var_ifEntry(struct variable *vp,
            oid * name,
            size_t * length,
            int exact, size_t * var_len, WriteMethod ** write_method)
{
    int             interface;
    struct if_msghdr if_msg;
    static char     if_name[100];
    struct small_ifaddr sifa;
    char           *cp;

    interface =
        header_ifEntry(vp, name, length, exact, var_len, write_method);
    if (interface == MATCH_FAILED)
        return NULL;

    if (Interface_Scan_By_Index(interface, &if_msg, if_name, &sifa) != 0)
        return NULL;

    switch (vp->magic) {
    case IFINDEX:
        long_return = interface;
        return (u_char *) & long_return;
    case IFDESCR:
        cp = if_name;
        *var_len = strlen(if_name);
        return (u_char *) cp;
    case IFTYPE:
        long_return = (long) if_msg.ifm_data.ifi_type;
        return (u_char *) & long_return;
    case IFMTU:
        long_return = (long) if_msg.ifm_data.ifi_mtu;
        return (u_char *) & long_return;
    case IFSPEED:
#if STRUCT_IFNET_HAS_IF_BAUDRATE_IFS_VALUE
        long_return = (u_long) if_msg.ifm_data.ifi_baudrate.ifs_value <<
            if_msg.ifm_data.ifi_baudrate.ifs_log2;
#else
        long_return = (u_long) if_msg.ifm_data.ifi_baudrate;
#endif
        return (u_char *) & long_return;
    case IFPHYSADDRESS:
        /*
         * XXX 
         */
        return NULL;
    case IFADMINSTATUS:
        long_return = if_msg.ifm_flags & IFF_UP ? 1 : 2;
        return (u_char *) & long_return;
    case IFOPERSTATUS:
        long_return = if_msg.ifm_flags & IFF_RUNNING ? 1 : 2;
        return (u_char *) & long_return;
        /*
         * ifLastChange 
         */
    case IFINOCTETS:
        long_return = (u_long) if_msg.ifm_data.ifi_ibytes;
        return (u_char *) & long_return;
    case IFINUCASTPKTS:
        long_return =
            (u_long) if_msg.ifm_data.ifi_ipackets -
            if_msg.ifm_data.ifi_imcasts;
        return (u_char *) & long_return;
    case IFINNUCASTPKTS:
        long_return = (u_long) if_msg.ifm_data.ifi_imcasts;
        return (u_char *) & long_return;
    case IFINDISCARDS:
        long_return = (u_long) if_msg.ifm_data.ifi_iqdrops;
        return (u_char *) & long_return;
    case IFINERRORS:
        long_return = (u_long) if_msg.ifm_data.ifi_ierrors;
        return (u_char *) & long_return;
    case IFINUNKNOWNPROTOS:
        long_return = (u_long) if_msg.ifm_data.ifi_noproto;
        return (u_char *) & long_return;
    case IFOUTOCTETS:
        long_return = (u_long) if_msg.ifm_data.ifi_obytes;
        return (u_char *) & long_return;
    case IFOUTUCASTPKTS:
        long_return =
            (u_long) if_msg.ifm_data.ifi_opackets -
            if_msg.ifm_data.ifi_omcasts;
        return (u_char *) & long_return;
    case IFOUTNUCASTPKTS:
        long_return = (u_long) if_msg.ifm_data.ifi_omcasts;
        return (u_char *) & long_return;
    case IFOUTDISCARDS:
#ifdef if_odrops
        long_return = (u_long) if_msg.ifm_data.ifi_odrops;
#else
#if NO_DUMMY_VALUES
        return NULL;
#endif
        long_return = 0;
#endif
        return (u_char *) & long_return;
    case IFOUTERRORS:
        long_return = (u_long) if_msg.ifm_data.ifi_oerrors;
        return (u_char *) & long_return;
    case IFLASTCHANGE:
#ifdef irix6
        long_return = 0;
#else
        if (if_msg.ifm_data.ifi_lastchange.tv_sec == 0 &&
            if_msg.ifm_data.ifi_lastchange.tv_usec == 0)
            long_return = 0;
        else if (if_msg.ifm_data.ifi_lastchange.tv_sec < starttime.tv_sec)
            long_return = 0;
        else {
            long_return = (u_long)
                ((if_msg.ifm_data.ifi_lastchange.tv_sec -
                  starttime.tv_sec) * 100 +
                 (if_msg.ifm_data.ifi_lastchange.tv_usec -
                  starttime.tv_usec) / 10000);
        }
#endif
        return (u_char *) & long_return;
    default:
        return 0;
    }
}

int
Interface_Scan_Next(short *Index,
                    char *Name,
                    struct ifnet *Retifnet, struct in_ifaddr *Retin_ifaddr)
{
    return 0;
}

#else                           /* not USE_SYSCTL_IFLIST */

        /*********************
	 *
	 *  Kernel & interface information,
	 *   and internal forward declarations
	 *
	 *********************/

#ifndef HAVE_NET_IF_MIB_H

#ifndef solaris2
#ifndef hpux11
static int      Interface_Scan_By_Index(int, char *, struct ifnet *,
                                        struct in_ifaddr *);
static int      Interface_Get_Ether_By_Index(int, u_char *);
#else
static int      Interface_Scan_By_Index(int, char *, nmapi_phystat *);
#endif
#endif



        /*********************
	 *
	 *  System specific implementation functions
	 *
	 *********************/


#ifndef solaris2
#ifndef hpux

u_char         *
var_ifEntry(struct variable *vp,
            oid * name,
            size_t * length,
            int exact, size_t * var_len, WriteMethod ** write_method)
{
    static struct ifnet ifnet;
    int             interface;
    static struct in_ifaddr in_ifaddr;
    static char     Name[16];
    char           *cp;
    conf_if_list   *if_ptr = conf_list;
#if STRUCT_IFNET_HAS_IF_LASTCHANGE_TV_SEC
    struct timeval  now;
#endif

    interface =
        header_ifEntry(vp, name, length, exact, var_len, write_method);
    if (interface == MATCH_FAILED)
        return NULL;

    Interface_Scan_By_Index(interface, Name, &ifnet, &in_ifaddr);
    while (if_ptr && strcmp(Name, if_ptr->name))
        if_ptr = if_ptr->next;

    switch (vp->magic) {
    case IFINDEX:
        long_return = interface;
        return (u_char *) & long_return;
    case IFDESCR:
        cp = Name;
        *var_len = strlen(cp);
        return (u_char *) cp;
    case IFTYPE:
        if (if_ptr)
            long_return = if_ptr->type;
        else {
#if STRUCT_IFNET_HAS_IF_TYPE
            long_return = ifnet.if_type;
#else
            long_return = 1;    /* OTHER */
#endif
        }
        return (u_char *) & long_return;
    case IFMTU:{
            long_return = (long) ifnet.if_mtu;
            return (u_char *) & long_return;
        }
    case IFSPEED:
        if (if_ptr)
            long_return = if_ptr->speed;
        else {
#if STRUCT_IFNET_HAS_IF_BAUDRATE
            long_return = ifnet.if_baudrate;
#elif STRUCT_IFNET_HAS_IF_SPEED
            long_return = ifnet.if_speed;
#elif STRUCT_IFNET_HAS_IF_TYPE && defined(IFT_ETHER)
            if (ifnet.if_type == IFT_ETHER)
                long_return = 10000000;
            if (ifnet.if_type == IFT_P10)
                long_return = 10000000;
            if (ifnet.if_type == IFT_P80)
                long_return = 80000000;
            if (ifnet.if_type == IFT_ISDNBASIC)
                long_return = 64000;    /* EDSS1 only */
            if (ifnet.if_type == IFT_ISDNPRIMARY)
                long_return = 64000 * 30;
#else
#if NO_DUMMY_VALUES
            return NULL;
#endif
            long_return = (u_long) 10000000;
#endif
        }
        return (u_char *) & long_return;
    case IFPHYSADDRESS:
        Interface_Get_Ether_By_Index(interface, return_buf);
        *var_len = 6;
        if ((return_buf[0] == 0) && (return_buf[1] == 0) &&
            (return_buf[2] == 0) && (return_buf[3] == 0) &&
            (return_buf[4] == 0) && (return_buf[5] == 0))
            *var_len = 0;
        return (u_char *) return_buf;
    case IFADMINSTATUS:
        long_return = ifnet.if_flags & IFF_UP ? 1 : 2;
        return (u_char *) & long_return;
    case IFOPERSTATUS:
        long_return = ifnet.if_flags & IFF_RUNNING ? 1 : 2;
        return (u_char *) & long_return;
    case IFLASTCHANGE:
#if defined(STRUCT_IFNET_HAS_IF_LASTCHANGE_TV_SEC) && !(defined(freebsd2) && __FreeBSD_version < 199607)
        /*
         * XXX - SNMP's ifLastchange is time when op. status changed
         * * FreeBSD's if_lastchange is time when packet was input or output
         * * (at least in 2.1.0-RELEASE. Changed in later versions of the kernel?)
         */
        /*
         * FreeBSD's if_lastchange before the 2.1.5 release is the time when
         * * a packet was last input or output.  In the 2.1.5 and later releases,
         * * this is fixed, thus the 199607 comparison.
         */
        if (ifnet.if_lastchange.tv_sec == 0 &&
            ifnet.if_lastchange.tv_usec == 0)
            long_return = 0;
        else if (ifnet.if_lastchange.tv_sec < starttime.tv_sec)
            long_return = 0;
        else {
            long_return = (u_long)
                ((ifnet.if_lastchange.tv_sec - starttime.tv_sec) * 100
                 + (ifnet.if_lastchange.tv_usec -
                    starttime.tv_usec) / 10000);
        }
#else
#if NO_DUMMY_VALUES
        return NULL;
#endif
        long_return = 0;        /* XXX */
#endif
        return (u_char *) & long_return;
    case IFINOCTETS:
#ifdef STRUCT_IFNET_HAS_IF_IBYTES
        long_return = (u_long) ifnet.if_ibytes;
#else
#if NO_DUMMY_VALUES
        return NULL;
#endif
        long_return = (u_long) ifnet.if_ipackets * 308; /* XXX */
#endif
        return (u_char *) & long_return;
    case IFINUCASTPKTS:
        {
            long_return = (u_long) ifnet.if_ipackets;
#if STRUCT_IFNET_HAS_IF_IMCASTS
            long_return -= (u_long) ifnet.if_imcasts;
#endif
        }
        return (u_char *) & long_return;
    case IFINNUCASTPKTS:
#if STRUCT_IFNET_HAS_IF_IMCASTS
        long_return = (u_long) ifnet.if_imcasts;
#else
#if NO_DUMMY_VALUES
        return NULL;
#endif
        long_return = (u_long) 0;       /* XXX */
#endif
        return (u_char *) & long_return;
    case IFINDISCARDS:
#if STRUCT_IFNET_HAS_IF_IQDROPS
        long_return = (u_long) ifnet.if_iqdrops;
#else
#if NO_DUMMY_VALUES
        return NULL;
#endif
        long_return = (u_long) 0;       /* XXX */
#endif
        return (u_char *) & long_return;
    case IFINERRORS:
        long_return = (u_long) ifnet.if_ierrors;
        return (u_char *) & long_return;
    case IFINUNKNOWNPROTOS:
#if STRUCT_IFNET_HAS_IF_NOPROTO
        long_return = (u_long) ifnet.if_noproto;
#else
#if NO_DUMMY_VALUES
        return NULL;
#endif
        long_return = (u_long) 0;       /* XXX */
#endif
        return (u_char *) & long_return;
    case IFOUTOCTETS:
#ifdef STRUCT_IFNET_HAS_IF_OBYTES
        long_return = (u_long) ifnet.if_obytes;
#else
#if NO_DUMMY_VALUES
        return NULL;
#endif
        long_return = (u_long) ifnet.if_opackets * 308; /* XXX */
#endif
        return (u_char *) & long_return;
    case IFOUTUCASTPKTS:
        {
            long_return = (u_long) ifnet.if_opackets;
#if STRUCT_IFNET_HAS_IF_OMCASTS
            long_return -= (u_long) ifnet.if_omcasts;
#endif
        }
        return (u_char *) & long_return;
    case IFOUTNUCASTPKTS:
#if STRUCT_IFNET_HAS_IF_OMCASTS
        long_return = (u_long) ifnet.if_omcasts;
#else
#if NO_DUMMY_VALUES
        return NULL;
#endif
        long_return = (u_long) 0;       /* XXX */
#endif
        return (u_char *) & long_return;
    case IFOUTDISCARDS:
        long_return = ifnet.if_snd.ifq_drops;
        return (u_char *) & long_return;
    case IFOUTERRORS:
        long_return = ifnet.if_oerrors;
        return (u_char *) & long_return;
    case IFOUTQLEN:
        long_return = ifnet.if_snd.ifq_len;
        return (u_char *) & long_return;
    case IFSPECIFIC:
        *var_len = nullOidLen;
        return (u_char *) nullOid;
    default:
        DEBUGMSGTL(("snmpd", "unknown sub-id %d in var_ifEntry\n",
                    vp->magic));
    }
    return NULL;
}

#else                           /* hpux */

u_char         *
var_ifEntry(struct variable *vp,
            oid * name,
            size_t * length,
            int exact, size_t * var_len, WriteMethod ** write_method)
{
#if defined(hpux11)
    static nmapi_phystat ifnet;
#else
    static struct ifnet ifnet;
#endif
    register int    interface;
#if !defined(hpux11)
    static struct in_ifaddr in_ifaddrVar;
#endif
#if defined(hpux11)
    static char     Name[MAX_PHYSADDR_LEN];
#else
    static char     Name[16];
#endif
    register char  *cp;
#if STRUCT_IFNET_HAS_IF_LASTCHANGE_TV_SEC
    struct timeval  now;
#endif
#if !defined(hpux11)
    struct nmparms  hp_nmparms;
    static mib_ifEntry hp_ifEntry;
    int             hp_fd;
    int             hp_len = sizeof(hp_ifEntry);
#endif


    interface =
        header_ifEntry(vp, name, length, exact, var_len, write_method);
    if (interface == MATCH_FAILED)
        return NULL;

#if defined(hpux11)
    Interface_Scan_By_Index(interface, Name, &ifnet);
#else
    Interface_Scan_By_Index(interface, Name, &ifnet, &in_ifaddrVar);
#endif

#if !defined(hpux11)
    /*
     * Additional information about the interfaces is available under
     * HP-UX through the network management interface '/dev/netman'
     */
    hp_ifEntry.ifIndex = interface;
    hp_nmparms.objid = ID_ifEntry;
    hp_nmparms.buffer = (char *) &hp_ifEntry;
    hp_nmparms.len = &hp_len;
    if ((hp_fd = open("/dev/netman", O_RDONLY)) != -1) {
        if (ioctl(hp_fd, NMIOGET, &hp_nmparms) != -1) {
            close(hp_fd);
        } else {
            close(hp_fd);
            hp_fd = -1;         /* failed */
        }
    }
#endif

    switch (vp->magic) {
    case IFINDEX:
        long_return = interface;
        return (u_char *) & long_return;
    case IFDESCR:
#if defined(hpux11)
        cp = ifnet.if_entry.ifDescr;
#else
        if (hp_fd != -1)
            cp = hp_ifEntry.ifDescr;
        else
            cp = Name;
#endif
        *var_len = strlen(cp);
        return (u_char *) cp;
    case IFTYPE:
#if defined(hpux11)
        long_return = ifnet.if_entry.ifType;
#else
        if (hp_fd != -1)
            long_return = hp_ifEntry.ifType;
        else
            long_return = 1;    /* OTHER */
#endif
        return (u_char *) & long_return;
    case IFMTU:{
#if defined(hpux11)
            long_return = (long) ifnet.if_entry.ifMtu;
#else
            long_return = (long) ifnet.if_mtu;
#endif
            return (u_char *) & long_return;
        }
    case IFSPEED:
#if defined(hpux11)
        long_return = ifnet.if_entry.ifSpeed;
#else
        if (hp_fd != -1)
            long_return = hp_ifEntry.ifSpeed;
        else
            long_return = (u_long) 1;   /* OTHER */
#endif
        return (u_char *) & long_return;
    case IFPHYSADDRESS:
#if defined(hpux11)
        *var_len = ifnet.if_entry.ifPhysAddress.o_length;
        return (u_char *) ifnet.if_entry.ifPhysAddress.o_bytes;
#else
        Interface_Get_Ether_By_Index(interface, return_buf);
        *var_len = 6;
        if ((return_buf[0] == 0) && (return_buf[1] == 0) &&
            (return_buf[2] == 0) && (return_buf[3] == 0) &&
            (return_buf[4] == 0) && (return_buf[5] == 0))
            *var_len = 0;
        return (u_char *) return_buf;
#endif
    case IFADMINSTATUS:
#if defined(hpux11)
        long_return = ifnet.if_entry.ifAdmin;
#else
        long_return = ifnet.if_flags & IFF_UP ? 1 : 2;
#endif
        return (u_char *) & long_return;
    case IFOPERSTATUS:
#if defined(hpux11)
        long_return = ifnet.if_entry.ifOper;
#else
        long_return = ifnet.if_flags & IFF_RUNNING ? 1 : 2;
#endif
        return (u_char *) & long_return;
    case IFLASTCHANGE:
#if defined(hpux11)
        long_return = ifnet.if_entry.ifLastChange;
#else
        if (hp_fd != -1)
            long_return = hp_ifEntry.ifLastChange;
        else
            long_return = 0;    /* XXX */
#endif
        return (u_char *) & long_return;
    case IFINOCTETS:
#if defined(hpux11)
        long_return = ifnet.if_entry.ifInOctets;
#else
        if (hp_fd != -1)
            long_return = hp_ifEntry.ifInOctets;
        else
            long_return = (u_long) ifnet.if_ipackets * 308;     /* XXX */
#endif
        return (u_char *) & long_return;
    case IFINUCASTPKTS:
#if defined(hpux11)
        long_return = ifnet.if_entry.ifInUcastPkts;
#else
        if (hp_fd != -1)
            long_return = hp_ifEntry.ifInUcastPkts;
        else
            long_return = (u_long) ifnet.if_ipackets;
#endif
        return (u_char *) & long_return;
    case IFINNUCASTPKTS:
#if defined(hpux11)
        long_return = ifnet.if_entry.ifInNUcastPkts;
#else
        if (hp_fd != -1)
            long_return = hp_ifEntry.ifInNUcastPkts;
        else
            long_return = (u_long) 0;   /* XXX */
#endif
        return (u_char *) & long_return;
    case IFINDISCARDS:
#if defined(hpux11)
        long_return = ifnet.if_entry.ifInDiscards;
#else
        if (hp_fd != -1)
            long_return = hp_ifEntry.ifInDiscards;
        else
            long_return = (u_long) 0;   /* XXX */
#endif
        return (u_char *) & long_return;
    case IFINERRORS:
#if defined(hpux11)
        long_return = ifnet.if_entry.ifInErrors;
#else
        long_return = ifnet.if_ierrors;
#endif
        return (u_char *) & long_return;
    case IFINUNKNOWNPROTOS:
#if defined(hpux11)
        long_return = ifnet.if_entry.ifInUnknownProtos;
#else
        if (hp_fd != -1)
            long_return = hp_ifEntry.ifInUnknownProtos;
        else
            long_return = (u_long) 0;   /* XXX */
#endif
        return (u_char *) & long_return;
    case IFOUTOCTETS:
#if defined(hpux11)
        long_return = ifnet.if_entry.ifOutOctets;
#else
        if (hp_fd != -1)
            long_return = hp_ifEntry.ifOutOctets;
        else
            long_return = (u_long) ifnet.if_opackets * 308;     /* XXX */
#endif
        return (u_char *) & long_return;
    case IFOUTUCASTPKTS:
#if defined(hpux11)
        long_return = ifnet.if_entry.ifOutUcastPkts;
#else
        if (hp_fd != -1)
            long_return = hp_ifEntry.ifOutUcastPkts;
        else
            long_return = (u_long) ifnet.if_opackets;
#endif
        return (u_char *) & long_return;
    case IFOUTNUCASTPKTS:
#if defined(hpux11)
        long_return = ifnet.if_entry.ifOutNUcastPkts;
#else
        if (hp_fd != -1)
            long_return = hp_ifEntry.ifOutNUcastPkts;
        else
            long_return = (u_long) 0;   /* XXX */
#endif
        return (u_char *) & long_return;
    case IFOUTDISCARDS:
#if defined(hpux11)
        long_return = ifnet.if_entry.ifOutDiscards;
#else
        long_return = ifnet.if_snd.ifq_drops;
#endif
        return (u_char *) & long_return;
    case IFOUTERRORS:
#if defined(hpux11)
        long_return = ifnet.if_entry.ifOutErrors;
#else
        long_return = ifnet.if_oerrors;
#endif
        return (u_char *) & long_return;
    case IFOUTQLEN:
#if defined(hpux11)
        long_return = ifnet.if_entry.ifOutQlen;
#else
        long_return = ifnet.if_snd.ifq_len;
#endif
        return (u_char *) & long_return;
    case IFSPECIFIC:
        *var_len = nullOidLen;
        return (u_char *) nullOid;
    default:
        DEBUGMSGTL(("snmpd", "unknown sub-id %d in var_ifEntry\n",
                    vp->magic));
    }
    return NULL;
}

#endif                          /* hpux */
#else                           /* solaris2 */

static int
IF_cmp(void *addr, void *ep)
{
    DEBUGMSGTL(("mibII/interfaces", "... IF_cmp %d %d\n",
                ((mib2_ifEntry_t *) ep)->ifIndex,
                ((mib2_ifEntry_t *) addr)->ifIndex));
    if (((mib2_ifEntry_t *) ep)->ifIndex ==
        ((mib2_ifEntry_t *) addr)->ifIndex)
        return (0);
    else
        return (1);
}

u_char         *
var_ifEntry(struct variable * vp,
            oid * name,
            size_t * length,
            int exact, size_t * var_len, WriteMethod ** write_method)
{
    int             interface;
    mib2_ifEntry_t  ifstat;


    interface =
        header_ifEntry(vp, name, length, exact, var_len, write_method);
    if (interface == MATCH_FAILED)
        return NULL;

    if (getMibstat(MIB_INTERFACES, &ifstat, sizeof(mib2_ifEntry_t),
                   GET_EXACT, &IF_cmp, &interface) != 0) {
        DEBUGMSGTL(("mibII/interfaces", "... no mib stats\n"));
        return NULL;
    }
    switch (vp->magic) {
    case IFINDEX:
        long_return = ifstat.ifIndex;
        return (u_char *) & long_return;
    case IFDESCR:
        *var_len = ifstat.ifDescr.o_length;
        (void) memcpy(return_buf, ifstat.ifDescr.o_bytes, *var_len);
        return (u_char *) return_buf;
    case IFTYPE:
        long_return = (u_long) ifstat.ifType;
        return (u_char *) & long_return;
    case IFMTU:
        long_return = (u_long) ifstat.ifMtu;
        return (u_char *) & long_return;
    case IFSPEED:
        long_return = (u_long) ifstat.ifSpeed;
        return (u_char *) & long_return;
    case IFPHYSADDRESS:
        *var_len = ifstat.ifPhysAddress.o_length;
        (void) memcpy(return_buf, ifstat.ifPhysAddress.o_bytes, *var_len);
        return (u_char *) return_buf;
    case IFADMINSTATUS:
        long_return = (u_long) ifstat.ifAdminStatus;
        return (u_char *) & long_return;
    case IFOPERSTATUS:
        long_return = (u_long) ifstat.ifOperStatus;
        return (u_char *) & long_return;
    case IFLASTCHANGE:
        long_return = (u_long) ifstat.ifLastChange;
        return (u_char *) & long_return;
    case IFINOCTETS:
        long_return = (u_long) ifstat.ifInOctets;
        return (u_char *) & long_return;
    case IFINUCASTPKTS:
        long_return = (u_long) ifstat.ifInUcastPkts;
        return (u_char *) & long_return;
    case IFINNUCASTPKTS:
        long_return = (u_long) ifstat.ifInNUcastPkts;
        return (u_char *) & long_return;
    case IFINDISCARDS:
        long_return = (u_long) ifstat.ifInDiscards;
        return (u_char *) & long_return;
    case IFINERRORS:
        long_return = (u_long) ifstat.ifInErrors;
        return (u_char *) & long_return;
    case IFINUNKNOWNPROTOS:
        long_return = (u_long) ifstat.ifInUnknownProtos;
        return (u_char *) & long_return;
    case IFOUTOCTETS:
        long_return = (u_long) ifstat.ifOutOctets;
        return (u_char *) & long_return;
    case IFOUTUCASTPKTS:
        long_return = (u_long) ifstat.ifOutUcastPkts;
        return (u_char *) & long_return;
    case IFOUTNUCASTPKTS:
        long_return = (u_long) ifstat.ifOutNUcastPkts;
        return (u_char *) & long_return;
    case IFOUTDISCARDS:
        long_return = (u_long) ifstat.ifOutDiscards;
        return (u_char *) & long_return;
    case IFOUTERRORS:
        long_return = (u_long) ifstat.ifOutErrors;
        return (u_char *) & long_return;
    case IFOUTQLEN:
        long_return = (u_long) ifstat.ifOutQLen;
        return (u_char *) & long_return;
    default:
        DEBUGMSGTL(("snmpd", "unknown sub-id %d in var_ifEntry\n",
                    vp->magic));
    }
    return NULL;
}

#endif                          /* solaris2 */



        /*********************
	 *
	 *  Internal implementation functions
	 *
	 *********************/


#ifndef solaris2

#if !defined(sunV3) && !defined(linux) && !defined(hpux11)
static struct in_ifaddr savein_ifaddr;
#endif
#if !defined(hpux11)
static struct ifnet *ifnetaddr, saveifnet, *saveifnetaddr;
static char     saveName[16];
#endif
static int      saveIndex = 0;

/**
* Determines network interface speed. It is system specific. Only linux
* realization is made. 
*/
unsigned int getIfSpeed(int fd, struct ifreq ifr){
	unsigned int retspeed = 10000000;
	return retspeed;
}


void
Interface_Scan_Init(void)
{
#ifdef linux
    char            line[256], ifname_buf[64], *ifname, *ptr;
    struct ifreq    ifrq;
    struct ifnet  **ifnetaddr_ptr;
    FILE           *devin;
    unsigned long   rec_pkt, rec_oct, rec_err, rec_drop;
    unsigned long   snd_pkt, snd_oct, snd_err, snd_drop, coll;
    int             i, fd;
    conf_if_list   *if_ptr;
    const char     *scan_line_2_2 =
        "%lu %lu %lu %lu %*lu %*lu %*lu %*lu %lu %lu %lu %lu %*lu %lu";
    const char     *scan_line_2_0 =
        "%lu %lu %*lu %*lu %*lu %lu %lu %*lu %*lu %lu";
    const char     *scan_line_to_use;

#endif

#if !defined(hpux11)
    auto_nlist(IFNET_SYMBOL, (char *) &ifnetaddr, sizeof(ifnetaddr));
#endif
    saveIndex = 0;

#ifdef linux
    /*
     * free old list: 
     */
    while (ifnetaddr_list) {
        struct ifnet   *old = ifnetaddr_list;
        ifnetaddr_list = ifnetaddr_list->if_next;
        free(old->if_name);
        free(old->if_unit);
        free(old);
    }

    ifnetaddr = 0;
    ifnetaddr_ptr = &ifnetaddr_list;

    if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        DEBUGMSGTL(("snmpd",
                    "socket open failure in Interface_Scan_Init\n"));
        return; /** exit (1); **/
    }

    /*
     * build up ifnetaddr list by hand: 
     */

    /*
     * at least linux v1.3.53 says EMFILE without reason... 
     */
    if (!(devin = fopen("/proc/net/dev", "r"))) {
        close(fd);
        snmp_log(LOG_ERR, "cannot open /proc/net/dev - continuing...\n");
        return; /** exit (1); **/
    }

    i = 0;

    /*
     * read the second line (a header) and determine the fields we
     * should read from.  This should be done in a better way by
     * actually looking for the field names we want.  But thats too
     * much work for today.  -- Wes 
     */
    fgets(line, sizeof(line), devin);
    fgets(line, sizeof(line), devin);
    if (strstr(line, "compressed")) {
        scan_line_to_use = scan_line_2_2;
        DEBUGMSGTL(("mibII/interfaces",
                    "using linux 2.2 kernel /proc/net/dev\n"));
    } else {
        scan_line_to_use = scan_line_2_0;
        DEBUGMSGTL(("mibII/interfaces",
                    "using linux 2.0 kernel /proc/net/dev\n"));
    }


    while (fgets(line, sizeof(line), devin)) {
        struct ifnet   *nnew;
        char           *stats, *ifstart = line;

        if (line[strlen(line) - 1] == '\n')
            line[strlen(line) - 1] = '\0';

        while (*ifstart && *ifstart == ' ')
            ifstart++;

        if (!*ifstart || ((stats = strrchr(ifstart, ':')) == NULL)) {
            snmp_log(LOG_ERR,
                     "/proc/net/dev data format error, line ==|%s|", line);
            continue;
        }
        if ((scan_line_to_use == scan_line_2_2) && ((stats - line) < 6)) {
            snmp_log(LOG_ERR,
                     "/proc/net/dev data format error, line ==|%s|", line);
        }

        *stats   = 0;
        strncpy(ifname_buf, ifstart, sizeof(ifname_buf));
        ifname_buf[ sizeof(ifname_buf)-1 ] = 0;
        *stats++ = ':';
        while (*stats == ' ')
            stats++;

        if ((scan_line_to_use == scan_line_2_2 &&
             sscanf(stats, scan_line_to_use, &rec_oct, &rec_pkt, &rec_err,
                    &rec_drop, &snd_oct, &snd_pkt, &snd_err, &snd_drop,
                    &coll) != 9) || (scan_line_to_use == scan_line_2_0
                                     && sscanf(stats, scan_line_to_use,
                                               &rec_pkt, &rec_err,
                                               &snd_pkt, &snd_err,
                                               &coll) != 5)) {
            if ((scan_line_to_use == scan_line_2_2)
                && !strstr(line, "No statistics available"))
                snmp_log(LOG_ERR,
                         "/proc/net/dev data format error, line ==|%s|",
                         line);
            continue;
        }

        nnew = (struct ifnet *) calloc(1, sizeof(struct ifnet));
        if (nnew == NULL)
            break;              /* alloc error */

        /*
         * chain in: 
         */
        *ifnetaddr_ptr = nnew;
        ifnetaddr_ptr = &nnew->if_next;
        i++;

        /*
         * linux previous to 1.3.~13 may miss transmitted loopback pkts: 
         */
        if (!strcmp(ifname_buf, "lo") && rec_pkt > 0 && !snd_pkt)
            snd_pkt = rec_pkt;

        nnew->if_ipackets = rec_pkt;
        nnew->if_ierrors = rec_err;
        nnew->if_opackets = snd_pkt;
        nnew->if_oerrors = snd_err;
        nnew->if_collisions = coll;
        if (scan_line_to_use == scan_line_2_2) {
            nnew->if_ibytes = rec_oct;
            nnew->if_obytes = snd_oct;
            nnew->if_iqdrops = rec_drop;
            nnew->if_snd.ifq_drops = snd_drop;
        } else {
            nnew->if_ibytes = rec_pkt * 308;
            nnew->if_obytes = snd_pkt * 308;
        }

        /*
         * ifnames are given as ``   eth0'': split in ``eth'' and ``0'': 
         */
        for (ifname = ifname_buf; *ifname && *ifname == ' '; ifname++);

        /*
         * set name and interface# : 
         */
        nnew->if_name = (char *) strdup(ifname);
        for (ptr = nnew->if_name; *ptr && (*ptr < '0' || *ptr > '9');
             ptr++);
        nnew->if_unit = strdup(*ptr ? ptr : "");
        *ptr = 0;

        strncpy(ifrq.ifr_name, ifname, sizeof(ifrq.ifr_name));
        ifrq.ifr_name[ sizeof(ifrq.ifr_name)-1 ] = 0;
        if (ioctl(fd, SIOCGIFADDR, &ifrq) < 0)
            memset((char *) &nnew->if_addr, 0, sizeof(nnew->if_addr));
        else
            nnew->if_addr = ifrq.ifr_addr;

        strncpy(ifrq.ifr_name, ifname, sizeof(ifrq.ifr_name));
        ifrq.ifr_name[ sizeof(ifrq.ifr_name)-1 ] = 0;
        if (ioctl(fd, SIOCGIFBRDADDR, &ifrq) < 0)
            memset((char *) &nnew->ifu_broadaddr, 0,
                   sizeof(nnew->ifu_broadaddr));
        else
            nnew->ifu_broadaddr = ifrq.ifr_broadaddr;

        strncpy(ifrq.ifr_name, ifname, sizeof(ifrq.ifr_name));
        ifrq.ifr_name[ sizeof(ifrq.ifr_name)-1 ] = 0;
        if (ioctl(fd, SIOCGIFNETMASK, &ifrq) < 0)
            memset((char *) &nnew->ia_subnetmask, 0,
                   sizeof(nnew->ia_subnetmask));
        else
            nnew->ia_subnetmask = ifrq.ifr_netmask;

        strncpy(ifrq.ifr_name, ifname, sizeof(ifrq.ifr_name));
        ifrq.ifr_name[ sizeof(ifrq.ifr_name)-1 ] = 0;
        nnew->if_flags = ioctl(fd, SIOCGIFFLAGS, &ifrq) < 0
            ? 0 : ifrq.ifr_flags;

        nnew->if_type = 0;

        strncpy(ifrq.ifr_name, ifname, sizeof(ifrq.ifr_name));
        ifrq.ifr_name[ sizeof(ifrq.ifr_name)-1 ] = 0;
        if (ioctl(fd, SIOCGIFHWADDR, &ifrq) < 0)
            memset(nnew->if_hwaddr, (0), 6);
        else {
            memcpy(nnew->if_hwaddr, ifrq.ifr_hwaddr.sa_data, 6);

#ifdef ARPHRD_LOOPBACK
            switch (ifrq.ifr_hwaddr.sa_family) {
            case ARPHRD_ETHER:
                nnew->if_type = 6;
                break;
            case ARPHRD_TUNNEL:
            case ARPHRD_TUNNEL6:
#ifdef ARPHRD_IPGRE
            case ARPHRD_IPGRE:
#endif
            case ARPHRD_SIT:
                nnew->if_type = 131;
                break;          /* tunnel */
            case ARPHRD_SLIP:
            case ARPHRD_CSLIP:
            case ARPHRD_SLIP6:
            case ARPHRD_CSLIP6:
                nnew->if_type = 28;
                break;          /* slip */
            case ARPHRD_PPP:
                nnew->if_type = 23;
                break;          /* ppp */
            case ARPHRD_LOOPBACK:
                nnew->if_type = 24;
                break;          /* softwareLoopback */
            case ARPHRD_FDDI:
                nnew->if_type = 15;
                break;
            case ARPHRD_ARCNET:
                nnew->if_type = 35;
                break;
            case ARPHRD_LOCALTLK:
                nnew->if_type = 42;
                break;
#ifdef ARPHRD_HIPPI
            case ARPHRD_HIPPI:
                nnew->if_type = 47;
                break;
#endif
#ifdef ARPHRD_ATM
            case ARPHRD_ATM:
                nnew->if_type = 37;
                break;
#endif
                /*
                 * XXX: more if_arp.h:ARPHDR_xxx to IANAifType mappings... 
                 */
            }
#endif
        }

        strncpy(ifrq.ifr_name, ifname, sizeof(ifrq.ifr_name));
        ifrq.ifr_name[ sizeof(ifrq.ifr_name)-1 ] = 0;
        nnew->if_metric = ioctl(fd, SIOCGIFMETRIC, &ifrq) < 0
            ? 0 : ifrq.ifr_metric;

#ifdef SIOCGIFMTU
        strncpy(ifrq.ifr_name, ifname, sizeof(ifrq.ifr_name));
        ifrq.ifr_name[ sizeof(ifrq.ifr_name)-1 ] = 0;
        nnew->if_mtu = (ioctl(fd, SIOCGIFMTU, &ifrq) < 0)
            ? 0 : ifrq.ifr_mtu;
#else
        nnew->if_mtu = 0;
#endif

        for (if_ptr = conf_list; if_ptr; if_ptr = if_ptr->next)
            if (!strcmp(if_ptr->name, ifname))
                break;

        if (if_ptr) {
            nnew->if_type = if_ptr->type;
            nnew->if_speed = if_ptr->speed;
        } else {
            /*
             * do only guess if_type from name, if we could not read
             * * it before from SIOCGIFHWADDR 
             */
            if (!nnew->if_type)
                nnew->if_type = if_type_from_name(nnew->if_name);
            nnew->if_speed = nnew->if_type == 6 ? getIfSpeed(fd, ifrq) :
                nnew->if_type == 24 ? 10000000 :
                nnew->if_type == 9 ? 4000000 : 0;
            /*Zero speed means link problem*/
            if(nnew->if_speed == 0 && nnew->if_flags & IFF_UP){
                nnew->if_flags &= ~IFF_RUNNING;
            }
        }

    }                           /* while (fgets ... */

    ifnetaddr = ifnetaddr_list;

    if (snmp_get_do_debugging()) {
        {
            struct ifnet   *x = ifnetaddr;
            DEBUGMSGTL(("mibII/interfaces", "* see: known interfaces:"));
            while (x) {
                DEBUGMSG(("mibII/interfaces", " %s", x->if_name));
                x = x->if_next;
            }
            DEBUGMSG(("mibII/interfaces", "\n"));
        }                       /* XXX */
    }

    fclose(devin);
    close(fd);
#endif                          /* linux */
}



#if defined(sunV3) || defined(linux)
/*
 * **  4.2 BSD doesn't have ifaddr
 * **  
 */
int
Interface_Scan_Next(short *Index,
                    char *Name,
                    struct ifnet *Retifnet, struct in_ifaddr *dummy)
{
    struct ifnet    ifnet;
    register char  *cp;

    while (ifnetaddr) {
        /*
         *      Get the "ifnet" structure and extract the device name
         */
#ifndef linux
        klookup((unsigned long) ifnetaddr, (char *) &ifnet, sizeof ifnet);
        klookup((unsigned long) ifnet.if_name, (char *) saveName,
                sizeof saveName);
#else
        ifnet = *ifnetaddr;
        strncpy(saveName, ifnet.if_name, sizeof(saveName));
#endif
        if (strcmp(saveName, "ip") == 0) {
            ifnetaddr = ifnet.if_next;
            continue;
        }



        saveName[sizeof(saveName) - 1] = '\0';
        cp = (char *) strchr(saveName, '\0');
#ifdef linux
        strncat(cp, ifnet.if_unit, sizeof(saveName)-strlen(saveName)-1);
        saveName[sizeof(saveName) - 1] = '\0';
#else
        string_append_int(cp, ifnet.if_unit);
#endif
        if (1 || strcmp(saveName, "lo0") != 0) {        /* XXX */

            if (Index)
                *Index = ++saveIndex;
            if (Retifnet)
                *Retifnet = ifnet;
            if (Name)
                strcpy(Name, saveName);
            saveifnet = ifnet;
            saveifnetaddr = ifnetaddr;
            ifnetaddr = ifnet.if_next;

            return (1);         /* DONE */
        }
        ifnetaddr = ifnet.if_next;
    }
    return (0);                 /* EOF */
}

#ifdef linux
int
Interface_Index_By_Name(char *Name, int Len)
{
    short           ifIndex = 0;
    char            ifName[20];

    Interface_Scan_Init();
    while (Interface_Scan_Next(&ifIndex, ifName, NULL, NULL)
           && strcmp(Name, ifName));
    return ifIndex;
}
#endif


#else                           /* sunV3 || linux */

#if defined(netbsd1) || defined(openbsd2)
#define ia_next ia_list.tqe_next
#define if_next if_list.tqe_next
#endif

#if defined(hpux11)

int
Interface_Scan_Next(short *Index, char *Name, nmapi_phystat * Retifnet)
{
    static nmapi_phystat *if_ptr = (nmapi_phystat *) 0;
    int             count = Interface_Scan_Get_Count();
    unsigned int    ulen;
    int             ret;

    if (!if_ptr) {
        if (count)
            if_ptr =
                (nmapi_phystat *) malloc(sizeof(nmapi_phystat) * count);
        else
            return (0);         /* EOF */
    }

    if (saveIndex >= count)
        return (0);             /* EOF */

    ulen = (unsigned int) count *sizeof(nmapi_phystat);
    if ((ret = get_physical_stat(if_ptr, &ulen)) < 0)
        return (0);             /* EOF */

    if (Retifnet)
        *Retifnet = if_ptr[saveIndex];
    if (Name)
        strcpy(Name, if_ptr[saveIndex].nm_device);
    saveIndex++;
    if (Index)
        *Index = saveIndex;
    return (1);                 /* DONE */
}

#else                           /* hpux11 */

int
Interface_Scan_Next(short *Index,
                    char *Name,
                    struct ifnet *Retifnet, struct in_ifaddr *Retin_ifaddr)
{
    struct ifnet    ifnet;
    struct in_ifaddr *ia, in_ifaddr;
    short           has_ipaddr = 0;
#if !STRUCT_IFNET_HAS_IF_XNAME
    register char  *cp;
#endif

    while (ifnetaddr) {
        /*
         *      Get the "ifnet" structure and extract the device name
         */
        klookup((unsigned long) ifnetaddr, (char *) &ifnet, sizeof ifnet);
#if STRUCT_IFNET_HAS_IF_XNAME
#if defined(netbsd1) || defined(openbsd2)
        strncpy(saveName, ifnet.if_xname, sizeof saveName);
#else
        klookup((unsigned long) ifnet.if_xname, (char *) saveName,
                sizeof saveName);
#endif
        saveName[sizeof(saveName) - 1] = '\0';
#else
        klookup((unsigned long) ifnet.if_name, (char *) saveName,
                sizeof saveName);

        saveName[sizeof(saveName) - 1] = '\0';
        cp = strchr(saveName, '\0');
        string_append_int(cp, ifnet.if_unit);
#endif
        if (1 || strcmp(saveName, "lo0") != 0) {        /* XXX */
            /*
             *  Try to find an address for this interface
             */

            auto_nlist(IFADDR_SYMBOL, (char *) &ia, sizeof(ia));
#ifdef netbsd1
            ia = (struct in_ifaddr *) ifnet.if_addrlist.tqh_first;
#endif
            while (ia) {
                klookup((unsigned long) ia, (char *) &in_ifaddr,
                        sizeof(in_ifaddr));
                {
#ifdef netbsd1
#define CP(x)	((char *)(x))
                    char           *cp;
                    struct sockaddr *sa;
                    cp = (CP(in_ifaddr.ia_ifa.ifa_addr) - CP(ia)) +
                        CP(&in_ifaddr);
                    sa = (struct sockaddr *) cp;

                    if (sa->sa_family == AF_INET)
#endif
                        if (in_ifaddr.ia_ifp == ifnetaddr) {
                            has_ipaddr = 1;     /* this IF has IP-address */
                            break;
                        }
                }
#ifdef netbsd1
                ia = (struct in_ifaddr *) in_ifaddr.ia_ifa.ifa_list.
                    tqe_next;
#else
                ia = in_ifaddr.ia_next;
#endif
            }

#if !defined(netbsd1) && !defined(freebsd2) && !defined(openbsd2) && !defined(STRUCT_IFNET_HAS_IF_ADDRLIST)
            ifnet.if_addrlist = (struct ifaddr *) ia;   /* WRONG DATA TYPE; ONLY A FLAG */
#endif
            /*
             * ifnet.if_addrlist = (struct ifaddr *)&ia->ia_ifa;   
             *
             * WRONG DATA TYPE; ONLY A FLAG 
             */

            if (Index)
                *Index = ++saveIndex;
            if (Retifnet)
                *Retifnet = ifnet;
            if (Retin_ifaddr && has_ipaddr)     /* assign the in_ifaddr only
                                                 * if the IF has IP-address */
                *Retin_ifaddr = in_ifaddr;
            if (Name)
                strcpy(Name, saveName);
            saveifnet = ifnet;
            saveifnetaddr = ifnetaddr;
            savein_ifaddr = in_ifaddr;
            ifnetaddr = ifnet.if_next;

            return (1);         /* DONE */
        }
        ifnetaddr = ifnet.if_next;
    }
    return (0);                 /* EOF */
}

#endif                          /* hpux11 */

#endif                          /* sunV3 || linux */

#if defined(hpux11)

static int
Interface_Scan_By_Index(int Index, char *Name, nmapi_phystat * Retifnet)
{
    short           i;

    Interface_Scan_Init();
    while (Interface_Scan_Next(&i, Name, Retifnet)) {
        if (i == Index)
            break;
    }
    if (i != Index)
        return (-1);            /* Error, doesn't exist */
    return (0);                 /* DONE */
}

#else                           /* hpux11 */

static int
Interface_Scan_By_Index(int Index,
                        char *Name,
                        struct ifnet *Retifnet,
                        struct in_ifaddr *Retin_ifaddr)
{
    short           i;

    Interface_Scan_Init();
    while (Interface_Scan_Next(&i, Name, Retifnet, Retin_ifaddr)) {
        if (i == Index)
            break;
    }
    if (i != Index)
        return (-1);            /* Error, doesn't exist */
    return (0);                 /* DONE */
}

#endif                          /* hpux11 */

static int      Interface_Count = 0;

#if defined(hpux11)

static int
Interface_Scan_Get_Count(void)
{
    if (!Interface_Count) {
        int             fd;
        struct nmparms  p;
        int             val;
        unsigned int    ulen;
        int             ret;

        if ((fd = open_mib("/dev/ip", O_RDONLY, 0, NM_ASYNC_OFF)) >= 0) {
            p.objid = ID_ifNumber;
            p.buffer = (void *) &val;
            ulen = sizeof(int);
            p.len = &ulen;
            if ((ret = get_mib_info(fd, &p)) == 0)
                Interface_Count = val;
            close_mib(fd);
        }
    }
    return (Interface_Count);
}

#else                           /* hpux11 */

static time_t   scan_time = 0;

int
Interface_Scan_Get_Count(void)
{
    time_t          time_now = time(NULL);

    if (!Interface_Count || (time_now > scan_time + 60)) {
        scan_time = time_now;
        Interface_Scan_Init();
        Interface_Count = 0;
        while (Interface_Scan_Next(NULL, NULL, NULL, NULL) != 0) {
            Interface_Count++;
        }
    }
    return (Interface_Count);
}


static int
Interface_Get_Ether_By_Index(int Index, u_char * EtherAddr)
{
    short           i;
#if !(defined(linux) || defined(netbsd1) || defined(bsdi2) || defined(openbsd2))
    struct arpcom   arpcom;
#else                           /* is linux or netbsd1 */
    struct arpcom {
        char            ac_enaddr[6];
    } arpcom;
#if defined(netbsd1) || defined(bsdi2) || defined(openbsd2)
    struct sockaddr_dl sadl;
    struct ifaddr   ifaddr;
    u_long          ifaddraddr;
#endif
#endif

#if defined(mips) || defined(hpux) || defined(osf4) || defined(osf3)
    memset(arpcom.ac_enaddr, 0, sizeof(arpcom.ac_enaddr));
#else
    memset(&arpcom.ac_enaddr, 0, sizeof(arpcom.ac_enaddr));
#endif
    memset(EtherAddr, 0, sizeof(arpcom.ac_enaddr));

    if (saveIndex != Index) {   /* Optimization! */

        Interface_Scan_Init();

        while (Interface_Scan_Next((short *) &i, NULL, NULL, NULL) != 0) {
            if (i == Index)
                break;
        }
        if (i != Index)
            return (-1);        /* Error, doesn't exist */
    }
#ifdef freebsd2
    if (saveifnet.if_type != IFT_ETHER) {
        return (0);             /* Not an ethernet if */
    }
#endif
    /*
     *  the arpcom structure is an extended ifnet structure which
     *  contains the ethernet address.
     */
#ifndef linux
#if !(defined(netbsd1) || defined(bsdi2) || defined(openbsd2))
    klookup((unsigned long) saveifnetaddr, (char *) &arpcom,
            sizeof arpcom);
#else                           /* netbsd1 or bsdi2 or openbsd2 */

#if defined(netbsd1) || defined(openbsd2)
#define if_addrlist if_addrlist.tqh_first
#define ifa_next    ifa_list.tqe_next
#endif

    ifaddraddr = (unsigned long) saveifnet.if_addrlist;
    while (ifaddraddr) {
        klookup(ifaddraddr, (char *) &ifaddr, sizeof ifaddr);
        klookup((unsigned long) ifaddr.ifa_addr, (char *) &sadl,
                sizeof sadl);
        if (sadl.sdl_family == AF_LINK
            && (saveifnet.if_type == IFT_ETHER
                || saveifnet.if_type == IFT_ISO88025
                || saveifnet.if_type == IFT_FDDI)) {
            memcpy(arpcom.ac_enaddr, sadl.sdl_data + sadl.sdl_nlen,
                   sizeof(arpcom.ac_enaddr));
            break;
        }
        ifaddraddr = (unsigned long) ifaddr.ifa_next;
    }
#endif                          /* netbsd1 or bsdi2 or openbsd2 */

#else                           /* linux */
    memcpy(arpcom.ac_enaddr, saveifnetaddr->if_hwaddr, 6);
#endif
    if (strncmp("lo", saveName, 2) == 0) {
        /*
         *  Loopback doesn't have a HW addr, so return 00:00:00:00:00:00
         */
        memset(EtherAddr, 0, sizeof(arpcom.ac_enaddr));

    } else {

#if defined(mips) || defined(hpux) || defined(osf4) || defined(osf3)
        memcpy(EtherAddr, (char *) arpcom.ac_enaddr,
               sizeof(arpcom.ac_enaddr));
#else
        memcpy(EtherAddr, (char *) &arpcom.ac_enaddr,
               sizeof(arpcom.ac_enaddr));
#endif


    }
    return (0);                 /* DONE */
}

#endif                          /* hpux11 */

#else                           /* solaris2 */

int
Interface_Scan_Get_Count(void)
{
    int             i, sd;

    if ((sd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
        return (0);
    if (ioctl(sd, SIOCGIFNUM, &i) == -1) {
        close(sd);
        return (0);
    } else {
        close(sd);
        return (i);
    }
}

int
Interface_Index_By_Name(char *Name, int Len)
{
    int             i, sd, lastlen = 0, interfaces = 0;
    struct ifconf   ifc;
    struct ifreq   *ifrp = NULL;
    char           *buf = NULL;

    if (Name == 0) {
        return 0;
    }
    if ((sd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        return 0;
    }

    /*
     * Cope with lots of interfaces and brokenness of ioctl SIOCGIFCONF
     * on some platforms; see W. R. Stevens, ``Unix Network Programming
     * Volume I'', p.435.  
     */

    for (i = 8;; i += 8) {
        buf = calloc(i, sizeof(struct ifreq));
        if (buf == NULL) {
            close(sd);
            return 0;
        }
        ifc.ifc_len = i * sizeof(struct ifreq);
        ifc.ifc_buf = (caddr_t) buf;

        if (ioctl(sd, SIOCGIFCONF, (char *) &ifc) < 0) {
            if (errno != EINVAL || lastlen != 0) {
                /*
                 * Something has gone genuinely wrong.  
                 */
                free(buf);
                close(sd);
                return 0;
            }
            /*
             * Otherwise, it could just be that the buffer is too small.  
             */
        } else {
            if (ifc.ifc_len == lastlen) {
                /*
                 * The length is the same as the last time; we're done.  
                 */
                break;
            }
            lastlen = ifc.ifc_len;
        }
        free(buf);
    }

    ifrp = ifc.ifc_req;
    interfaces = (ifc.ifc_len / sizeof(struct ifreq)) + 1;

    for (i = 1; i < interfaces; i++, ifrp++) {
        if (strncmp(ifrp->ifr_name, Name, Len) == 0) {
            free(buf);
            close(sd);
            return i;
        }
    }

    free(buf);
    close(sd);
    return 0;
}

#endif                          /* solaris2 */

#else                           /* HAVE_NET_IF_MIB_H */

/*
 * This code attempts to do the right thing for FreeBSD.  Note that
 * the statistics could be gathered through use of of the
 * net.route.0.link.iflist.0 sysctl (which we already use to get the
 * hardware address of the interfaces), rather than using the ifmib
 * code, but eventually I will implement dot3Stats and we will have to
 * use the ifmib interface.  ifmib is also a much more natural way of
 * mapping the SNMP MIB onto sysctl(3).
 */

#include <net/if.h>
#include <net/if_dl.h>
#include <net/if_mib.h>
#include <net/route.h>

static int      header_interfaces(struct variable *, oid *, size_t *, int,
                                  size_t *, WriteMethod ** write);
static int      header_ifEntry(struct variable *, oid *, size_t *, int,
                               size_t *, WriteMethod ** write);
u_char         *var_ifEntry(struct variable *, oid *, size_t *, int,
                            size_t *, WriteMethod ** write);

static char    *physaddrbuf;
static int      nphysaddrs;
struct sockaddr_dl **physaddrs;

void
init_interfaces_setup(void)
{
    int             naddrs, ilen, bit;
    static int      mib[6]
    = { CTL_NET, PF_ROUTE, 0, AF_LINK, NET_RT_IFLIST, 0 };
    char           *cp;
    size_t          len;
    struct rt_msghdr *rtm;
    struct if_msghdr *ifm;
    struct ifa_msghdr *ifam;
    struct sockaddr *sa;

    naddrs = 0;
    if (physaddrs)
        free(physaddrs);
    if (physaddrbuf)
        free(physaddrbuf);
    physaddrbuf = 0;
    physaddrs = 0;
    nphysaddrs = 0;
    len = 0;
    if (sysctl(mib, 6, 0, &len, 0, 0) < 0)
        return;

    cp = physaddrbuf = malloc(len);
    if (physaddrbuf == 0)
        return;
    if (sysctl(mib, 6, physaddrbuf, &len, 0, 0) < 0) {
        free(physaddrbuf);
        physaddrbuf = 0;
        return;
    }

  loop:
    ilen = len;
    cp = physaddrbuf;
    while (ilen > 0) {
        rtm = (struct rt_msghdr *) cp;
        if (rtm->rtm_version != RTM_VERSION || rtm->rtm_type != RTM_IFINFO) {
            free(physaddrs);
            physaddrs = 0;
            free(physaddrbuf);
            physaddrbuf = 0;
        }
        ifm = (struct if_msghdr *) rtm;
#if defined(freebsd3) || defined(freebsd4) || defined(freebsd5)
        if (physaddrs != 0)
            physaddrs[naddrs] = (void *) (ifm + 1);
        naddrs++;
#endif
        ilen -= ifm->ifm_msglen;
        cp += ifm->ifm_msglen;
        rtm = (struct rt_msghdr *) cp;
        while (ilen > 0 && rtm->rtm_type == RTM_NEWADDR) {
#if defined(freebsd3) || defined(freebsd4) || defined(freebsd5)
            ilen -= rtm->rtm_msglen;
            cp += rtm->rtm_msglen;
#else
            int             is_alias = 0;
            ifam = (struct ifa_msghdr *) rtm;
            ilen -= sizeof(*ifam);
            cp += sizeof(*ifam);
            sa = (struct sockaddr *) cp;
#define ROUND(x) (((x) + sizeof(long) - 1) & ~sizeof(long))
            for (bit = 1; bit && ilen > 0; bit <<= 1) {
                if (!(ifam->ifam_addrs & bit))
                    continue;
                ilen -= ROUND(sa->sa_len);
                cp += ROUND(sa->sa_len);

                if (bit == RTA_IFA) {
                    if (physaddrs)
#define satosdl(sa) ((struct sockaddr_dl *)(sa))
                        physaddrs[naddrs++]
                            = satosdl(sa);
                    else
                        naddrs++;
                }
                sa = (struct sockaddr *) cp;
            }
#endif
            rtm = (struct rt_msghdr *) cp;
        }
    }
    if (physaddrs) {
        nphysaddrs = naddrs;
        return;
    }
    physaddrs = malloc(naddrs * sizeof(*physaddrs));
    if (physaddrs == 0)
        return;
    naddrs = 0;
    goto loop;

}

static int
get_phys_address(int iindex, char **ap, int *len)
{
    int             i;
    int             once = 1;

    do {
        for (i = 0; i < nphysaddrs; i++) {
            if (physaddrs[i]->sdl_index == iindex)
                break;
        }
        if (i < nphysaddrs)
            break;
        init_interfaces_setup();
    } while (once--);

    if (i < nphysaddrs) {
        *ap = LLADDR(physaddrs[i]);
        *len = physaddrs[i]->sdl_alen;
        return 0;
    }
    return -1;
}

int
Interface_Scan_Get_Count(void)
{
    static int      count_oid[5] = { CTL_NET, PF_LINK, NETLINK_GENERIC,
        IFMIB_SYSTEM, IFMIB_IFCOUNT
    };
    size_t          len;
    int             count;

    len = sizeof count;
    if (sysctl(count_oid, 5, &count, &len, (void *) 0, (size_t) 0) < 0)
        return -1;
    return count;
}


u_char         *
var_ifEntry(struct variable * vp,
            oid * name,
            size_t * length,
            int exact, size_t * var_len, WriteMethod ** write_method)
{
    int             interface;
    static int      sname[6] = { CTL_NET, PF_LINK, NETLINK_GENERIC,
        IFMIB_IFDATA, 0, IFDATA_GENERAL
    };
    static struct ifmibdata ifmd;
    size_t          len;
    char           *cp;

    interface = header_ifEntry(vp, name, length, exact, var_len,
                               write_method);
    if (interface == MATCH_FAILED)
        return NULL;

    sname[4] = interface;
    len = sizeof ifmd;
    if (sysctl(sname, 6, &ifmd, &len, 0, 0) < 0)
        return NULL;

    switch (vp->magic) {
    case IFINDEX:
        long_return = interface;
        return (u_char *) & long_return;
    case IFDESCR:
        cp = ifmd.ifmd_name;
        *var_len = strlen(cp);
        return (u_char *) cp;
    case IFTYPE:
        long_return = ifmd.ifmd_data.ifi_type;
        return (u_char *) & long_return;
    case IFMTU:
        long_return = (long) ifmd.ifmd_data.ifi_mtu;
        return (u_char *) & long_return;
    case IFSPEED:
        long_return = ifmd.ifmd_data.ifi_baudrate;
        return (u_char *) & long_return;
    case IFPHYSADDRESS:
        {
            char           *cp;
            if (get_phys_address(interface, &cp, var_len))
                return NULL;
            else
                return cp;
        }
    case IFADMINSTATUS:
        long_return = ifmd.ifmd_flags & IFF_UP ? 1 : 2;
        return (u_char *) & long_return;
    case IFOPERSTATUS:
        long_return = ifmd.ifmd_flags & IFF_RUNNING ? 1 : 2;
        return (u_char *) & long_return;
    case IFLASTCHANGE:
        if (ifmd.ifmd_data.ifi_lastchange.tv_sec == 0 &&
            ifmd.ifmd_data.ifi_lastchange.tv_usec == 0) {
            long_return = 0;
        } else if (ifmd.ifmd_data.ifi_lastchange.tv_sec < starttime.tv_sec) {
            long_return = 0;
        } else {
            long_return = (u_long)
                ((ifmd.ifmd_data.ifi_lastchange.tv_sec -
                  starttime.tv_sec) * 100 +
                 ((ifmd.ifmd_data.ifi_lastchange.tv_usec -
                   starttime.tv_usec) / 10000));
        }
        return (u_char *) & long_return;
    case IFINOCTETS:
        long_return = (u_long) ifmd.ifmd_data.ifi_ibytes;
        return (u_char *) & long_return;
    case IFINUCASTPKTS:
        long_return = (u_long) ifmd.ifmd_data.ifi_ipackets;
        long_return -= (u_long) ifmd.ifmd_data.ifi_imcasts;
        return (u_char *) & long_return;
    case IFINNUCASTPKTS:
        long_return = (u_long) ifmd.ifmd_data.ifi_imcasts;
        return (u_char *) & long_return;
    case IFINDISCARDS:
        long_return = (u_long) ifmd.ifmd_data.ifi_iqdrops;
        return (u_char *) & long_return;
    case IFINERRORS:
        long_return = ifmd.ifmd_data.ifi_ierrors;
        return (u_char *) & long_return;
    case IFINUNKNOWNPROTOS:
        long_return = (u_long) ifmd.ifmd_data.ifi_noproto;
        return (u_char *) & long_return;
    case IFOUTOCTETS:
        long_return = (u_long) ifmd.ifmd_data.ifi_obytes;
        return (u_char *) & long_return;
    case IFOUTUCASTPKTS:
        long_return = (u_long) ifmd.ifmd_data.ifi_opackets;
        long_return -= (u_long) ifmd.ifmd_data.ifi_omcasts;
        return (u_char *) & long_return;
    case IFOUTNUCASTPKTS:
        long_return = (u_long) ifmd.ifmd_data.ifi_omcasts;
        return (u_char *) & long_return;
    case IFOUTDISCARDS:
        long_return = ifmd.ifmd_snd_drops;
        return (u_char *) & long_return;
    case IFOUTERRORS:
        long_return = ifmd.ifmd_data.ifi_oerrors;
        return (u_char *) & long_return;
    case IFOUTQLEN:
        long_return = ifmd.ifmd_snd_len;
        return (u_char *) & long_return;
    case IFSPECIFIC:
        *var_len = nullOidLen;
        return (u_char *) nullOid;
    default:
        DEBUGMSGTL(("snmpd", "unknown sub-id %d in var_ifEntry\n",
                    vp->magic));
    }
    return NULL;
}

#endif                          /* HAVE_NET_IF_MIB_H */
#endif                          /* !USE_SYSCTL_IFLIST */

#else                           /* WIN32 */
#include <iphlpapi.h>

WriteMethod     writeIfEntry;
long            admin_status = 0;
long            oldadmin_status = 0;

static int
header_ifEntry(struct variable *vp,
               oid * name,
               size_t * length,
               int exact, size_t * var_len, WriteMethod ** write_method)
{
#define IFENTRY_NAME_LENGTH	10
    oid             newname[MAX_OID_LEN];
    register int    ifIndex;
    int             result, count;
    DWORD           status = NO_ERROR;
    DWORD           statusRetry = NO_ERROR;
    DWORD           dwActualSize = 0;
    PMIB_IFTABLE    pIfTable = NULL;

    DEBUGMSGTL(("mibII/interfaces", "var_ifEntry: "));
    DEBUGMSGOID(("mibII/interfaces", name, *length));
    DEBUGMSG(("mibII/interfaces", " %d\n", exact));

    memcpy((char *) newname, (char *) vp->name,
           (int) vp->namelen * sizeof(oid));
    /*
     * find "next" ifIndex 
     */


    /*
     * query for buffer size needed 
     */
    status = GetIfTable(pIfTable, &dwActualSize, TRUE);

    if (status == ERROR_INSUFFICIENT_BUFFER) {
        /*
         * need more space 
         */
        pIfTable = (PMIB_IFTABLE) malloc(dwActualSize);
        if (pIfTable != NULL) {
            /*
             * Get the sorted IF table 
             */
            GetIfTable(pIfTable, &dwActualSize, TRUE);
        }
    }
    count = pIfTable->dwNumEntries;
    for (ifIndex = 0; ifIndex < count; ifIndex++) {
        newname[IFENTRY_NAME_LENGTH] =
            (oid) pIfTable->table[ifIndex].dwIndex;
        result =
            snmp_oid_compare(name, *length, newname,
                             (int) vp->namelen + 1);
        if ((exact && (result == 0)) || (!exact && (result < 0)))
            break;
    }
    if (ifIndex > count) {
        DEBUGMSGTL(("mibII/interfaces", "... index out of range\n"));
        return MATCH_FAILED;
    }


    memcpy((char *) name, (char *) newname,
           ((int) vp->namelen + 1) * sizeof(oid));
    *length = vp->namelen + 1;
    *write_method = 0;
    *var_len = sizeof(long);    /* default to 'long' results */

    DEBUGMSGTL(("mibII/interfaces", "... get I/F stats "));
    DEBUGMSGOID(("mibII/interfaces", name, *length));
    DEBUGMSG(("mibII/interfaces", "\n"));

    count = pIfTable->table[ifIndex].dwIndex;
    free(pIfTable);
    return count;
}



u_char         *
var_interfaces(struct variable * vp,
               oid * name,
               size_t * length,
               int exact, size_t * var_len, WriteMethod ** write_method)
{
    if (header_generic(vp, name, length, exact, var_len, write_method) ==
        MATCH_FAILED)
        return NULL;

    switch (vp->magic) {
    case IFNUMBER:
        GetNumberOfInterfaces(&long_return);
        return (u_char *) & long_return;
    default:
        DEBUGMSGTL(("snmpd", "unknown sub-id %d in var_interfaces\n",
                    vp->magic));
    }
    return NULL;
}

u_char         *
var_ifEntry(struct variable * vp,
            oid * name,
            size_t * length,
            int exact, size_t * var_len, WriteMethod ** write_method)
{
    int             ifIndex;
    static char     Name[16];
    conf_if_list   *if_ptr = conf_list;
    static MIB_IFROW ifRow;

    ifIndex =
        header_ifEntry(vp, name, length, exact, var_len, write_method);
    if (ifIndex == MATCH_FAILED)
        return NULL;

    /*
     * Get the If Table Row by passing index as argument 
     */
    ifRow.dwIndex = ifIndex;
    if (GetIfEntry(&ifRow) != NO_ERROR)
        return NULL;
    switch (vp->magic) {
    case IFINDEX:
        long_return = ifIndex;
        return (u_char *) & long_return;
    case IFDESCR:
        *var_len = ifRow.dwDescrLen;
        return (u_char *) ifRow.bDescr;
    case IFTYPE:
        long_return = ifRow.dwType;
        return (u_char *) & long_return;
    case IFMTU:
        long_return = (long) ifRow.dwMtu;
        return (u_char *) & long_return;
    case IFSPEED:
        long_return = (long) ifRow.dwSpeed;
        return (u_char *) & long_return;
    case IFPHYSADDRESS:
        *var_len = ifRow.dwPhysAddrLen;
        memcpy(return_buf, ifRow.bPhysAddr, *var_len);
        return (u_char *) return_buf;
    case IFADMINSTATUS:
        long_return = ifRow.dwAdminStatus;
        admin_status = long_return;
        *write_method = writeIfEntry;
        return (u_char *) & long_return;
    case IFOPERSTATUS:
        long_return = ifRow.dwOperStatus;
        return (u_char *) & long_return;
    case IFLASTCHANGE:
        long_return = ifRow.dwLastChange;
        return (u_char *) & long_return;
    case IFINOCTETS:
        long_return = ifRow.dwInOctets;
        return (u_char *) & long_return;
    case IFINUCASTPKTS:
        long_return = ifRow.dwInUcastPkts;
        return (u_char *) & long_return;
    case IFINNUCASTPKTS:
        long_return = ifRow.dwInNUcastPkts;
        return (u_char *) & long_return;
    case IFINDISCARDS:
        long_return = ifRow.dwInDiscards;
        return (u_char *) & long_return;
    case IFINERRORS:
        long_return = ifRow.dwInErrors;
        return (u_char *) & long_return;
    case IFINUNKNOWNPROTOS:
        long_return = ifRow.dwInUnknownProtos;
        return (u_char *) & long_return;
    case IFOUTOCTETS:
        long_return = ifRow.dwOutOctets;
        return (u_char *) & long_return;
    case IFOUTUCASTPKTS:
        long_return = ifRow.dwOutUcastPkts;
        return (u_char *) & long_return;
    case IFOUTNUCASTPKTS:
        long_return = ifRow.dwOutNUcastPkts;
        return (u_char *) & long_return;
    case IFOUTDISCARDS:
        long_return = ifRow.dwOutDiscards;
        return (u_char *) & long_return;
    case IFOUTERRORS:
        long_return = ifRow.dwOutErrors;
        return (u_char *) & long_return;
    case IFOUTQLEN:
        long_return = ifRow.dwOutQLen;
        return (u_char *) & long_return;
    case IFSPECIFIC:
        *var_len = nullOidLen;
        return (u_char *) nullOid;
    default:
        DEBUGMSGTL(("snmpd", "unknown sub-id %d in var_ifEntry\n",
                    vp->magic));
    }
    return NULL;
}


int
writeIfEntry(int action,
             u_char * var_val,
             u_char var_val_type,
             size_t var_val_len,
             u_char * statP, oid * name, size_t name_len)
{
    MIB_IFROW       ifEntryRow;
    if ((char) name[9] != IFADMINSTATUS) {
        return SNMP_ERR_NOTWRITABLE;
    }

    switch (action) {
    case RESERVE1:             /* Check values for acceptability */
        if (var_val_type != ASN_INTEGER) {
            snmp_log(LOG_ERR, "not integer\n");
            return SNMP_ERR_WRONGTYPE;
        }
        if (var_val_len > sizeof(int)) {
            snmp_log(LOG_ERR, "bad length\n");
            return SNMP_ERR_WRONGLENGTH;
        }

        /*
         * The dwAdminStatus member can be MIB_IF_ADMIN_STATUS_UP or MIB_IF_ADMIN_STATUS_DOWN 
         */
        if (!(((int) (*var_val) == MIB_IF_ADMIN_STATUS_UP) ||
              ((int) (*var_val) == MIB_IF_ADMIN_STATUS_DOWN))) {
            snmp_log(LOG_ERR, "not supported admin state\n");
            return SNMP_ERR_WRONGVALUE;
        }
        break;

    case RESERVE2:             /* Allocate memory and similar resources */
        break;

    case ACTION:
        /*
         * Save the old value, in case of UNDO 
         */

        oldadmin_status = admin_status;
        admin_status = (int) *var_val;
        break;

    case UNDO:                 /* Reverse the SET action and free resources */
        admin_status = oldadmin_status;
        break;

    case COMMIT:               /* Confirm the SET, performing any irreversible actions,
                                 * and free resources */
        ifEntryRow.dwIndex = (int) name[10];
        ifEntryRow.dwAdminStatus = admin_status;
        /*
         * Only UP and DOWN status are supported. Thats why done in COMMIT 
         */
        if (SetIfEntry(&ifEntryRow) != NO_ERROR) {
            snmp_log(LOG_ERR,
                     "Error in writeIfEntry case COMMIT with index: %d & adminStatus %d\n",
                     ifEntryRow.dwIndex, ifEntryRow.dwAdminStatus);
            return SNMP_ERR_COMMITFAILED;
        }

    case FREE:                 /* Free any resources allocated */
        /*
         * No resources have been allocated 
         */
        break;
    }
    return SNMP_ERR_NOERROR;
}                               /* end of writeIfEntry */
#endif                          /* WIN32 */
