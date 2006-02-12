/* Code to manipulate interface information, shared between ifconfig and
   netstat. 

   10/1998 partly rewriten by Andi Kleen to support an interface list.   
   I don't claim that the list operations are efficient @).  

   8/2000  Andi Kleen make the list operations a bit more efficient.
   People are crazy enough to use thousands of aliases now.

   $Id: interface.c,v 1.14 2001/02/10 19:31:15 pb Exp $
 */

#include "config.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>

#if HAVE_AFIPX
#if (__GLIBC__ > 2) || (__GLIBC__ == 2 && __GLIBC_MINOR__ >= 1)
#include <netipx/ipx.h>
#else
#include "ipx.h"
#endif
#endif

#if HAVE_AFECONET
#include <neteconet/ec.h>
#endif

#ifdef HAVE_HWSLIP
#include <linux/if_slip.h>
#include <net/if_arp.h>
#endif

#include "net-support.h"
#include "pathnames.h"
#include "version.h"
#include "proc.h"

#include "interface.h"
#include "sockets.h"
#include "util.h"
#include "intl.h"

#ifdef IFF_PORTSEL
const char *if_port_text[][4] =
{
  /* Keep in step with <linux/netdevice.h> */
    {"unknown", NULL, NULL, NULL},
    {"10base2", "bnc", "coax", NULL},
    {"10baseT", "utp", "tpe", NULL},
    {"AUI", "thick", "db15", NULL},
    {"100baseT", NULL, NULL, NULL},
    {"100baseTX", NULL, NULL, NULL},
    {"100baseFX", NULL, NULL, NULL},
    {NULL, NULL, NULL, NULL},
};
#endif

#define IPV6_ADDR_ANY		0x0000U

#define IPV6_ADDR_UNICAST      	0x0001U
#define IPV6_ADDR_MULTICAST    	0x0002U
#define IPV6_ADDR_ANYCAST	0x0004U

#define IPV6_ADDR_LOOPBACK	0x0010U
#define IPV6_ADDR_LINKLOCAL	0x0020U
#define IPV6_ADDR_SITELOCAL	0x0040U

#define IPV6_ADDR_COMPATv4	0x0080U

#define IPV6_ADDR_SCOPE_MASK	0x00f0U

#define IPV6_ADDR_MAPPED	0x1000U
#define IPV6_ADDR_RESERVED	0x2000U		/* reserved address space */

int procnetdev_vsn = 1;

int ife_short;

static struct interface *int_list, *int_last;

static int if_readlist_proc(char *);

static struct interface *add_interface(char *name)
{
    struct interface *ife, **nextp, *new;

    for (ife = int_last; ife; ife = ife->prev) {
	    int n = nstrcmp(ife->name, name); 
	    if (n == 0) 
		    return ife; 
	    if (n < 0) 
		    break; 
    }
    new(new); 
    safe_strncpy(new->name, name, IFNAMSIZ); 
    nextp = ife ? &ife->next : &int_list;
    new->prev = ife;
    new->next = *nextp; 
    if (new->next) 
	    new->next->prev = new; 
    else
	    int_last = new; 
    *nextp = new; 
    return new; 
}

struct interface *lookup_interface(char *name)
{
    struct interface *ife = NULL;

    if (if_readlist_proc(name) < 0) 
	    return NULL; 
    ife = add_interface(name); 
    return ife;
}

int for_all_interfaces(int (*doit) (struct interface *, void *), void *cookie)
{
    struct interface *ife;

    if (!int_list && (if_readlist() < 0))
	return -1;
    for (ife = int_list; ife; ife = ife->next) {
	int err = doit(ife, cookie);
	if (err)
	    return err;
    }
    return 0;
}

int free_interface_list(void)
{
    struct interface *ife;
    while ((ife = int_list) != NULL) {
	int_list = ife->next;
	free(ife);
    }
    return 0;
}

