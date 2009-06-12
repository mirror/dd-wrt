//==========================================================================
//
//      sys/net/bridgestp.c
//
//     
//
//==========================================================================
//####COPYRIGHTBEGIN####
//                                                                          
// -------------------------------------------                              
// The contents of this file are subject to the Red Hat eCos Public License 
// Version 1.1 (the "License"); you may not use this file except in         
// compliance with the License.  You may obtain a copy of the License at    
// http://www.redhat.com/                                                   
//                                                                          
// Software distributed under the License is distributed on an "AS IS"      
// basis, WITHOUT WARRANTY OF ANY KIND, either express or implied.  See the 
// License for the specific language governing rights and limitations under 
// the License.                                                             
//                                                                          
// The Original Code is eCos - Embedded Configurable Operating System,      
// released September 30, 1998.                                             
//                                                                          
// The Initial Developer of the Original Code is Red Hat.                   
// Portions created by Red Hat are                                          
// Copyright (C) 1998, 1999, 2000 Red Hat, Inc.                             
// All Rights Reserved.                                                     
// -------------------------------------------                              
//                                                                          
//####COPYRIGHTEND####
//####BSDCOPYRIGHTBEGIN####
//
// -------------------------------------------
//
// Portions of this software may have been derived from OpenBSD or other sources,
// and are covered by the appropriate copyright disclaimers included herein.
//
// -------------------------------------------
//
//####BSDCOPYRIGHTEND####
//==========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):    Jason L. Wright (jason@thought.net)  
// Contributors: Manu Sharma (manu.sharma@ascom.com)
// Date:         2000-07-18
// Purpose:      Implementation of the spanning tree protocol as defined in
//               ISO/IEC Final DIS 15802-3 (IEEE P802.1D/D17), May 25, 1998.
//               (In English: IEEE 802.1D, Draft 17, 1998)
// Description:  
//              
//
//####DESCRIPTIONEND####
//
//==========================================================================
//
/*	$OpenBSD: bridgestp.c,v 1.15 2002/12/10 13:22:55 markus Exp $	*/

/*
 * Copyright (c) 2000 Jason L. Wright (jason@thought.net)
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

/*
 * Implementation of the spanning tree protocol as defined in
 * ISO/IEC Final DIS 15802-3 (IEEE P802.1D/D17), May 25, 1998.
 * (In English: IEEE 802.1D, Draft 17, 1998)
 */

#ifdef __ECOS
#include <pkgconf/net.h>
#include <stdio.h>
#else
#include "bridge.h"
#endif

#ifdef CYGPKG_NET_BRIDGE_STP_CODE
#if NBRIDGE 

#include <sys/param.h>
#ifndef __ECOS
#include <sys/systm.h>
#endif
#include <sys/mbuf.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#ifndef __ECOS
#include <sys/device.h>
#endif
#include <sys/kernel.h>

#include <net/if.h>
#include <net/if_types.h>
#include <net/if_llc.h>
#include <net/netisr.h>

#ifdef INET
#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <netinet/in_var.h>
#include <netinet/ip.h>
#include <netinet/if_ether.h>
#endif

#if NBPFILTER > 0
#include <net/bpf.h>
#endif

#include <net/if_bridge.h>

#define STP_DEBUG
#define STP_DETAILED_DEBUG
#ifdef STP_DEBUG
#define STPLOG(x) diag_printf x
#else
#define STPLOG(x)
#endif

#ifdef STP_DETAILED_DEBUG
#define STP_OPERATION_LOG(x) diag_printf x
#else
#define STP_OPERATION_LOG(x)
#endif

/* BPDU message types */
#define	BSTP_MSGTYPE_CFG	0x00		/* Configuration */
#define	BSTP_MSGTYPE_TCN	0x80		/* Topology chg notification */

/* BPDU flags */
#define	BSTP_FLAG_TC		0x01		/* Topology change */
#define	BSTP_FLAG_TCA		0x80		/* Topology change ack */

#define	BSTP_MESSAGE_AGE_INCR	(1 * 256)	/* in 256ths of a second */
#define	BSTP_TICK_VAL		(1 * 256)	/* in 256ths of a second */

/*
 * Because BPDU's do not make nicely aligned structures, two different
 * declarations are used: bstp_?bpdu (wire representation, packed) and
 * bstp_*_unit (internal, nicely aligned version).
 */

/* configuration bridge protocol data unit */
struct bstp_cbpdu {
	u_int8_t	cbu_dsap;		/* LLC: destination sap */
	u_int8_t	cbu_ssap;		/* LLC: source sap */
	u_int8_t	cbu_ctl;		/* LLC: control */
	u_int16_t	cbu_protoid;		/* protocol id */
	u_int8_t	cbu_protover;		/* protocol version */
	u_int8_t	cbu_bpdutype;		/* message type */
	u_int8_t	cbu_flags;		/* flags (below) */

	/* root id */
	u_int16_t	cbu_rootpri;		/* root priority */
	u_int8_t	cbu_rootaddr[6];	/* root address */

	u_int32_t	cbu_rootpathcost;	/* root path cost */

	/* bridge id */
	u_int16_t	cbu_bridgepri;		/* bridge priority */
	u_int8_t	cbu_bridgeaddr[6];	/* bridge address */

	u_int16_t	cbu_portid;		/* port id */
	u_int16_t	cbu_messageage;		/* current message age */
	u_int16_t	cbu_maxage;		/* maximum age */
	u_int16_t	cbu_hellotime;		/* hello time */
	u_int16_t	cbu_forwarddelay;	/* forwarding delay */
} __attribute__((__packed__));

/* topology change notification bridge protocol data unit */
struct bstp_tbpdu {
	u_int8_t	tbu_dsap;		/* LLC: destination sap */
	u_int8_t	tbu_ssap;		/* LLC: source sap */
	u_int8_t	tbu_ctl;		/* LLC: control */
	u_int16_t	tbu_protoid;		/* protocol id */
	u_int8_t	tbu_protover;		/* protocol version */
	u_int8_t	tbu_bpdutype;		/* message type */
} __attribute__((__packed__));

u_int8_t bstp_etheraddr[] = { 0x01, 0x80, 0xc2, 0x00, 0x00, 0x00 };
u_int8_t bstp_init_done;

void bstp_initialization(struct bridge_softc *);
void bstp_stop(struct bridge_softc *);
void bstp_initialize_port(struct bridge_softc *, struct bridge_iflist *);
void bstp_ifupdstatus(struct bridge_softc *, struct bridge_iflist *);
void bstp_enable_port(struct bridge_softc *, struct bridge_iflist *);
void bstp_disable_port(struct bridge_softc *, struct bridge_iflist *);
void bstp_enable_change_detection(struct bridge_iflist *);
void bstp_disable_change_detection(struct bridge_iflist *);
int bstp_root_bridge(struct bridge_softc *sc);
int bstp_supersedes_port_info(struct bridge_softc *, struct bridge_iflist *,
    struct bstp_config_unit *);
