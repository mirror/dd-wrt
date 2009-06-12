//==========================================================================
//
//      ./lib/current/include/mib.h
//
//
//==========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.
//
// eCos is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 or (at your option) any later version.
//
// eCos is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// for more details.
//
// You should have received a copy of the GNU General Public License along
// with eCos; if not, write to the Free Software Foundation, Inc.,
// 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
//
// As a special exception, if other files instantiate templates or use macros
// or inline functions from this file, or you compile this file and link it
// with other works to produce a work based on this file, this file does not
// by itself cause the resulting work to be covered by the GNU General Public
// License. However the source code for this file must still be made available
// in accordance with section (3) of the GNU General Public License.
//
// This exception does not invalidate any other reasons why a work based on
// this file might be covered by the GNU General Public License.
//
// Alternative licenses for eCos may be arranged by contacting Red Hat, Inc.
// at http://sources.redhat.com/ecos/ecos-license/
// -------------------------------------------
//####ECOSGPLCOPYRIGHTEND####
//####UCDSNMPCOPYRIGHTBEGIN####
//
// -------------------------------------------
//
// Portions of this software may have been derived from the UCD-SNMP
// project,  <http://ucd-snmp.ucdavis.edu/>  from the University of
// California at Davis, which was originally based on the Carnegie Mellon
// University SNMP implementation.  Portions of this software are therefore
// covered by the appropriate copyright disclaimers included herein.
//
// The release used was version 4.1.2 of May 2000.  "ucd-snmp-4.1.2"
// -------------------------------------------
//
//####UCDSNMPCOPYRIGHTEND####
//==========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):    hmt
// Contributors: hmt
// Date:         2000-05-30
// Purpose:      Port of UCD-SNMP distribution to eCos.
// Description:  
//              
//
//####DESCRIPTIONEND####
//
//==========================================================================
/********************************************************************
       Copyright 1989, 1991, 1992 by Carnegie Mellon University

			  Derivative Work -
Copyright 1996, 1998, 1999, 2000 The Regents of the University of California

			 All Rights Reserved

Permission to use, copy, modify and distribute this software and its
documentation for any purpose and without fee is hereby granted,
provided that the above copyright notice appears in all copies and
that both that copyright notice and this permission notice appear in
supporting documentation, and that the name of CMU and The Regents of
the University of California not be used in advertising or publicity
pertaining to distribution of the software without specific written
permission.

CMU AND THE REGENTS OF THE UNIVERSITY OF CALIFORNIA DISCLAIM ALL
WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING ALL IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS.  IN NO EVENT SHALL CMU OR
THE REGENTS OF THE UNIVERSITY OF CALIFORNIA BE LIABLE FOR ANY SPECIAL,
INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING
FROM THE LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF
CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
*********************************************************************/
#ifndef MIB_H
#define MIB_H