static int if_readconf(void)
{
    int numreqs = 30;
    struct ifconf ifc;
    struct ifreq *ifr;
    int n, err = -1;
    int skfd;

    /* SIOCGIFCONF currently seems to only work properly on AF_INET sockets
       (as of 2.1.128) */ 
    skfd = get_socket_for_af(AF_INET);
    if (skfd < 0) {
	fprintf(stderr, _("warning: no inet socket available: %s\n"),
		strerror(errno));
	/* Try to soldier on with whatever socket we can get hold of.  */
	skfd = sockets_open(0);
	if (skfd < 0)
	    return -1;
    }

    ifc.ifc_buf = NULL;
    for (;;) {
	ifc.ifc_len = sizeof(struct ifreq) * numreqs;
	ifc.ifc_buf = xrealloc(ifc.ifc_buf, ifc.ifc_len);

	if (ioctl(skfd, SIOCGIFCONF, &ifc) < 0) {
	    perror("SIOCGIFCONF");
	    goto out;
	}
	if (ifc.ifc_len == sizeof(struct ifreq) * numreqs) {
	    /* assume it overflowed and try again */
	    numreqs += 10;
	    continue;
	}
	break;
    }

    ifr = ifc.ifc_req;
    for (n = 0; n < ifc.ifc_len; n += sizeof(struct ifreq)) {
	add_interface(ifr->ifr_name);
	ifr++;
    }
    err = 0;

out:
    free(ifc.ifc_buf);
    return err;
}

static char *get_name(char *name, char *p)
{
    while (isspace(*p))
	p++;
    while (*p) {
	if (isspace(*p))
	    break;
	if (*p == ':') {	/* could be an alias */
	    char *dot = p, *dotname = name;
	    *name++ = *p++;
	    while (isdigit(*p))
		*name++ = *p++;
	    if (*p != ':') {	/* it wasn't, backup */
		p = dot;
		name = dotname;
	    }
	    if (*p == '\0')
		return NULL;
	    p++;
	    break;
	}
	*name++ = *p++;
    }
    *name++ = '\0';
    return p;
}

static int procnetdev_version(char *buf)
{
    if (strstr(buf, "compressed"))
	return 3;
    if (strstr(buf, "bytes"))
	return 2;
    return 1;
}

static int get_dev_fields(char *bp, struct interface *ife)
{
    switch (procnetdev_vsn) {
    case 3:
	sscanf(bp,
	"%llu %llu %lu %lu %lu %lu %lu %lu %llu %llu %lu %lu %lu %lu %lu %lu",
	       &ife->stats.rx_bytes,
	       &ife->stats.rx_packets,
	       &ife->stats.rx_errors,
	       &ife->stats.rx_dropped,
	       &ife->stats.rx_fifo_errors,
	       &ife->stats.rx_frame_errors,
	       &ife->stats.rx_compressed,
	       &ife->stats.rx_multicast,

	       &ife->stats.tx_bytes,
	       &ife->stats.tx_packets,
	       &ife->stats.tx_errors,
	       &ife->stats.tx_dropped,
	       &ife->stats.tx_fifo_errors,
	       &ife->stats.collisions,
	       &ife->stats.tx_carrier_errors,
	       &ife->stats.tx_compressed);
	break;
    case 2:
	sscanf(bp, "%llu %llu %lu %lu %lu %lu %llu %llu %lu %lu %lu %lu %lu",
	       &ife->stats.rx_bytes,
	       &ife->stats.rx_packets,
	       &ife->stats.rx_errors,
	       &ife->stats.rx_dropped,
	       &ife->stats.rx_fifo_errors,
	       &ife->stats.rx_frame_errors,

	       &ife->stats.tx_bytes,
	       &ife->stats.tx_packets,
	       &ife->stats.tx_errors,
	       &ife->stats.tx_dropped,
	       &ife->stats.tx_fifo_errors,
	       &ife->stats.collisions,
	       &ife->stats.tx_carrier_errors);
	ife->stats.rx_multicast = 0;
	break;
    case 1:
	sscanf(bp, "%llu %lu %lu %lu %lu %llu %lu %lu %lu %lu %lu",
	       &ife->stats.rx_packets,
	       &ife->stats.rx_errors,
	       &ife->stats.rx_dropped,
	       &ife->stats.rx_fifo_errors,
	       &ife->stats.rx_frame_errors,

	       &ife->stats.tx_packets,
	       &ife->stats.tx_errors,
	       &ife->stats.tx_dropped,
	       &ife->stats.tx_fifo_errors,
	       &ife->stats.collisions,
	       &ife->stats.tx_carrier_errors);
	ife->stats.rx_bytes = 0;
	ife->stats.tx_bytes = 0;
	ife->stats.rx_multicast = 0;
	break;
    }
    return 0;
}

