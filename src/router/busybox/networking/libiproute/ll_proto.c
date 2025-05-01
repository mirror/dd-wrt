/* vi: set sw=4 ts=4: */
/*
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version
 * 2 of the License, or (at your option) any later version.
 *
 * Authors: Alexey Kuznetsov, <kuznet@ms2.inr.ac.ru>
 */
#include "libbb.h"
#include "rt_names.h"
#include "utils.h"

#include <netinet/if_ether.h>
#define ETH_P_PAE	0x888E		/* Port Access Entity (IEEE 802.1X) */
#define ETH_P_PROFINET	0x8892		/* PROFINET			*/
#define ETH_P_REALTEK	0x8899          /* Multiple proprietary protocols */
#define ETH_P_AOE	0x88A2		/* ATA over Ethernet		*/
#define ETH_P_ETHERCAT	0x88A4		/* EtherCAT			*/
#define ETH_P_8021AD	0x88A8          /* 802.1ad Service VLAN		*/
#define ETH_P_802_EX1	0x88B5		/* 802.1 Local Experimental 1.  */
#define ETH_P_PREAUTH	0x88C7		/* 802.11 Preauthentication */
#define ETH_P_TIPC	0x88CA		/* TIPC 			*/
#define ETH_P_LLDP	0x88CC		/* Link Layer Discovery Protocol */
#define ETH_P_MRP	0x88E3		/* Media Redundancy Protocol	*/
#define ETH_P_MACSEC	0x88E5		/* 802.1ae MACsec */
#define ETH_P_8021AH	0x88E7          /* 802.1ah Backbone Service Tag */
#define ETH_P_MVRP	0x88F5          /* 802.1Q MVRP                  */
#define ETH_P_1588	0x88F7		/* IEEE 1588 Timesync */
#define ETH_P_NCSI	0x88F8		/* NCSI protocol		*/
#define ETH_P_PRP	0x88FB		/* IEC 62439-3 PRP/HSRv0	*/
#define ETH_P_CFM	0x8902		/* Connectivity Fault Management */
#define ETH_P_FCOE	0x8906		/* Fibre Channel over Ethernet  */
#define ETH_P_IBOE	0x8915		/* Infiniband over Ethernet	*/
#define ETH_P_TDLS	0x890D          /* TDLS */
#define ETH_P_FIP	0x8914		/* FCoE Initialization Protocol */
#define ETH_P_80221	0x8917		/* IEEE 802.21 Media Independent Handover Protocol */
#define ETH_P_HSR	0x892F		/* IEC 62439-3 HSRv1	*/
#define ETH_P_NSH	0x894F		/* Network Service Header */
#define ETH_P_LOOPBACK	0x9000		/* Ethernet loopback packet, per IEEE 802.3 */
#define ETH_P_QINQ1	0x9100		/* deprecated QinQ VLAN [ NOT AN OFFICIALLY REGISTERED ID ] */
#define ETH_P_QINQ2	0x9200		/* deprecated QinQ VLAN [ NOT AN OFFICIALLY REGISTERED ID ] */
#define ETH_P_QINQ3	0x9300		/* deprecated QinQ VLAN [ NOT AN OFFICIALLY REGISTERED ID ] */
#define ETH_P_EDSA	0xDADA		/* Ethertype DSA [ NOT AN OFFICIALLY REGISTERED ID ] */
#define ETH_P_DSA_8021Q	0xDADB		/* Fake VLAN Header for DSA [ NOT AN OFFICIALLY REGISTERED ID ] */
#define ETH_P_DSA_A5PSW	0xE001		/* A5PSW Tag Value [ NOT AN OFFICIALLY REGISTERED ID ] */
#define ETH_P_IFE	0xED3E		/* ForCES inter-FE LFB type */
#define ETH_P_AF_IUCV   0xFBFB		/* IBM af_iucv [ NOT AN OFFICIALLY REGISTERED ID ] */

/* Please conditionalize exotic protocols on CONFIG_something */