int bstp_designated_port(struct bridge_softc *, struct bridge_iflist *);
int bstp_designated_for_some_port(struct bridge_softc *);
void bstp_transmit_config(struct bridge_softc *, struct bridge_iflist *);
void bstp_transmit_tcn(struct bridge_softc *);
struct mbuf *bstp_input(struct bridge_softc *, struct ifnet *,
    struct ether_header *, struct mbuf *);
void bstp_received_config_bpdu(struct bridge_softc *, struct bridge_iflist *,
    struct bstp_config_unit *);
void bstp_received_tcn_bpdu(struct bridge_softc *, struct bridge_iflist *,
    struct bstp_tcn_unit *);
void bstp_record_config_information(struct bridge_softc *,
    struct bridge_iflist *, struct bstp_config_unit *);
void bstp_record_config_timeout_values(struct bridge_softc *,
    struct bstp_config_unit *);
void bstp_config_bpdu_generation(struct bridge_softc *);
void bstp_send_config_bpdu(struct bridge_iflist *, struct bstp_config_unit *);
void bstp_configuration_update(struct bridge_softc *);
void bstp_root_selection(struct bridge_softc *);
void bstp_designated_port_selection(struct bridge_softc *);
void bstp_become_designated_port(struct bridge_softc *, struct bridge_iflist *);
void bstp_port_state_selection(struct bridge_softc *);
void bstp_make_forwarding(struct bridge_softc *, struct bridge_iflist *);
void bstp_make_blocking(struct bridge_softc *, struct bridge_iflist *);
void bstp_set_port_state(struct bridge_iflist *, u_int8_t);
void bstp_set_bridge_priority(struct bridge_softc *, u_int64_t);
void bstp_set_port_priority(struct bridge_softc *, struct bridge_iflist *,
    u_int16_t);
void bstp_set_path_cost(struct bridge_softc *, struct bridge_iflist *,
    u_int32_t);
void bstp_topology_change_detection(struct bridge_softc *);
void bstp_topology_change_acknowledged(struct bridge_softc *);
void bstp_acknowledge_topology_change(struct bridge_softc *,
    struct bridge_iflist *);

void bstp_tick(void *);
void bstp_timer_start(struct bridge_timer *, u_int16_t);
void bstp_timer_stop(struct bridge_timer *);
int bstp_timer_expired(struct bridge_timer *, u_int16_t);

void bstp_hold_timer_expiry(struct bridge_softc *, struct bridge_iflist *);
void bstp_message_age_timer_expiry(struct bridge_softc *,
    struct bridge_iflist *);
void bstp_forward_delay_timer_expiry(struct bridge_softc *,
    struct bridge_iflist *);
void bstp_topology_change_timer_expiry(struct bridge_softc *);
void bstp_tcn_timer_expiry(struct bridge_softc *);
void bstp_hello_timer_expiry(struct bridge_softc *);

char stp_buf [32];
char *
stp_port_state (unsigned int state) {
        memset (stp_buf, 0x0, 32);
        switch (state) {
                case BSTP_IFSTATE_DISABLED   : sprintf (stp_buf, " Disabled "); break;
                case BSTP_IFSTATE_LISTENING  : sprintf (stp_buf, " Listn'ng "); break;
                case BSTP_IFSTATE_LEARNING   : sprintf (stp_buf, " Lrn'ng "); break;
                case BSTP_IFSTATE_FORWARDING : sprintf (stp_buf, " Fwd'ng "); break;
                case BSTP_IFSTATE_BLOCKING   : sprintf (stp_buf, " Blk'ng "); break;
                default: sprintf (stp_buf, " UNKNOWN "); break;
        }
        return stp_buf;
}

void
bstp_transmit_config(sc, bif)
	struct bridge_softc *sc;
	struct bridge_iflist *bif;
{
	if (bif->bif_hold_timer.active) {
		bif->bif_config_pending = 1;
		return;
	}

	bif->bif_config_bpdu.cu_message_type = BSTP_MSGTYPE_CFG;
	bif->bif_config_bpdu.cu_rootid = sc->sc_designated_root;
	bif->bif_config_bpdu.cu_root_path_cost = sc->sc_root_path_cost;
	bif->bif_config_bpdu.cu_bridge_id = sc->sc_bridge_id;
	bif->bif_config_bpdu.cu_port_id = bif->bif_port_id;

	if (bstp_root_bridge(sc))
		bif->bif_config_bpdu.cu_message_age = 0;
	else
		bif->bif_config_bpdu.cu_message_age =
		    sc->sc_root_port->bif_message_age_timer.value +
		    BSTP_MESSAGE_AGE_INCR;

	bif->bif_config_bpdu.cu_max_age = sc->sc_max_age;
	bif->bif_config_bpdu.cu_hello_time = sc->sc_hello_time;
	bif->bif_config_bpdu.cu_forward_delay = sc->sc_forward_delay;
	bif->bif_config_bpdu.cu_topology_change_acknowledgment
	    = bif->bif_topology_change_acknowledge;
	bif->bif_config_bpdu.cu_topology_change = sc->sc_topology_change;

	if (bif->bif_config_bpdu.cu_message_age < sc->sc_max_age) {
		bif->bif_topology_change_acknowledge = 0;
		bif->bif_config_pending = 0;
		bstp_send_config_bpdu(bif, &bif->bif_config_bpdu);
		bstp_timer_start(&bif->bif_hold_timer, 0);
	}
}

void
bstp_send_config_bpdu(bif, cu)
	struct bridge_iflist *bif;
	struct bstp_config_unit *cu;
{
	struct arpcom *arp;
	struct ifnet *ifp;
	struct mbuf *m;
	struct ether_header eh;
	struct bstp_cbpdu bpdu;
	int s, error;

	s = splimp();
	ifp = bif->ifp;
	arp = (struct arpcom *)ifp;

	if ((ifp->if_flags & IFF_RUNNING) == 0) {
		splx(s);
		return;
	}
#ifdef ALTQ
	if (!ALTQ_IS_ENABLED(&ifp->if_snd))
#endif
	if (IF_QFULL(&ifp->if_snd)) {
		splx(s);
		return;
	}

	MGETHDR(m, M_DONTWAIT, MT_DATA);
	if (m == NULL) {
		splx(s);
		return;
	}
	m->m_pkthdr.rcvif = ifp;
	m->m_pkthdr.len = sizeof(eh) + sizeof(bpdu);
	m->m_len = m->m_pkthdr.len;

	bpdu.cbu_ssap = bpdu.cbu_dsap = LLC_8021D_LSAP;
	bpdu.cbu_ctl = LLC_UI;
	bpdu.cbu_protoid = htons(0);
	bpdu.cbu_protover = 0;
	bpdu.cbu_bpdutype = cu->cu_message_type;
	bpdu.cbu_flags = (cu->cu_topology_change ? BSTP_FLAG_TC : 0) |
	    (cu->cu_topology_change_acknowledgment ? BSTP_FLAG_TCA : 0);

	bpdu.cbu_rootpri = htons(cu->cu_rootid >> 48);
	bpdu.cbu_rootaddr[0] = cu->cu_rootid >> 40;
	bpdu.cbu_rootaddr[1] = cu->cu_rootid >> 32;
	bpdu.cbu_rootaddr[2] = cu->cu_rootid >> 24;
	bpdu.cbu_rootaddr[3] = cu->cu_rootid >> 16;
	bpdu.cbu_rootaddr[4] = cu->cu_rootid >> 8;
	bpdu.cbu_rootaddr[5] = cu->cu_rootid >> 0;