static int if_readlist_proc(char *target)
{
    static int proc_read; 
    FILE *fh;
    char buf[512];
    struct interface *ife;
    int err;

    if (proc_read) 
	    return 0; 
    if (!target) 
	    proc_read = 1;

    fh = fopen(_PATH_PROCNET_DEV, "r");
    if (!fh) {
		fprintf(stderr, _("Warning: cannot open %s (%s). Limited output.\n"),
			_PATH_PROCNET_DEV, strerror(errno)); 
		return if_readconf();
	}	
    fgets(buf, sizeof buf, fh);	/* eat line */
    fgets(buf, sizeof buf, fh);

#if 0				/* pretty, but can't cope with missing fields */
    fmt = proc_gen_fmt(_PATH_PROCNET_DEV, 1, fh,
		       "face", "",	/* parsed separately */
		       "bytes", "%lu",
		       "packets", "%lu",
		       "errs", "%lu",
		       "drop", "%lu",
		       "fifo", "%lu",
		       "frame", "%lu",
		       "compressed", "%lu",
		       "multicast", "%lu",
		       "bytes", "%lu",
		       "packets", "%lu",
		       "errs", "%lu",
		       "drop", "%lu",
		       "fifo", "%lu",
		       "colls", "%lu",
		       "carrier", "%lu",
		       "compressed", "%lu",
		       NULL);
    if (!fmt)
	return -1;
#else
    procnetdev_vsn = procnetdev_version(buf);
#endif

    err = 0;
    while (fgets(buf, sizeof buf, fh)) {
	char *s, name[IFNAMSIZ];
	s = get_name(name, buf);    
	ife = add_interface(name);
	get_dev_fields(s, ife);
	ife->statistics_valid = 1;
	if (target && !strcmp(target,name))
		break;
    }
    if (ferror(fh)) {
	perror(_PATH_PROCNET_DEV);
	err = -1;
	proc_read = 0; 
    }

#if 0
    free(fmt);
#endif
    fclose(fh);
    return err;
}

int if_readlist(void) 
{ 
    int err = if_readlist_proc(NULL); 
    if (!err)
	    err = if_readconf();
    return err;
} 

/* Support for fetching an IPX address */

#if HAVE_AFIPX
static int ipx_getaddr(int sock, int ft, struct ifreq *ifr)
{
    ((struct sockaddr_ipx *) &ifr->ifr_addr)->sipx_type = ft;
    return ioctl(sock, SIOCGIFADDR, ifr);
}
#endif

