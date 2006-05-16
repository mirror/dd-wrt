#
#   $Id: diff.if_ethersubr.c,v 1.1 2004/04/27 01:34:21 dyang Exp $
#
#   Libnet FreeBSD source etheraddr patch
#   (c) 1999 Nicholas Brawn <nick@feralmonkey.org>
#   (c) 1998, 1999  Mike D. Schiffman <mike@infonexus.com>
#                   route|daemon9 <route@infonexus.com>
#
# Specify Source Hardware Address Patch (FreeBSD 3.x)
#
# This patch enables you to specify a source ethernet address other than your
# own when creating custom ethernet packets.  
#
# To use, copy the patch into /sys/net. Create a backup of if_ethersubr.c,
# then: patch < diff.if_ethersubr.c
#
# Recompile your kernel and reboot. You will now have a new sysctl MIB, 
# net.link.ether.inet.specify_src_hwaddr. To use, you must set it to > 0:
#
# root@kungfu:~# sysctl -w net.link.ether.inet.specify_src_hwaddr=1
# net.link.ether.inet.specify_src_hwaddr: 0 -> 1
# root@kungfu:~# 
# 
# -Nicholas Brawn 

*** if_ethersubr.c.orig	Mon May 24 20:57:44 1999
--- if_ethersubr.c	Sun May 30 15:40:15 1999
***************
*** 111,116 ****
--- 111,120 ----
  #include <net/if_vlan_var.h>
  #endif /* NVLAN > 0 */
  
+ static 	int specify_src_hwaddr = 0;
+ SYSCTL_INT(_net_link_ether_inet, OID_AUTO, specify_src_hwaddr, CTLFLAG_RW, 
+ 	&specify_src_hwaddr, 0, ""); 
+ 
  static	int ether_resolvemulti __P((struct ifnet *, struct sockaddr **, 
  				    struct sockaddr *));
  u_char	etherbroadcastaddr[6] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };
***************
*** 133,138 ****
--- 137,143 ----
  	short type;
  	int s, error = 0;
   	u_char edst[6];
+ 	u_char esrc[6];
  	register struct mbuf *m = m0;
  	register struct rtentry *rt;
  	register struct ether_header *eh;
***************
*** 333,338 ****
--- 338,345 ----
  		loop_copy = -1; /* if this is for us, don't do it */
  		eh = (struct ether_header *)dst->sa_data;
   		(void)memcpy(edst, eh->ether_dhost, sizeof (edst));
+ 		if (specify_src_hwaddr)
+ 			(void)memcpy(esrc, eh->ether_shost, sizeof (esrc));
  		type = eh->ether_type;
  		break;
  
***************
*** 353,359 ****
  	(void)memcpy(&eh->ether_type, &type,
  		sizeof(eh->ether_type));
   	(void)memcpy(eh->ether_dhost, edst, sizeof (edst));
!  	(void)memcpy(eh->ether_shost, ac->ac_enaddr,
  	    sizeof(eh->ether_shost));
  
  	/*
--- 360,369 ----
  	(void)memcpy(&eh->ether_type, &type,
  		sizeof(eh->ether_type));
   	(void)memcpy(eh->ether_dhost, edst, sizeof (edst));
!  	if (specify_src_hwaddr)
!  		(void)memcpy(eh->ether_shost, esrc, sizeof(esrc));
!  	else
!  		(void)memcpy(eh->ether_shost, ac->ac_enaddr,
  	    sizeof(eh->ether_shost));
  
  	/*