	bpdu.cbu_rootpathcost = htonl(cu->cu_root_path_cost);

	bpdu.cbu_bridgepri = htons(cu->cu_rootid >> 48);
	bpdu.cbu_bridgeaddr[0] = cu->cu_rootid >> 40;
	bpdu.cbu_bridgeaddr[1] = cu->cu_rootid >> 32;
	bpdu.cbu_bridgeaddr[2] = cu->cu_rootid >> 24;
	bpdu.cbu_bridgeaddr[3] = cu->cu_rootid >> 16;
	bpdu.cbu_bridgeaddr[4] = cu->cu_rootid >> 8;
	bpdu.cbu_bridgeaddr[5] = cu->cu_rootid >> 0;

	bpdu.cbu_portid = htons(cu->cu_port_id);
	bpdu.cbu_messageage = htons(cu->cu_message_age);
	bpdu.cbu_maxage = htons(cu->cu_max_age);
	bpdu.cbu_hellotime = htons(cu->cu_hello_time);
	bpdu.cbu_forwarddelay = htons(cu->cu_forward_delay);

	bcopy(arp->ac_enaddr, eh.ether_shost, ETHER_ADDR_LEN);
	bcopy(bstp_etheraddr, eh.ether_dhost, ETHER_ADDR_LEN);
	eh.ether_type = htons(sizeof(bpdu));

	bcopy(&eh, m->m_data, sizeof(eh));
	bcopy(&bpdu, m->m_data + sizeof(eh), sizeof(bpdu));

        STPLOG (("        - <%s>: Tx with Port 0x%x; Age %d; Flags %d\n", bif->ifp->if_xname,
                  cu->cu_port_id, cu->cu_message_age, bpdu.cbu_flags));

	IFQ_ENQUEUE(&ifp->if_snd, m, NULL, error);
	if (error == 0 && (ifp->if_flags & IFF_OACTIVE) == 0)
		(*ifp->if_start)(ifp);
	splx(s);
}

int
bstp_root_bridge(sc)
	struct bridge_softc *sc;
{
	return (sc->sc_designated_root == sc->sc_bridge_id);
}

int
bstp_supersedes_port_info(sc, bif, cu)
	struct bridge_softc *sc;
	struct bridge_iflist *bif;
	struct bstp_config_unit *cu;
{
	if (cu->cu_rootid < bif->bif_designated_root) {
		return (1);
        }
	if (cu->cu_rootid > bif->bif_designated_root) {
		return (0);
        }

	if (cu->cu_root_path_cost < bif->bif_designated_cost) {
		return (1);
        }
	if (cu->cu_root_path_cost > bif->bif_designated_cost) {
		return (0);
        }

	if (cu->cu_bridge_id < bif->bif_designated_bridge) {
		return (1);
        }
	if (cu->cu_bridge_id > bif->bif_designated_bridge) {
		return (0);
        }

	if (sc->sc_bridge_id != cu->cu_bridge_id) {
		return (1);
        }
	if (cu->cu_port_id <= bif->bif_designated_port) {
		return (1); 
        }
	return (0);
}

void
bstp_record_config_information(sc, bif, cu)
	struct bridge_softc *sc;
	struct bridge_iflist *bif;
	struct bstp_config_unit *cu;
{
	bif->bif_designated_root = cu->cu_rootid;
	bif->bif_designated_cost = cu->cu_root_path_cost;
	bif->bif_designated_bridge = cu->cu_bridge_id;
	bif->bif_designated_port = cu->cu_port_id;
	bstp_timer_start(&bif->bif_message_age_timer, cu->cu_message_age);
}

void
bstp_record_config_timeout_values(sc, config)
	struct bridge_softc *sc;
	struct bstp_config_unit *config;
{
	sc->sc_max_age = config->cu_max_age;
	sc->sc_hello_time = config->cu_hello_time;
	sc->sc_forward_delay = config->cu_forward_delay;
	sc->sc_topology_change = config->cu_topology_change;
}

void
bstp_config_bpdu_generation(sc)
	struct bridge_softc *sc;
{
	struct bridge_iflist *bif;

        STPLOG (("STP : Tx Hello BPDU ...\n"));
	LIST_FOREACH(bif, &sc->sc_iflist, next) {
		if (!(bif->bif_flags & IFBIF_STP))
			continue;
		if (bstp_designated_port(sc, bif) &&
		    (bif->bif_state != BSTP_IFSTATE_DISABLED)) {
			bstp_transmit_config(sc, bif);
                } else {
                        STPLOG (("        - <%s>: No Tx\n", bif->ifp->if_xname));
                }
	}
}

int
bstp_designated_port(sc, bif)
	struct bridge_softc *sc;
	struct bridge_iflist *bif;
{
	return ((bif->bif_designated_bridge == sc->sc_bridge_id)
	    && (bif->bif_designated_port == bif->bif_port_id));
}

void
bstp_transmit_tcn(sc)
	struct bridge_softc *sc;
{
	struct bstp_tbpdu bpdu;
	struct bridge_iflist *bif = sc->sc_root_port;
	struct ifnet *ifp = bif->ifp;
	struct arpcom *arp = (struct arpcom *)ifp;
	struct ether_header *eh;
	struct mbuf *m;
	int s, error;

	if ((ifp->if_flags & IFF_RUNNING) == 0)
		return;

	MGETHDR(m, M_DONTWAIT, MT_DATA);
	if (m == NULL)
		return;

        STPLOG (("STP : Tx TCN BPDU on <%s> ...\n", ifp->if_xname));

	m->m_pkthdr.rcvif = ifp;
	m->m_pkthdr.len = sizeof(*eh) + sizeof(bpdu);
	m->m_len = m->m_pkthdr.len;

	eh = mtod(m, struct ether_header *);
	bcopy(arp->ac_enaddr, eh->ether_shost, ETHER_ADDR_LEN);
	bcopy(bstp_etheraddr, eh->ether_dhost, ETHER_ADDR_LEN);
	eh->ether_type = htons(sizeof(bpdu));

	bpdu.tbu_ssap = bpdu.tbu_dsap = LLC_8021D_LSAP;
	bpdu.tbu_ctl = LLC_UI;
	bpdu.tbu_protoid = 0;
	bpdu.tbu_protover = 0;
	bpdu.tbu_bpdutype = BSTP_MSGTYPE_TCN;
	bcopy(&bpdu, m->m_data + sizeof(*eh), sizeof(bpdu));

	s = splimp();
	IFQ_ENQUEUE(&ifp->if_snd, m, NULL, error);
	if (error == 0 && (ifp->if_flags & IFF_OACTIVE) == 0)
		(*ifp->if_start)(ifp);
	m = NULL;

	splx(s);
	if (m != NULL)
		m_freem(m);
}

void
bstp_configuration_update(sc)
	struct bridge_softc *sc;
{
        STP_OPERATION_LOG (("   %s \n", __FUNCTION__));
	bstp_root_selection(sc);
	bstp_designated_port_selection(sc);
}