static const uint16_t llproto_ids[] ALIGN2 = {
#define __PF(f,n) ETH_P_##f,
__PF(LOOP,loop)
__PF(PUP,pup)
#ifdef ETH_P_PUPAT
__PF(PUPAT,pupat)
#endif
__PF(IP,ip)
__PF(X25,x25)
__PF(ARP,arp)
__PF(BPQ,bpq)
#ifdef ETH_P_IEEEPUP
__PF(IEEEPUP,ieeepup)
#endif
#ifdef ETH_P_IEEEPUPAT
__PF(IEEEPUPAT,ieeepupat)
#endif
__PF(DEC,dec)
__PF(DNA_DL,dna_dl)
__PF(DNA_RC,dna_rc)
__PF(DNA_RT,dna_rt)
__PF(LAT,lat)
__PF(DIAG,diag)
__PF(CUST,cust)
__PF(SCA,sca)
__PF(RARP,rarp)
__PF(ATALK,atalk)
__PF(AARP,aarp)
__PF(IPX,ipx)
__PF(IPV6,ipv6)
#ifdef ETH_P_PPP_DISC
__PF(PPP_DISC,ppp_disc)
#endif
#ifdef ETH_P_PPP_SES
__PF(PPP_SES,ppp_ses)
#endif
#ifdef ETH_P_ATMMPOA
__PF(ATMMPOA,atmmpoa)
#endif
#ifdef ETH_P_ATMFATE
__PF(ATMFATE,atmfate)
#endif

__PF(802_3,802_3)
__PF(AX25,ax25)
__PF(ALL,all)
__PF(802_2,802_2)
__PF(SNAP,snap)
__PF(DDCMP,ddcmp)
__PF(WAN_PPP,wan_ppp)
__PF(PPP_MP,ppp_mp)
__PF(LOCALTALK,localtalk)
__PF(PPPTALK,ppptalk)
__PF(TR_802_2,tr_802_2)
__PF(MOBITEX,mobitex)
__PF(CONTROL,control)
__PF(IRDA,irda)
#ifdef ETH_P_ECONET
__PF(ECONET,econet)
#endif
__PF(TIPC,tipc)
__PF(PROFINET,profinet)
__PF(AOE,aoe)
__PF(ETHERCAT,ethercat)
__PF(8021Q,802.1Q)
__PF(8021AD,802.1ad)
__PF(MPLS_UC,mpls_uc)
__PF(MPLS_MC,mpls_mc)
__PF(TEB,teb)
0x8100,
0x88cc,
ETH_P_IP,
};
#undef __PF

/* Keep declarations above and below in sync! */

static const char llproto_names[] ALIGN1 =
#define __PF(f,n) #n "\0"
__PF(LOOP,loop)
__PF(PUP,pup)
#ifdef ETH_P_PUPAT
__PF(PUPAT,pupat)
#endif
__PF(IP,ip)
__PF(X25,x25)
__PF(ARP,arp)
__PF(BPQ,bpq)
#ifdef ETH_P_IEEEPUP
__PF(IEEEPUP,ieeepup)
#endif
#ifdef ETH_P_IEEEPUPAT
__PF(IEEEPUPAT,ieeepupat)
#endif
__PF(DEC,dec)
__PF(DNA_DL,dna_dl)
__PF(DNA_RC,dna_rc)
__PF(DNA_RT,dna_rt)
__PF(LAT,lat)
__PF(DIAG,diag)
__PF(CUST,cust)
__PF(SCA,sca)
__PF(RARP,rarp)
__PF(ATALK,atalk)
__PF(AARP,aarp)
__PF(IPX,ipx)
__PF(IPV6,ipv6)
#ifdef ETH_P_PPP_DISC
__PF(PPP_DISC,ppp_disc)
#endif
#ifdef ETH_P_PPP_SES
__PF(PPP_SES,ppp_ses)
#endif
#ifdef ETH_P_ATMMPOA
__PF(ATMMPOA,atmmpoa)
#endif
#ifdef ETH_P_ATMFATE
__PF(ATMFATE,atmfate)
#endif

__PF(802_3,802_3)
__PF(AX25,ax25)
__PF(ALL,all)
__PF(802_2,802_2)
__PF(SNAP,snap)
__PF(DDCMP,ddcmp)
__PF(WAN_PPP,wan_ppp)
__PF(PPP_MP,ppp_mp)
__PF(LOCALTALK,localtalk)
__PF(PPPTALK,ppptalk)
__PF(TR_802_2,tr_802_2)
__PF(MOBITEX,mobitex)
__PF(CONTROL,control)
__PF(IRDA,irda)
#ifdef ETH_P_ECONET
__PF(ECONET,econet)
#endif
__PF(TIPC,tipc)
__PF(PROFINET,profinet)
__PF(AOE,aoe)
__PF(ETHERCAT,ethercat)
__PF(8021Q,802.1Q)
__PF(8021AD,802.1ad)
__PF(MPLS_UC,mpls_uc)
__PF(MPLS_MC,mpls_mc)
__PF(TEB,teb)

"802.1Q" "\0"
"LLDP" "\0"
"ipv4" "\0"
;
#undef __PF


const char* FAST_FUNC ll_proto_n2a(unsigned short id, char *buf, int len)
{
	unsigned i;
	id = ntohs(id);
	for (i = 0; i < ARRAY_SIZE(llproto_ids); i++) {
		if (llproto_ids[i] == id)
			return nth_string(llproto_names, i);
	}
	snprintf(buf, len, "[%u]", id);
	return buf;
}

int FAST_FUNC ll_proto_a2n(unsigned short *id, char *buf)
{
	unsigned i;
	const char *name = llproto_names;
	for (i = 0; i < ARRAY_SIZE(llproto_ids); i++) {
		if (strcasecmp(name, buf) == 0) {
			i = llproto_ids[i];
			goto good;
		}
		name += strlen(name) + 1;
	}
	errno = 0;
	i = bb_strtou(buf, NULL, 0);
	if (errno || i > 0xffff)
		return -1;
 good:
	*id = htons(i);
	return 0;
}