#ifdef __cplusplus
extern "C" {
#endif
/*
 * mib.h - Definitions for the variables as defined in the MIB
 *
 * Update: 1998-07-17 <jhy@gsu.edu>
 * Added prototypes for print_oid_report* functions.
 */
/***********************************************************
	Copyright 1988, 1989 by Carnegie Mellon University

                      All Rights Reserved

Permission to use, copy, modify, and distribute this software and its 
documentation for any purpose and without fee is hereby granted, 
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in 
supporting documentation, and that the name of CMU not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.  

CMU DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
CMU BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.
******************************************************************/

#ifdef CMU_COMPATIBLE

struct	mib_system {
    char    sysDescr[32];   /* textual description */
    u_char  sysObjectID[16];/* OBJECT IDENTIFIER of system */
    u_char  ObjIDLen;	    /* length of sysObjectID */
    u_int  sysUpTime;	    /* Uptime in 100/s of a second */    
};

struct mib_interface {
    int    ifNumber;	    /* number of interfaces */
};

struct mib_ifEntry {
    int    ifIndex;	    /* index of this interface	*/
    char    ifDescr[32];    /* english description of interface	*/
    int    ifType;	    /* network type of device	*/
    int    ifMtu;	    /* size of largest packet in bytes	*/
    u_int  ifSpeed;	    /* bandwidth in bits/sec	*/
    u_char  ifPhysAddress[11];	/* interface's address */
    u_char  PhysAddrLen;    /* length of physAddr */
    int    ifAdminStatus;  /* desired state of interface */
    int    ifOperStatus;   /* current operational status */
    u_int  ifLastChange;   /* value of sysUpTime when current state entered */
    u_int  ifInOctets;	    /* number of octets received on interface */
    u_int  ifInUcastPkts;  /* number of unicast packets delivered */
    u_int  ifInNUcastPkts; /* number of broadcasts or multicasts */
    u_int  ifInDiscards;   /* number of packets discarded with no error */
    u_int  ifInErrors;	    /* number of packets containing errors */
    u_int  ifInUnknownProtos;	/* number of packets with unknown protocol */
    u_int  ifOutOctets;    /* number of octets transmitted */
    u_int  ifOutUcastPkts; /* number of unicast packets sent */
    u_int  ifOutNUcastPkts;/* number of broadcast or multicast pkts */
    u_int  ifOutDiscards;  /* number of packets discarded with no error */
    u_int  ifOutErrors;    /* number of pkts discarded with an error */
    u_int  ifOutQLen;	    /* number of packets in output queue */
};

struct mib_atEntry {
    int    atIfIndex;	    /* interface on which this entry maps */
    u_char  atPhysAddress[11]; /* physical address of destination */
    u_char  PhysAddressLen; /* length of atPhysAddress */
    u_int  atNetAddress;   /* IP address of physical address */
};

struct mib_ip {
    int    ipForwarding;   /* 1 if gateway, 2 if host */
    int    ipDefaultTTL;   /* default TTL for pkts originating here */
    u_int  ipInReceives;   /* no. of IP packets received from interfaces */
    u_int  ipInHdrErrors;  /* number of pkts discarded due to header errors */
    u_int  ipInAddrErrors; /* no. of pkts discarded due to bad address */
    u_int  ipForwDatagrams;/* number pf pkts forwarded through this entity */
    u_int  ipInUnknownProtos;/* no. of local-addressed pkts w/unknown proto */
    u_int  ipInDiscards;   /* number of error-free packets discarded */
    u_int  ipInDelivers;   /* number of datagrams delivered to upper level */
    u_int  ipOutRequests;  /* number of IP datagrams originating locally */
    u_int  ipOutDiscards;  /* number of error-free output IP pkts discarded */
    u_int  ipOutNoRoutes;  /* number of IP pkts discarded due to no route */
    int    ipReasmTimeout; /* seconds fragment is held awaiting reassembly */
    u_int  ipReasmReqds;   /* no. of fragments needing reassembly (here) */
    u_int  ipReasmOKs;	    /* number of fragments reassembled */
    u_int  ipReasmFails;   /* number of failures in IP reassembly */
    u_int  ipFragOKs;	    /* number of datagrams fragmented here */
    u_int  ipFragFails;    /* no. pkts unable to be fragmented here */
    u_int  ipFragCreates;  /* number of IP fragments created here */
};

struct mib_ipAddrEntry {
    u_int  ipAdEntAddr;    /* IP address of this entry */
    int    ipAdEntIfIndex; /* IF for this entry */
    u_int  ipAdEntNetMask; /* subnet mask of this entry */
    int    ipAdEntBcastAddr;/* read the MIB for this one */
};

struct mib_ipRouteEntry {
    u_int  ipRouteDest;    /* destination IP addr for this route */
    int    ipRouteIfIndex; /* index of local IF for this route */
    int    ipRouteMetric1; /* Primary routing metric */
    int    ipRouteMetric2; /* Alternate routing metric */
    int    ipRouteMetric3; /* Alternate routing metric */
    int    ipRouteMetric4; /* Alternate routing metric */
    u_int  ipRouteNextHop; /* IP addr of next hop */
    int    ipRouteType;    /* Type of this route */
    int    ipRouteProto;   /* How this route was learned */
    int    ipRouteAge;	    /* No. of seconds since updating this route */
};

struct mib_icmp {
    u_int  icmpInMsgs;	    /* Total of ICMP msgs received */
    u_int  icmpInErrors;   /* Total of ICMP msgs received with errors */
    u_int  icmpInDestUnreachs;
    u_int  icmpInTimeExcds;
    u_int  icmpInParmProbs;
    u_int  icmpInSrcQuenchs;
    u_int  icmpInRedirects;
    u_int  icmpInEchos;
    u_int  icmpInEchoReps;
    u_int  icmpInTimestamps;
    u_int  icmpInTimestampReps;
    u_int  icmpInAddrMasks;
    u_int  icmpInAddrMaskReps;
    u_int  icmpOutMsgs;
    u_int  icmpOutErrors;
    u_int  icmpOutDestUnreachs;
    u_int  icmpOutTimeExcds;
    u_int  icmpOutParmProbs;
    u_int  icmpOutSrcQuenchs;
    u_int  icmpOutRedirects;
    u_int  icmpOutEchos;
    u_int  icmpOutEchoReps;
    u_int  icmpOutTimestamps;
    u_int  icmpOutTimestampReps;
    u_int  icmpOutAddrMasks;
    u_int  icmpOutAddrMaskReps;
};

struct	mib_tcp {
    int    tcpRtoAlgorithm;	/* retransmission timeout algorithm */
    int    tcpRtoMin;		/* minimum retransmission timeout (mS) */
    int    tcpRtoMax;		/* maximum retransmission timeout (mS) */ 
    int    tcpMaxConn;		/* maximum tcp connections possible */
    u_int  tcpActiveOpens;	/* number of SYN-SENT -> CLOSED transitions */
    u_int  tcpPassiveOpens;	/* number of SYN-RCVD -> LISTEN transitions */
    u_int  tcpAttemptFails;/*(SYN-SENT,SYN-RCVD)->CLOSED or SYN-RCVD->LISTEN*/
    u_int  tcpEstabResets;	/* (ESTABLISHED,CLOSE-WAIT) -> CLOSED */
    u_int  tcpCurrEstab;	/* number in ESTABLISHED or CLOSE-WAIT state */
    u_int  tcpInSegs;		/* number of segments received */
    u_int  tcpOutSegs;		/* number of segments sent */
    u_int  tcpRetransSegs;	/* number of retransmitted segments */
};

struct mib_tcpConnEntry {
    int    tcpConnState;	/* State of this connection */
    u_int  tcpConnLocalAddress;/* local IP address for this connection */
    int    tcpConnLocalPort;	/* local port for this connection */
    u_int  tcpConnRemAddress;	/* remote IP address for this connection */
    int    tcpConnRemPort;	/* remote port for this connection */
};

struct mib_udp {
    u_int  udpInDatagrams; /* No. of UDP datagrams delivered to users */
    u_int  udpNoPorts;	    /* No. of UDP datagrams to port with no listener */
    u_int  udpInErrors;    /* No. of UDP datagrams unable to be delivered */
    u_int  udpOutDatagrams;/* No. of UDP datagrams sent from this entity */
};

struct	mib_egp {
    u_int  egpInMsgs;	/* No. of EGP msgs received without error */
    u_int  egpInErrors;/* No. of EGP msgs received with error */
    u_int  egpOutMsgs;	/* No. of EGP msgs sent */
    u_int  egpOutErrors;/* No. of (outgoing) EGP msgs dropped due to error */
};

struct	mib_egpNeighEntry {
    int    egpNeighState;  /* local EGP state with this entry's neighbor */
    u_int  egpNeighAddr;   /* IP address of this entry's neighbor */
};

#endif /* CMU_COMPATIBLE */

#define MIB 1, 3, 6, 1, 2, 1

#define MIB_IFTYPE_OTHER		    1
#define MIB_IFTYPE_REGULAR1822		    2
#define MIB_IFTYPE_HDH1822		    3
#define MIB_IFTYPE_DDNX25		    4
#define MIB_IFTYPE_RFC877X25		    5
#define MIB_IFTYPE_ETHERNETCSMACD	    6
#define MIB_IFTYPE_ISO88023CSMACD	    7
#define MIB_IFTYPE_ISO88024TOKENBUS	    8
#define MIB_IFTYPE_ISO88025TOKENRING	    9
#define MIB_IFTYPE_ISO88026MAN		    10
#define MIB_IFTYPE_STARLAN		    11
#define MIB_IFTYPE_PROTEON10MBIT	    12
#define MIB_IFTYPE_PROTEON80MBIT	    13
#define MIB_IFTYPE_HYPERCHANNEL		    14
#define MIB_IFTYPE_FDDI			    15
#define MIB_IFTYPE_LAPB			    16
#define MIB_IFTYPE_SDLC			    17
#define MIB_IFTYPE_T1CARRIER		    18
#define MIB_IFTYPE_CEPT			    19
#define MIB_IFTYPE_BASICISDN		    20
#define MIB_IFTYPE_PRIMARYISDN		    21
#define MIB_IFTYPE_PROPPOINTTOPOINTSERIAL   22

#define MIB_IFSTATUS_UP		1
#define MIB_IFSTATUS_DOWN	2
#define MIB_IFSTATUS_TESTING	3

#define MIB_FORWARD_GATEWAY	1
#define MIB_FORWARD_HOST	2

#define MIB_IPROUTETYPE_OTHER	1
#define MIB_IPROUTETYPE_INVALID	2
#define MIB_IPROUTETYPE_DIRECT	3
#define MIB_IPROUTETYPE_REMOTE	4

#define MIB_IPROUTEPROTO_OTHER	    1
#define MIB_IPROUTEPROTO_LOCAL	    2
#define MIB_IPROUTEPROTO_NETMGMT    3
#define MIB_IPROUTEPROTO_ICMP	    4
#define MIB_IPROUTEPROTO_EGP	    5
#define MIB_IPROUTEPROTO_GGP	    6
#define MIB_IPROUTEPROTO_HELLO	    7
#define MIB_IPROUTEPROTO_RIP	    8
#define MIB_IPROUTEPROTO_ISIS	    9
#define MIB_IPROUTEPROTO_ESIS	    10
#define MIB_IPROUTEPROTO_CISCOIGRP  11
#define MIB_IPROUTEPROTO_BBNSPFIGP  12
#define MIB_IPROUTEPROTO_OIGP	    13

#define MIB_TCPRTOALG_OTHER	1
#define MIB_TCPRTOALG_CONSTANT	2
#define MIB_TCPRTOALG_RSRE	3
#define MIB_TCPRTOALG_VANJ	4

#define MIB_TCPCONNSTATE_CLOSED		1
#define MIB_TCPCONNSTATE_LISTEN		2
#define MIB_TCPCONNSTATE_SYNSENT	3
#define MIB_TCPCONNSTATE_SYNRECEIVED	4
#define MIB_TCPCONNSTATE_ESTABLISHED	5
#define MIB_TCPCONNSTATE_FINWAIT1	6
#define MIB_TCPCONNSTATE_FINWAIT2	7
#define MIB_TCPCONNSTATE_CLOSEWAIT	8
#define MIB_TCPCONNSTATE_LASTACK	9
#define MIB_TCPCONNSTATE_CLOSING	10
#define MIB_TCPCONNSTATE_TIMEWAIT	11

#define MIB_EGPNEIGHSTATE_IDLE		1
#define MIB_EGPNEIGHSTATE_AQUISITION	2
#define MIB_EGPNEIGHSTATE_DOWN		3
#define MIB_EGPNEIGHSTATE_UP		4
#define MIB_EGPNEIGHSTATE_CEASE		5

struct variable_list;

void print_mib (FILE *);
void print_ascii_dump (FILE *);
int read_objid (const char *, oid *, size_t *);
void register_mib_handlers (void);
void init_mib (void);
void print_variable (oid *, size_t, struct variable_list *);
void fprint_variable (FILE *, oid *, size_t, struct variable_list *);
void sprint_variable (char *, oid *, size_t, struct variable_list *);
void print_value (oid *, size_t, struct variable_list *);
void fprint_value (FILE *, oid *, size_t, struct variable_list *);
void sprint_value (char *, oid *, size_t, struct variable_list *);
void print_objid (oid *, size_t);
void fprint_objid (FILE *, oid *, size_t);
char *sprint_objid (char *, oid *, size_t);
void print_description (oid *, size_t);
void fprint_description (FILE *, oid *, size_t);
int get_module_node (const char *, const char *, oid *, size_t *);
int get_wild_node(const char *, oid *, size_t *);
int get_node (const char *, oid *, size_t *);
struct tree *get_symbol (oid *, size_t, struct tree *, char *);
struct tree *get_tree (oid *, size_t, struct tree *);
struct tree *get_tree_head (void);
void  set_function (struct tree *);
void sprint_hexstring (char *, const u_char *, size_t);
void sprint_asciistring(char *buf, u_char *cp, size_t len);

void print_oid_report (FILE *);
void print_oid_report_enable_labeledoid (void);
void print_oid_report_enable_oid (void);
void print_oid_report_enable_suffix (void);
void print_oid_report_enable_symbolic (void);

void clear_tree_flags(register struct tree *tp);

char *snmp_out_toggle_options(char *);
void snmp_out_toggle_options_usage(const char *, FILE *);
char *snmp_in_toggle_options(char *);
void snmp_in_toggle_options_usage(const char *, FILE *);

#ifdef __cplusplus
}
#endif

#endif /* MIB_H */