void
bstp_root_selection(sc)
	struct bridge_softc *sc;
{
	struct bridge_iflist *root_port = NULL, *bif;

        STP_OPERATION_LOG (("   => Root Port selection ...\n", __FUNCTION__));
	LIST_FOREACH(bif, &sc->sc_iflist, next) {
                STP_OPERATION_LOG (("      - <%s>", bif->ifp->if_xname));
		if (!(bif->bif_flags & IFBIF_STP)) {
                        STP_OPERATION_LOG ((" STP not configured\n"));
			continue;
                }
		if (bstp_designated_port(sc, bif)) {
                        STP_OPERATION_LOG ((" dsgntd port\n"));
			continue;
                }
		if (bif->bif_state == BSTP_IFSTATE_DISABLED) {
                        STP_OPERATION_LOG ((" .. in state DISABLED \n"));
			continue;
                }
		if (bif->bif_designated_root >= sc->sc_bridge_id) {
                        STP_OPERATION_LOG ((" root: ifp dsgntd >= sc\n"));
			continue;
                }
		if (root_port == NULL) {
                        STP_OPERATION_LOG ((" Set as Root Port\n"));
			goto set_port;
                }

		if (bif->bif_designated_root < root_port->bif_designated_root) {
                        STP_OPERATION_LOG ((" ifp dsgntd < root dsgntd; Set as Root Port \n"));
			goto set_port;
                }
		if (bif->bif_designated_root > root_port->bif_designated_root) {
                        STP_OPERATION_LOG ((" root: ifp dsgntd > root dsgntd \n"));
			continue;
                }

		if ((bif->bif_designated_cost + bif->bif_path_cost) <
		    (root_port->bif_designated_cost + root_port->bif_path_cost)) {
                        STP_OPERATION_LOG ((" (dsgntd_cost+path_cost): ifp < root; Set as Root Port \n"));
			goto set_port;
                }
		if ((bif->bif_designated_cost + bif->bif_path_cost) >
		    (root_port->bif_designated_cost + root_port->bif_path_cost)) {
                        STP_OPERATION_LOG ((" (dsgntd_cost+path_cost): ifp > root \n"));
			continue;
                }

		if (bif->bif_designated_bridge < root_port->bif_designated_bridge) {
                        STP_OPERATION_LOG ((" bridge: ifp dsgntd < root dsgntd; Set as Root Port \n"));
			goto set_port;
                }
		if (bif->bif_designated_bridge > root_port->bif_designated_bridge) {
                        STP_OPERATION_LOG ((" bridge: ifp dsgntd > root dsgntd \n"));
			continue;
                }

		if (bif->bif_designated_port < root_port->bif_designated_port) {
                        STP_OPERATION_LOG ((" port: ifp dsgntd (%d) < root dsgntd (%d); Set as Root Port\n",
                                 bif->bif_designated_port, root_port->bif_designated_port));
			goto set_port;
                }
		if (bif->bif_designated_port > root_port->bif_designated_port) {
                        STP_OPERATION_LOG ((" port: ifp dsgntd (%d) > root dsgntd (%d)\n",
                                 bif->bif_designated_port, root_port->bif_designated_port));
			continue;
                }

		if (bif->bif_port_id >= root_port->bif_port_id) {
                        STP_OPERATION_LOG ((" ifp port_id (%d) >= root port_id (%d) 8 \n",
                                 bif->bif_port_id, root_port->bif_port_id));
			continue;
                }
set_port:
		root_port = bif;
	}

	sc->sc_root_port = root_port;
	if (root_port == NULL) {
		sc->sc_designated_root = sc->sc_bridge_id;
		sc->sc_root_path_cost = 0;
                STPLOG (("      Root Port : <SELF ROOT>, Path Cost : 0\n"));
	} else {
		sc->sc_designated_root = root_port->bif_designated_root;
		sc->sc_root_path_cost = root_port->bif_designated_cost +
		    root_port->bif_path_cost;
                STPLOG (("      Root Port : <%s>, Path Cost : 0\n", 
                          root_port->ifp->if_xname, sc->sc_root_path_cost));
	}
}

void
bstp_designated_port_selection(sc)
	struct bridge_softc *sc;
{
	struct bridge_iflist *bif;

        STP_OPERATION_LOG (("   => Designated Port selection ...\n"));
	LIST_FOREACH(bif, &sc->sc_iflist, next) {
		if (!(bif->bif_flags & IFBIF_STP))
			continue;
		if (bstp_designated_port(sc, bif)) {
                        STPLOG (("      - <%s>: already designated \n", bif->ifp->if_xname));
			goto designated;
                }
		if (bif->bif_designated_root != sc->sc_designated_root) {
                        STPLOG (("      - <%s>: becomes designated\n", bif->ifp->if_xname));
			goto designated;
                }

                /* 
                 * PLC network is special, we cannot go as per standards because of
                 * our network topology and behavior
                 *
		if (sc->sc_root_path_cost < bif->bif_designated_cost) {
                        STPLOG (("        - <%s>: becomes designated path cost (1)\n", bif->ifp->if_xname));
			goto designated;
                }
		if (sc->sc_root_path_cost > bif->bif_designated_cost) {
                        STPLOG (("        - <%s>: Cannot be designated; Root cost (%d) > Desig cost (%d) \n", 
                                 bif->ifp->if_xname, sc->sc_root_path_cost, bif->bif_designated_cost));
			continue;
                }
                */

		if (sc->sc_bridge_id < bif->bif_designated_bridge) {
                        STPLOG (("      - <%s>: becomes dsgntd (self-brdgid < dsgntd-brdgid); \n", bif->ifp->if_xname));
			goto designated;
                }
		if (sc->sc_bridge_id > bif->bif_designated_bridge) {
                        STPLOG (("      - <%s>: Cannot be dsgntd (self-brdgid > dsgntd-brdgid); \n", bif->ifp->if_xname));
			continue;
                }

		if (bif->bif_port_id > bif->bif_designated_port) {
                        STPLOG (("      - <%s>: Cannot be dsgntd (self-portid > bsgntd-portid); \n", bif->ifp->if_xname));
			continue;
                }

                /* 
                 * PLC network is special, we cannot go as per standards because of
                 * our network topology and behavior
                 */
		if (sc->sc_root_path_cost < bif->bif_designated_cost) {
                        STPLOG (("      - <%s>: becomes dsgntd path cost (1)\n", bif->ifp->if_xname));
			goto designated;
                }
		if (sc->sc_root_path_cost > bif->bif_designated_cost) {
                        STPLOG (("      - <%s>: Cannot be dsgntd; Root cost (%d) > Desig cost (%d) \n", 
                                 bif->ifp->if_xname, sc->sc_root_path_cost, bif->bif_designated_cost));
			continue;
                }
designated:
		bstp_become_designated_port(sc, bif);
	}
}

void
bstp_become_designated_port(sc, bif)
	struct bridge_softc *sc;
	struct bridge_iflist *bif;
{
	bif->bif_designated_root = sc->sc_designated_root;
	bif->bif_designated_cost = sc->sc_root_path_cost;
	bif->bif_designated_bridge = sc->sc_bridge_id;
	bif->bif_designated_port = bif->bif_port_id;