/* Fetch the interface configuration from the kernel. */
int if_fetch(struct interface *ife)
{
    struct ifreq ifr;
    int fd;
    char *ifname = ife->name; 

    strcpy(ifr.ifr_name, ifname);
    if (ioctl(skfd, SIOCGIFFLAGS, &ifr) < 0)
	return (-1);
    ife->flags = ifr.ifr_flags;

    strcpy(ifr.ifr_name, ifname);
    if (ioctl(skfd, SIOCGIFHWADDR, &ifr) < 0)
	memset(ife->hwaddr, 0, 32);
    else
	memcpy(ife->hwaddr, ifr.ifr_hwaddr.sa_data, 8);

    ife->type = ifr.ifr_hwaddr.sa_family;

    strcpy(ifr.ifr_name, ifname);
    if (ioctl(skfd, SIOCGIFMETRIC, &ifr) < 0)
	ife->metric = 0;
    else
	ife->metric = ifr.ifr_metric;

    strcpy(ifr.ifr_name, ifname);
    if (ioctl(skfd, SIOCGIFMTU, &ifr) < 0)
	ife->mtu = 0;
    else
	ife->mtu = ifr.ifr_mtu;

#ifdef HAVE_HWSLIP
    if (ife->type == ARPHRD_SLIP || ife->type == ARPHRD_CSLIP ||
	ife->type == ARPHRD_SLIP6 || ife->type == ARPHRD_CSLIP6 ||
	ife->type == ARPHRD_ADAPT) {
#ifdef SIOCGOUTFILL
	strcpy(ifr.ifr_name, ifname);
	if (ioctl(skfd, SIOCGOUTFILL, &ifr) < 0)
	    ife->outfill = 0;
	else
	    ife->outfill = (unsigned int) ifr.ifr_data;
#endif
#ifdef SIOCGKEEPALIVE
	strcpy(ifr.ifr_name, ifname);
	if (ioctl(skfd, SIOCGKEEPALIVE, &ifr) < 0)
	    ife->keepalive = 0;
	else
	    ife->keepalive = (unsigned int) ifr.ifr_data;
#endif
    }
#endif

    strcpy(ifr.ifr_name, ifname);
    if (ioctl(skfd, SIOCGIFMAP, &ifr) < 0)
	memset(&ife->map, 0, sizeof(struct ifmap));
    else
	memcpy(&ife->map, &ifr.ifr_map, sizeof(struct ifmap));

    strcpy(ifr.ifr_name, ifname);
    if (ioctl(skfd, SIOCGIFMAP, &ifr) < 0)
	memset(&ife->map, 0, sizeof(struct ifmap));
    else
	ife->map = ifr.ifr_map;

#ifdef HAVE_TXQUEUELEN
    strcpy(ifr.ifr_name, ifname);
    if (ioctl(skfd, SIOCGIFTXQLEN, &ifr) < 0)
	ife->tx_queue_len = -1;	/* unknown value */
    else
	ife->tx_queue_len = ifr.ifr_qlen;
#else
    ife->tx_queue_len = -1;	/* unknown value */
#endif

#if HAVE_AFINET
    /* IPv4 address? */
    fd = get_socket_for_af(AF_INET);
    if (fd >= 0) {
	strcpy(ifr.ifr_name, ifname);
	ifr.ifr_addr.sa_family = AF_INET;
	if (ioctl(fd, SIOCGIFADDR, &ifr) == 0) {
	    ife->has_ip = 1;
	    ife->addr = ifr.ifr_addr;
	    strcpy(ifr.ifr_name, ifname);
	    if (ioctl(fd, SIOCGIFDSTADDR, &ifr) < 0)
	        memset(&ife->dstaddr, 0, sizeof(struct sockaddr));
	    else
	        ife->dstaddr = ifr.ifr_dstaddr;

	    strcpy(ifr.ifr_name, ifname);
	    if (ioctl(fd, SIOCGIFBRDADDR, &ifr) < 0)
	        memset(&ife->broadaddr, 0, sizeof(struct sockaddr));
	    else
		ife->broadaddr = ifr.ifr_broadaddr;

	    strcpy(ifr.ifr_name, ifname);
	    if (ioctl(fd, SIOCGIFNETMASK, &ifr) < 0)
		memset(&ife->netmask, 0, sizeof(struct sockaddr));
	    else
		ife->netmask = ifr.ifr_netmask;
	} else
	    memset(&ife->addr, 0, sizeof(struct sockaddr));
    }
#endif

#if HAVE_AFATALK
    /* DDP address maybe ? */
    fd = get_socket_for_af(AF_APPLETALK);
    if (fd >= 0) {
	strcpy(ifr.ifr_name, ifname);
	if (ioctl(fd, SIOCGIFADDR, &ifr) == 0) {
	    ife->ddpaddr = ifr.ifr_addr;
	    ife->has_ddp = 1;
	}
    }
#endif

#if HAVE_AFIPX
    /* Look for IPX addresses with all framing types */
    fd = get_socket_for_af(AF_IPX);
    if (fd >= 0) {
	strcpy(ifr.ifr_name, ifname);
	if (!ipx_getaddr(fd, IPX_FRAME_ETHERII, &ifr)) {
	    ife->has_ipx_bb = 1;
	    ife->ipxaddr_bb = ifr.ifr_addr;
	}
	strcpy(ifr.ifr_name, ifname);
	if (!ipx_getaddr(fd, IPX_FRAME_SNAP, &ifr)) {
	    ife->has_ipx_sn = 1;
	    ife->ipxaddr_sn = ifr.ifr_addr;
	}
	strcpy(ifr.ifr_name, ifname);
	if (!ipx_getaddr(fd, IPX_FRAME_8023, &ifr)) {
	    ife->has_ipx_e3 = 1;
	    ife->ipxaddr_e3 = ifr.ifr_addr;
	}
	strcpy(ifr.ifr_name, ifname);
	if (!ipx_getaddr(fd, IPX_FRAME_8022, &ifr)) {
	    ife->has_ipx_e2 = 1;
	    ife->ipxaddr_e2 = ifr.ifr_addr;
	}
    }
#endif

#if HAVE_AFECONET
    /* Econet address maybe? */
    fd = get_socket_for_af(AF_ECONET);
    if (fd >= 0) {
	strcpy(ifr.ifr_name, ifname);
	if (ioctl(fd, SIOCGIFADDR, &ifr) == 0) {
	    ife->ecaddr = ifr.ifr_addr;
	    ife->has_econet = 1;
	}
    }
#endif

    return 0;
}

