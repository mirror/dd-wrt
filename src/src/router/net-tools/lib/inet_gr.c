/*
   $Id: inet_gr.c,v 1.13 2000/10/08 01:00:44 ecki Exp $

   Modifications:
   1998-07-01 - Arnaldo Carvalho de Melo - GNU gettext instead of catgets
   1999-01-01 - Bernd Eckenfels          - fixed the routing cache printouts
   1999-10-07 - Kurt Garloff <garloff@suse.de> - do host (instead of network) name
						lookup for gws and hosts
 */

#include "config.h"

#if HAVE_AFINET
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <arpa/nameser.h>
/* #include <net/route.h> realy broken */
#include <ctype.h>
#include <errno.h>
#include <netdb.h>
#include <resolv.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include "version.h"
#include "net-support.h"
#include "pathnames.h"
#include "intl.h"
#include "net-features.h"
#include "proc.h"
extern struct aftype inet_aftype;

extern char *INET_sprintmask(struct sockaddr *sap, int numeric, 
			     unsigned int netmask);

int rprint_fib(int ext, int numeric)
{
    char buff[1024], iface[16], flags[64];
    char gate_addr[128], net_addr[128];
    char mask_addr[128];
    int num, iflags, metric, refcnt, use, mss, window, irtt;
    FILE *fp = fopen(_PATH_PROCNET_ROUTE, "r");
    char *fmt;

    if (!fp) {
        perror(_PATH_PROCNET_ROUTE);
        printf(_("INET (IPv4) not configured in this system.\n"));
	return 1;
    }
    printf(_("Kernel IP routing table\n"));

    if (ext == 1)
	printf(_("Destination     Gateway         Genmask         "
		 "Flags Metric Ref    Use Iface\n"));
    if (ext == 2)
	printf(_("Destination     Gateway         Genmask         "
		 "Flags   MSS Window  irtt Iface\n"));
    if (ext >= 3)
	printf(_("Destination     Gateway         Genmask         "
		 "Flags Metric Ref    Use Iface    "
		 "MSS   Window irtt\n"));

    irtt = 0;
    window = 0;
    mss = 0;

    fmt = proc_gen_fmt(_PATH_PROCNET_ROUTE, 0, fp,
		       "Iface", "%16s",
		       "Destination", "%128s",
		       "Gateway", "%128s",
		       "Flags", "%X",
		       "RefCnt", "%d",
		       "Use", "%d",
		       "Metric", "%d",
		       "Mask", "%128s",
		       "MTU", "%d",
		       "Window", "%d",
		       "IRTT", "%d",
		       NULL);
    /* "%16s %128s %128s %X %d %d %d %128s %d %d %d\n" */

    if (!fmt)
	return 1;

    while (fgets(buff, 1023, fp)) {
        struct sockaddr snet_target, snet_gateway, snet_mask;
	struct sockaddr_in *sin_netmask;

	num = sscanf(buff, fmt,
		     iface, net_addr, gate_addr,
		     &iflags, &refcnt, &use, &metric, mask_addr,
		     &mss, &window, &irtt);
	if (num < 10 || !(iflags & RTF_UP))
	    continue;

	/* Fetch and resolve the target address. */
	(void) inet_aftype.input(1, net_addr, &snet_target);

	/* Fetch and resolve the gateway address. */
	(void) inet_aftype.input(1, gate_addr, &snet_gateway);

	/* Fetch and resolve the genmask. */
	(void) inet_aftype.input(1, mask_addr, &snet_mask);
	
	sin_netmask = (struct sockaddr_in *)&snet_mask;
	strcpy(net_addr, INET_sprintmask(&snet_target, 
					 (numeric | 0x8000 | (iflags & RTF_HOST? 0x4000: 0)),
					 sin_netmask->sin_addr.s_addr));
	net_addr[15] = '\0';

	strcpy(gate_addr, inet_aftype.sprint(&snet_gateway, numeric | 0x4000));
	gate_addr[15] = '\0';

	strcpy(mask_addr, inet_aftype.sprint(&snet_mask, 1));
	mask_addr[15] = '\0';

	/* Decode the flags. */
	flags[0] = '\0';
	if (iflags & RTF_UP)
	    strcat(flags, "U");
	if (iflags & RTF_GATEWAY)
	    strcat(flags, "G");
#if HAVE_RTF_REJECT
	if (iflags & RTF_REJECT)
	    strcpy(flags, "!");
#endif
	if (iflags & RTF_HOST)
	    strcat(flags, "H");
	if (iflags & RTF_REINSTATE)
	    strcat(flags, "R");
	if (iflags & RTF_DYNAMIC)
	    strcat(flags, "D");
	if (iflags & RTF_MODIFIED)
	    strcat(flags, "M");
	if (iflags & RTF_DEFAULT)
	    strcat(flags, "d");
	if (iflags & RTF_ALLONLINK)
	    strcat(flags, "a");
	if (iflags & RTF_ADDRCONF)
	    strcat(flags, "c");
	if (iflags & RTF_NONEXTHOP)
	    strcat(flags, "o");
	if (iflags & RTF_EXPIRES)
	    strcat(flags, "e");
	if (iflags & RTF_CACHE)
	    strcat(flags, "c");
	if (iflags & RTF_FLOW)
	    strcat(flags, "f");
	if (iflags & RTF_POLICY)
	    strcat(flags, "p");
	if (iflags & RTF_LOCAL)
	    strcat(flags, "l");
	if (iflags & RTF_MTU)
	    strcat(flags, "u");
	if (iflags & RTF_WINDOW)
	    strcat(flags, "w");
	if (iflags & RTF_IRTT)
	    strcat(flags, "i");
	if (iflags & RTF_NOTCACHED) /* 2.0.36 */
	    strcat(flags, "n");

	/* Print the info. */
	if (ext == 1) {
#if HAVE_RTF_REJECT
	    if (iflags & RTF_REJECT)
		printf("%-15s -               %-15s %-5s %-6d -  %7d -\n",
		       net_addr, mask_addr, flags, metric, use);
	    else
#endif
		printf("%-15s %-15s %-15s %-5s %-6d %-2d %7d %s\n",
		       net_addr, gate_addr, mask_addr, flags,
		       metric, refcnt, use, iface);
	}
	if (ext == 2) {
#if HAVE_RTF_REJECT
	    if (iflags & RTF_REJECT)
		printf("%-15s -               %-15s %-5s     - -          - -\n",
		       net_addr, mask_addr, flags);
	    else
#endif
		printf("%-15s %-15s %-15s %-5s %5d %-5d %6d %s\n",
		       net_addr, gate_addr, mask_addr, flags,
		       mss, window, irtt, iface);
	}
	if (ext >= 3) {
#if HAVE_RTF_REJECT
	    if (iflags & RTF_REJECT)
		printf("%-15s -               %-15s %-5s %-6d -  %7d -        -     -      -\n",
		       net_addr, mask_addr, flags, metric, use);
	    else
#endif
		printf("%-15s %-15s %-15s %-5s %-6d %-3d %6d %-6.6s   %-5d %-6d %d\n",
		       net_addr, gate_addr, mask_addr, flags,
		       metric, refcnt, use, iface, mss, window, irtt);
	}
    }

    free(fmt);
    (void) fclose(fp);
    return (0);
}