        STP_OPERATION_LOG (("               => %s values (cost:%d, port:0x%x)\n", 
                            __FUNCTION__, bif->ifp->if_xname, bif->bif_designated_cost, bif->bif_designated_port));
        //STP_OPERATION_LOG (("(cost:%d, port:0x%x)\n", bif->bif_designated_cost, bif->bif_designated_port));
}

void
bstp_port_state_selection(sc)
	struct bridge_softc *sc;
{
	struct bridge_iflist *bif;

	LIST_FOREACH(bif, &sc->sc_iflist, next) {
		if (!(bif->bif_flags & IFBIF_STP))
			continue;
		if (bif == sc->sc_root_port) {
			bif->bif_config_pending = 0;
			bif->bif_topology_change_acknowledge = 0;
			bstp_make_forwarding(sc, bif);
		} else if (bstp_designated_port(sc, bif)) {
			bstp_timer_stop(&bif->bif_message_age_timer);
			bstp_make_forwarding(sc, bif);
		} else {
			bif->bif_config_pending = 0;
			bif->bif_topology_change_acknowledge = 0;
			bstp_make_blocking(sc, bif);
		}
	}
}

void
bstp_make_forwarding(sc, bif)
	struct bridge_softc *sc;
	struct bridge_iflist *bif;
{
	if (bif->bif_state == BSTP_IFSTATE_BLOCKING) {
		bstp_set_port_state(bif, BSTP_IFSTATE_LISTENING);
		bstp_timer_start(&bif->bif_forward_delay_timer, 0);
	}
}

void
bstp_make_blocking(sc, bif)
	struct bridge_softc *sc;
	struct bridge_iflist *bif;
{
	if ((bif->bif_state != BSTP_IFSTATE_DISABLED) &&
	    (bif->bif_state != BSTP_IFSTATE_BLOCKING)) {
		if ((bif->bif_state == BSTP_IFSTATE_FORWARDING) ||
		    (bif->bif_state == BSTP_IFSTATE_LEARNING)) {
			if (bif->bif_change_detection_enabled) {
				bstp_topology_change_detection(sc);
			}
			bridge_rtdelete(sc, bif->ifp, 1);
		}
		bstp_set_port_state(bif, BSTP_IFSTATE_BLOCKING);
		bstp_timer_stop(&bif->bif_forward_delay_timer);
	}
}

void
bstp_set_port_state(bif, state)
	struct bridge_iflist *bif;
	u_int8_t state;
{
	bif->bif_state = state;
}

void
bstp_topology_change_detection(sc)
	struct bridge_softc *sc;
{
	if (bstp_root_bridge(sc)) {
                STPLOG (("   -> %s : Root Bridge\n", __FUNCTION__));
		sc->sc_topology_change = 1;
		bstp_timer_start(&sc->sc_topology_change_timer, 0);
	} else if (!sc->sc_topology_change_detected) {
                STPLOG (("   -> %s : NOT Root Bridge, Tx TCN\n", __FUNCTION__));
		bstp_transmit_tcn(sc);
		bstp_timer_start(&sc->sc_tcn_timer, 0);
	}
	sc->sc_topology_change_detected = 1;
}

void
bstp_topology_change_acknowledged(sc)
	struct bridge_softc *sc;
{
	sc->sc_topology_change_detected = 0;
	bstp_timer_stop(&sc->sc_tcn_timer);
}

void
bstp_acknowledge_topology_change(sc, bif)
	struct bridge_softc *sc;
	struct bridge_iflist *bif;
{
	bif->bif_topology_change_acknowledge = 1;
	bstp_transmit_config(sc, bif);
}

struct mbuf *
bstp_input(sc, ifp, eh, m)
	struct bridge_softc *sc;
	struct ifnet *ifp;
	struct ether_header *eh;
	struct mbuf *m;
{
	struct bridge_iflist *bif = NULL;
	struct bstp_tbpdu tpdu;
	struct bstp_cbpdu cpdu;
	struct bstp_config_unit cu;
	struct bstp_tcn_unit tu;
	u_int16_t len;

	LIST_FOREACH(bif, &sc->sc_iflist, next) {
		if (!(bif->bif_flags & IFBIF_STP))
			continue;
		if (bif->ifp == ifp)
			break;
	}

	if (bif == NULL)
		goto out;

	len = ntohs(eh->ether_type);
	if (len < sizeof(tpdu)) {
		goto out;
        }
	if (m->m_pkthdr.len > len)
		m_adj(m, len - m->m_pkthdr.len);
	if ((m = m_pullup(m, sizeof(tpdu))) == NULL) {
		goto out;
        }
	bcopy(mtod(m, struct tpdu *), &tpdu, sizeof(tpdu));

	if (tpdu.tbu_dsap != LLC_8021D_LSAP ||
	    tpdu.tbu_ssap != LLC_8021D_LSAP ||
	    tpdu.tbu_ctl != LLC_UI) {
		goto out;
        }
	if (tpdu.tbu_protoid != 0 || tpdu.tbu_protover != 0) {
		goto out;
         }

	switch (tpdu.tbu_bpdutype) {
	case BSTP_MSGTYPE_TCN:

                STPLOG (("STP : Rx TCN on <%s> from %02X:%02X:%02X:%02X:%02X:%02X \n", bif->ifp->if_xname,
                      eh->ether_shost[0], eh->ether_shost[1], eh->ether_shost[2],
                      eh->ether_shost[3], eh->ether_shost[4], eh->ether_shost[5]));

		tu.tu_message_type = tpdu.tbu_bpdutype;
		bstp_received_tcn_bpdu(sc, bif, &tu);
		break;

	case BSTP_MSGTYPE_CFG:

                STPLOG (("STP : Rx CFG on <%s> from %02X:%02X:%02X:%02X:%02X:%02X \n", bif->ifp->if_xname,
                      eh->ether_shost[0], eh->ether_shost[1], eh->ether_shost[2],
                      eh->ether_shost[3], eh->ether_shost[4], eh->ether_shost[5]));

		if ((m = m_pullup(m, sizeof(cpdu))) == NULL)
			goto out;
		bcopy(mtod(m, struct bstp_cpdu *), &cpdu, sizeof(cpdu));

		cu.cu_rootid =
		    (((u_int64_t)ntohs(cpdu.cbu_rootpri)) << 48) |
		    (((u_int64_t)cpdu.cbu_rootaddr[0]) << 40) |
		    (((u_int64_t)cpdu.cbu_rootaddr[1]) << 32) |
		    (((u_int64_t)cpdu.cbu_rootaddr[2]) << 24) |
		    (((u_int64_t)cpdu.cbu_rootaddr[3]) << 16) |
		    (((u_int64_t)cpdu.cbu_rootaddr[4]) << 8) |
		    (((u_int64_t)cpdu.cbu_rootaddr[5]) << 0);

		cu.cu_bridge_id =
		    (((u_int64_t)ntohs(cpdu.cbu_bridgepri)) << 48) |
		    (((u_int64_t)cpdu.cbu_bridgeaddr[0]) << 40) |
		    (((u_int64_t)cpdu.cbu_bridgeaddr[1]) << 32) |
		    (((u_int64_t)cpdu.cbu_bridgeaddr[2]) << 24) |
		    (((u_int64_t)cpdu.cbu_bridgeaddr[3]) << 16) |
		    (((u_int64_t)cpdu.cbu_bridgeaddr[4]) << 8) |
		    (((u_int64_t)cpdu.cbu_bridgeaddr[5]) << 0);

		cu.cu_root_path_cost = ntohl(cpdu.cbu_rootpathcost);
		cu.cu_message_age = ntohs(cpdu.cbu_messageage);
		cu.cu_max_age = ntohs(cpdu.cbu_maxage);
		cu.cu_hello_time = ntohs(cpdu.cbu_hellotime);
		cu.cu_forward_delay = ntohs(cpdu.cbu_forwarddelay);
		cu.cu_port_id = ntohs(cpdu.cbu_portid);
		cu.cu_message_type = cpdu.cbu_bpdutype;
		cu.cu_topology_change_acknowledgment =
		    (cpdu.cbu_flags & BSTP_FLAG_TCA) ? 1 : 0;
		cu.cu_topology_change =
		    (cpdu.cbu_flags & BSTP_FLAG_TC) ? 1 : 0;
		bstp_received_config_bpdu(sc, bif, &cu);
		break;
	default:
		goto out;
	}

out:
	if (m)
		m_freem(m);
	return (NULL);
}