int do_if_fetch(struct interface *ife)
{ 
    if (if_fetch(ife) < 0) {
	char *errmsg; 
	if (errno == ENODEV) { 
	    /* Give better error message for this case. */ 
	    errmsg = _("Device not found"); 
	} else { 
	    errmsg = strerror(errno); 
	}
  	fprintf(stderr, _("%s: error fetching interface information: %s\n"),
		ife->name, errmsg);
	return -1;
    }
    return 0; 
}

int do_if_print(struct interface *ife, void *cookie)
{
    int *opt_a = (int *) cookie;
    int res; 

    res = do_if_fetch(ife); 
    if (res >= 0) {   
	if ((ife->flags & IFF_UP) || *opt_a)
	    ife_print(ife);
    }
    return res;
}

void ife_print_short(struct interface *ptr)
{
    printf("%-5.5s ", ptr->name);
    printf("%5d %3d", ptr->mtu, ptr->metric);
    /* If needed, display the interface statistics. */
    if (ptr->statistics_valid) {
	printf("%8llu %6lu %6lu %6lu",
	       ptr->stats.rx_packets, ptr->stats.rx_errors,
	       ptr->stats.rx_dropped, ptr->stats.rx_fifo_errors);
	printf("%8llu %6lu %6lu %6lu ",
	       ptr->stats.tx_packets, ptr->stats.tx_errors,
	       ptr->stats.tx_dropped, ptr->stats.tx_fifo_errors);
    } else {
	printf("%-56s", _("     - no statistics available -"));
    }
    /* DONT FORGET TO ADD THE FLAGS IN ife_print_long, too */
    if (ptr->flags == 0)
	printf(_("[NO FLAGS]"));
    if (ptr->flags & IFF_ALLMULTI)
	printf("A");
    if (ptr->flags & IFF_BROADCAST)
	printf("B");
    if (ptr->flags & IFF_DEBUG)
	printf("D");
    if (ptr->flags & IFF_LOOPBACK)
	printf("L");
    if (ptr->flags & IFF_MULTICAST)
	printf("M");
#ifdef HAVE_DYNAMIC
    if (ptr->flags & IFF_DYNAMIC)
	printf("d");
#endif
    if (ptr->flags & IFF_PROMISC)
	printf("P");
    if (ptr->flags & IFF_NOTRAILERS)
	printf("N");
    if (ptr->flags & IFF_NOARP)
	printf("O");
    if (ptr->flags & IFF_POINTOPOINT)
	printf("P");
    if (ptr->flags & IFF_SLAVE)
	printf("s");
    if (ptr->flags & IFF_MASTER)
	printf("m");
    if (ptr->flags & IFF_RUNNING)
	printf("R");
    if (ptr->flags & IFF_UP)
	printf("U");
    /* DONT FORGET TO ADD THE FLAGS IN ife_print_long, too */
    printf("\n");
}