int rprint_cache(int ext, int numeric)
{
    char buff[1024], iface[16], flags[64];
    char gate_addr[128], dest_addr[128], specdst[128];
    char src_addr[128];
    struct sockaddr snet;
    unsigned int iflags;
    int num, format, metric, refcnt, use, mss, window, irtt, hh, hhref, hhuptod, arp, tos;
    char *fmt = NULL;

    FILE *fp = fopen(_PATH_PROCNET_RTCACHE, "r");

    if (!fp) {
        perror(_PATH_PROCNET_RTCACHE);
        printf(_("INET (IPv4) not configured in this system.\n"));
	return 1;
    }

   /* Okay, first thing we need to know is the format of the rt_cache. 
    * I am aware of two possible layouts:
    * 2.2.0
    * "Iface\tDestination\tGateway \tFlags\t\tRefCnt\tUse\tMetric\tSource\t\tMTU\tWindow\tIRTT\tTOS\tHHRef\tHHUptod\tSpecDst"
    * "%s\t%08lX\t%08lX\t%8X\t%d\t%u\t%d\t%08lX\t%d\t%u\t%u\t%02X\t%d\t%1d\t%08X" 
    *
    * 2.0.36
    * "Iface\tDestination\tGateway \tFlags\tRefCnt\tUse\tMetric\tSource\t\tMTU\tWindow\tIRTT\tHH\tARP"
    * "%s\t%08lX\t%08lX\t%02X\t%d\t%u\t%d\t%08lX\t%d\t%lu\t%u\t%d\t%1d"
    */
    
    format = proc_guess_fmt(_PATH_PROCNET_RTCACHE, fp, "IRTT",1,"TOS",2,"HHRef",4,"HHUptod",8,"SpecDst",16,"HH",32,"ARP",64,NULL);

    printf(_("Kernel IP routing cache\n"));

    switch(format) {
    	case -1: /* I/O Error */
	  perror(_PATH_PROCNET_RTCACHE);
	  exit(-1);
    	  break;
    	case 63: /* 2.2.0 Format */
	  format = 2;
    	  break;
    	case 97: /* 2.0.36 Format */
    	  format = 1;
    	  break;
    	default:
    	  printf("ERROR: proc_guess_fmt(%s,... returned: %d\n",_PATH_PROCNET_RTCACHE, format);
	  break;
    }
    
    rewind(fp);

    if (ext == 1)
      printf(_("Source          Destination     Gateway         "
               "Flags Metric Ref    Use Iface\n"));
    if (ext == 2)
      printf(_("Source          Destination     Gateway         "
               "Flags   MSS Window  irtt Iface\n"));

    if (format == 1) {
      if (ext >= 3)
	printf(_("Source          Destination     Gateway         "
		 "Flags Metric Ref    Use Iface    "
		 "MSS   Window irtt  HH  Arp\n"));

      fmt = proc_gen_fmt(_PATH_PROCNET_RTCACHE, 0, fp,
		       "Iface", "%16s",
		       "Destination", "%128s",
		       "Gateway", "%128s",
		       "Flags", "%X",
		       "RefCnt", "%d",
		       "Use", "%d",
		       "Metric", "%d",
		       "Source", "%128s",
		       "MTU", "%d",
		       "Window", "%d",
		       "IRTT", "%d",
		       "HH", "%d",
		       "ARP", "%d",
		       NULL);
      /* "%16s %128s %128s %X %d %d %d %128s %d %d %d %d %d\n" */
    }

    if (format == 2) {
      if (ext >= 3)
	printf(_("Source          Destination     Gateway         "
		 "Flags Metric Ref    Use Iface    "
		 "MSS   Window irtt  TOS HHRef HHUptod     SpecDst\n"));
        fmt = proc_gen_fmt(_PATH_PROCNET_RTCACHE, 0, fp,
		       "Iface", "%16s",
		       "Destination", "%128s",
		       "Gateway", "%128s",
		       "Flags", "%X",
		       "RefCnt", "%d",
		       "Use", "%d",
		       "Metric", "%d",
		       "Source", "%128s",
		       "MTU", "%d",
		       "Window", "%d",
		       "IRTT", "%d",
		       "TOS", "%d",
		       "HHRef", "%d",
		       "HHUptod", "%d",
		       "SpecDst", "%128s",
		       NULL);
      /* "%16s %128s %128s %X %d %d %d %128s %d %d %d %d %d %128s\n" */
    }


    irtt = 0;
    window = 0;
    mss = 0;
    hh = 0; hhref = 0; hhuptod = 0;
    arp = 0; tos = 0;
    while (fgets(buff, 1023, fp)) {
        if (format == 1) {
  	  num = sscanf(buff, fmt,
		     iface, dest_addr, gate_addr,
		     &iflags, &refcnt, &use, &metric, src_addr,
		     &mss, &window, &irtt, &hh, &arp);
	  if (num < 12)
	    continue;
	}
        if (format == 2) {
  	  num = sscanf(buff, fmt,
		     iface, dest_addr, gate_addr,
		     &iflags, &refcnt, &use, &metric, src_addr,
		     &mss, &window, &irtt, &tos, &hhref, &hhuptod, &specdst);
	  if (num < 12)
	    continue;
	}
	

	/* Fetch and resolve the target address. */
	(void) inet_aftype.input(1, dest_addr, &snet);
	strcpy(dest_addr, inet_aftype.sprint(&snet, numeric));
	dest_addr[15] = '\0';

	/* Fetch and resolve the gateway address. */
	(void) inet_aftype.input(1, gate_addr, &snet);
	strcpy(gate_addr, inet_aftype.sprint(&snet, numeric));
	gate_addr[15] = '\0';

	/* Fetch and resolve the source. */
	(void) inet_aftype.input(1, src_addr, &snet);
	strcpy(src_addr, inet_aftype.sprint(&snet, numeric));
	src_addr[15] = '\0';

	/* Fetch and resolve the SpecDst addrerss. */
	(void) inet_aftype.input(1, specdst, &snet);
	strcpy(specdst, inet_aftype.sprint(&snet, numeric));
	specdst[15] = '\0';

	/* Decode the flags. */
	flags[0] = '\0';
if (format == 1) {
	if (iflags & RTF_UP)
	    strcat(flags, "U");
	if (iflags & RTF_HOST)
	    strcat(flags, "H");
}
	if (iflags & RTF_GATEWAY)
	    strcat(flags, "G");
#if HAVE_RTF_REJECT
	if (iflags & RTF_REJECT)
	    strcpy(flags, "!");
#endif
	if (iflags & RTF_REINSTATE)
	    strcat(flags, "R");
	if (iflags & RTF_DYNAMIC)
	    strcat(flags, "D");
	if (iflags & RTF_MODIFIED)
	    strcat(flags, "M");

/* possible collision with 2.0 flags U and H */
if (format == 2) {
	if (iflags & RTCF_DEAD)
	    strcat(flags, "-");
	if (iflags & RTCF_ONLINK)
	    strcat(flags, "o");
}
	if (iflags & RTCF_NOTIFY)
	    strcat(flags, "n");
	if (iflags & RTCF_DIRECTDST)
	    strcat(flags, "d");
	if (iflags & RTCF_TPROXY)
	    strcat(flags, "t");
	if (iflags & RTCF_FAST)
	    strcat(flags, "f");
	if (iflags & RTCF_MASQ)
	    strcat(flags, "q");
	if (iflags & RTCF_SNAT)
	    strcat(flags, "Ns");
	if (iflags & RTCF_DOREDIRECT)
	    strcat(flags, "r");
	if (iflags & RTCF_DIRECTSRC)
	    strcat(flags, "i");
	if (iflags & RTCF_DNAT)
	    strcat(flags, "Nd");
	if (iflags & RTCF_BROADCAST)
	    strcat(flags, "b");
	if (iflags & RTCF_MULTICAST)
	    strcat(flags, "m");
	if (iflags & RTCF_REJECT)
	    strcat(flags, "#");
	if (iflags & RTCF_LOCAL)
	    strcat(flags, "l");
	/* Print the info. */
	if (ext == 1) {
		printf("%-15s %-15s %-15s %-5s %-6d %-2d %7d %s\n",
		       src_addr, dest_addr, gate_addr, flags,
		       metric, refcnt, use, iface);
	}
	if (ext == 2) {
		printf("%-15s %-15s %-15s %-5s %5d %-5d %6d %s\n",
		       src_addr, dest_addr, gate_addr, flags,
		       mss, window, irtt, iface);
	}
	if (format == 1) {
	  if (ext >= 3) {
		printf("%-15s %-15s %-15s %-5s %-6d %-3d %6d %-6.6s   %-5d %-6d %-5d %-3d %d\n",
		       src_addr, dest_addr, gate_addr, flags,
		 metric, refcnt, use, iface, mss, window, irtt, hh, arp);
	  }
	}
	if (format == 2) {
	  if (ext >= 3) {
		printf("%-15s %-15s %-15s %-5s %-6d %-3d %6d %-6.6s   %-5d %-6d %-5d %-3d %-3d   %-3d %15s\n",
		       src_addr, dest_addr, gate_addr, flags,
		 metric, refcnt, use, iface, mss, window, irtt, tos, hhref, hhuptod, specdst);
	  }
	}
    }

    free(fmt);
    (void) fclose(fp);
    return (0);
}

int INET_rprint(int options)
{
    int ext = options & FLAG_EXT;
    int numeric = options & (FLAG_NUM_HOST | FLAG_SYM);
    int rc = E_INTERN;

    if (options & FLAG_FIB)
	if ((rc = rprint_fib(ext, numeric)))
	    return (rc);
    if (options & FLAG_CACHE)
	rc = rprint_cache(ext, numeric);

    return (rc);
}

#endif				/* HAVE_AFINET */