void
bstp_received_config_bpdu(sc, bif, cu)
	struct bridge_softc *sc;
	struct bridge_iflist *bif;
	struct bstp_config_unit *cu;
{
	int root;

	root = bstp_root_bridge(sc);

	if (bif->bif_state != BSTP_IFSTATE_DISABLED) {
		if (bstp_supersedes_port_info(sc, bif, cu)) {
                        STPLOG (("   Rx CFG on <%s> superseds port info; RE-Evaluating...\n", bif->ifp->if_xname));
			bstp_record_config_information(sc, bif, cu);
			bstp_configuration_update(sc);
			bstp_port_state_selection(sc);

			if ((!bstp_root_bridge(sc)) && root) {
				bstp_timer_stop(&sc->sc_hello_timer);

				if (sc->sc_topology_change_detected) {
					bstp_timer_stop(&sc->sc_topology_change_timer);
					bstp_transmit_tcn(sc);
					bstp_timer_start(&sc->sc_tcn_timer, 0);
				}
			}

			if (bif == sc->sc_root_port) {
				bstp_record_config_timeout_values(sc, cu);
				bstp_config_bpdu_generation(sc);

				if (cu->cu_topology_change_acknowledgment) {
					bstp_topology_change_acknowledged(sc);
                                }
			}
		} else if (bstp_designated_port(sc, bif)) {
                        STPLOG (("   Port <%s> supersedes Rx CFG info\n", bif->ifp->if_xname));
			bstp_transmit_config(sc, bif);
                }
	}
}

void
bstp_received_tcn_bpdu(sc, bif, tcn)
	struct bridge_softc *sc;
	struct bridge_iflist *bif;
	struct bstp_tcn_unit *tcn;
{
	if (bif->bif_state != BSTP_IFSTATE_DISABLED &&
	    bstp_designated_port(sc, bif)) {
		bstp_topology_change_detection(sc);
		bstp_acknowledge_topology_change(sc, bif);
	}
}

void
bstp_hello_timer_expiry(sc)
	struct bridge_softc *sc;
{
	bstp_config_bpdu_generation(sc);
	bstp_timer_start(&sc->sc_hello_timer, 0);
}

void
bstp_message_age_timer_expiry(sc, bif)
	struct bridge_softc *sc;
	struct bridge_iflist *bif;
{
	int root;

	root = bstp_root_bridge(sc);
	bstp_become_designated_port(sc, bif);
	bstp_configuration_update(sc);
	bstp_port_state_selection(sc);

	if ((bstp_root_bridge(sc)) && (!root)) {
		sc->sc_max_age = sc->sc_bridge_max_age;
		sc->sc_hello_time = sc->sc_bridge_hello_time;
		sc->sc_forward_delay = sc->sc_bridge_forward_delay;
		bstp_topology_change_detection(sc);
		bstp_timer_stop(&sc->sc_tcn_timer);
		bstp_config_bpdu_generation(sc);
		bstp_timer_start(&sc->sc_hello_timer, 0);
	}
}

void
bstp_forward_delay_timer_expiry(sc, bif)
	struct bridge_softc *sc;
	struct bridge_iflist *bif;
{
	if (bif->bif_state == BSTP_IFSTATE_LISTENING) {
		bstp_set_port_state(bif, BSTP_IFSTATE_LEARNING);
		bstp_timer_start(&bif->bif_forward_delay_timer, 0);
	} else if (bif->bif_state == BSTP_IFSTATE_LEARNING) {
		bstp_set_port_state(bif, BSTP_IFSTATE_FORWARDING);
		if (bstp_designated_for_some_port(sc) &&
		    bif->bif_change_detection_enabled)
			bstp_topology_change_detection(sc);
	}
}

int
bstp_designated_for_some_port(sc)
	struct bridge_softc *sc;
{

	struct bridge_iflist *bif;

	LIST_FOREACH(bif, &sc->sc_iflist, next) {
		if (!(bif->bif_flags & IFBIF_STP))
			continue;
		if (bif->bif_designated_bridge == sc->sc_bridge_id)
			return (1);
	}
	return (0);
}

void
bstp_tcn_timer_expiry(sc)
	struct bridge_softc *sc;
{
        STPLOG (("  STP: tcn timer expired \n"));
	bstp_transmit_tcn(sc);
	bstp_timer_start(&sc->sc_tcn_timer, 0);
}

void
bstp_topology_change_timer_expiry(sc)
	struct bridge_softc *sc;
{
        STPLOG (("  STP: topology change timer expired \n"));
	sc->sc_topology_change_detected = 0;
	sc->sc_topology_change = 0;
}

void
bstp_hold_timer_expiry(sc, bif)
	struct bridge_softc *sc;
	struct bridge_iflist *bif;
{
	if (bif->bif_config_pending)
		bstp_transmit_config(sc, bif);
}

void
bstp_initialization(sc)
	struct bridge_softc *sc;
{
	struct bridge_iflist *bif, *mif;
	struct arpcom *ac, *mac;

	mif = NULL; mac = NULL;
        /* Browse through the ethernet address of each interface and pick the
         * one which has lowest value
         */
	LIST_FOREACH(bif, &sc->sc_iflist, next) {
		if (!(bif->bif_flags & IFBIF_STP)) {
			continue;
                }
		if (bif->ifp->if_type != IFT_ETHER) {
			continue;
                }
		bif->bif_port_id = (bif->bif_priority << 8) |
		    (bif->ifp->if_index & 0xff);

		if (mif == NULL) {
			mif = bif;
			mac = (struct arpcom *)bif->ifp;
			continue;
		}
		ac = (struct arpcom *)bif->ifp;
		if (memcmp(ac->ac_enaddr, mac->ac_enaddr, ETHER_ADDR_LEN) < 0) {
			mif = bif;
			mac = (struct arpcom *)bif->ifp;
			continue;
		}
	}