void ife_print_long(struct interface *ptr)
{
    struct aftype *ap;
    struct hwtype *hw;
    int hf;
    int can_compress = 0;
    unsigned long long rx, tx, short_rx, short_tx;
    char Rext[5]="b";
    char Text[5]="b";

#if HAVE_AFIPX
    static struct aftype *ipxtype = NULL;
#endif
#if HAVE_AFECONET
    static struct aftype *ectype = NULL;
#endif
#if HAVE_AFATALK
    static struct aftype *ddptype = NULL;
#endif
#if HAVE_AFINET6
    FILE *f;
    char addr6[40], devname[20];
    struct sockaddr_in6 sap;
    int plen, scope, dad_status, if_idx;
    extern struct aftype inet6_aftype;
    char addr6p[8][5];
#endif

    ap = get_afntype(ptr->addr.sa_family);
    if (ap == NULL)
	ap = get_afntype(0);

    hf = ptr->type;

    if (hf == ARPHRD_CSLIP || hf == ARPHRD_CSLIP6)
	can_compress = 1;

    hw = get_hwntype(hf);
    if (hw == NULL)
	hw = get_hwntype(-1);

    printf(_("%-9.9s Link encap:%s  "), ptr->name, hw->title);
    /* For some hardware types (eg Ash, ATM) we don't print the 
       hardware address if it's null.  */
    if (hw->print != NULL && (! (hw_null_address(hw, ptr->hwaddr) &&
				  hw->suppress_null_addr)))
	printf(_("HWaddr %s  "), hw->print(ptr->hwaddr));
#ifdef IFF_PORTSEL
    if (ptr->flags & IFF_PORTSEL) {
	printf(_("Media:%s"), if_port_text[ptr->map.port][0]);
	if (ptr->flags & IFF_AUTOMEDIA)
	    printf(_("(auto)"));
    }
#endif
    printf("\n");

#if HAVE_AFINET
    if (ptr->has_ip) {
	printf(_("          %s addr:%s "), ap->name,
	       ap->sprint(&ptr->addr, 1));
	if (ptr->flags & IFF_POINTOPOINT) {
	    printf(_(" P-t-P:%s "), ap->sprint(&ptr->dstaddr, 1));
	}
	if (ptr->flags & IFF_BROADCAST) {
	    printf(_(" Bcast:%s "), ap->sprint(&ptr->broadaddr, 1));
	}
	printf(_(" Mask:%s\n"), ap->sprint(&ptr->netmask, 1));
    }
#endif

#if HAVE_AFINET6
    /* FIXME: should be integrated into interface.c.   */

    if ((f = fopen(_PATH_PROCNET_IFINET6, "r")) != NULL) {
	while (fscanf(f, "%4s%4s%4s%4s%4s%4s%4s%4s %02x %02x %02x %02x %20s\n",
		      addr6p[0], addr6p[1], addr6p[2], addr6p[3],
		      addr6p[4], addr6p[5], addr6p[6], addr6p[7],
		  &if_idx, &plen, &scope, &dad_status, devname) != EOF) {
	    if (!strcmp(devname, ptr->name)) {
		sprintf(addr6, "%s:%s:%s:%s:%s:%s:%s:%s",
			addr6p[0], addr6p[1], addr6p[2], addr6p[3],
			addr6p[4], addr6p[5], addr6p[6], addr6p[7]);
		inet6_aftype.input(1, addr6, (struct sockaddr *) &sap);
		printf(_("          inet6 addr: %s/%d"),
		 inet6_aftype.sprint((struct sockaddr *) &sap, 1), plen);
		printf(_(" Scope:"));
		switch (scope) {
		case 0:
		    printf(_("Global"));
		    break;
		case IPV6_ADDR_LINKLOCAL:
		    printf(_("Link"));
		    break;
		case IPV6_ADDR_SITELOCAL:
		    printf(_("Site"));
		    break;
		case IPV6_ADDR_COMPATv4:
		    printf(_("Compat"));
		    break;
		case IPV6_ADDR_LOOPBACK:
		    printf(_("Host"));
		    break;
		default:
		    printf(_("Unknown"));
		}
		printf("\n");
	    }
	}
	fclose(f);
    }
#endif

#if HAVE_AFIPX
    if (ipxtype == NULL)
	ipxtype = get_afntype(AF_IPX);

    if (ipxtype != NULL) {
	if (ptr->has_ipx_bb)
	    printf(_("          IPX/Ethernet II addr:%s\n"),
		   ipxtype->sprint(&ptr->ipxaddr_bb, 1));
	if (ptr->has_ipx_sn)
	    printf(_("          IPX/Ethernet SNAP addr:%s\n"),
		   ipxtype->sprint(&ptr->ipxaddr_sn, 1));
	if (ptr->has_ipx_e2)
	    printf(_("          IPX/Ethernet 802.2 addr:%s\n"),
		   ipxtype->sprint(&ptr->ipxaddr_e2, 1));
	if (ptr->has_ipx_e3)
	    printf(_("          IPX/Ethernet 802.3 addr:%s\n"),
		   ipxtype->sprint(&ptr->ipxaddr_e3, 1));
    }
#endif

#if HAVE_AFATALK
    if (ddptype == NULL)
	ddptype = get_afntype(AF_APPLETALK);
    if (ddptype != NULL) {
	if (ptr->has_ddp)
	    printf(_("          EtherTalk Phase 2 addr:%s\n"), ddptype->sprint(&ptr->ddpaddr, 1));
    }
#endif

#if HAVE_AFECONET
    if (ectype == NULL)
	ectype = get_afntype(AF_ECONET);
    if (ectype != NULL) {
	if (ptr->has_econet)
	    printf(_("          econet addr:%s\n"), ectype->sprint(&ptr->ecaddr, 1));
    }
#endif

    printf("          ");
    /* DONT FORGET TO ADD THE FLAGS IN ife_print_short, too */
    if (ptr->flags == 0)
	printf(_("[NO FLAGS] "));
    if (ptr->flags & IFF_UP)
	printf(_("UP "));
    if (ptr->flags & IFF_BROADCAST)
	printf(_("BROADCAST "));
    if (ptr->flags & IFF_DEBUG)
	printf(_("DEBUG "));
    if (ptr->flags & IFF_LOOPBACK)
	printf(_("LOOPBACK "));
    if (ptr->flags & IFF_POINTOPOINT)
	printf(_("POINTOPOINT "));
    if (ptr->flags & IFF_NOTRAILERS)
	printf(_("NOTRAILERS "));
    if (ptr->flags & IFF_RUNNING)
	printf(_("RUNNING "));
    if (ptr->flags & IFF_NOARP)
	printf(_("NOARP "));
    if (ptr->flags & IFF_PROMISC)
	printf(_("PROMISC "));
    if (ptr->flags & IFF_ALLMULTI)
	printf(_("ALLMULTI "));
    if (ptr->flags & IFF_SLAVE)
	printf(_("SLAVE "));
    if (ptr->flags & IFF_MASTER)
	printf(_("MASTER "));
    if (ptr->flags & IFF_MULTICAST)
	printf(_("MULTICAST "));
#ifdef HAVE_DYNAMIC
    if (ptr->flags & IFF_DYNAMIC)
	printf(_("DYNAMIC "));
#endif
    /* DONT FORGET TO ADD THE FLAGS IN ife_print_short */
    printf(_(" MTU:%d  Metric:%d"),
	   ptr->mtu, ptr->metric ? ptr->metric : 1);
#ifdef SIOCSKEEPALIVE
    if (ptr->outfill || ptr->keepalive)
	printf(_("  Outfill:%d  Keepalive:%d"),
	       ptr->outfill, ptr->keepalive);
#endif
    printf("\n");

    /* If needed, display the interface statistics. */

    if (ptr->statistics_valid) {
	/* XXX: statistics are currently only printed for the primary address,
	 *      not for the aliases, although strictly speaking they're shared
	 *      by all addresses.
	 */
	printf("          ");

	printf(_("RX packets:%llu errors:%lu dropped:%lu overruns:%lu frame:%lu\n"),
	       ptr->stats.rx_packets, ptr->stats.rx_errors,
	       ptr->stats.rx_dropped, ptr->stats.rx_fifo_errors,
	       ptr->stats.rx_frame_errors);
	if (can_compress)
	    printf(_("             compressed:%lu\n"), ptr->stats.rx_compressed);

	rx = ptr->stats.rx_bytes;  
	tx = ptr->stats.tx_bytes;
	short_rx = rx * 10;  
	short_tx = tx * 10;
	if (rx > 1048576) { short_rx /= 1048576;  strcpy(Rext, "Mb"); }
	else if (rx > 1024) { short_rx /= 1024;  strcpy(Rext, "Kb"); }
	if (tx > 1048576) { short_tx /= 1048576;  strcpy(Text, "Mb"); }
	else if (tx > 1024) { short_tx /= 1024;  strcpy(Text, "Kb"); }

	printf("          ");
	printf(_("TX packets:%llu errors:%lu dropped:%lu overruns:%lu carrier:%lu\n"),
	       ptr->stats.tx_packets, ptr->stats.tx_errors,
	       ptr->stats.tx_dropped, ptr->stats.tx_fifo_errors,
	       ptr->stats.tx_carrier_errors);
	printf(_("          collisions:%lu "), ptr->stats.collisions);
	if (can_compress)
	    printf(_("compressed:%lu "), ptr->stats.tx_compressed);
	if (ptr->tx_queue_len != -1)
	    printf(_("txqueuelen:%d "), ptr->tx_queue_len);
	printf("\n          ");
	printf(_("RX bytes:%llu (%lu.%lu %s)  TX bytes:%llu (%lu.%lu %s)\n"),
	       rx, (unsigned long)(short_rx / 10), 
	       (unsigned long)(short_rx % 10), Rext, 
	       tx, (unsigned long)(short_tx / 10), 
	       (unsigned long)(short_tx % 10), Text);
    }

    if ((ptr->map.irq || ptr->map.mem_start || ptr->map.dma ||
	 ptr->map.base_addr)) {
	printf("          ");
	if (ptr->map.irq)
	    printf(_("Interrupt:%d "), ptr->map.irq);
	if (ptr->map.base_addr >= 0x100)	/* Only print devices using it for 
						   I/O maps */
	    printf(_("Base address:0x%x "), ptr->map.base_addr);
	if (ptr->map.mem_start) {
	    printf(_("Memory:%lx-%lx "), ptr->map.mem_start, ptr->map.mem_end);
	}
	if (ptr->map.dma)
	    printf(_("DMA chan:%x "), ptr->map.dma);
	printf("\n");
    }
    printf("\n");
}

void ife_print(struct interface *i)
{
    if (ife_short)
	ife_print_short(i);
    else
	ife_print_long(i);
}