	if (mif == NULL) {
		bstp_stop(sc);
		return;
	}

        /* Configure bridge-id as specified in standards
         */
	sc->sc_bridge_id =
	    (((u_int64_t)sc->sc_bridge_priority) << 48) |
	    (((u_int64_t)mac->ac_enaddr[0]) << 40) |
	    (((u_int64_t)mac->ac_enaddr[1]) << 32) |
	    (mac->ac_enaddr[2] << 24) | (mac->ac_enaddr[3] << 16) |
	    (mac->ac_enaddr[4] << 8) | (mac->ac_enaddr[5]);

        /* Configure self as root bridge
         */
	sc->sc_designated_root = sc->sc_bridge_id;
	sc->sc_root_path_cost = 0;
	sc->sc_root_port = NULL;

	sc->sc_max_age = sc->sc_bridge_max_age;
	sc->sc_hello_time = sc->sc_bridge_hello_time;
	sc->sc_forward_delay = sc->sc_bridge_forward_delay;
	sc->sc_topology_change_detected = 0;
	sc->sc_topology_change = 0;
	bstp_timer_stop(&sc->sc_tcn_timer);
	bstp_timer_stop(&sc->sc_topology_change_timer);

        /* If there is a timeout already set on this, cancel it and restart.
         * The intent is to avoid setting duplicate timeouts. If the timeout
         * is already set, untimeout would cancel it.
         */
#ifdef __ECOS
        if (!bstp_init_done) {
          untimeout(bstp_tick, sc);
          timeout(bstp_tick, sc, hz);
          bstp_init_done = 1;
        }
#else
	if (!timeout_initialized(&sc->sc_bstptimeout))
		timeout_set(&sc->sc_bstptimeout, bstp_tick, sc);
	if (!timeout_pending(&sc->sc_bstptimeout))
		timeout_add(&sc->sc_bstptimeout, hz);
#endif // __ECOS

	LIST_FOREACH(bif, &sc->sc_iflist, next) {
		if (bif->bif_flags & IFBIF_STP) {
                        STPLOG (("%s:%s ..(en). \n", __FUNCTION__, bif->ifp->if_xname));
			bstp_enable_port(sc, bif);
                }
		else {
                        STPLOG (("%s:%s ..(dis). \n", __FUNCTION__, bif->ifp->if_xname));
			bstp_disable_port(sc, bif);
                }
	}

	bstp_port_state_selection(sc);
	bstp_config_bpdu_generation(sc);
	bstp_timer_start(&sc->sc_hello_timer, 0);
}

void
bstp_stop(sc)
	struct bridge_softc *sc;
{

	struct bridge_iflist *bif;

        STPLOG (("        %s\n", __FUNCTION__));
	LIST_FOREACH(bif, &sc->sc_iflist, next) {
		bstp_set_port_state(bif, BSTP_IFSTATE_DISABLED);
		bstp_timer_stop(&bif->bif_hold_timer);
		bstp_timer_stop(&bif->bif_message_age_timer);
		bstp_timer_stop(&bif->bif_forward_delay_timer);
	}

#ifdef __ECOS
        if (bstp_init_done) {
          STPLOG (("##### untimeout ##### loc 1\n"));
          untimeout(bstp_tick, sc);
          bstp_init_done = 0;
        }
#else
	if (timeout_initialized(&sc->sc_bstptimeout) &&
	    timeout_pending(&sc->sc_bstptimeout))
		timeout_del(&sc->sc_bstptimeout);
#endif   //__ECOS

	bstp_timer_stop(&sc->sc_topology_change_timer);
	bstp_timer_stop(&sc->sc_tcn_timer);
	bstp_timer_stop(&sc->sc_hello_timer);

}

void
bstp_initialize_port(sc, bif)
	struct bridge_softc *sc;
	struct bridge_iflist *bif;
{
	bstp_become_designated_port(sc, bif);
	bstp_set_port_state(bif, BSTP_IFSTATE_BLOCKING);
	bif->bif_topology_change_acknowledge = 0;
	bif->bif_config_pending = 0;
	bstp_enable_change_detection(bif);
	bstp_timer_stop(&bif->bif_message_age_timer);
	bstp_timer_stop(&bif->bif_forward_delay_timer);
	bstp_timer_stop(&bif->bif_hold_timer);
}

void
bstp_enable_port(sc, bif)
	struct bridge_softc *sc;
	struct bridge_iflist *bif;
{
	bstp_initialize_port(sc, bif);
	bstp_port_state_selection(sc);
}

void
bstp_disable_port(sc, bif)
	struct bridge_softc *sc;
	struct bridge_iflist *bif;
{
	int root;

	root = bstp_root_bridge(sc);
	bstp_become_designated_port(sc, bif);
	bstp_set_port_state(bif, BSTP_IFSTATE_DISABLED);
	bif->bif_topology_change_acknowledge = 0;
	bif->bif_config_pending = 0;
	bstp_timer_stop(&bif->bif_message_age_timer);
	bstp_timer_stop(&bif->bif_forward_delay_timer);
	bstp_configuration_update(sc);
	bridge_rtdelete(sc, bif->ifp, 1);

	if (bstp_root_bridge(sc) && (!root))	{
		sc->sc_max_age = sc->sc_bridge_max_age;
		sc->sc_hello_time = sc->sc_bridge_hello_time;
		sc->sc_forward_delay = sc->sc_bridge_forward_delay;
		bstp_topology_change_detection(sc);
		bstp_timer_stop(&sc->sc_tcn_timer);
		bstp_config_bpdu_generation(sc);
		bstp_timer_start(&sc->sc_hello_timer, 0);
	}
}

void
bstp_set_bridge_priority(sc, new_bridge_id)
	struct bridge_softc *sc;
	u_int64_t new_bridge_id;
{
	int root;
	struct bridge_iflist *bif;

	root = bstp_root_bridge(sc);

	LIST_FOREACH(bif, &sc->sc_iflist, next) {
		if (!(bif->bif_flags & IFBIF_STP))
			continue;
		if (bstp_designated_port(sc, bif))
			bif->bif_designated_bridge = new_bridge_id;
	}

	sc->sc_bridge_id = new_bridge_id;

	bstp_configuration_update(sc);
	bstp_port_state_selection(sc);

	if (bstp_root_bridge(sc) && (!root)) {
		sc->sc_max_age = sc->sc_bridge_max_age;
		sc->sc_hello_time = sc->sc_bridge_hello_time;
		sc->sc_forward_delay = sc->sc_bridge_forward_delay;
		bstp_topology_change_detection(sc);
		bstp_timer_stop(&sc->sc_tcn_timer);
		bstp_config_bpdu_generation(sc);
		bstp_timer_start(&sc->sc_hello_timer, 0);
	}
}

void
bstp_set_port_priority(sc, bif, new_port_id)
	struct bridge_softc *sc;
	struct bridge_iflist *bif;
	u_int16_t new_port_id;
{
	if (bstp_designated_port(sc, bif))
		bif->bif_designated_port = new_port_id;

	bif->bif_port_id = new_port_id;

	if ((sc->sc_bridge_id == bif->bif_designated_bridge) &&
	    (bif->bif_port_id < bif->bif_designated_port)) {
		bstp_become_designated_port(sc, bif);
		bstp_port_state_selection(sc);
	}
}

void
bstp_set_path_cost(sc, bif, path_cost)
	struct bridge_softc *sc;
	struct bridge_iflist *bif;
	u_int32_t path_cost;
{
	bif->bif_path_cost = path_cost;
	bstp_configuration_update(sc);
	bstp_port_state_selection(sc);
}

void
bstp_enable_change_detection(bif)
	struct bridge_iflist *bif;
{
	bif->bif_change_detection_enabled = 1;
}

void
bstp_disable_change_detection(bif)
	struct bridge_iflist *bif;
{
	bif->bif_change_detection_enabled = 0;
}

void
bstp_ifupdstatus(sc, bif)
	struct bridge_softc *sc;
	struct bridge_iflist *bif;
{
	struct ifnet *ifp = bif->ifp;
#ifndef __ECOS
	struct ifmediareq ifmr;
#endif
	int err = EOPNOTSUPP;

	if (ifp->if_flags & IFF_UP) {
#ifndef __ECOS
		ifmr.ifm_count = 0;
		err = (*ifp->if_ioctl)(ifp, SIOCGIFMEDIA, (caddr_t)&ifmr);
#endif
		if (err) {
			if (bif->bif_state == BSTP_IFSTATE_DISABLED)
				bstp_enable_port(sc, bif);
                        STPLOG (("<%s:%s> ", bif->ifp->if_xname, stp_port_state(bif->bif_state)));
			return;
		}
#ifndef __ECOS
		if (!(ifmr.ifm_status & IFM_AVALID)) {
			if (bif->bif_state == BSTP_IFSTATE_DISABLED)
				bstp_enable_port(sc, bif);
			return;
		}

		if (ifmr.ifm_status & IFM_ACTIVE) {
			if (bif->bif_state == BSTP_IFSTATE_DISABLED)
				bstp_enable_port(sc, bif);
			return;
		}
#endif

		if (bif->bif_state != BSTP_IFSTATE_DISABLED) {
			bstp_disable_port(sc, bif);
                }

		return;
	}

	if (bif->bif_state != BSTP_IFSTATE_DISABLED)
		bstp_disable_port(sc, bif);
}

void
bstp_tick(vsc)
	void *vsc;
{
	struct bridge_softc *sc = vsc;
	struct bridge_iflist *bif;
	int s;

	s = splnet();

        STPLOG (("Port Status: ")); 
	LIST_FOREACH(bif, &sc->sc_iflist, next) {
		if (!(bif->bif_flags & IFBIF_STP))
			continue;
		bstp_ifupdstatus(sc, bif);
	}
        STPLOG (("\n"));

	if (bstp_timer_expired(&sc->sc_hello_timer, sc->sc_hello_time))
		bstp_hello_timer_expiry(sc);

	if (bstp_timer_expired(&sc->sc_tcn_timer, sc->sc_bridge_hello_time))
		bstp_tcn_timer_expiry(sc);

	if (bstp_timer_expired(&sc->sc_topology_change_timer,
                               sc->sc_topology_change_time))
                bstp_topology_change_timer_expiry(sc);

	LIST_FOREACH(bif, &sc->sc_iflist, next) {
		if (!(bif->bif_flags & IFBIF_STP))
			continue;
		if (bstp_timer_expired(&bif->bif_message_age_timer,
		    sc->sc_max_age))
			bstp_message_age_timer_expiry(sc, bif);
	}

	LIST_FOREACH(bif, &sc->sc_iflist, next) {
		if (!(bif->bif_flags & IFBIF_STP))
			continue;
		if (bstp_timer_expired(&bif->bif_forward_delay_timer,
		    sc->sc_forward_delay))
			bstp_forward_delay_timer_expiry(sc, bif);

		if (bstp_timer_expired(&bif->bif_hold_timer,
		    sc->sc_hold_time))
			bstp_hold_timer_expiry(sc, bif);
	}

#ifdef __ECOS
	if (sc->sc_if.if_flags & IFF_RUNNING) {
                timeout (bstp_tick, sc, hz);
        }
#else
        if (sc->sc_if.if_flags & IFF_RUNNING)
             timeout_add(&sc->sc_bstptimeout, hz);
#endif   //__ECOS

	splx(s);
}

void
bstp_timer_start(t, v)
	struct bridge_timer *t;
	u_int16_t v;
{
	t->value = v;
	t->active = 1;
}

void
bstp_timer_stop(t)
	struct bridge_timer *t;
{
	t->value = 0;
	t->active = 0;
}

int
bstp_timer_expired(t, v)
	struct bridge_timer *t;
	u_int16_t v;
{
	if (!t->active)
		return (0);
	t->value += BSTP_TICK_VAL;
	if (t->value >= v) {
		bstp_timer_stop(t);
		return (1);
	}
	return (0);

}

int
bstp_ioctl(ifp, cmd, data)
	struct ifnet *ifp;
	u_long cmd;
	caddr_t	data;
{
	struct ifbrparam *bp = (struct ifbrparam *)data;
	struct bridge_softc *sc = (struct bridge_softc *)ifp;
	int r = 0, err = 0;

	switch (cmd) {
	case SIOCBRDGGPRI:
		bp->ifbrp_prio = sc->sc_bridge_priority;
		break;
	case SIOCBRDGGMA:
		bp->ifbrp_maxage = sc->sc_bridge_max_age >> 8;
		break;
	case SIOCBRDGGHT:
		bp->ifbrp_hellotime = sc->sc_bridge_hello_time >> 8;
		break;
	case SIOCBRDGGFD:
		bp->ifbrp_fwddelay = sc->sc_bridge_forward_delay >> 8;
		break;
	case SIOCBRDGSPRI:
		sc->sc_bridge_priority = bp->ifbrp_prio;
		r = 1;
		break;
	case SIOCBRDGSMA:
		if (bp->ifbrp_maxage == 0) {
			err = EINVAL;
			break;
		}
		sc->sc_bridge_max_age = bp->ifbrp_maxage << 8;
		r = 1;
		break;
	case SIOCBRDGSHT:
		if (bp->ifbrp_hellotime == 0) {
			err = EINVAL;
			break;
		}
		sc->sc_bridge_hello_time = bp->ifbrp_hellotime << 8;
		r = 1;
		break;
	case SIOCBRDGSFD:
		if (bp->ifbrp_fwddelay == 0) {
			err = EINVAL;
			break;
		}
		sc->sc_bridge_forward_delay = bp->ifbrp_fwddelay << 8;
		r = 1;
		break;
	case SIOCBRDGSIFCOST:
	case SIOCBRDGSIFPRIO:
	case SIOCBRDGSIFFLGS:
	case SIOCBRDGADD:
	case SIOCBRDGDEL:
                r = 1;
		break;
	default:
		break;
	}

	if (r)
		bstp_initialization(sc);

	return (err);
}

#endif   /* NBRIDGE */
#endif   /* CYGPKG_NET_BRIDGE_STP_CODE */
