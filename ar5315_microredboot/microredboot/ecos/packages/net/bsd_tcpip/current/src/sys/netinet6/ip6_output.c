//==========================================================================
//
//      src/sys/netinet6/ip6_output.c
//
//==========================================================================
//####BSDCOPYRIGHTBEGIN####
//
// -------------------------------------------
//
// Portions of this software may have been derived from OpenBSD, 
// FreeBSD or other sources, and are covered by the appropriate
// copyright disclaimers included herein.
//
// Portions created by Red Hat are
// Copyright (C) 2002 Red Hat, Inc. All Rights Reserved.
//
// -------------------------------------------
//
//####BSDCOPYRIGHTEND####
//==========================================================================

/*	$KAME: ip6_output.c,v 1.272 2001/12/26 01:03:28 jinmei Exp $	*/

/*
 * Copyright (C) 1995, 1996, 1997, and 1998 WIDE Project.
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
 * 3. Neither the name of the project nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE PROJECT AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE PROJECT OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

/*
 * Copyright (c) 1982, 1986, 1988, 1990, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *	@(#)ip_output.c	8.3 (Berkeley) 1/21/94
 */

#include <sys/param.h>
#include <sys/malloc.h>
#include <sys/mbuf.h>
#include <sys/errno.h>
#include <sys/protosw.h>
#include <sys/socket.h>
#include <sys/socketvar.h>

#include <net/if.h>
#include <net/route.h>

#include <netinet/in.h>
#include <netinet/in_var.h>
#include <netinet6/in6_var.h>
#include <netinet/ip6.h>
#include <netinet/icmp6.h>
#include <netinet6/ip6_var.h>
#include <netinet/in_pcb.h>
#include <netinet6/nd6.h>
#include <netinet6/ip6protosw.h>
#include <netinet6/scope6_var.h>

#ifdef IPSEC
#ifdef __OpenBSD__
#include <netinet/ip_ah.h>
#include <netinet/ip_esp.h>
#include <netinet/udp.h>
#include <netinet/tcp.h>
#include <net/pfkeyv2.h>

extern u_int8_t get_sa_require  __P((struct inpcb *));

extern int ipsec_auth_default_level;
extern int ipsec_esp_trans_default_level;
extern int ipsec_esp_network_default_level;
extern int ipsec_ipcomp_default_level;
#else
#include <netinet6/ipsec.h>
#include <netkey/key.h>
#endif
#endif /* IPSEC */

#if defined(IPV6FIREWALL) || (defined(__FreeBSD__) && __FreeBSD__ >= 4)
#include <netinet6/ip6_fw.h>
#endif

#ifdef MIP6
#include <sys/syslog.h>
#include <netinet6/mip6.h>
#endif /* MIP6 */

struct ip6_exthdrs {
	struct mbuf *ip6e_ip6;
	struct mbuf *ip6e_hbh;
	struct mbuf *ip6e_dest1;
	struct mbuf *ip6e_rthdr;
	struct mbuf *ip6e_haddr; /* for MIP6 */
	struct mbuf *ip6e_dest2;
};

static int ip6_pcbopt __P((int, u_char *, int, struct ip6_pktopts **, int));
#if defined(__FreeBSD__) && __FreeBSD__ >= 3
static int ip6_pcbopts __P((struct ip6_pktopts **, struct mbuf *,
			    struct socket *, struct sockopt *));
static int ip6_getpcbopt __P((struct ip6_pktopts *, int, struct sockopt *));
#else
static int ip6_pcbopts __P((struct ip6_pktopts **, struct mbuf *,
			    struct socket *));
static int ip6_getpcbopt __P((struct ip6_pktopts *, int, struct mbuf **));
#endif
static int ip6_setpktoption __P((int, u_char *, int, struct ip6_pktopts *, int,
				 int, int));
static int ip6_setmoptions __P((int, struct ip6_moptions **, struct mbuf *));
static int ip6_getmoptions __P((int, struct ip6_moptions *, struct mbuf **));
static int ip6_copyexthdr __P((struct mbuf **, caddr_t, int));
static int ip6_insertfraghdr __P((struct mbuf *, struct mbuf *, int,
				  struct ip6_frag **));
static int ip6_insert_jumboopt __P((struct ip6_exthdrs *, u_int32_t));
static int ip6_splithdr __P((struct mbuf *, struct ip6_exthdrs *));
#ifdef NEW_STRUCT_ROUTE
static int ip6_getpmtu __P((struct route *, struct route *, struct ifnet *,
			    struct in6_addr *, u_long *));
#else
static int ip6_getpmtu __P((struct route *, struct route *, struct ifnet *,
			    struct in6_addr *, u_long *));
#endif

#ifdef __bsdi__
#if _BSDI_VERSION < 199802
extern struct ifnet loif;
#else
extern struct ifnet *loifp;
#endif
#endif
#if defined(__NetBSD__)
extern struct ifnet loif[NLOOP];
#endif


/*
 * IP6 output. The packet in mbuf chain m contains a skeletal IP6
 * header (with pri, len, nxt, hlim, src, dst).
 * This function may modify ver and hlim only.
 * The mbuf chain containing the packet will be freed.
 * The mbuf opt, if present, will not be freed.
 *
 * type of "mtu": rt_rmx.rmx_mtu is u_long, ifnet.ifr_mtu is int, and
 * nd_ifinfo.linkmtu is u_int32_t.  so we use u_long to hold largest one,
 * which is rt_rmx.rmx_mtu.
 */
int
ip6_output(m0, opt, ro, flags, im6o, ifpp)
	struct mbuf *m0;
	struct ip6_pktopts *opt;
#ifdef NEW_STRUCT_ROUTE
	struct route *ro;
#else
	struct route_in6 *ro;
#endif
	int flags;
	struct ip6_moptions *im6o;
	struct ifnet **ifpp;		/* XXX: just for statistics */
{
	struct ip6_hdr *ip6, *mhip6;
	struct ifnet *ifp, *origifp;
	struct mbuf *m = m0;
	int hlen, tlen, len, off;
#ifdef NEW_STRUCT_ROUTE
	struct route ip6route;
#else
	struct route_in6 ip6route;
#endif
	struct rtentry *rt = NULL;
	struct sockaddr_in6 *dst;
	int error = 0;
	struct in6_ifaddr *ia = NULL;
	u_long mtu;
	u_int32_t optlen = 0, plen = 0, unfragpartlen = 0;
	struct ip6_exthdrs exthdrs;
	struct in6_addr finaldst;
#ifdef NEW_STRUCT_ROUTE
	struct route *ro_pmtu = NULL;
#else
	struct route_in6 *ro_pmtu = NULL;
#endif
	int hdrsplit = 0;
#ifdef __OpenBSD__
	u_int8_t sproto = 0;
#else
	int needipsec = 0;
#endif
#if defined(__NetBSD__) && defined(PFIL_HOOKS)
	struct packet_filter_hook *pfh;
	struct mbuf *m1;
	int rv;
#endif /* PFIL_HOOKS */
#if defined(__bsdi__) && _BSDI_VERSION < 199802
	struct ifnet *loifp = &loif;
#endif
#ifdef MIP6
	struct mip6_pktopts mip6opt;
#ifdef NEW_STRUCT_ROUTE
	struct route mip6_ip6route;
#else
	struct route_in6 mip6_ip6route;
#endif
#endif /* MIP6 */
#ifdef IPSEC
#ifdef __OpenBSD__
	struct m_tag *mtag;
	union sockaddr_union sdst;
	struct tdb_ident *tdbi;
	u_int32_t sspi;
	struct inpcb *inp;
	struct tdb *tdb;
	int s;

	inp = NULL;     /* XXX */
	if (inp && (inp->inp_flags & INP_IPV6) == 0)
		panic("ip6_output: IPv4 pcb is passed");
#else
	int needipsectun = 0;
	struct socket *so;
	struct secpolicy *sp = NULL;

	/* for AH processing. stupid to have "socket" variable in IP layer... */
	so = ipsec_getsocket(m);
	(void)ipsec_setsocket(m, NULL);
	ip6 = mtod(m, struct ip6_hdr *);
#endif
#endif /* IPSEC */

#define MAKE_EXTHDR(hp, mp)						\
    do {								\
	if (hp) {							\
		struct ip6_ext *eh = (struct ip6_ext *)(hp);		\
		error = ip6_copyexthdr((mp), (caddr_t)(hp), 		\
				       ((eh)->ip6e_len + 1) << 3);	\
		if (error)						\
			goto freehdrs;					\
	}								\
    } while (0)
	
	bzero(&exthdrs, sizeof(exthdrs));

	if (opt) {
		/* Hop-by-Hop options header */
		MAKE_EXTHDR(opt->ip6po_hbh, &exthdrs.ip6e_hbh);
		/* Destination options header(1st part) */
		if (opt->ip6po_rthdr) {
			/*
			 * Destination options header(1st part)
			 * This only makes sence with a routing header.
			 * See Section 9.2 of
			 * draft-ietf-ipngwg-rfc2292bis-02.txt.
			 * Disabling this part just for MIP6 convenience is
			 * a bad idea.  We need to think carefully about a
			 * way to make the advanced API coexist with MIP6
			 * options, which might automatically be inserted in
			 * the kernel.
			 */
			MAKE_EXTHDR(opt->ip6po_dest1, &exthdrs.ip6e_dest1);
		}
		/* Routing header */
		MAKE_EXTHDR(opt->ip6po_rthdr, &exthdrs.ip6e_rthdr);
		/* Destination options header(2nd part) */
		MAKE_EXTHDR(opt->ip6po_dest2, &exthdrs.ip6e_dest2);
	}
#ifdef MIP6
	bzero((caddr_t)&mip6opt, sizeof(mip6opt));
	if ((flags & IPV6_FORWARDING) == 0) {
		/*
		 * XXX: reconsider the following routine.
		 */
		/*
		 * MIP6 extention headers handling.
		 * insert HA, BU, BA, BR options if necessary.
		 */
		if (mip6_exthdr_create(m, opt, &mip6opt))
			goto freehdrs;

		if (((opt != NULL) && (opt->ip6po_rthdr != NULL))
		    || (mip6opt.mip6po_rthdr != NULL)) {
			m_freem(exthdrs.ip6e_rthdr);
			if (mip6opt.mip6po_rthdr != NULL) {
				/*
				 * there is no rthdr specified in the
				 * ip6_pktopts.  but mip6 create a
				 * rthdr for the router optimization
				 * purpose.
				 */
				MAKE_EXTHDR(mip6opt.mip6po_rthdr,
					    &exthdrs.ip6e_rthdr);
			} else {
				/*
				 * there is a rthdr specified in the
				 * ip6_pktopts.  if mip6 require the
				 * route optimization, the rthdr for
				 * that purpose is already included in
				 * the ip6po_rthdr in the
				 * mip6_destopt_create().
				 */
				MAKE_EXTHDR(opt->ip6po_rthdr,
					    &exthdrs.ip6e_rthdr);
			}
			/*
			 * if a routing header exists dest1 must be
			 * inserted if it exists.
			 */
			if ((opt != NULL) && (opt->ip6po_dest1)) {
				m_freem(exthdrs.ip6e_dest1);
				MAKE_EXTHDR(opt->ip6po_dest1,
					    &exthdrs.ip6e_dest1);
			}
		}
		MAKE_EXTHDR(mip6opt.mip6po_haddr, &exthdrs.ip6e_haddr);
		if (mip6opt.mip6po_dest2) {
			m_freem(exthdrs.ip6e_dest2);
			MAKE_EXTHDR(mip6opt.mip6po_dest2, &exthdrs.ip6e_dest2);
		}
	} else {
		/*
		 * this is the forwarding packet.  do not modify any
		 * extension headers.
		 */
	}
#endif /* MIP6 */

#ifdef IPSEC
#ifdef __OpenBSD__
	/*
	 * splnet is chosen over spltdb because we are not allowed to
	 * lower the level, and udp6_output calls us in splnet(). XXX check
	 */
	s = splnet();

	/*
	 * Check if there was an outgoing SA bound to the flow
	 * from a transport protocol.
	 */
	ip6 = mtod(m, struct ip6_hdr *);

	/* Do we have any pending SAs to apply ? */
	mtag = m_tag_find(m, PACKET_TAG_IPSEC_PENDING_TDB, NULL);
	if (mtag != NULL) {
#ifdef DIAGNOSTIC
		if (mtag->m_tag_len != sizeof (struct tdb_ident))
			panic("ip6_output: tag of length %d (should be %d",
			    mtag->m_tag_len, sizeof (struct tdb_ident));
#endif
		tdbi = (struct tdb_ident *)(mtag + 1);
		tdb = gettdb(tdbi->spi, &tdbi->dst, tdbi->proto);
		if (tdb == NULL)
			error = -EINVAL;
		m_tag_delete(m, mtag);
	}
	else
		tdb = ipsp_spd_lookup(m, AF_INET6, sizeof(struct ip6_hdr),
		    &error, IPSP_DIRECTION_OUT, NULL, inp);

	if (tdb == NULL) {
	        splx(s);

		if (error == 0) {
		        /*
			 * No IPsec processing required, we'll just send the
			 * packet out.
			 */
		        sproto = 0;

			/* Fall through to routing/multicast handling */
		} else {
		        /*
			 * -EINVAL is used to indicate that the packet should
			 * be silently dropped, typically because we've asked
			 * key management for an SA.
			 */
		        if (error == -EINVAL) /* Should silently drop packet */
				error = 0;

			goto freehdrs;
		}
	} else {
		/* Loop detection */
		for (mtag = m_tag_first(m); mtag != NULL;
		    mtag = m_tag_next(m, mtag)) {
			if (mtag->m_tag_id != PACKET_TAG_IPSEC_OUT_DONE &&
			    mtag->m_tag_id !=
			    PACKET_TAG_IPSEC_OUT_CRYPTO_NEEDED)
				continue;
			tdbi = (struct tdb_ident *)(mtag + 1);
			if (tdbi->spi == tdb->tdb_spi &&
			    tdbi->proto == tdb->tdb_sproto &&
			    !bcmp(&tdbi->dst, &tdb->tdb_dst,
			    sizeof(union sockaddr_union))) {
				splx(s);
				sproto = 0; /* mark as no-IPsec-needed */
				goto done_spd;
			}
		}

	        /* We need to do IPsec */
	        bcopy(&tdb->tdb_dst, &sdst, sizeof(sdst));
		sspi = tdb->tdb_spi;
		sproto = tdb->tdb_sproto;
	        splx(s);

#if 1 /* XXX */
		/* if we have any extension header, we cannot perform IPsec */
		if (exthdrs.ip6e_hbh || exthdrs.ip6e_dest1 ||
#ifdef MIP6
		    exthdrs.ip6e_haddr ||
#endif /* MIP6 */
		    exthdrs.ip6e_rthdr || exthdrs.ip6e_dest2) {
			error = EHOSTUNREACH;
			goto freehdrs;
		}
#endif
	}

	/* Fall through to the routing/multicast handling code */
 done_spd:
#else
	/* get a security policy for this packet */
	if (so == NULL)
		sp = ipsec6_getpolicybyaddr(m, IPSEC_DIR_OUTBOUND, 0, &error);
	else
		sp = ipsec6_getpolicybysock(m, IPSEC_DIR_OUTBOUND, so, &error);

	if (sp == NULL) {
		ipsec6stat.out_inval++;
		goto freehdrs;
	}

	error = 0;

	/* check policy */
	switch (sp->policy) {
	case IPSEC_POLICY_DISCARD:
		/*
		 * This packet is just discarded.
		 */
		ipsec6stat.out_polvio++;
		goto freehdrs;

	case IPSEC_POLICY_BYPASS:
	case IPSEC_POLICY_NONE:
		/* no need to do IPsec. */
		needipsec = 0;
		break;
	
	case IPSEC_POLICY_IPSEC:
		if (sp->req == NULL) {
			/* acquire a policy */
			error = key_spdacquire(sp);
			goto freehdrs;
		}
		needipsec = 1;
		break;

	case IPSEC_POLICY_ENTRUST:
	default:
		printf("ip6_output: Invalid policy found. %d\n", sp->policy);
	}
#endif /* OpenBSD */
#endif /* IPSEC */

	/*
	 * Calculate the total length of the extension header chain.
	 * Keep the length of the unfragmentable part for fragmentation.
	 */
	optlen = 0;
	if (exthdrs.ip6e_hbh) optlen += exthdrs.ip6e_hbh->m_len;
	if (exthdrs.ip6e_dest1) optlen += exthdrs.ip6e_dest1->m_len;
	if (exthdrs.ip6e_rthdr) optlen += exthdrs.ip6e_rthdr->m_len;
#ifdef MIP6
	if (exthdrs.ip6e_haddr) optlen += exthdrs.ip6e_haddr->m_len;
#endif /* MIP6 */
	unfragpartlen = optlen + sizeof(struct ip6_hdr);
	/* NOTE: we don't add AH/ESP length here. do that later. */
	if (exthdrs.ip6e_dest2) optlen += exthdrs.ip6e_dest2->m_len;

	/*
	 * If we need IPsec, or there is at least one extension header,
	 * separate IP6 header from the payload.
	 */
#ifdef __OpenBSD__
	if ((sproto || optlen) && !hdrsplit)
#else
	if ((needipsec || optlen) && !hdrsplit)
#endif
	{
		if ((error = ip6_splithdr(m, &exthdrs)) != 0) {
			m = NULL;
			goto freehdrs;
		}
		m = exthdrs.ip6e_ip6;
		hdrsplit++;
	}

	/* adjust pointer */
	ip6 = mtod(m, struct ip6_hdr *);

	/* adjust mbuf packet header length */
	m->m_pkthdr.len += optlen;
	plen = m->m_pkthdr.len - sizeof(*ip6);

	/* If this is a jumbo payload, insert a jumbo payload option. */
	if (plen > IPV6_MAXPACKET) {
		if (!hdrsplit) {
			if ((error = ip6_splithdr(m, &exthdrs)) != 0) {
				m = NULL;
				goto freehdrs;
			}
			m = exthdrs.ip6e_ip6;
			hdrsplit++;
		}
		/* adjust pointer */
		ip6 = mtod(m, struct ip6_hdr *);
		if ((error = ip6_insert_jumboopt(&exthdrs, plen)) != 0)
			goto freehdrs;
		ip6->ip6_plen = 0;
	} else
		ip6->ip6_plen = htons(plen);

	/*
	 * Concatenate headers and fill in next header fields.
	 * Here we have, on "m"
	 *	IPv6 payload
	 * and we insert headers accordingly.  Finally, we should be getting:
	 *	IPv6 hbh dest1 rthdr ah* [esp* dest2 payload]
	 *
	 * during the header composing process, "m" points to IPv6 header.
	 * "mprev" points to an extension header prior to esp.
	 */
	{
		u_char *nexthdrp = &ip6->ip6_nxt;
		struct mbuf *mprev = m;

		/*
		 * we treat dest2 specially.  this makes IPsec processing
		 * much easier.  the goal here is to make mprev point the
		 * mbuf prior to dest2.
		 *
		 * result: IPv6 dest2 payload
		 * m and mprev will point to IPv6 header.
		 */
		if (exthdrs.ip6e_dest2) {
			if (!hdrsplit)
				panic("assumption failed: hdr not split");
			exthdrs.ip6e_dest2->m_next = m->m_next;
			m->m_next = exthdrs.ip6e_dest2;
			*mtod(exthdrs.ip6e_dest2, u_char *) = ip6->ip6_nxt;
			ip6->ip6_nxt = IPPROTO_DSTOPTS;
		}

#define MAKE_CHAIN(m, mp, p, i)\
    do {\
	if (m) {\
		if (!hdrsplit) \
			panic("assumption failed: hdr not split"); \
		*mtod((m), u_char *) = *(p);\
		*(p) = (i);\
		p = mtod((m), u_char *);\
		(m)->m_next = (mp)->m_next;\
		(mp)->m_next = (m);\
		(mp) = (m);\
	}\
    } while (0)
		/*
		 * result: IPv6 hbh dest1 rthdr dest2 payload
		 * m will point to IPv6 header.  mprev will point to the
		 * extension header prior to dest2 (rthdr in the above case).
		 */
		MAKE_CHAIN(exthdrs.ip6e_hbh, mprev,
			   nexthdrp, IPPROTO_HOPOPTS);
		MAKE_CHAIN(exthdrs.ip6e_dest1, mprev,
			   nexthdrp, IPPROTO_DSTOPTS);
		MAKE_CHAIN(exthdrs.ip6e_rthdr, mprev,
			   nexthdrp, IPPROTO_ROUTING);
#ifdef MIP6
		/*
		 * XXX
		 * MIP6 homeaddress destination option must reside
		 * after rthdr and before ah/esp/frag hdr.
		 * this order is not recommended in the ipv6 spec of course.
		 * result: IPv6 hbh dest1 rthdr ha dest2 payload.
		 */
		MAKE_CHAIN(exthdrs.ip6e_haddr, mprev,
			   nexthdrp, IPPROTO_DSTOPTS);
#endif /* MIP6 */

#if defined(IPSEC) && !defined(__OpenBSD__)
		if (!needipsec)
			goto skip_ipsec2;

		/*
		 * pointers after IPsec headers are not valid any more.
		 * other pointers need a great care too.
		 * (IPsec routines should not mangle mbufs prior to AH/ESP)
		 */
		exthdrs.ip6e_dest2 = NULL;

	    {
		struct ip6_rthdr *rh = NULL;
		int segleft_org = 0;
		struct ipsec_output_state state;

		if (exthdrs.ip6e_rthdr) {
			rh = mtod(exthdrs.ip6e_rthdr, struct ip6_rthdr *);
			segleft_org = rh->ip6r_segleft;
			rh->ip6r_segleft = 0;
		}

		bzero(&state, sizeof(state));
		state.m = m;
		error = ipsec6_output_trans(&state, nexthdrp, mprev, sp, flags,
			&needipsectun);
		m = state.m;
		if (error) {
			/* mbuf is already reclaimed in ipsec6_output_trans. */
			m = NULL;
			switch (error) {
			case EHOSTUNREACH:
			case ENETUNREACH:
			case EMSGSIZE:
			case ENOBUFS:
			case ENOMEM:
				break;
			default:
				printf("ip6_output (ipsec): error code %d\n", error);
				/* fall through */
			case ENOENT:
				/* don't show these error codes to the user */
				error = 0;
				break;
			}
			goto bad;
		}
		if (exthdrs.ip6e_rthdr) {
			/* ah6_output doesn't modify mbuf chain */
			rh->ip6r_segleft = segleft_org;
		}
	    }
skip_ipsec2:;
#endif
	}

#ifdef MIP6
	if ((flags & IPV6_FORWARDING) == 0) {
		/*
		 * After the IPsec processing the IPv6 header source
		 * address (this is the homeaddress of this node) and
		 * the address currently stored in the Home Address
		 * destination option (this is the coa of this node)
		 * must be swapped.
		 */
		if ((error = mip6_addr_exchange(m, exthdrs.ip6e_haddr)) != 0) {
			mip6log((LOG_ERR,
				 "%s:%d: "
				 "addr exchange between haddr and "
				 "coa failed.\n",
				 __FILE__, __LINE__));
			goto bad;
		}
	} else {
		/*
		 * this is the forwarding packet.  The typical (and
		 * only ?) case is multicast packet forwarding.  The
		 * swapping has been already done before (if
		 * necessary).  we must not touch any extension
		 * headers at all.
		 */
	}
#endif /* MIP6 */

	/*
	 * If there is a routing header, replace destination address field
	 * with the first hop of the routing header.
	 */
	if (exthdrs.ip6e_rthdr) {
		struct ip6_rthdr *rh =
			(struct ip6_rthdr *)(mtod(exthdrs.ip6e_rthdr,
						  struct ip6_rthdr *));
		struct ip6_rthdr0 *rh0;
		struct in6_addr *addr;

		finaldst = ip6->ip6_dst;
		switch (rh->ip6r_type) {
		case IPV6_RTHDR_TYPE_0:
			 rh0 = (struct ip6_rthdr0 *)rh;
			 addr = (struct in6_addr *)(rh0 + 1);

			 ip6->ip6_dst = *addr;
			 bcopy((caddr_t)(addr + 1), (caddr_t)addr,
				 sizeof(struct in6_addr) * (rh0->ip6r0_segleft - 1)
				 );
			 *(addr + rh0->ip6r0_segleft - 1) = finaldst;
			 break;
		default:	/* is it possible? */
			 error = EINVAL;
			 goto bad;
		}
	}

	/* Source address validation */
	if (!(flags & IPV6_UNSPECSRC) &&
	    IN6_IS_ADDR_UNSPECIFIED(&ip6->ip6_src)) {
		/*
		 * XXX: we can probably assume validation in the caller, but
		 * we explicitly check the address here for safety.
		 */
		error = EOPNOTSUPP;
		ip6stat.ip6s_badscope++;
		goto bad;
	}
	if (IN6_IS_ADDR_MULTICAST(&ip6->ip6_src)) {
		error = EOPNOTSUPP;
		ip6stat.ip6s_badscope++;
		goto bad;
	}

	ip6stat.ip6s_localout++;

	/*
	 * Route packet.
	 */
	if (ro == 0) {
		ro = &ip6route;
		bzero((caddr_t)ro, sizeof(*ro));
	}
	ro_pmtu = ro;
	if (opt && opt->ip6po_rthdr)
		ro = &opt->ip6po_route;
#ifdef MIP6
	else if (exthdrs.ip6e_rthdr) {
		struct sockaddr_in6 *firsthop;
		struct ip6_hdr *ip6
			= mtod(m, struct ip6_hdr *); /* needed ? */;

		ro = &mip6_ip6route;
		bzero((caddr_t)ro, sizeof(*ro));
		firsthop = (struct sockaddr_in6 *)&ro->ro_dst;
		bzero(firsthop, sizeof(*firsthop));
		firsthop->sin6_family = AF_INET6;
		firsthop->sin6_len = sizeof(struct sockaddr_in6);
		firsthop->sin6_addr = ip6->ip6_dst;
	}
#endif /* MIP6 */
	dst = (struct sockaddr_in6 *)&ro->ro_dst;

#ifdef IPSEC
#ifdef __OpenBSD__
	/*
	 * Check if the packet needs encapsulation.
	 * ipsp_process_packet will never come back to here.
	 */
	if (sproto != 0) {
	        s = splnet();

		/* fill in IPv6 header which would be filled later */
		if (!IN6_IS_ADDR_MULTICAST(&ip6->ip6_dst)) {
			if (opt && opt->ip6po_hlim != -1)
				ip6->ip6_hlim = opt->ip6po_hlim & 0xff;
		} else {
			if (im6o != NULL)
				ip6->ip6_hlim = im6o->im6o_multicast_hlim;
			else
				ip6->ip6_hlim = ip6_defmcasthlim;
			if (opt && opt->ip6po_hlim != -1)
				ip6->ip6_hlim = opt->ip6po_hlim & 0xff;

			/*
			 * XXX what should we do if ip6_hlim == 0 and the packet
			 * gets tunnelled?
			 */
		}

		tdb = gettdb(sspi, &sdst, sproto);
		if (tdb == NULL) {
			splx(s);
			error = EHOSTUNREACH;
			m_freem(m);
			goto done;
		}

		/* Latch to PCB */
		if (inp)
			tdb_add_inp(tdb, inp, 0);

		m->m_flags &= ~(M_BCAST | M_MCAST);	/* just in case */

		/* Callee frees mbuf */
		error = ipsp_process_packet(m, tdb, AF_INET6, 0);
		splx(s);

		return error;  /* Nothing more to be done */
	}
#else
	if (needipsec && needipsectun) {
		struct ipsec_output_state state;

		/*
		 * All the extension headers will become inaccessible
		 * (since they can be encrypted).
		 * Don't panic, we need no more updates to extension headers
		 * on inner IPv6 packet (since they are now encapsulated).
		 *
		 * IPv6 [ESP|AH] IPv6 [extension headers] payload
		 */
		bzero(&exthdrs, sizeof(exthdrs));
		exthdrs.ip6e_ip6 = m;

		bzero(&state, sizeof(state));
		state.m = m;
		state.ro = (struct route *)ro;
		state.dst = (struct sockaddr *)dst;

		error = ipsec6_output_tunnel(&state, sp, flags);

		m = state.m;
#ifdef NEW_STRUCT_ROUTE
		ro = state.ro;
#else
		ro = (struct route_in6 *)state.ro;
#endif
		dst = (struct sockaddr_in6 *)state.dst;
		if (error) {
			/* mbuf is already reclaimed in ipsec6_output_tunnel. */
			m0 = m = NULL;
			m = NULL;
			switch (error) {
			case EHOSTUNREACH:
			case ENETUNREACH:
			case EMSGSIZE:
			case ENOBUFS:
			case ENOMEM:
				break;
			default:
				printf("ip6_output (ipsec): error code %d\n", error);
				/* fall through */
			case ENOENT:
				/* don't show these error codes to the user */
				error = 0;
				break;
			}
			goto bad;
		}

		exthdrs.ip6e_ip6 = m;
	}
#endif /* OpenBSD */
#endif /* IPSEC */

	/* if specified, fill in the traffic class field. */
	if (opt) {
		ip6->ip6_flow &= ~htonl(0xff << 20);
		if (opt->ip6po_tclass >= 0)
			ip6->ip6_flow |=
			    htonl((opt->ip6po_tclass & 0xff) << 20);
	}
	/* fill in or override the hop limit field, if necessary. */
	if (opt && opt->ip6po_hlim != -1)
		ip6->ip6_hlim = opt->ip6po_hlim & 0xff;
	else if (IN6_IS_ADDR_MULTICAST(&ip6->ip6_dst)) {
		if (im6o != NULL)
			ip6->ip6_hlim = im6o->im6o_multicast_hlim;
		else
			ip6->ip6_hlim = ip6_defmcasthlim;
	}

	{
		/*
		 * XXX: using a block just to define a local variables is not
		 * a good style....
		 */
		struct ifnet *ifp0 = NULL;
		struct sockaddr_in6 src;
		struct sockaddr_in6 dst0;
		int clone = 0;
		int64_t zone;

		/*
		 * XXX: sockaddr_in6 for the destination should be passed
		 * from the upper layer with a proper scope zone ID, in order
		 * to make a copy here. 
		 */
		bzero(&dst0, sizeof(dst0));
		dst0.sin6_family = AF_INET6;
		dst0.sin6_len = sizeof(dst0);
		dst0.sin6_addr = ip6->ip6_dst;
#ifdef SCOPEDROUTING
		/* XXX: in6_recoverscope will clear the embedded ID */
		error = in6_recoverscope(&dst0, &dst0.sin6_addr, NULL);
		if (error != 0) {
			ip6stat.ip6s_badscope++;
			in6_ifstat_inc(ifp, ifs6_out_discard);
			goto bad;
		}
#endif

#if defined(__bsdi__) || defined(__FreeBSD__)
		if (ro != &ip6route && !IN6_IS_ADDR_MULTICAST(&ip6->ip6_dst))
			clone = 1;
#endif

		if ((error = in6_selectroute(&dst0, opt, im6o, ro,
					     &ifp, &rt, clone)) != 0) {
			switch (error) {
			case EHOSTUNREACH:
				ip6stat.ip6s_noroute++;
				break;
			case EADDRNOTAVAIL:
			default:
				break; /* XXX statistics? */
			}
			if (ifp != NULL)
				in6_ifstat_inc(ifp, ifs6_out_discard);
			goto bad;
		}
		if (rt == NULL) {
			/*
			 * If in6_selectroute() does not return a route entry,
			 * dst may not have been updated. 
			 */
			*dst = dst0;	/* XXX */
		}

		/*
		 * then rt (for unicast) and ifp must be non-NULL valid values.
		 */
		if ((flags & IPV6_FORWARDING) == 0) {
			/* XXX: the FORWARDING flag can be set for mrouting. */
			in6_ifstat_inc(ifp, ifs6_out_request);
		}
		if (rt != NULL) {
			ia = (struct in6_ifaddr *)(rt->rt_ifa);
			rt->rt_use++;
		}

		/*
		 * The outgoing interface must be in the zone of source and
		 * destination addresses.  We should use ia_ifp to support the
		 * case of sending packets to an address of our own.
		 */
		if (ia != NULL && ia->ia_ifp)
			ifp0 = ia->ia_ifp;
		else
			ifp0 = ifp;
		/* XXX: we should not do this conversion for every packet. */
		bzero(&src, sizeof(src));
		src.sin6_family = AF_INET6;
		src.sin6_len = sizeof(src);
		src.sin6_addr = ip6->ip6_src;
		if ((error = in6_recoverscope(&src, &ip6->ip6_src, NULL))
		    != 0) {
			goto badscope;
		}
		if ((zone = in6_addr2zoneid(ifp0, &src.sin6_addr)) < 0 ||
		    zone != src.sin6_scope_id) {
#ifdef SCOPEDEBUG		/* will be removed shortly */
			printf("ip6 output: bad source scope %s for %s on %s\n",
			       ip6_sprintf(&ip6->ip6_src),
			       ip6_sprintf(&ip6->ip6_dst), if_name(ifp0));
#endif
			goto badscope;
		}
		/* XXX: in6_recoverscope will clear the embedded ID */
		if ((error = in6_recoverscope(&dst0, &ip6->ip6_dst, NULL))
		    != 0) {
			goto badscope;
		}
		if ((zone = in6_addr2zoneid(ifp0, &dst0.sin6_addr)) < 0 ||
		    zone != dst0.sin6_scope_id) {
#ifdef SCOPEDEBUG		/* will be removed shortly */
			printf("ip6 output: bad dst scope %s on %s\n",
			       ip6_sprintf(&dst0.sin6_addr), if_name(ifp0));
#endif
			goto badscope;
		}

		/* scope check is done. */
		goto routefound;

	  badscope:
		ip6stat.ip6s_badscope++;
		in6_ifstat_inc(ifp0, ifs6_out_discard);
		if (error == 0)
			error = EHOSTUNREACH; /* XXX */
		goto bad;
	}

  routefound:
	if (rt) {
		if (opt && opt->ip6po_nextroute.ro_rt) {
			/*
			 * The nexthop is explicitly specified by the
			 * application.  We assume the next hop is an IPv6
			 * address.
			 */
			dst = (struct sockaddr_in6 *)opt->ip6po_nexthop;
		}
		else if ((rt->rt_flags & RTF_GATEWAY))
			dst = (struct sockaddr_in6 *)rt->rt_gateway;
	}

	if (!IN6_IS_ADDR_MULTICAST(&ip6->ip6_dst)) {
		m->m_flags &= ~(M_BCAST | M_MCAST); /* just in case */
	} else {
		struct	in6_multi *in6m;

		m->m_flags = (m->m_flags & ~M_BCAST) | M_MCAST;

		in6_ifstat_inc(ifp, ifs6_out_mcast);

		/*
		 * Confirm that the outgoing interface supports multicast.
		 */
		if (!(ifp->if_flags & IFF_MULTICAST)) {
			ip6stat.ip6s_noroute++;
			in6_ifstat_inc(ifp, ifs6_out_discard);
			error = ENETUNREACH;
			goto bad;
		}
		IN6_LOOKUP_MULTI(ip6->ip6_dst, ifp, in6m);
		if (in6m != NULL &&
		   (im6o == NULL || im6o->im6o_multicast_loop)) {
			/*
			 * If we belong to the destination multicast group
			 * on the outgoing interface, and the caller did not
			 * forbid loopback, loop back a copy.
			 */
			ip6_mloopback(ifp, m, dst);
		} else {
			/*
			 * If we are acting as a multicast router, perform
			 * multicast forwarding as if the packet had just
			 * arrived on the interface to which we are about
			 * to send.  The multicast forwarding function
			 * recursively calls this function, using the
			 * IPV6_FORWARDING flag to prevent infinite recursion.
			 *
			 * Multicasts that are looped back by ip6_mloopback(),
			 * above, will be forwarded by the ip6_input() routine,
			 * if necessary.
			 */
			if (ip6_mrouter && (flags & IPV6_FORWARDING) == 0) {
				/*
				 * XXX: ip6_mforward expects that rcvif is NULL
				 * when it is called from the originating path.
				 * However, it is not always the case, since
				 * some versions of MGETHDR() does not
				 * initialize the field.  
				 */
				m->m_pkthdr.rcvif = NULL;
				if (ip6_mforward(ip6, ifp, m) != 0) {
					m_freem(m);
					goto done;
				}
			}
		}
		/*
		 * Multicasts with a hoplimit of zero may be looped back,
		 * above, but must not be transmitted on a network.
		 * Also, multicasts addressed to the loopback interface
		 * are not sent -- the above call to ip6_mloopback() will
		 * loop back a copy if this host actually belongs to the
		 * destination group on the loopback interface.
		 */
		if (ip6->ip6_hlim == 0 || (ifp->if_flags & IFF_LOOPBACK) ||
		    IN6_IS_ADDR_MC_INTFACELOCAL(&ip6->ip6_dst)) {
			m_freem(m);
			goto done;
		}
	}

	/*
	 * Fill the outgoing inteface to tell the upper layer
	 * to increment per-interface statistics.
	 */
	if (ifpp)
		*ifpp = ifp;

	/*
	 * Upper-layer reachability confirmation
	 */
	if (opt && (opt->ip6po_flags & IP6PO_REACHCONF))
		nd6_nud_hint(rt, NULL, 0);

	/* Determine path MTU. */
	if ((error = ip6_getpmtu(ro_pmtu, ro, ifp, &finaldst, &mtu)) != 0)
		goto bad;

	/*
	 * An advanced API option (IPV6_USE_MIN_MTU) overrides mtu setting.
	 * We ignore the specified MTU if it is larger than the already-known
	 * path MTU.
	 */
	if (mtu > IPV6_MMTU && opt && (opt->ip6po_flags & IP6PO_MINMTU))
		mtu = IPV6_MMTU;

	/* Fake scoped addresses */
	if ((ifp->if_flags & IFF_LOOPBACK) != 0) {
		/*
		 * If source or destination address is a scoped address, and
		 * the packet is going to be sent to a loopback interface,
		 * we should keep the original interface.
		 */

		/*
		 * XXX: this is a very experimental and temporary solution.
		 * We eventually have sockaddr_in6 and use the sin6_scope_id
		 * field of the structure here.
		 * We rely on the consistency between two scope zone ids
		 * of source and destination, which should already be assured.
		 * Larger scopes than link will be supported in the future. 
		 */
		origifp = NULL;
		if (IN6_IS_SCOPE_LINKLOCAL(&ip6->ip6_src)) {
#if defined(__FreeBSD__) && __FreeBSD__ >= 5
			origifp = ifnet_byindex(ntohs(ip6->ip6_src.s6_addr16[1]));
#else
			origifp = ifindex2ifnet[ntohs(ip6->ip6_src.s6_addr16[1])];
#endif
		} else if (IN6_IS_SCOPE_LINKLOCAL(&ip6->ip6_dst)) {
#if defined(__FreeBSD__) && __FreeBSD__ >= 5
			origifp = ifnet_byindex(ntohs(ip6->ip6_dst.s6_addr16[1]));
#else
			origifp = ifindex2ifnet[ntohs(ip6->ip6_dst.s6_addr16[1])];
#endif
		} else if (IN6_IS_ADDR_MC_INTFACELOCAL(&ip6->ip6_dst)) {
#if defined(__FreeBSD__) && __FreeBSD__ >= 5
			origifp = ifnet_byindex(ntohs(ip6->ip6_dst.s6_addr16[1]));
#else
			origifp = ifindex2ifnet[ntohs(ip6->ip6_dst.s6_addr16[1])];
#endif
		}

		/*
		 * XXX: origifp can be NULL even in those two cases above.
		 * For example, if we remove the (only) link-local address
		 * from the loopback interface, and try to send a link-local
		 * address without link-id information.  Then the source
		 * address is ::1, and the destination address is the
		 * link-local address with its s6_addr16[1] being zero.
		 * What is worse, if the packet goes to the loopback interface
		 * by a default rejected route, the null pointer would be
		 * passed to looutput, and the kernel would hang.
		 * The following last resort would prevent such disaster.
		 */
		if (origifp == NULL)
			origifp = ifp;
	}
	else
		origifp = ifp;
#ifndef SCOPEDROUTING
	/*
	 * clear embedded scope identifiers if necessary.
	 * in6_clearscope will touch the addresses only when necessary.
	 */
	in6_clearscope(&ip6->ip6_src);
	in6_clearscope(&ip6->ip6_dst);
#endif

#if defined(IPV6FIREWALL) || (defined(__FreeBSD__) && __FreeBSD__ >= 4)
	/*
	 * Check with the firewall...
	 */
#if defined(__FreeBSD__) && __FreeBSD__ >= 4
	if (ip6_fw_enable && ip6_fw_chk_ptr) {
#else
	if (ip6_fw_chk_ptr) {
#endif
		m->m_pkthdr.rcvif = NULL;	/* XXX */
		/* If ipfw says divert, we have to just drop packet */
		if ((*ip6_fw_chk_ptr)(&ip6, ifp, &m)) {
			m_freem(m);
			goto done;
		}
		if (!m) {
			error = EACCES;
			goto done;
		}
	}
#endif

	/*
	 * If the outgoing packet contains a hop-by-hop options header,
	 * it must be examined and processed even by the source node.
	 * (RFC 2460, section 4.)
	 */
	if (exthdrs.ip6e_hbh) {
		struct ip6_hbh *hbh = mtod(exthdrs.ip6e_hbh, struct ip6_hbh *);
		u_int32_t dummy; /* XXX unused */
		u_int32_t plen = 0; /* XXX: ip6_process will check the value */

#ifdef DIAGNOSTIC
		if ((hbh->ip6h_len + 1) << 3 > exthdrs.ip6e_hbh->m_len)
			panic("ip6e_hbh is not continuous");
#endif
		/*
		 *  XXX: if we have to send an ICMPv6 error to the sender,
		 *       we need the M_LOOP flag since icmp6_error() expects
		 *       the IPv6 and the hop-by-hop options header are
		 *       continuous unless the flag is set.
		 */
		m->m_flags |= M_LOOP;
		m->m_pkthdr.rcvif = ifp;
		if (ip6_process_hopopts(m,
					(u_int8_t *)(hbh + 1),
					((hbh->ip6h_len + 1) << 3) -
					sizeof(struct ip6_hbh),
					&dummy, &plen) < 0) {
			/* m was already freed at this point */
			error = EINVAL;/* better error? */
			goto done;
		}
		m->m_flags &= ~M_LOOP; /* XXX */
		m->m_pkthdr.rcvif = NULL;
	}

#if defined(__NetBSD__) && defined(PFIL_HOOKS)
	/*
	 * Run through list of hooks for output packets.
	 */
	m1 = m;
	pfh = pfil_hook_get(PFIL_OUT, &inetsw[ip_protox[IPPROTO_IPV6]].pr_pfh);
	for (; pfh; pfh = pfh->pfil_link.tqe_next)
		if (pfh->pfil_func) {
		    	rv = pfh->pfil_func(ip6, sizeof(*ip6), ifp, 1, &m1);
			if (rv) {
				error = EHOSTUNREACH;
				goto done;
			}
			m = m1;
			if (m == NULL)
				goto done;
			ip6 = mtod(m, struct ip6_hdr *);
		}
#endif /* PFIL_HOOKS */

#if defined(__OpenBSD__) && NPF > 0
	if (pf_test6(PF_OUT, ifp, &m) != PF_PASS) {
		error = EHOSTUNREACH;
		m_freem(m);
		goto done;
	}
	ip6 = mtod(m, struct ip6_hdr *);
#endif 

	/*
	 * Send the packet to the outgoing interface.
	 * If necessary, do IPv6 fragmentation before sending.
	 */
	tlen = m->m_pkthdr.len;
	/*
	 * Even if the DONTFRAG option is specified, we cannot send the packet
	 * when the data length is larger than the MTU of the outgoing
	 * interface.
	 * Notify the error by sending IPV6_PATHMTU ancillary data as well
	 * as returning an error code (the latter is not described in the API
	 * spec.)
	 */
	if (opt && (opt->ip6po_flags & IP6PO_DONTFRAG) && tlen > ifp->if_mtu
#ifdef notyet
	    && !(ifp->if_flags & IFF_FRAGMENTABLE)
#endif
		) {
		u_int32_t mtu32;
		struct ip6ctlparam ip6cp;

		mtu32 = (u_int32_t)mtu;
		bzero(&ip6cp, sizeof(ip6cp));
		ip6cp.ip6c_cmdarg = (void *)&mtu32;
		pfctlinput2(PRC_MSGSIZE, &ro_pmtu->ro_dst, (void *)&ip6cp);

		error = EMSGSIZE;
		goto bad;
	}
	if (tlen <= mtu || (opt && (opt->ip6po_flags & IP6PO_DONTFRAG))
#ifdef notyet
	    /*
	     * On any link that cannot convey a 1280-octet packet in one piece,
	     * link-specific fragmentation and reassembly must be provided at
	     * a layer below IPv6. [RFC 2460, sec.5]
	     * Thus if the interface has ability of link-level fragmentation,
	     * we can just send the packet even if the packet size is
	     * larger than the link's MTU.
	     * XXX: IFF_FRAGMENTABLE (or such) flag has not been defined yet...
	     */
	
	    || (ifp->if_flags & IFF_FRAGMENTABLE)
#endif
	    )
	{
		struct in6_ifaddr *ia6;

		ip6 = mtod(m, struct ip6_hdr *);
		ia6 = in6_ifawithifp(ifp, &ip6->ip6_src);
		if (ia6) {
			/* Record statistics for this interface address. */
#if defined(__NetBSD__) && defined(IFA_STATS)
			ia6->ia_ifa.ifa_data.ifad_outbytes +=
				m->m_pkthdr.len;
#elif defined(__FreeBSD__) && __FreeBSD__ >= 4
			ia6->ia_ifa.if_opackets++;
			ia6->ia_ifa.if_obytes += m->m_pkthdr.len;
#elif defined(__bsdi__) && _BSDI_VERSION >= 199802
			ia6->ia_ifa.ifa_opackets++;
			ia6->ia_ifa.ifa_obytes += m->m_pkthdr.len;
#endif
		}
#if defined(IPSEC) && !defined(__OpenBSD__)
		/* clean ipsec history once it goes out of the node */
		ipsec_delaux(m);
#endif
		error = nd6_output(ifp, origifp, m, dst, rt);
		goto done;
	} else if (mtu < IPV6_MMTU) {
		/*
		 * note that path MTU is never less than IPV6_MMTU
		 * (see icmp6_input).
		 */
		error = EMSGSIZE;
		in6_ifstat_inc(ifp, ifs6_out_fragfail);
		goto bad;
	} else if (ip6->ip6_plen == 0) { /* jumbo payload cannot be fragmented */
		error = EMSGSIZE;
		in6_ifstat_inc(ifp, ifs6_out_fragfail);
		goto bad;
	} else {
		struct mbuf **mnext, *m_frgpart;
		struct ip6_frag *ip6f;
		u_int32_t id = htonl(ip6_id++);
		u_char nextproto;
		struct ip6ctlparam ip6cp;
		u_int32_t mtu32;

		/*
		 * Too large for the destination or interface;
		 * fragment if possible.
		 * Must be able to put at least 8 bytes per fragment.
		 */
		hlen = unfragpartlen;
		if (mtu > IPV6_MAXPACKET)
			mtu = IPV6_MAXPACKET;

		/* Notify a proper path MTU to applications. */
		mtu32 = (u_int32_t)mtu;
		bzero(&ip6cp, sizeof(ip6cp));
		ip6cp.ip6c_cmdarg = (void *)&mtu32;
		pfctlinput2(PRC_MSGSIZE, &ro_pmtu->ro_dst, (void *)&ip6cp);

		len = (mtu - hlen - sizeof(struct ip6_frag)) & ~7;
		if (len < 8) {
			error = EMSGSIZE;
			in6_ifstat_inc(ifp, ifs6_out_fragfail);
			goto bad;
		}

		mnext = &m->m_nextpkt;

		/*
		 * Change the next header field of the last header in the
		 * unfragmentable part.
		 */
#ifdef MIP6
		if (exthdrs.ip6e_haddr) {
			nextproto = *mtod(exthdrs.ip6e_haddr, u_char *);
			*mtod(exthdrs.ip6e_haddr, u_char *) = IPPROTO_FRAGMENT;
		} else
#endif /* MIP6 */
		if (exthdrs.ip6e_rthdr) {
			nextproto = *mtod(exthdrs.ip6e_rthdr, u_char *);
			*mtod(exthdrs.ip6e_rthdr, u_char *) = IPPROTO_FRAGMENT;
		} else if (exthdrs.ip6e_dest1) {
			nextproto = *mtod(exthdrs.ip6e_dest1, u_char *);
			*mtod(exthdrs.ip6e_dest1, u_char *) = IPPROTO_FRAGMENT;
		} else if (exthdrs.ip6e_hbh) {
			nextproto = *mtod(exthdrs.ip6e_hbh, u_char *);
			*mtod(exthdrs.ip6e_hbh, u_char *) = IPPROTO_FRAGMENT;
		} else {
			nextproto = ip6->ip6_nxt;
			ip6->ip6_nxt = IPPROTO_FRAGMENT;
		}

		/*
		 * Loop through length of segment after first fragment,
		 * make new header and copy data of each part and link onto
		 * chain.
		 */
		m0 = m;
		for (off = hlen; off < tlen; off += len) {
			MGETHDR(m, M_DONTWAIT, MT_HEADER);
			if (!m) {
				error = ENOBUFS;
				ip6stat.ip6s_odropped++;
				goto sendorfree;
			}
			m->m_pkthdr.rcvif = NULL;
			m->m_flags = m0->m_flags & M_COPYFLAGS;
			*mnext = m;
			mnext = &m->m_nextpkt;
			m->m_data += max_linkhdr;
			mhip6 = mtod(m, struct ip6_hdr *);
			*mhip6 = *ip6;
			m->m_len = sizeof(*mhip6);
			error = ip6_insertfraghdr(m0, m, hlen, &ip6f);
			if (error) {
				ip6stat.ip6s_odropped++;
				goto sendorfree;
			}
			ip6f->ip6f_offlg = htons((u_short)((off - hlen) & ~7));
			if (off + len >= tlen)
				len = tlen - off;
			else
				ip6f->ip6f_offlg |= IP6F_MORE_FRAG;
			mhip6->ip6_plen = htons((u_short)(len + hlen +
							  sizeof(*ip6f) -
							  sizeof(struct ip6_hdr)));
			if ((m_frgpart = m_copy(m0, off, len)) == 0) {
				error = ENOBUFS;
				ip6stat.ip6s_odropped++;
				goto sendorfree;
			}
			m_cat(m, m_frgpart);
			m->m_pkthdr.len = len + hlen + sizeof(*ip6f);
			m->m_pkthdr.rcvif = (struct ifnet *)0;
			ip6f->ip6f_reserved = 0;
			ip6f->ip6f_ident = id;
			ip6f->ip6f_nxt = nextproto;
			ip6stat.ip6s_ofragments++;
			in6_ifstat_inc(ifp, ifs6_out_fragcreat);
		}

		in6_ifstat_inc(ifp, ifs6_out_fragok);
	}

	/*
	 * Remove leading garbages.
	 */
sendorfree:
	m = m0->m_nextpkt;
	m0->m_nextpkt = 0;
	m_freem(m0);
	for (m0 = m; m; m = m0) {
		m0 = m->m_nextpkt;
		m->m_nextpkt = 0;
		if (error == 0) {
			struct in6_ifaddr *ia6;
			ip6 = mtod(m, struct ip6_hdr *);
			ia6 = in6_ifawithifp(ifp, &ip6->ip6_src);
			if (ia6) {
				/*
				 * Record statistics for this interface
				 * address.
				 */
#if defined(__NetBSD__) && defined(IFA_STATS)
				ia6->ia_ifa.ifa_data.ifad_outbytes +=
					m->m_pkthdr.len;
#elif defined(__FreeBSD__) && __FreeBSD__ >= 4
				ia6->ia_ifa.if_opackets++;
				ia6->ia_ifa.if_obytes += m->m_pkthdr.len;
#elif defined(__bsdi__) && _BSDI_VERSION >= 199802
				ia6->ia_ifa.ifa_opackets++;
				ia6->ia_ifa.ifa_obytes += m->m_pkthdr.len;
#endif
			}
#if defined(IPSEC) && !defined(__OpenBSD__)
			/* clean ipsec history once it goes out of the node */
			ipsec_delaux(m);
#endif
			error = nd6_output(ifp, origifp, m, dst, rt);
		} else
			m_freem(m);
	}

	if (error == 0)
		ip6stat.ip6s_fragmented++;

done:
	if (ro == &ip6route && ro->ro_rt) { /* brace necessary for RTFREE */
		RTFREE(ro->ro_rt);
	} else if (ro_pmtu == &ip6route && ro_pmtu->ro_rt) {
		RTFREE(ro_pmtu->ro_rt);
	}

#if defined(IPSEC) && !defined(__OpenBSD__)
	if (sp != NULL)
		key_freesp(sp);
#endif /* IPSEC */

#ifdef MIP6
	mip6_destopt_discard(&mip6opt);
#endif /* MIP6 */

	return(error);

freehdrs:
#ifdef MIP6
	mip6_destopt_discard(&mip6opt);
#endif /* MIP6 */
	m_freem(exthdrs.ip6e_hbh);	/* m_freem will check if mbuf is 0 */
	m_freem(exthdrs.ip6e_dest1);
	m_freem(exthdrs.ip6e_rthdr);
#ifdef MIP6
	m_freem(exthdrs.ip6e_haddr);
#endif /* MIP6 */
	m_freem(exthdrs.ip6e_dest2);
	/* fall through */
bad:
	m_freem(m);
	goto done;
}

static int
ip6_copyexthdr(mp, hdr, hlen)
	struct mbuf **mp;
	caddr_t hdr;
	int hlen;
{
	struct mbuf *m;

	if (hlen > MCLBYTES)
		return(ENOBUFS); /* XXX */

	MGET(m, M_DONTWAIT, MT_DATA);
	if (!m)
		return(ENOBUFS);

	if (hlen > MLEN) {
		MCLGET(m, M_DONTWAIT);
		if ((m->m_flags & M_EXT) == 0) {
			m_free(m);
			return(ENOBUFS);
		}
	}
	m->m_len = hlen;
	if (hdr)
		bcopy(hdr, mtod(m, caddr_t), hlen);

	*mp = m;
	return(0);
}

/*
 * Insert jumbo payload option.
 */
static int
ip6_insert_jumboopt(exthdrs, plen)
	struct ip6_exthdrs *exthdrs;
	u_int32_t plen;
{
	struct mbuf *mopt;
	u_char *optbuf;
	u_int32_t v;

#define JUMBOOPTLEN	8	/* length of jumbo payload option and padding */

	/*
	 * If there is no hop-by-hop options header, allocate new one.
	 * If there is one but it doesn't have enough space to store the
	 * jumbo payload option, allocate a cluster to store the whole options.
	 * Otherwise, use it to store the options.
	 */
	if (exthdrs->ip6e_hbh == 0) {
		MGET(mopt, M_DONTWAIT, MT_DATA);
		if (mopt == 0)
			return(ENOBUFS);
		mopt->m_len = JUMBOOPTLEN;
		optbuf = mtod(mopt, u_char *);
		optbuf[1] = 0;	/* = ((JUMBOOPTLEN) >> 3) - 1 */
		exthdrs->ip6e_hbh = mopt;
	} else {
		struct ip6_hbh *hbh;

		mopt = exthdrs->ip6e_hbh;
		if (M_TRAILINGSPACE(mopt) < JUMBOOPTLEN) {
			/*
			 * XXX assumption:
			 * - exthdrs->ip6e_hbh is not referenced from places
			 *   other than exthdrs.
			 * - exthdrs->ip6e_hbh is not an mbuf chain.
			 */
			int oldoptlen = mopt->m_len;
			struct mbuf *n;

			/*
			 * XXX: give up if the whole (new) hbh header does
			 * not fit even in an mbuf cluster.
			 */
			if (oldoptlen + JUMBOOPTLEN > MCLBYTES)
				return(ENOBUFS);

			/*
			 * As a consequence, we must always prepare a cluster
			 * at this point.
			 */
			MGET(n, M_DONTWAIT, MT_DATA);
			if (n) {
				MCLGET(n, M_DONTWAIT);
				if ((n->m_flags & M_EXT) == 0) {
					m_freem(n);
					n = NULL;
				}
			}
			if (!n)
				return(ENOBUFS);
			n->m_len = oldoptlen + JUMBOOPTLEN;
			bcopy(mtod(mopt, caddr_t), mtod(n, caddr_t),
			      oldoptlen);
			optbuf = mtod(n, caddr_t) + oldoptlen;
			m_freem(mopt);
			mopt = exthdrs->ip6e_hbh = n;
		} else {
			optbuf = mtod(mopt, u_char *) + mopt->m_len;
			mopt->m_len += JUMBOOPTLEN;
		}
		optbuf[0] = IP6OPT_PADN;
		optbuf[1] = 1;

		/*
		 * Adjust the header length according to the pad and
		 * the jumbo payload option.
		 */
		hbh = mtod(mopt, struct ip6_hbh *);
		hbh->ip6h_len += (JUMBOOPTLEN >> 3);
	}

	/* fill in the option. */
	optbuf[2] = IP6OPT_JUMBO;
	optbuf[3] = 4;
	v = (u_int32_t)htonl(plen + JUMBOOPTLEN);
	bcopy(&v, &optbuf[4], sizeof(u_int32_t));

	/* finally, adjust the packet header length */
	exthdrs->ip6e_ip6->m_pkthdr.len += JUMBOOPTLEN;

	return(0);
#undef JUMBOOPTLEN
}

/*
 * Insert fragment header and copy unfragmentable header portions.
 */
static int
ip6_insertfraghdr(m0, m, hlen, frghdrp)
	struct mbuf *m0, *m;
	int hlen;
	struct ip6_frag **frghdrp;
{
	struct mbuf *n, *mlast;

	if (hlen > sizeof(struct ip6_hdr)) {
		n = m_copym(m0, sizeof(struct ip6_hdr),
			    hlen - sizeof(struct ip6_hdr), M_DONTWAIT);
		if (n == 0)
			return(ENOBUFS);
		m->m_next = n;
	} else
		n = m;

	/* Search for the last mbuf of unfragmentable part. */
	for (mlast = n; mlast->m_next; mlast = mlast->m_next)
		;

	if ((mlast->m_flags & M_EXT) == 0 &&
	    M_TRAILINGSPACE(mlast) >= sizeof(struct ip6_frag)) {
		/* use the trailing space of the last mbuf for the fragment hdr */
		*frghdrp =
			(struct ip6_frag *)(mtod(mlast, caddr_t) + mlast->m_len);
		mlast->m_len += sizeof(struct ip6_frag);
		m->m_pkthdr.len += sizeof(struct ip6_frag);
	} else {
		/* allocate a new mbuf for the fragment header */
		struct mbuf *mfrg;

		MGET(mfrg, M_DONTWAIT, MT_DATA);
		if (mfrg == 0)
			return(ENOBUFS);
		mfrg->m_len = sizeof(struct ip6_frag);
		*frghdrp = mtod(mfrg, struct ip6_frag *);
		mlast->m_next = mfrg;
	}

	return(0);
}

static int
ip6_getpmtu(ro_pmtu, ro, ifp, dst, mtup)
#ifdef NEW_STRUCT_ROUTE
	struct route *ro_pmtu, *ro;
#else
	struct route_in6 *ro_pmtu, *ro;
#endif
	struct ifnet *ifp;
	struct in6_addr *dst;	/* XXX: should be sockaddr_in6 */
	u_long *mtup;
{
	u_int32_t mtu = 0;
	int error = 0;

	if (ro_pmtu != ro) {
		/* The first hop and the final destination may differ. */
		struct sockaddr_in6 *sa6_dst =
			(struct sockaddr_in6 *)&ro_pmtu->ro_dst;
		if (ro_pmtu->ro_rt && ((ro_pmtu->ro_rt->rt_flags & RTF_UP)
				       == 0 ||
				       !IN6_ARE_ADDR_EQUAL(&sa6_dst->sin6_addr,
							   dst))) {
			RTFREE(ro_pmtu->ro_rt);
			ro_pmtu->ro_rt = (struct rtentry *)NULL;
		}
		if (ro_pmtu->ro_rt == NULL) {
			bzero(sa6_dst, sizeof(*sa6_dst));
			sa6_dst->sin6_family = AF_INET6;
			sa6_dst->sin6_len = sizeof(struct sockaddr_in6);
			sa6_dst->sin6_addr = *dst;

#ifdef __bsdi__			/* bsdi needs rtcalloc to clone a route. */
			rtcalloc((struct route *)ro_pmtu);
#else
			rtalloc((struct route *)ro_pmtu);
#endif
		}
	}
	if (ro_pmtu->ro_rt) {
		u_int32_t ifmtu;

		if (ifp == NULL)
			ifp = ro_pmtu->ro_rt->rt_ifp;
		ifmtu = nd_ifinfo[ifp->if_index].linkmtu;
		mtu = ro_pmtu->ro_rt->rt_rmx.rmx_mtu;
		if (mtu > ifmtu || mtu == 0) {
			/*
			 * The MTU on the route is larger than the MTU on
			 * the interface!  This shouldn't happen, unless the
			 * MTU of the interface has been changed after the
			 * interface was brought up.  Change the MTU in the
			 * route to match the interface MTU (as long as the
			 * field isn't locked).
			 *
			 * if MTU on the route is 0, we need to fix the MTU.
			 * this case happens with path MTU discovery timeouts.
			 */
			 mtu = ifmtu;
			 if ((ro_pmtu->ro_rt->rt_rmx.rmx_locks & RTV_MTU) == 0)
				 ro_pmtu->ro_rt->rt_rmx.rmx_mtu = mtu; /* XXX */
		}
	} else if (ifp) {
		mtu = nd_ifinfo[ifp->if_index].linkmtu;
	} else
		error = EHOSTUNREACH; /* XXX */

	*mtup = mtu;
	return(error);
}

/*
 * IP6 socket option processing.
 */
#if defined(__FreeBSD__) && __FreeBSD__ >= 3
int
ip6_ctloutput(so, sopt)
	struct socket *so;
	struct sockopt *sopt;
#else
int
ip6_ctloutput(op, so, level, optname, mp)
	int op;
	struct socket *so;
	int level, optname;
	struct mbuf **mp;
#endif
{
	int privileged, optdatalen;
	void *optdata;
	struct ip6_recvpktopts *rcvopts;
#if defined(IPSEC) && defined(__OpenBSD__)
	struct proc *p = curproc; /* XXX */
	struct tdb *tdb;
	struct tdb_ident *tdbip, tdbi;
	int s;
#endif
#if defined(__FreeBSD__) && __FreeBSD__ >= 3
	struct inpcb *in6p = sotoinpcb(so);
	int error, optval;
	int level, op, optname;
	int optlen;
	struct proc *p;

	if (!sopt) {
		panic("ip6_ctloutput: arg soopt is NULL");
	}
        level = sopt->sopt_level;
        op = sopt->sopt_dir;
        optname = sopt->sopt_name;
        optlen = sopt->sopt_valsize;
        p = sopt->sopt_p;
#else
#ifdef HAVE_NRL_INPCB
	struct inpcb *inp = sotoinpcb(so);
#define in6p inp
#else  /* !NRL */
	struct in6pcb *in6p = sotoin6pcb(so);
#endif /* HAVE_NRL_INPCB */
	struct mbuf *m = *mp;
	int error, optval;
	int optlen;
#if defined(__NetBSD__) || (defined(__FreeBSD__) && __FreeBSD__ >= 3)
	struct proc *p = curproc;	/* XXX */
#endif

	optlen = m ? m->m_len : 0;
#endif /* FreeBSD >= 3 */
	error = optval = 0;
        privileged = 1;

	rcvopts = in6p->in6p_inputopts;

	if (level == IPPROTO_IPV6) {
		switch (op) {

#if defined(__FreeBSD__) && __FreeBSD__ >= 3
		case SOPT_SET:
#else
		case PRCO_SETOPT:
#endif
			switch (optname) {
			case IPV6_2292PKTOPTIONS:
#ifdef IPV6_PKTOPTIONS
			case IPV6_PKTOPTIONS:
#endif
			{
#if defined(__FreeBSD__) && __FreeBSD__ >= 3
				struct mbuf *m;

				error = soopt_getm(sopt, &m); /* XXX */
				if (error)
					break;
				error = soopt_mcopyin(sopt, m); /* XXX */
				if (error)
					break;
				error = ip6_pcbopts(&in6p->in6p_outputopts,
						    m, so, sopt);
				m_freem(m); /* XXX */
#else
				error = ip6_pcbopts(&in6p->in6p_outputopts,
						    m, so);
#endif /* FreeBSD >= 3 */
				break;
			}

			/*
			 * Use of some Hop-by-Hop options or some
			 * Destination options, might require special
			 * privilege.  That is, normal applications
			 * (without special privilege) might be forbidden
			 * from setting certain options in outgoing packets,
			 * and might never see certain options in received
			 * packets. [RFC 2292 Section 6]
			 * KAME specific note:
			 *  KAME prevents non-privileged users from sending or
			 *  receiving ANY hbh/dst options in order to avoid
			 *  overhead of parsing options in the kernel.
			 */
			case IPV6_RECVHOPOPTS:
			case IPV6_RECVDSTOPTS:
			case IPV6_RECVRTHDRDSTOPTS:
				/* fall through */
			case IPV6_UNICAST_HOPS:
			case IPV6_HOPLIMIT:
			case IPV6_FAITH:

			case IPV6_RECVPKTINFO:
			case IPV6_RECVHOPLIMIT:
			case IPV6_RECVRTHDR:
			case IPV6_RECVPATHMTU:
			case IPV6_RECVTCLASS:
			case IPV6_V6ONLY:
			case IPV6_AUTOFLOWLABEL:
				if (optlen != sizeof(int)) {
					error = EINVAL;
					break;
				}
#if defined(__FreeBSD__) && __FreeBSD__ >= 3
				error = sooptcopyin(sopt, &optval,
					sizeof optval, sizeof optval);
				if (error)
					break;
#else
				optval = *mtod(m, int *);
#endif
				switch (optname) {

				case IPV6_UNICAST_HOPS:
					if (optval < -1 || optval >= 256)
						error = EINVAL;
					else {
						/* -1 = kernel default */
						in6p->in6p_hops = optval;
#if defined(__FreeBSD__) && __FreeBSD__ >= 3
						if ((in6p->in6p_vflag &
						     INP_IPV4) != 0)
							in6p->inp_ip_ttl = optval;
#endif
					}
					break;
#define OPTSET(bit) \
do { \
	if (optval) \
		in6p->in6p_flags |= (bit); \
	else \
		in6p->in6p_flags &= ~(bit); \
} while (0)
#define OPTSET2292(bit) \
do { \
	in6p->in6p_flags |= IN6P_RFC2292; \
	if (optval) \
		in6p->in6p_flags |= (bit); \
	else \
		in6p->in6p_flags &= ~(bit); \
} while (0)
#define OPTBIT(bit) (in6p->in6p_flags & (bit) ? 1 : 0)

				case IPV6_RECVPKTINFO:
					/* cannot mix with RFC2292 */
					if (OPTBIT(IN6P_RFC2292)) {
						error = EINVAL;
						break;
					}
					OPTSET(IN6P_PKTINFO);
					if (OPTBIT(IN6P_PKTINFO) == 0)
						ip6_reset_rcvopt(rcvopts, IPV6_RECVPKTINFO);
					break;

				case IPV6_HOPLIMIT:
				{
					struct ip6_pktopts **optp;

					/* cannot mix with RFC2292 */
					if (OPTBIT(IN6P_RFC2292)) {
						error = EINVAL;
						break;
					}
					optp = &in6p->in6p_outputopts;
					error = ip6_pcbopt(IPV6_HOPLIMIT,
							   (u_char *)&optval,
							   sizeof(optval),
							   optp,
							   privileged);
					break;
				}

				case IPV6_RECVHOPLIMIT:
					/* cannot mix with RFC2292 */
					if (OPTBIT(IN6P_RFC2292)) {
						error = EINVAL;
						break;
					}
					OPTSET(IN6P_HOPLIMIT);
					if (OPTBIT(IN6P_HOPLIMIT) == 0)
						ip6_reset_rcvopt(rcvopts, IPV6_RECVHOPLIMIT);
					break;

				case IPV6_RECVHOPOPTS:
					/* cannot mix with RFC2292 */
					if (OPTBIT(IN6P_RFC2292)) {
						error = EINVAL;
						break;
					}
					OPTSET(IN6P_HOPOPTS);
					if (OPTBIT(IN6P_HOPOPTS) == 0)
						ip6_reset_rcvopt(rcvopts, IPV6_RECVHOPOPTS);
					break;

				case IPV6_RECVDSTOPTS:
					/* cannot mix with RFC2292 */
					if (OPTBIT(IN6P_RFC2292)) {
						error = EINVAL;
						break;
					}
					OPTSET(IN6P_DSTOPTS);
					if (OPTBIT(IN6P_DSTOPTS) == 0)
						ip6_reset_rcvopt(rcvopts, IPV6_RECVDSTOPTS);
					break;

				case IPV6_RECVRTHDRDSTOPTS:
					/* cannot mix with RFC2292 */
					if (OPTBIT(IN6P_RFC2292)) {
						error = EINVAL;
						break;
					}
					OPTSET(IN6P_RTHDRDSTOPTS);
					if (OPTBIT(IN6P_RTHDRDSTOPTS) == 0)
						ip6_reset_rcvopt(rcvopts, IPV6_RECVRTHDRDSTOPTS);
					break;

				case IPV6_RECVRTHDR:
					/* cannot mix with RFC2292 */
					if (OPTBIT(IN6P_RFC2292)) {
						error = EINVAL;
						break;
					}
					OPTSET(IN6P_RTHDR);
					if (OPTBIT(IN6P_RTHDR) == 0)
						ip6_reset_rcvopt(rcvopts, IPV6_RECVRTHDR);
					break;

				case IPV6_FAITH:
					OPTSET(IN6P_FAITH);
					break;

				case IPV6_RECVPATHMTU:
					OPTSET(IN6P_MTU);
					break;

				case IPV6_V6ONLY:
					/*
					 * make setsockopt(IPV6_V6ONLY)
					 * available only prior to bind(2).
					 * see ipng mailing list, Jun 22 2001.
					 */
					if (in6p->in6p_lport ||
					    !IN6_IS_ADDR_UNSPECIFIED(&in6p->in6p_laddr))
					{
						error = EINVAL;
						break;
					}
#ifdef __NetBSD__
#ifdef INET6_BINDV6ONLY
					if (!optval)
						error = EINVAL;
#else
					OPTSET(IN6P_IPV6_V6ONLY);
#endif
#elif (defined(__FreeBSD__) && __FreeBSD__ >= 3) || (defined(__bsdi__) && _BSDI_VERSION >= 199802)
					OPTSET(IN6P_IPV6_V6ONLY);
#else
					if ((ip6_v6only && optval) ||
					    (!ip6_v6only && !optval))
						error = 0;
					else
						error = EINVAL;
#endif
					break;
				case IPV6_RECVTCLASS:
					/* cannot mix with RFC2292 XXX */
					if (OPTBIT(IN6P_RFC2292)) {
						error = EINVAL;
						break;
					}
					OPTSET(IN6P_TCLASS);
					break;
				case IPV6_AUTOFLOWLABEL:
					OPTSET(IN6P_AUTOFLOWLABEL);
					break;

				}
				break;

			case IPV6_OTCLASS:
			{
				struct ip6_pktopts **optp;
				u_int8_t tclass;

				if (optlen != sizeof(tclass)) {
					error = EINVAL;
					break;
				}
#if defined(__FreeBSD__) && __FreeBSD__ >= 3
				error = sooptcopyin(sopt, &tclass,
					sizeof tclass, sizeof tclass);
				if (error)
					break;
#else
				tclass = *mtod(m, u_int8_t *);
#endif
				optp = &in6p->in6p_outputopts;
				error = ip6_pcbopt(optname,
						   (u_char *)&tclass,
						   sizeof(tclass),
						   optp,
						   privileged);
				break;
			}

			case IPV6_TCLASS:
			case IPV6_DONTFRAG:
			case IPV6_USE_MIN_MTU:
				if (optlen != sizeof(optval)) {
					error = EINVAL;
					break;
				}
#if defined(__FreeBSD__) && __FreeBSD__ >= 3
				error = sooptcopyin(sopt, &optval,
					sizeof optval, sizeof optval);
				if (error)
					break;
#else
				optval = *mtod(m, int *);
#endif
				{
					struct ip6_pktopts **optp;
					optp = &in6p->in6p_outputopts;
					error = ip6_pcbopt(optname,
							   (u_char *)&optval,
							   sizeof(optval),
							   optp,
							   privileged);
					break;
				}

			case IPV6_2292PKTINFO:
			case IPV6_2292HOPLIMIT:
			case IPV6_2292HOPOPTS:
			case IPV6_2292DSTOPTS:
			case IPV6_2292RTHDR:
				/* RFC 2292 */
				if (optlen != sizeof(int)) {
					error = EINVAL;
					break;
				}
#if defined(__FreeBSD__) && __FreeBSD__ >= 3
				error = sooptcopyin(sopt, &optval,
					sizeof optval, sizeof optval);
				if (error)
					break;
#else
				optval = *mtod(m, int *);
#endif
				switch (optname) {
				case IPV6_2292PKTINFO:
					OPTSET2292(IN6P_PKTINFO);
					if (OPTBIT(IN6P_PKTINFO) == 0)
						ip6_reset_rcvopt(rcvopts, IPV6_RECVPKTINFO);
					break;
				case IPV6_2292HOPLIMIT:
					OPTSET2292(IN6P_HOPLIMIT);
					if (OPTBIT(IN6P_HOPLIMIT) == 0)
						ip6_reset_rcvopt(rcvopts, IPV6_RECVHOPLIMIT);
					break;
				case IPV6_2292HOPOPTS:
					/*
					 * Check super-user privilege.
					 * See comments for IPV6_RECVHOPOPTS.
					 */
					OPTSET2292(IN6P_HOPOPTS);
					if (OPTBIT(IN6P_HOPOPTS) == 0)
						ip6_reset_rcvopt(rcvopts, IPV6_RECVHOPOPTS);
					break;
				case IPV6_2292DSTOPTS:
					OPTSET2292(IN6P_DSTOPTS|IN6P_RTHDRDSTOPTS); /* XXX */
					if (OPTBIT(IN6P_DSTOPTS) == 0) {
						ip6_reset_rcvopt(rcvopts, IPV6_RECVDSTOPTS);
						ip6_reset_rcvopt(rcvopts, IPV6_RECVRTHDRDSTOPTS);
					}
					break;
				case IPV6_2292RTHDR:
					OPTSET2292(IN6P_RTHDR);
					if (OPTBIT(IN6P_RTHDR) == 0)
						ip6_reset_rcvopt(rcvopts, IPV6_RECVRTHDR);
					break;
				}
				break;
			case IPV6_PKTINFO:
			case IPV6_HOPOPTS:
			case IPV6_RTHDR:
			case IPV6_DSTOPTS:
			case IPV6_RTHDRDSTOPTS:
			case IPV6_NEXTHOP:
			{
				/* new advanced API (2292bis) */
				u_char *optbuf;
				int optlen;
				struct ip6_pktopts **optp;

				/* cannot mix with RFC2292 */
				if (OPTBIT(IN6P_RFC2292)) {
					error = EINVAL;
					break;
				}

#if defined(__FreeBSD__) && __FreeBSD__ >= 3
				optbuf = sopt->sopt_val;
				optlen = sopt->sopt_valsize;
#else  /* !fbsd3 */
				if (m && m->m_next) {
					error = EINVAL;	/* XXX */
					break;
				}
				if (m) {
					optbuf = mtod(m, u_char *);
					optlen = m->m_len;
				} else {
					optbuf = NULL;
					optlen = 0;
				}
#endif
				optp = &in6p->in6p_outputopts;
				error = ip6_pcbopt(optname,
						   optbuf, optlen,
						   optp, privileged);
				break;
			}
#undef OPTSET

			case IPV6_MULTICAST_IF:
			case IPV6_MULTICAST_HOPS:
			case IPV6_MULTICAST_LOOP:
			case IPV6_JOIN_GROUP:
			case IPV6_LEAVE_GROUP:
#if defined(__FreeBSD__) && __FreeBSD__ >= 3
			    {
				struct mbuf *m;
				if (sopt->sopt_valsize > MLEN) {
					error = EMSGSIZE;
					break;
				}
				/* XXX */
				MGET(m, sopt->sopt_p ? M_WAIT : M_DONTWAIT, MT_HEADER);
				if (m == 0) {
					error = ENOBUFS;
					break;
				}
				m->m_len = sopt->sopt_valsize;
				error = sooptcopyin(sopt, mtod(m, char *),
						    m->m_len, m->m_len);
				error =	ip6_setmoptions(sopt->sopt_name,
							&in6p->in6p_moptions,
							m);
				(void)m_free(m);
			    }
#else
				error =	ip6_setmoptions(optname,
							&in6p->in6p_moptions,
							m);
#if defined(__bsdi__) && _BSDI_VERSION >= 199802
				if (in6p->in6p_moptions != NULL)
					in6p->in6p_flags |= INP_IPV6_MCAST; /* XXX */
#endif
#endif
				break;

#ifndef __bsdi__
			case IPV6_PORTRANGE:
#if defined(__FreeBSD__) && __FreeBSD__ >= 3
				error = sooptcopyin(sopt, &optval,
				    sizeof optval, sizeof optval);
				if (error)
					break;
#else
				optval = *mtod(m, int *);
#endif

				switch (optval) {
				case IPV6_PORTRANGE_DEFAULT:
					in6p->in6p_flags &= ~(IN6P_LOWPORT);
					in6p->in6p_flags &= ~(IN6P_HIGHPORT);
					break;

				case IPV6_PORTRANGE_HIGH:
					in6p->in6p_flags &= ~(IN6P_LOWPORT);
					in6p->in6p_flags |= IN6P_HIGHPORT;
					break;

				case IPV6_PORTRANGE_LOW:
					in6p->in6p_flags &= ~(IN6P_HIGHPORT);
					in6p->in6p_flags |= IN6P_LOWPORT;
					break;

				default:
					error = EINVAL;
					break;
				}
				break;
#endif

#ifdef __OpenBSD__
			case IPSEC6_OUTSA:
#ifndef IPSEC
				error = EINVAL;
#else
				s = spltdb();
				if (m == 0 || m->m_len != sizeof(struct tdb_ident)) {
					error = EINVAL;
				} else {
					tdbip = mtod(m, struct tdb_ident *);
					tdb = gettdb(tdbip->spi, &tdbip->dst,
					    tdbip->proto);
					if (tdb == NULL)
						error = ESRCH;
					else
						tdb_add_inp(tdb, inp, 0);
				}
				splx(s);
#endif
				break;

			case IPV6_AUTH_LEVEL:
			case IPV6_ESP_TRANS_LEVEL:
			case IPV6_ESP_NETWORK_LEVEL:
			case IPV6_IPCOMP_LEVEL:
#ifndef IPSEC
				error = EINVAL;
#else
				if (m == 0 || m->m_len != sizeof(int)) {
					error = EINVAL;
					break;
				}
				optval = *mtod(m, int *);

				if (optval < IPSEC_LEVEL_BYPASS || 
				    optval > IPSEC_LEVEL_UNIQUE) {
					error = EINVAL;
					break;
				}
					
				switch (optname) {
				case IPV6_AUTH_LEVEL:
					inp->inp_seclevel[SL_AUTH] = optval;
					break;

				case IPV6_ESP_TRANS_LEVEL:
					inp->inp_seclevel[SL_ESP_TRANS] = optval;
					break;

				case IPV6_ESP_NETWORK_LEVEL:
					inp->inp_seclevel[SL_ESP_NETWORK] = optval;
					break;

				case IPV6_IPCOMP_LEVEL:
					inp->inp_seclevel[SL_IPCOMP] = optval;
					break;
				}
				if (!error)
					inp->inp_secrequire = get_sa_require(inp);
#endif
				break;
#endif /* OpenBSD */

#if defined(IPSEC) && !defined(__OpenBSD__)
			case IPV6_IPSEC_POLICY:
			    {
				caddr_t req = NULL;
				size_t len = 0;
#if defined(__FreeBSD__) && __FreeBSD__ >= 3
				struct mbuf *m;
#endif

#if defined(__FreeBSD__) && __FreeBSD__ >= 3
				if ((error = soopt_getm(sopt, &m)) != 0) /* XXX */
					break;
				if ((error = soopt_mcopyin(sopt, m)) != 0) /* XXX */
					break;
#endif
				if (m) {
					req = mtod(m, caddr_t);
					len = m->m_len;
				}
				error = ipsec6_set_policy(in6p, optname, req,
							  len, privileged);
#if defined(__FreeBSD__) && __FreeBSD__ >= 3
				m_freem(m);
#endif
			    }
				break;
#endif /* KAME IPSEC */

#if defined(IPV6FIREWALL) || (defined(__FreeBSD__) && __FreeBSD__ >= 4)
			case IPV6_FW_ADD:
			case IPV6_FW_DEL:
			case IPV6_FW_FLUSH:
			case IPV6_FW_ZERO:
			    {
#if defined(__FreeBSD__) && __FreeBSD__ >= 3
				struct mbuf *m;
				struct mbuf **mp = &m;
#endif

#if defined(__FreeBSD__) && __FreeBSD__ >= 3
				if (ip6_fw_ctl_ptr == NULL)
					return EINVAL;
				/* XXX */
				if ((error = soopt_getm(sopt, &m)) != 0)
					break;
				/* XXX */
				if ((error = soopt_mcopyin(sopt, m)) != 0)
					break;
#else
				if (ip6_fw_ctl_ptr == NULL) {
					if (m) (void)m_free(m);
					return EINVAL;
				}
#endif
				error = (*ip6_fw_ctl_ptr)(optname, mp);
				m = *mp;
			    }
				break;
#endif

			default:
				error = ENOPROTOOPT;
				break;
			}
#if !(defined(__FreeBSD__) && __FreeBSD__ >= 3)
			if (m)
				(void)m_free(m);
#endif
			break;

#if defined(__FreeBSD__) && __FreeBSD__ >= 3
		case SOPT_GET:
#else
		case PRCO_GETOPT:
#endif
			switch (optname) {

			case IPV6_2292PKTOPTIONS:
#ifdef IPV6_PKTOPTIONS
			case IPV6_PKTOPTIONS:
#endif
#if defined(__FreeBSD__) && __FreeBSD__ >= 3
				if (in6p->in6p_inputopts &&
				    in6p->in6p_inputopts->head) {
					struct mbuf *m;
					m = m_copym(in6p->in6p_inputopts->head,
					    0, M_COPYALL, M_WAIT);
					error = soopt_mcopyout(sopt, m);
					if (error == 0)
						m_freem(m);
				} else
					sopt->sopt_valsize = 0;
#else
				if (in6p->in6p_inputopts &&
				    in6p->in6p_inputopts->head) {
					*mp = m_copym(in6p->in6p_inputopts->head,
						      0, M_COPYALL, M_WAIT);
				} else {
					*mp = m_get(M_WAIT, MT_SOOPTS);
					(*mp)->m_len = 0;
				}
#endif
				break;

			case IPV6_RECVHOPOPTS:
			case IPV6_RECVDSTOPTS:
			case IPV6_RECVRTHDRDSTOPTS:
			case IPV6_UNICAST_HOPS:
			case IPV6_RECVPKTINFO:
			case IPV6_RECVHOPLIMIT:
			case IPV6_RECVRTHDR:
			case IPV6_USE_MIN_MTU:
			case IPV6_RECVPATHMTU:
			case IPV6_DONTFRAG:

			case IPV6_FAITH:
			case IPV6_V6ONLY:
#ifndef __bsdi__
			case IPV6_PORTRANGE:
#endif
			case IPV6_RECVTCLASS:
			case IPV6_AUTOFLOWLABEL:
				switch (optname) {

				case IPV6_UNICAST_HOPS:
					optval = in6p->in6p_hops;
					break;

				case IPV6_RECVPKTINFO:
					optval = OPTBIT(IN6P_PKTINFO);
					break;

				case IPV6_RECVHOPLIMIT:
					optval = OPTBIT(IN6P_HOPLIMIT);
					break;

				case IPV6_RECVHOPOPTS:
					optval = OPTBIT(IN6P_HOPOPTS);
					break;

				case IPV6_RECVDSTOPTS:
					optval = OPTBIT(IN6P_DSTOPTS);
					break;

				case IPV6_RECVRTHDRDSTOPTS:
					optval = OPTBIT(IN6P_RTHDRDSTOPTS);
					break;

				case IPV6_RECVPATHMTU:
					optval = OPTBIT(IN6P_MTU);
					break;

				case IPV6_FAITH:
					optval = OPTBIT(IN6P_FAITH);
					break;

				case IPV6_V6ONLY:
#if (defined(__FreeBSD__) && __FreeBSD__ >= 3) || defined(__NetBSD__) || (defined(__bsdi__) && _BSDI_VERSION >= 199802) 
					optval = OPTBIT(IN6P_IPV6_V6ONLY);
#else
					optval = (ip6_v6only != 0); /* XXX */
#endif
					break;

#ifndef __bsdi__
				case IPV6_PORTRANGE:
				    {
					int flags;
					flags = in6p->in6p_flags;
					if (flags & IN6P_HIGHPORT)
						optval = IPV6_PORTRANGE_HIGH;
					else if (flags & IN6P_LOWPORT)
						optval = IPV6_PORTRANGE_LOW;
					else
						optval = 0;
					break;
				    }
#endif
				case IPV6_RECVTCLASS:
					optval = OPTBIT(IN6P_TCLASS);
					break;

				case IPV6_AUTOFLOWLABEL:
					optval = OPTBIT(IN6P_AUTOFLOWLABEL);
					break;

#define PKTOPTBIT(bit) ((in6p->in6p_outputopts && \
		         (in6p->in6p_outputopts->ip6po_flags & (bit))) ? 1 : 0)
				case IPV6_DONTFRAG:
					optval = PKTOPTBIT(IP6PO_DONTFRAG);
					break;

				case IPV6_USE_MIN_MTU:
					optval = PKTOPTBIT(IP6PO_MINMTU);
					break;
#undef PKTOPTBIT
				}
				if (error)
					break;
#if defined(__FreeBSD__) && __FreeBSD__ >= 3
				error = sooptcopyout(sopt, &optval,
					sizeof optval);
#else
				*mp = m = m_get(M_WAIT, MT_SOOPTS);
				m->m_len = sizeof(int);
				*mtod(m, int *) = optval;
#endif
				break;

			case IPV6_PATHMTU:
			{
				u_long pmtu = 0;
				struct ip6_mtuinfo mtuinfo;
#ifdef NEW_STRUCT_ROUTE
				struct route *ro = &in6p->in6p_route;
#else
				struct route_in6 *ro = (struct route_in6 *)&in6p->in6p_route;
#endif

				if (!(so->so_state & SS_ISCONNECTED))
					return(ENOTCONN);
				/*
				 * XXX: we dot not consider the case of source
				 * routing, nor optional information to specify
				 * the outgoing interface.
				 */
				error = ip6_getpmtu(ro, NULL, NULL,
						    &in6p->in6p_faddr, &pmtu);
				if (error)
					break;
				if (pmtu > IPV6_MAXPACKET)
					pmtu = IPV6_MAXPACKET;

				bzero(&mtuinfo, sizeof(mtuinfo));
				mtuinfo.ip6m_mtu = (u_int32_t)pmtu;
				optdata = (void *)&mtuinfo;
				optdatalen = sizeof(mtuinfo);
#if defined(__FreeBSD__) && __FreeBSD__ >= 3
				error = sooptcopyout(sopt, optdata,
						     optdatalen);
#else  /* !FreeBSD3 */
					if (optdatalen > MCLBYTES)
						return(EMSGSIZE); /* XXX */
					*mp = m = m_get(M_WAIT, MT_SOOPTS);
					if (optdatalen > MLEN)
						MCLGET(m, M_WAIT);
					m->m_len = optdatalen;
					bcopy(optdata, mtod(m, void *),
					      optdatalen);
#endif /* FreeBSD3 */
				break;
			}

			case IPV6_2292PKTINFO:
			case IPV6_2292HOPLIMIT:
			case IPV6_2292HOPOPTS:
			case IPV6_2292RTHDR:
			case IPV6_2292DSTOPTS:
				if (optname == IPV6_2292HOPOPTS ||
				    optname == IPV6_2292DSTOPTS ||
				    !privileged)
					return(EPERM);
				switch (optname) {
				case IPV6_2292PKTINFO:
					optval = OPTBIT(IN6P_PKTINFO);
					break;
				case IPV6_2292HOPLIMIT:
					optval = OPTBIT(IN6P_HOPLIMIT);
					break;
				case IPV6_2292HOPOPTS:
					optval = OPTBIT(IN6P_HOPOPTS);
					break;
				case IPV6_2292RTHDR:
					optval = OPTBIT(IN6P_RTHDR);
					break;
				case IPV6_2292DSTOPTS:
					optval = OPTBIT(IN6P_DSTOPTS|IN6P_RTHDRDSTOPTS);
					break;
				}
#if defined(__FreeBSD__) && __FreeBSD__ >= 3
				error = sooptcopyout(sopt, &optval,
					sizeof optval);
#else
				*mp = m = m_get(M_WAIT, MT_SOOPTS);
				m->m_len = sizeof(int);
				*mtod(m, int *) = optval;
#endif /* FreeBSD3 */
				break;
			case IPV6_PKTINFO:
			case IPV6_HOPOPTS:
			case IPV6_RTHDR:
			case IPV6_DSTOPTS:
			case IPV6_RTHDRDSTOPTS:
			case IPV6_NEXTHOP:
			case IPV6_OTCLASS:
			case IPV6_TCLASS:
#if defined(__FreeBSD__) && __FreeBSD__ >= 3
				error = ip6_getpcbopt(in6p->in6p_outputopts,
						      optname, sopt);
#else
				error = ip6_getpcbopt(in6p->in6p_outputopts,
						      optname, mp);
#endif
				break;

			case IPV6_MULTICAST_IF:
			case IPV6_MULTICAST_HOPS:
			case IPV6_MULTICAST_LOOP:
			case IPV6_JOIN_GROUP:
			case IPV6_LEAVE_GROUP:
#if defined(__FreeBSD__) && __FreeBSD__ >= 3
			    {
				struct mbuf *m;
				error = ip6_getmoptions(sopt->sopt_name,
						in6p->in6p_moptions, &m);
				if (error == 0)
					error = sooptcopyout(sopt,
						mtod(m, char *), m->m_len);
				m_freem(m);
			    }
#else
				error = ip6_getmoptions(optname, in6p->in6p_moptions, mp);
#endif
				break;

#ifdef __OpenBSD__
			case IPSEC6_OUTSA:
#ifndef IPSEC
				error = EINVAL;
#else
				s = spltdb();
				if (inp->inp_tdb_out == NULL) {
					error = ENOENT;
				} else {
					tdbi.spi = inp->inp_tdb_out->tdb_spi;
					tdbi.dst = inp->inp_tdb_out->tdb_dst;
					tdbi.proto = inp->inp_tdb_out->tdb_sproto;
					*mp = m = m_get(M_WAIT, MT_SOOPTS);
					m->m_len = sizeof(tdbi);
					bcopy((caddr_t)&tdbi, mtod(m, caddr_t),
					    (unsigned)m->m_len);
				}
				splx(s);
#endif
				break;

			case IPV6_AUTH_LEVEL:
			case IPV6_ESP_TRANS_LEVEL:
			case IPV6_ESP_NETWORK_LEVEL:
			case IPV6_IPCOMP_LEVEL:
#ifndef IPSEC
				m->m_len = sizeof(int);
				*mtod(m, int *) = IPSEC_LEVEL_NONE;
#else
				m->m_len = sizeof(int);
				switch (optname) {
				case IPV6_AUTH_LEVEL:
					optval = inp->inp_seclevel[SL_AUTH];
					break;

				case IPV6_ESP_TRANS_LEVEL:
					optval =
					    inp->inp_seclevel[SL_ESP_TRANS];
					break;

				case IPV6_ESP_NETWORK_LEVEL:
					optval =
					    inp->inp_seclevel[SL_ESP_NETWORK];
					break;

				case IPV6_IPCOMP_LEVEL:
					optval = inp->inp_seclevel[SL_IPCOMP];
					break;
				}
				*mtod(m, int *) = optval;
#endif
				break;
#endif /* OpenBSD */

#if defined(IPSEC) && !defined(__OpenBSD__)
			case IPV6_IPSEC_POLICY:
			  {
				caddr_t req = NULL;
				size_t len = 0;
#if defined(__FreeBSD__) && __FreeBSD__ >= 3
				struct mbuf *m = NULL;
				struct mbuf **mp = &m;

				error = soopt_getm(sopt, &m); /* XXX */
				if (error != 0)
					break;
				error = soopt_mcopyin(sopt, m); /* XXX */
				if (error != 0)
					break;
#endif
				if (m) {
					req = mtod(m, caddr_t);
					len = m->m_len;
				}
				error = ipsec6_get_policy(in6p, req, len, mp);
#if defined(__FreeBSD__) && __FreeBSD__ >= 3
				if (error == 0)
					error = soopt_mcopyout(sopt, m); /* XXX */
				if (error == 0 && m)
					m_freem(m);
#endif
				break;
			  }
#endif /* KAME IPSEC */

#if defined(IPV6FIREWALL) || (defined(__FreeBSD__) && __FreeBSD__ >= 4)
			case IPV6_FW_GET:
			  {
#if defined(__FreeBSD__) && __FreeBSD__ >= 3
				struct mbuf *m;
				struct mbuf **mp = &m;
#endif

				if (ip6_fw_ctl_ptr == NULL)
			        {
#if !(defined(__FreeBSD__) && __FreeBSD__ >= 3)
					if (m)
						(void)m_free(m);
#endif
					return EINVAL;
				}
				error = (*ip6_fw_ctl_ptr)(optname, mp);
#if defined(__FreeBSD__) && __FreeBSD__ >= 3
				if (error == 0)
					error = soopt_mcopyout(sopt, m); /* XXX */
				if (error == 0 && m)
					m_freem(m);
#endif
			  }
				break;
#endif

			default:
				error = ENOPROTOOPT;
				break;
			}
			break;
		}
	} else {		/* level != IPPROTO_IPV6 */
		error = EINVAL;
#if !(defined(__FreeBSD__) && __FreeBSD__ >= 3)
		if (op == PRCO_SETOPT && *mp)
			(void)m_free(*mp);
#endif
	}
	return(error);
}

#ifndef offsetof
#define	offsetof(type, member)	((size_t)(&((type *)0)->member)) /* XXX */
#endif
#if defined(__FreeBSD__) && __FreeBSD__ >= 3
int
ip6_raw_ctloutput(so, sopt)
	struct socket *so;
	struct sockopt *sopt;
#else
int
ip6_raw_ctloutput(op, so, level, optname, mp)
	int op;
	struct socket *so;
	int level, optname;
	struct mbuf **mp;
#endif
{
	int error = 0, optval, optlen;
	const int icmp6off = offsetof(struct icmp6_hdr, icmp6_cksum);
#ifdef HAVE_NRL_INPCB
	struct inpcb *inp = sotoinpcb(so);
#else
	struct in6pcb *in6p = sotoin6pcb(so);
#endif
#if defined(__FreeBSD__) && __FreeBSD__ >= 3
	int level, op, optname;
	struct proc *p;
#else
	struct mbuf *m = *mp;
#endif /* FreeBSD >= 3 */

#if defined(__FreeBSD__) && __FreeBSD__ >= 3
	if (!sopt) {
		panic("ip6_ctloutput: arg soopt is NULL");
	}
        level = sopt->sopt_level;
        op = sopt->sopt_dir;
        optname = sopt->sopt_name;
        optlen = sopt->sopt_valsize;
        p = sopt->sopt_p;
#else
	optlen = m ? m->m_len : 0;
#endif /* FreeBSD >= 3 */

	if (level != IPPROTO_IPV6) {
#if !(defined(__FreeBSD__) && __FreeBSD__ >= 3)
		if (op == PRCO_SETOPT && *mp)
			(void)m_free(*mp);
#endif
		return(EINVAL);
	}
		
	switch (optname) {
	case IPV6_CHECKSUM:
		/*
		 * For ICMPv6 sockets, no modification allowed for checksum
		 * offset, permit "no change" values to help existing apps.
		 *
		 * XXX 2292bis says: "An attempt to set IPV6_CHECKSUM
		 * for an ICMPv6 socket will fail."
		 * The current behavior does not meet 2292bis.
		 */
		switch (op) {
#if defined(__FreeBSD__) && __FreeBSD__ >= 3
		case SOPT_SET:
#else
		case PRCO_SETOPT:
#endif
			if (optlen != sizeof(int)) {
				error = EINVAL;
				break;
			}
#if defined(__FreeBSD__) && __FreeBSD__ >= 3
			error = sooptcopyin(sopt, &optval, sizeof(optval),
					    sizeof(optval));
			if (error)
				break;
#else
			optval = *mtod(m, int *);
#endif
			if ((optval % 2) != 0) {
				/* the API assumes even offset values */
				error = EINVAL;
			} else if (so->so_proto->pr_protocol ==
				   IPPROTO_ICMPV6) {
				if (optval != icmp6off)
					error = EINVAL;
			} else
				in6p->in6p_cksum = optval;
			break;

#if defined(__FreeBSD__) && __FreeBSD__ >= 3
		case SOPT_GET:
#else
		case PRCO_GETOPT:
#endif
			if (so->so_proto->pr_protocol == IPPROTO_ICMPV6)
				optval = icmp6off;
			else
				optval = in6p->in6p_cksum;

#if defined(__FreeBSD__) && __FreeBSD__ >= 3
			error = sooptcopyout(sopt, &optval, sizeof(optval));
#else
			*mp = m = m_get(M_WAIT, MT_SOOPTS);
			m->m_len = sizeof(int);
			*mtod(m, int *) = optval;
#endif /* FreeBSD3 */
			break;
		default:
			error = EINVAL;
			break;
		}

	default:
		error = ENOPROTOOPT;
		break;
	}

#if !(defined(__FreeBSD__) && __FreeBSD__ >= 3)
	if (op == PRCO_SETOPT && m)
		(void)m_free(m);
#endif

	return(error);
}

#ifdef HAVE_NRL_INPCB
#undef in6p
#endif

/*
 * Set up IP6 options in pcb for insertion in output packets or
 * specifying behavior of outgoing packets.
 */
static int
#if defined(__FreeBSD__) && __FreeBSD__ >= 3
ip6_pcbopts(pktopt, m, so, sopt)
#else
ip6_pcbopts(pktopt, m, so)
#endif
	struct ip6_pktopts **pktopt;
	struct mbuf *m;
	struct socket *so;
#if defined(__FreeBSD__) && __FreeBSD__ >= 3
	struct sockopt *sopt;
#endif
{
	struct ip6_pktopts *opt = *pktopt;
	int error = 0;
	int priv = 1;

	/* turn off any old options. */
	if (opt) {
#ifdef DIAGNOSTIC
	    if (opt->ip6po_pktinfo || opt->ip6po_nexthop ||
		opt->ip6po_hbh || opt->ip6po_dest1 || opt->ip6po_dest2 ||
		opt->ip6po_rhinfo.ip6po_rhi_rthdr)
		    printf("ip6_pcbopts: all specified options are cleared.\n");
#endif
		ip6_clearpktopts(opt, -1);
	} else
		opt = malloc(sizeof(*opt), M_IP6OPT, M_WAITOK);
	*pktopt = NULL;

	if (!m || m->m_len == 0) {
		/*
		 * Only turning off any previous options.
		 */
		if (opt)
			free(opt, M_IP6OPT);
		return(0);
	}

	if ((error = ip6_setpktoptions(m, opt, NULL, priv, 1)) != 0) {
		ip6_clearpktopts(opt, -1); /* XXX: discard all options */
		return(error);
	}
	*pktopt = opt;
	return(0);
}

/*
 * initialize ip6_pktopts.  beware that there are non-zero default values in
 * the struct.
 */
void
init_ip6pktopts(opt)
	struct ip6_pktopts *opt;
{

	bzero(opt, sizeof(*opt));
	opt->ip6po_hlim = -1;	/* -1 means default hop limit */
	opt->ip6po_tclass = -1;	/* -1 means default traffic class */
}

#define sin6tosa(sin6)	((struct sockaddr *)(sin6)) /* XXX */
static int
ip6_pcbopt(optname, buf, len, pktopt, priv)
	int optname, len, priv;
	u_char *buf;
	struct ip6_pktopts **pktopt;
{
	struct ip6_pktopts *opt;
	
	if (*pktopt == NULL) {
		*pktopt = malloc(sizeof(struct ip6_pktopts), M_IP6OPT,
				 M_WAITOK);
		init_ip6pktopts(*pktopt);
		(*pktopt)->needfree = 1;
	}
	opt = *pktopt;

	return(ip6_setpktoption(optname, buf, len, opt, priv, 1, 0));
}

#if defined(__FreeBSD__) && __FreeBSD__ >= 3
static int
ip6_getpcbopt(pktopt, optname, sopt)
	struct ip6_pktopts *pktopt;
	struct sockopt *sopt;
	int optname;
#else
static int
ip6_getpcbopt(pktopt, optname, mp)
	struct ip6_pktopts *pktopt;
	int optname;
	struct mbuf **mp;
#endif
{
	void *optdata = NULL;
	int optdatalen = 0;
	struct ip6_ext *ip6e;
	int error = 0;
	struct in6_pktinfo null_pktinfo;
	int deftclass = 0;
#if !(defined(__FreeBSD__) && __FreeBSD__ >= 3)
	struct mbuf *m;
#endif

	switch (optname) {
	case IPV6_PKTINFO:
		if (pktopt && pktopt->ip6po_pktinfo)
			optdata = (void *)pktopt->ip6po_pktinfo;
		else {
			/* XXX: we don't have to do this every time... */
			bzero(&null_pktinfo, sizeof(null_pktinfo));
			optdata = (void *)&null_pktinfo;
		}
		optdatalen = sizeof(struct in6_pktinfo);
		break;
	case IPV6_OTCLASS:
		/* XXX */
		return(EINVAL);
	case IPV6_TCLASS:
		if (pktopt && pktopt->ip6po_tclass >= 0)
			optdata = (void *)&pktopt->ip6po_tclass;
		else
			optdata = (void *)&deftclass;
		optdatalen = sizeof(int);
		break;
	case IPV6_HOPOPTS:
		if (pktopt && pktopt->ip6po_hbh) {
			optdata = (void *)pktopt->ip6po_hbh;
			ip6e = (struct ip6_ext *)pktopt->ip6po_hbh;
			optdatalen = (ip6e->ip6e_len + 1) << 3;
		}
		break;
	case IPV6_RTHDR:
		if (pktopt && pktopt->ip6po_rthdr) {
			optdata = (void *)pktopt->ip6po_rthdr;
			ip6e = (struct ip6_ext *)pktopt->ip6po_rthdr;
			optdatalen = (ip6e->ip6e_len + 1) << 3;
		}
		break;
	case IPV6_RTHDRDSTOPTS:
		if (pktopt && pktopt->ip6po_dest1) {
			optdata = (void *)pktopt->ip6po_dest1;
			ip6e = (struct ip6_ext *)pktopt->ip6po_dest1;
			optdatalen = (ip6e->ip6e_len + 1) << 3;
		}
		break;
	case IPV6_DSTOPTS:
		if (pktopt && pktopt->ip6po_dest2) {
			optdata = (void *)pktopt->ip6po_dest2;
			ip6e = (struct ip6_ext *)pktopt->ip6po_dest2;
			optdatalen = (ip6e->ip6e_len + 1) << 3;
		}
		break;
	case IPV6_NEXTHOP:
		if (pktopt && pktopt->ip6po_nexthop) {
			optdata = (void *)pktopt->ip6po_nexthop;
			optdatalen = pktopt->ip6po_nexthop->sa_len;
		}
		break;
	default:		/* should not happen */
		printf("ip6_getpcbopt: unexpected option: %d\n", optname);
		return(ENOPROTOOPT);
	}

#if defined(__FreeBSD__) && __FreeBSD__ >= 3
	error = sooptcopyout(sopt, optdata, optdatalen);
#else  /* !FreeBSD3 */
	if (optdatalen > MCLBYTES)
		return(EMSGSIZE); /* XXX */
	*mp = m = m_get(M_WAIT, MT_SOOPTS);
	if (optdatalen > MLEN)
		MCLGET(m, M_WAIT);
	m->m_len = optdatalen;
	if (optdatalen)
		bcopy(optdata, mtod(m, void *), optdatalen);
#endif /* FreeBSD3 */

	return(error);
}

void
ip6_clearpktopts(pktopt, optname)
	struct ip6_pktopts *pktopt;
	int optname;
{
	int needfree = pktopt->needfree;

	if (pktopt == NULL)
		return;

	if (optname == -1 || optname == IPV6_PKTINFO) {
		if (needfree && pktopt->ip6po_pktinfo)
			free(pktopt->ip6po_pktinfo, M_IP6OPT);
		pktopt->ip6po_pktinfo = NULL;
	}
	if (optname == -1 || optname == IPV6_HOPLIMIT)
		pktopt->ip6po_hlim = -1;
	if (optname == -1 || optname == IPV6_TCLASS)
		pktopt->ip6po_tclass = -1;
	if (optname == -1 || optname == IPV6_NEXTHOP) {
		if (pktopt->ip6po_nextroute.ro_rt) {
			RTFREE(pktopt->ip6po_nextroute.ro_rt);
			pktopt->ip6po_nextroute.ro_rt = NULL;
		}
		if (needfree && pktopt->ip6po_nexthop)
			free(pktopt->ip6po_nexthop, M_IP6OPT);
		pktopt->ip6po_nexthop = NULL;
	}
	if (optname == -1 || optname == IPV6_HOPOPTS) {
		if (needfree && pktopt->ip6po_hbh)
			free(pktopt->ip6po_hbh, M_IP6OPT);
		pktopt->ip6po_hbh = NULL;
	}
	if (optname == -1 || optname == IPV6_RTHDRDSTOPTS) {
		if (needfree && pktopt->ip6po_dest1)
			free(pktopt->ip6po_dest1, M_IP6OPT);
		pktopt->ip6po_dest1 = NULL;
	}
	if (optname == -1 || optname == IPV6_RTHDR) {
		if (needfree && pktopt->ip6po_rhinfo.ip6po_rhi_rthdr)
			free(pktopt->ip6po_rhinfo.ip6po_rhi_rthdr, M_IP6OPT);
		pktopt->ip6po_rhinfo.ip6po_rhi_rthdr = NULL;
		if (pktopt->ip6po_route.ro_rt) {
			RTFREE(pktopt->ip6po_route.ro_rt);
			pktopt->ip6po_route.ro_rt = NULL;
		}
	}
	if (optname == -1 || optname == IPV6_DSTOPTS) {
		if (needfree && pktopt->ip6po_dest2)
			free(pktopt->ip6po_dest2, M_IP6OPT);
		pktopt->ip6po_dest2 = NULL;
	}
}

#define PKTOPT_EXTHDRCPY(type) \
do {\
	if (src->type) {\
		int hlen =\
			(((struct ip6_ext *)src->type)->ip6e_len + 1) << 3;\
		dst->type = malloc(hlen, M_IP6OPT, canwait);\
		if (dst->type == NULL && canwait == M_NOWAIT)\
			goto bad;\
		bcopy(src->type, dst->type, hlen);\
	}\
} while (0)

struct ip6_pktopts *
ip6_copypktopts(src, canwait)
	struct ip6_pktopts *src;
	int canwait;
{
	struct ip6_pktopts *dst;

	if (src == NULL) {
		printf("ip6_clearpktopts: invalid argument\n");
		return(NULL);
	}

	dst = malloc(sizeof(*dst), M_IP6OPT, canwait);
	if (dst == NULL && canwait == M_NOWAIT)
		goto bad;
	bzero(dst, sizeof(*dst));
	dst->needfree = 1;

	dst->ip6po_hlim = src->ip6po_hlim;
	dst->ip6po_tclass = src->ip6po_tclass;
	dst->ip6po_flags = src->ip6po_flags;
	if (src->ip6po_pktinfo) {
		dst->ip6po_pktinfo = malloc(sizeof(*dst->ip6po_pktinfo),
					    M_IP6OPT, canwait);
		if (dst->ip6po_pktinfo == NULL && canwait == M_NOWAIT)
			goto bad;
		*dst->ip6po_pktinfo = *src->ip6po_pktinfo;
	}
	if (src->ip6po_nexthop) {
		dst->ip6po_nexthop = malloc(src->ip6po_nexthop->sa_len,
					    M_IP6OPT, canwait);
		if (dst->ip6po_nexthop == NULL && canwait == M_NOWAIT)
			goto bad;
		bcopy(src->ip6po_nexthop, dst->ip6po_nexthop,
		      src->ip6po_nexthop->sa_len);
	}
	PKTOPT_EXTHDRCPY(ip6po_hbh);
	PKTOPT_EXTHDRCPY(ip6po_dest1);
	PKTOPT_EXTHDRCPY(ip6po_dest2);
	PKTOPT_EXTHDRCPY(ip6po_rthdr); /* not copy the cached route */
	return(dst);

  bad:
	printf("ip6_copypktopts: copy failed");
	if (dst->ip6po_pktinfo) free(dst->ip6po_pktinfo, M_IP6OPT);
	if (dst->ip6po_nexthop) free(dst->ip6po_nexthop, M_IP6OPT);
	if (dst->ip6po_hbh) free(dst->ip6po_hbh, M_IP6OPT);
	if (dst->ip6po_dest1) free(dst->ip6po_dest1, M_IP6OPT);
	if (dst->ip6po_dest2) free(dst->ip6po_dest2, M_IP6OPT);
	if (dst->ip6po_rthdr) free(dst->ip6po_rthdr, M_IP6OPT);
	return(NULL);
}
#undef PKTOPT_EXTHDRCPY

void
ip6_freepcbopts(pktopt)
	struct ip6_pktopts *pktopt;
{
	if (pktopt == NULL)
		return;

	ip6_clearpktopts(pktopt, -1);

	free(pktopt, M_IP6OPT);
}

/*
 * Set the IP6 multicast options in response to user setsockopt().
 */
static int
ip6_setmoptions(optname, im6op, m)
	int optname;
	struct ip6_moptions **im6op;
	struct mbuf *m;
{
	int error = 0;
	u_int loop, ifindex;
	struct ipv6_mreq *mreq;
	struct ifnet *ifp;
	struct ip6_moptions *im6o = *im6op;
	struct route ro;
	struct sockaddr_in6 *dst;
	struct in6_multi_mship *imm;

	if (im6o == NULL) {
		/*
		 * No multicast option buffer attached to the pcb;
		 * allocate one and initialize to default values.
		 */
		im6o = (struct ip6_moptions *)
			malloc(sizeof(*im6o), M_IPMOPTS, M_WAITOK);
		*im6op = im6o;
		im6o->im6o_multicast_ifp = NULL;
		im6o->im6o_multicast_hlim = ip6_defmcasthlim;
		im6o->im6o_multicast_loop = IPV6_DEFAULT_MULTICAST_LOOP;
		LIST_INIT(&im6o->im6o_memberships);
	}

	switch (optname) {

	case IPV6_MULTICAST_IF:
		/*
		 * Select the interface for outgoing multicast packets.
		 */
		if (m == NULL || m->m_len != sizeof(u_int)) {
			error = EINVAL;
			break;
		}
		bcopy(mtod(m, u_int *), &ifindex, sizeof(ifindex));
		if (ifindex < 0 || if_index < ifindex) {
			error = ENXIO;	/* XXX EINVAL? */
			break;
		}
#if defined(__FreeBSD__) && __FreeBSD__ >= 5
		ifp = ifnet_byindex(ifindex);
#else
		ifp = ifindex2ifnet[ifindex];
#endif
		if (ifp == NULL || (ifp->if_flags & IFF_MULTICAST) == 0) {
			error = EADDRNOTAVAIL;
			break;
		}
		im6o->im6o_multicast_ifp = ifp;
		break;

	case IPV6_MULTICAST_HOPS:
	    {
		/*
		 * Set the IP6 hoplimit for outgoing multicast packets.
		 */
		int optval;
		if (m == NULL || m->m_len != sizeof(int)) {
			error = EINVAL;
			break;
		}
		bcopy(mtod(m, u_int *), &optval, sizeof(optval));
		if (optval < -1 || optval >= 256)
			error = EINVAL;
		else if (optval == -1)
			im6o->im6o_multicast_hlim = ip6_defmcasthlim;
		else
			im6o->im6o_multicast_hlim = optval;
		break;
	    }

	case IPV6_MULTICAST_LOOP:
		/*
		 * Set the loopback flag for outgoing multicast packets.
		 * Must be zero or one.
		 */
		if (m == NULL || m->m_len != sizeof(u_int)) {
			error = EINVAL;
			break;
		}
		bcopy(mtod(m, u_int *), &loop, sizeof(loop));
		if (loop > 1) {
			error = EINVAL;
			break;
		}
		im6o->im6o_multicast_loop = loop;
		break;

	case IPV6_JOIN_GROUP:
		/*
		 * Add a multicast group membership.
		 * Group must be a valid IP6 multicast address.
		 */
		if (m == NULL || m->m_len != sizeof(struct ipv6_mreq)) {
			error = EINVAL;
			break;
		}
		mreq = mtod(m, struct ipv6_mreq *);
		if (IN6_IS_ADDR_UNSPECIFIED(&mreq->ipv6mr_multiaddr)) {
			/*
			 * We use the unspecified address to specify to accept
			 * all multicast addresses. Only super user is allowed
			 * to do this.
			 */
		} else if (!IN6_IS_ADDR_MULTICAST(&mreq->ipv6mr_multiaddr)) {
			error = EINVAL;
			break;
		}

		/*
		 * If the interface is specified, validate it.
		 */
		if (mreq->ipv6mr_interface < 0
		 || if_index < mreq->ipv6mr_interface) {
			error = ENXIO;	/* XXX EINVAL? */
			break;
		}
		/*
		 * If no interface was explicitly specified, choose an
		 * appropriate one according to the given multicast address.
		 */
		if (mreq->ipv6mr_interface == 0) {
			/*
			 * Look up the routing table for the
			 * address, and choose the outgoing interface.
			 *   XXX: is it a good approach?
			 */
			ro.ro_rt = NULL;
			dst = (struct sockaddr_in6 *)&ro.ro_dst;
			bzero(dst, sizeof(*dst));
			dst->sin6_len = sizeof(struct sockaddr_in6);
			dst->sin6_family = AF_INET6;
			dst->sin6_addr = mreq->ipv6mr_multiaddr;
			rtalloc((struct route *)&ro);
			if (ro.ro_rt == NULL) {
				error = EADDRNOTAVAIL;
				break;
			}
			ifp = ro.ro_rt->rt_ifp;
			rtfree(ro.ro_rt);
		} else {
#if defined(__FreeBSD__) && __FreeBSD__ >= 5
			ifp = ifnet_byindex(mreq->ipv6mr_interface);
#else
			ifp = ifindex2ifnet[mreq->ipv6mr_interface];
#endif
		}

		/*
		 * See if we found an interface, and confirm that it
		 * supports multicast
		 */
		if (ifp == NULL || (ifp->if_flags & IFF_MULTICAST) == 0) {
			error = EADDRNOTAVAIL;
			break;
		}
		/*
		 * Put interface index into the multicast address,
		 * if the address has interface/link-local scope.
		 * XXX: the embedded form is a KAME-local hack. 
		 */
		if (IN6_IS_ADDR_MC_INTFACELOCAL(&mreq->ipv6mr_multiaddr) ||
		    IN6_IS_ADDR_MC_LINKLOCAL(&mreq->ipv6mr_multiaddr)) {
			mreq->ipv6mr_multiaddr.s6_addr16[1]
				= htons(mreq->ipv6mr_interface);
		}
		/*
		 * See if the membership already exists.
		 */
		for (imm = im6o->im6o_memberships.lh_first;
		     imm != NULL; imm = imm->i6mm_chain.le_next)
			if (imm->i6mm_maddr->in6m_ifp == ifp &&
			    IN6_ARE_ADDR_EQUAL(&imm->i6mm_maddr->in6m_addr,
					       &mreq->ipv6mr_multiaddr))
				break;
		if (imm != NULL) {
			error = EADDRINUSE;
			break;
		}
		/*
		 * Everything looks good; add a new record to the multicast
		 * address list for the given interface.
		 */
		imm = in6_joingroup(ifp, &mreq->ipv6mr_multiaddr, &error);
		if (!imm)
			break;
		LIST_INSERT_HEAD(&im6o->im6o_memberships, imm, i6mm_chain);
		break;

	case IPV6_LEAVE_GROUP:
		/*
		 * Drop a multicast group membership.
		 * Group must be a valid IP6 multicast address.
		 */
		if (m == NULL || m->m_len != sizeof(struct ipv6_mreq)) {
			error = EINVAL;
			break;
		}
		mreq = mtod(m, struct ipv6_mreq *);
		if (IN6_IS_ADDR_UNSPECIFIED(&mreq->ipv6mr_multiaddr)) {
		} else if (!IN6_IS_ADDR_MULTICAST(&mreq->ipv6mr_multiaddr)) {
			error = EINVAL;
			break;
		}
		/*
		 * If an interface address was specified, get a pointer
		 * to its ifnet structure.
		 */
		if (mreq->ipv6mr_interface < 0
		 || if_index < mreq->ipv6mr_interface) {
			error = ENXIO;	/* XXX EINVAL? */
			break;
		}
#if defined(__FreeBSD__) && __FreeBSD__ >= 5
		ifp = ifnet_byindex(mreq->ipv6mr_interface);
#else
		ifp = ifindex2ifnet[mreq->ipv6mr_interface];
#endif
		/*
		 * Put interface index into the multicast address,
		 * if the address has interface/link-local scope.
		 */
		if (IN6_IS_ADDR_MC_INTFACELOCAL(&mreq->ipv6mr_multiaddr) ||
		    IN6_IS_ADDR_MC_LINKLOCAL(&mreq->ipv6mr_multiaddr)) {
			mreq->ipv6mr_multiaddr.s6_addr16[1]
				= htons(mreq->ipv6mr_interface);
		}
		/*
		 * Find the membership in the membership list.
		 */
		for (imm = im6o->im6o_memberships.lh_first;
		     imm != NULL; imm = imm->i6mm_chain.le_next) {
			if ((ifp == NULL ||
			     imm->i6mm_maddr->in6m_ifp == ifp) &&
			    IN6_ARE_ADDR_EQUAL(&imm->i6mm_maddr->in6m_addr,
					       &mreq->ipv6mr_multiaddr))
				break;
		}
		if (imm == NULL) {
			/* Unable to resolve interface */
			error = EADDRNOTAVAIL;
			break;
		}
		/*
		 * Give up the multicast address record to which the
		 * membership points.
		 */
		LIST_REMOVE(imm, i6mm_chain);
		in6_leavegroup(imm);
		break;

	default:
		error = EOPNOTSUPP;
		break;
	}

	/*
	 * If all options have default values, no need to keep the mbuf.
	 */
	if (im6o->im6o_multicast_ifp == NULL &&
	    im6o->im6o_multicast_hlim == ip6_defmcasthlim &&
	    im6o->im6o_multicast_loop == IPV6_DEFAULT_MULTICAST_LOOP &&
	    im6o->im6o_memberships.lh_first == NULL) {
		free(*im6op, M_IPMOPTS);
		*im6op = NULL;
	}

	return(error);
}

/*
 * Return the IP6 multicast options in response to user getsockopt().
 */
static int
ip6_getmoptions(optname, im6o, mp)
	int optname;
	struct ip6_moptions *im6o;
	struct mbuf **mp;
{
	u_int *hlim, *loop, *ifindex;

#ifdef __FreeBSD__
	*mp = m_get(M_WAIT, MT_HEADER);		/* XXX */
#else
	*mp = m_get(M_WAIT, MT_SOOPTS);
#endif

	switch (optname) {

	case IPV6_MULTICAST_IF:
		ifindex = mtod(*mp, u_int *);
		(*mp)->m_len = sizeof(u_int);
		if (im6o == NULL || im6o->im6o_multicast_ifp == NULL)
			*ifindex = 0;
		else
			*ifindex = im6o->im6o_multicast_ifp->if_index;
		return(0);

	case IPV6_MULTICAST_HOPS:
		hlim = mtod(*mp, u_int *);
		(*mp)->m_len = sizeof(u_int);
		if (im6o == NULL)
			*hlim = ip6_defmcasthlim;
		else
			*hlim = im6o->im6o_multicast_hlim;
		return(0);

	case IPV6_MULTICAST_LOOP:
		loop = mtod(*mp, u_int *);
		(*mp)->m_len = sizeof(u_int);
		if (im6o == NULL)
			*loop = ip6_defmcasthlim;
		else
			*loop = im6o->im6o_multicast_loop;
		return(0);

	default:
		return(EOPNOTSUPP);
	}
}

/*
 * Discard the IP6 multicast options.
 */
void
ip6_freemoptions(im6o)
	struct ip6_moptions *im6o;
{
	struct in6_multi_mship *imm;

	if (im6o == NULL)
		return;

	while ((imm = im6o->im6o_memberships.lh_first) != NULL) {
		LIST_REMOVE(imm, i6mm_chain);
		in6_leavegroup(imm);
	}
	free(im6o, M_IPMOPTS);
}

/*
 * Set IPv6 outgoing packet options based on advanced API.
 */
int
ip6_setpktoptions(control, opt, stickyopt, priv, needcopy)
	struct mbuf *control;
	struct ip6_pktopts *opt, *stickyopt;
	int priv, needcopy;
{
	struct cmsghdr *cm = 0;

	if (control == 0 || opt == 0)
		return(EINVAL);

	if (stickyopt) {
		/*
		 * If stickyopt is provided, make a local copy of the options
		 * for this particular packet, then override them by ancillary
		 * objects.
		 * XXX: need to gain a reference for the cached route of the
		 * next hop in case of the overriding.
		 */
		*opt = *stickyopt;
		if (opt->ip6po_nextroute.ro_rt)
			opt->ip6po_nextroute.ro_rt->rt_refcnt++;
	} else
		init_ip6pktopts(opt);
	opt->needfree = needcopy;

	/*
	 * XXX: Currently, we assume all the optional information is stored
	 * in a single mbuf.
	 */
	if (control->m_next)
		return(EINVAL);

	for (; control->m_len; control->m_data += CMSG_ALIGN(cm->cmsg_len),
		     control->m_len -= CMSG_ALIGN(cm->cmsg_len)) {
		int error;

		if (control->m_len < CMSG_LEN(0))
			return(EINVAL);

		cm = mtod(control, struct cmsghdr *);
		if (cm->cmsg_len == 0 || cm->cmsg_len > control->m_len)
			return(EINVAL);
		if (cm->cmsg_level != IPPROTO_IPV6)
			continue;

		error = ip6_setpktoption(cm->cmsg_type, CMSG_DATA(cm),
					 cm->cmsg_len - CMSG_LEN(0),
					 opt, priv, needcopy, 1);
		if (error)
			return(error);
	}

	return(0);
}

/*
 * Set a particular packet option, as a sticky option or an ancillary data
 * item.  "len" can be 0 only when it's a sticky option.
 * We have 4 cases of combination of "sticky" and "cmsg":
 * "sticky=0, cmsg=0": impossible
 * "sticky=0, cmsg=1": RFC2292 or rfc2292bis ancillary data
 * "sticky=1, cmsg=0": rfc2292bis socket option
 * "sticky=1, cmsg=1": RFC2292 socket option
 */
static int
ip6_setpktoption(optname, buf, len, opt, priv, sticky, cmsg)
	int optname, len, priv, cmsg;
	u_char *buf;
	struct ip6_pktopts *opt;
	int sticky;
{
	if (!sticky && !cmsg) {
#ifdef DIAGNOSTIC
		printf("ip6_setpktoption: impossible case\n");
#endif
		return(EINVAL);
	}

	/*
	 * IPV6_2292xxx is for backward compatibility to RFC2292, and should
	 * not be specified in the context of rfc2292bis.  Conversely,
	 * rfc2292bis types should not be specified in the context of RFC2292.
	 *
	 */
	if (!cmsg) {
		switch (optname) {
		case IPV6_2292PKTINFO:
		case IPV6_2292HOPLIMIT:
		case IPV6_2292NEXTHOP:
		case IPV6_2292HOPOPTS:
		case IPV6_2292DSTOPTS:
		case IPV6_2292RTHDR:
		case IPV6_2292PKTOPTIONS:
			return(ENOPROTOOPT);
		}
	}
	if (sticky && cmsg) {
		switch (optname) {
		case IPV6_PKTINFO:
		case IPV6_HOPLIMIT:
		case IPV6_NEXTHOP:
		case IPV6_HOPOPTS:
		case IPV6_DSTOPTS:
		case IPV6_RTHDRDSTOPTS:
		case IPV6_RTHDR:
		case IPV6_REACHCONF:
		case IPV6_USE_MIN_MTU:
		case IPV6_DONTFRAG:
		case IPV6_OTCLASS:
		case IPV6_TCLASS:
			return(ENOPROTOOPT);
		}
	}

	switch (optname) {
	case IPV6_2292PKTINFO:
	case IPV6_PKTINFO:
	{
		struct ifnet *ifp = NULL;
		struct in6_pktinfo *pktinfo;

		if (len != sizeof(struct in6_pktinfo))
			return(EINVAL);

		pktinfo = (struct in6_pktinfo *)buf;

		/*
		 * An application can clear any sticky IPV6_PKTINFO option by
		 * doing a "regular" setsockopt with ipi6_addr being
		 * in6addr_any and ipi6_ifindex being zero.
		 * [rfc2292bis-02, Section 6]
		 */
		if (optname == IPV6_PKTINFO && opt->ip6po_pktinfo) {
			if (pktinfo->ipi6_ifindex == 0 &&
			    IN6_IS_ADDR_UNSPECIFIED(&pktinfo->ipi6_addr)) {
				ip6_clearpktopts(opt, optname);
				break;
			}
		}

		/* validate the interface index if specified. */
		if (pktinfo->ipi6_ifindex > if_index ||
		    pktinfo->ipi6_ifindex < 0) {
			 return(ENXIO);
		}
		if (pktinfo->ipi6_ifindex) {
#if defined(__FreeBSD__) && __FreeBSD__ >= 5
			ifp = ifnet_byindex(pktinfo->ipi6_ifindex);
#else
			ifp = ifindex2ifnet[pktinfo->ipi6_ifindex];
#endif
			if (ifp == NULL)
				return(ENXIO);
		}

		/*
		 * We store the address anyway, and let in6_selectsrc()
		 * validate the specified address.  This is because ipi6_addr
		 * may not have enough information about its scope zone, and
		 * we may need additional information (such as outgoing
		 * interface or the scope zone of a destination address) to
		 * disambiguate the scope.
		 * XXX: the delay of the validation may confuse the
		 * application when it is used as a sticky option.
		 */
		if (sticky) {
			if (opt->ip6po_pktinfo == NULL) {
				opt->ip6po_pktinfo = malloc(sizeof(*pktinfo),
							    M_IP6OPT,
							    M_WAITOK);
			}
			bcopy(pktinfo, opt->ip6po_pktinfo, sizeof(*pktinfo));
		} else
			opt->ip6po_pktinfo = pktinfo;
		break;
	}

	case IPV6_2292HOPLIMIT:
	case IPV6_HOPLIMIT:
	{
		int *hlimp;

		/*
		 * rfc2292bis-03 obsoleted the usage of sticky IPV6_HOPLIMIT
		 * to simplify the ordering among hoplimit options.
		 */
		if (optname == IPV6_HOPLIMIT && sticky)
			return(ENOPROTOOPT);

		if (len != sizeof(int))
			return(EINVAL);
		hlimp = (int *)buf;
		if (*hlimp < -1 || *hlimp > 255)
			return(EINVAL);

		opt->ip6po_hlim = *hlimp;
		break;
	}

	case IPV6_OTCLASS:
		if (len != sizeof(u_int8_t))
			return(EINVAL);

		opt->ip6po_tclass = *(u_int8_t *)buf;
		break;

	case IPV6_TCLASS:
	{
		int tclass;

		if (len != sizeof(int))
			return(EINVAL);
		tclass = *(int *)buf;
		if (tclass < -1 || tclass > 255)
			return(EINVAL);

		opt->ip6po_tclass = tclass;
		break;
	}

	case IPV6_2292NEXTHOP:
	case IPV6_NEXTHOP:

		if (len == 0) {	/* just remove the option */
			ip6_clearpktopts(opt, IPV6_NEXTHOP);
			break;
		}

		/* check if cmsg_len is large enough for sa_len */
		if (len < sizeof(struct sockaddr) || len < *buf)
			return(EINVAL);

		switch (((struct sockaddr *)buf)->sa_family) {
		case AF_INET6:
		{
			struct sockaddr_in6 *sa6 = (struct sockaddr_in6 *)buf;
			int error;

			if (sa6->sin6_len != sizeof(struct sockaddr_in6))
				return(EINVAL);

			if (IN6_IS_ADDR_UNSPECIFIED(&sa6->sin6_addr) ||
			    IN6_IS_ADDR_MULTICAST(&sa6->sin6_addr)) {
				return(EINVAL);
			}
			if ((error = scope6_check_id(sa6, ip6_use_defzone))
			    != 0) {
				return(error);
			}
#ifndef SCOPEDROUTING
			sa6->sin6_scope_id = 0; /* XXX */
#endif
			break;
		}
		case AF_LINK:	/* should eventually be supported */
		default:
			return(EAFNOSUPPORT);
		}

		/* turn off the previous option, then set the new option. */
		ip6_clearpktopts(opt, IPV6_NEXTHOP);
		if (sticky) {
			opt->ip6po_nexthop = malloc(*buf, M_IP6OPT, M_WAITOK);
			bcopy(buf, opt->ip6po_nexthop, *buf);
		} else
			opt->ip6po_nexthop = (struct sockaddr *)buf;
		break;
		
	case IPV6_2292HOPOPTS:
	case IPV6_HOPOPTS:
	{
		struct ip6_hbh *hbh;
		int hbhlen;

		/*
		 * XXX: We don't allow a non-privileged user to set ANY HbH
		 * options, since per-option restriction has too much
		 * overhead.
		 */

		if (len == 0) {
			ip6_clearpktopts(opt, IPV6_HOPOPTS);
			break;	/* just remove the option */
		}

		/* message length validation */
		if (len < sizeof(struct ip6_hbh))
			return(EINVAL);
		hbh = (struct ip6_hbh *)buf;
		hbhlen = (hbh->ip6h_len + 1) << 3;
		if (len != hbhlen)
			return(EINVAL);

		/* turn off the previous option, then set the new option. */
		ip6_clearpktopts(opt, IPV6_HOPOPTS);
		if (sticky) {
			opt->ip6po_hbh = malloc(hbhlen, M_IP6OPT, M_WAITOK);
			bcopy(hbh, opt->ip6po_hbh, hbhlen);
		} else
			opt->ip6po_hbh = hbh;

		break;
	}

	case IPV6_2292DSTOPTS:
	case IPV6_DSTOPTS:
	case IPV6_RTHDRDSTOPTS:
	{
		struct ip6_dest *dest, **newdest = NULL;
		int destlen;


		if (len == 0) {
			ip6_clearpktopts(opt, optname);
			break;	/* just remove the option */
		}

		/* message length validation */
		if (len < sizeof(struct ip6_dest))
			return(EINVAL);
		dest = (struct ip6_dest *)buf;
		destlen = (dest->ip6d_len + 1) << 3;
		if (len != destlen)
			return(EINVAL);

		/*
		 * Determine the position that the destination options header
		 * should be inserted; before or after the routing header.
		 */
		switch (optname) {
		case IPV6_2292DSTOPTS:
			/*
			 * The old advacned API is ambiguous on this point.
			 * Our approach is to determine the position based
			 * according to the existence of a routing header.
			 * Note, however, that this depends on the order of the
			 * extension headers in the ancillary data; the 1st
			 * part of the destination options header must appear
			 * before the routing header in the ancillary data,
			 * too.
			 * RFC2292bis solved the ambiguity by introducing
			 * separate ancillary data or option types.
			 */
			if (opt->ip6po_rthdr == NULL)
				newdest = &opt->ip6po_dest1;
			else
				newdest = &opt->ip6po_dest2;
			break;
		case IPV6_RTHDRDSTOPTS:
			newdest = &opt->ip6po_dest1;
			break;
		case IPV6_DSTOPTS:
			newdest = &opt->ip6po_dest2;
			break;
		}

		/* turn off the previous option, then set the new option. */
		ip6_clearpktopts(opt, optname);
		if (sticky) {
			*newdest = malloc(destlen, M_IP6OPT, M_WAITOK);
			bcopy(dest, *newdest, destlen);
		} else
			*newdest = dest;

		break;
	}

	case IPV6_2292RTHDR:
	case IPV6_RTHDR:
	{
		struct ip6_rthdr *rth;
		int rthlen;

		if (len == 0) {
			ip6_clearpktopts(opt, IPV6_RTHDR);
			break;	/* just remove the option */
		}

		/* message length validation */
		if (len < sizeof(struct ip6_rthdr))
			return(EINVAL);
		rth = (struct ip6_rthdr *)buf;
		rthlen = (rth->ip6r_len + 1) << 3;
		if (len != rthlen)
			return(EINVAL);

		switch (rth->ip6r_type) {
		case IPV6_RTHDR_TYPE_0:
			if (rth->ip6r_len == 0)	/* must contain one addr */
				return(EINVAL);
			if (rth->ip6r_len % 2) /* length must be even */
				return(EINVAL);
			if (rth->ip6r_len / 2 != rth->ip6r_segleft)
				return(EINVAL);
			break;
		default:
			return(EINVAL);	/* not supported */
		}

		/* turn off the previous option */
		ip6_clearpktopts(opt, IPV6_RTHDR);
		if (sticky) {
			opt->ip6po_rthdr = malloc(rthlen, M_IP6OPT, M_WAITOK);
			bcopy(rth, opt->ip6po_rthdr, rthlen);
		} else
			opt->ip6po_rthdr = rth;
		
		break;
	}

	case IPV6_REACHCONF:
		if (!cmsg)
			return(ENOPROTOOPT);

#if 0
		/*
		 * it looks dangerous to allow IPV6_REACHCONF to
		 * normal user.  it affects the ND state (system state)
		 * and can affect communication by others - jinmei
		 */
		if (!priv)
			return(EPERM);
#else
		/*
		 * we limit max # of subsequent userland reachability
		 * conformation by using ln->ln_byhint.
		 */
#endif
		if (len)
			return(EINVAL);
		opt->ip6po_flags |= IP6PO_REACHCONF;
		break;

	case IPV6_USE_MIN_MTU:
	case IPV6_DONTFRAG:
	{
		int on, flag = 0;

		if (len != sizeof(int))
			return(EINVAL);
		on = *(int *)buf;

		switch (optname) {
		case IPV6_USE_MIN_MTU:
			flag = IP6PO_MINMTU;
			break;
		case IPV6_DONTFRAG:
			flag = IP6PO_DONTFRAG;
			break;
		}

		if (on)
			opt->ip6po_flags |= flag;
		else
			opt->ip6po_flags &= ~flag;
		break;
	}

	default:
		return(ENOPROTOOPT);	
	} /* end of switch */

	return(0);
}

/*
 * Routine called from ip6_output() to loop back a copy of an IP6 multicast
 * packet to the input queue of a specified interface.  Note that this
 * calls the output routine of the loopback "driver", but with an interface
 * pointer that might NOT be &loif -- easier than replicating that code here.
 */
void
ip6_mloopback(ifp, m, dst)
	struct ifnet *ifp;
	struct mbuf *m;
	struct sockaddr_in6 *dst;
{
	struct mbuf *copym;
	struct ip6_hdr *ip6;

	copym = m_copy(m, 0, M_COPYALL);
	if (copym == NULL)
		return;

	/*
	 * Make sure to deep-copy IPv6 header portion in case the data
	 * is in an mbuf cluster, so that we can safely override the IPv6
	 * header portion later.
	 */
	if ((copym->m_flags & M_EXT) != 0 ||
	    copym->m_len < sizeof(struct ip6_hdr)) {
		copym = m_pullup(copym, sizeof(struct ip6_hdr));
		if (copym == NULL)
			return;
	}

#ifdef DIAGNOSTIC
	if (copym->m_len < sizeof(*ip6)) {
		m_freem(copym);
		return;
	}
#endif

	ip6 = mtod(copym, struct ip6_hdr *);
#ifndef SCOPEDROUTING
	/*
	 * clear embedded scope identifiers if necessary.
	 * in6_clearscope will touch the addresses only when necessary.
	 */
	in6_clearscope(&ip6->ip6_src);
	in6_clearscope(&ip6->ip6_dst);
#endif

#if defined(__FreeBSD__) && __FreeBSD__ >= 3
#if (__FreeBSD_version >= 410000)
	(void)if_simloop(ifp, copym, dst->sin6_family, (int)NULL);
#else
	(void)if_simloop(ifp, copym, (struct sockaddr *)dst, NULL);
#endif
#else
	(void)looutput(ifp, copym, (struct sockaddr *)dst, NULL);
#endif
}

/*
 * Chop IPv6 header off from the payload.
 */
static int
ip6_splithdr(m, exthdrs)
	struct mbuf *m;
	struct ip6_exthdrs *exthdrs;
{
	struct mbuf *mh;
	struct ip6_hdr *ip6;

	ip6 = mtod(m, struct ip6_hdr *);
	if (m->m_len > sizeof(*ip6)) {
		MGETHDR(mh, M_DONTWAIT, MT_HEADER);
		if (mh == 0) {
			m_freem(m);
			return ENOBUFS;
		}
#ifdef __OpenBSD__
		M_MOVE_PKTHDR(mh, m);
#else
		M_COPY_PKTHDR(mh, m);
#endif
		MH_ALIGN(mh, sizeof(*ip6));
		m->m_flags &= ~M_PKTHDR;
		m->m_len -= sizeof(*ip6);
		m->m_data += sizeof(*ip6);
		mh->m_next = m;
		m = mh;
		m->m_len = sizeof(*ip6);
		bcopy((caddr_t)ip6, mtod(m, caddr_t), sizeof(*ip6));
	}
	exthdrs->ip6e_ip6 = m;
	return 0;
}

/*
 * Compute IPv6 extension header length.
 */
#ifdef HAVE_NRL_INPCB
# define in6pcb	inpcb
# define in6p_outputopts	inp_outputopts6
#endif
int
ip6_optlen(in6p)
	struct in6pcb *in6p;
{
	int len;

	if (!in6p->in6p_outputopts)
		return 0;

	len = 0;
#define elen(x) \
    (((struct ip6_ext *)(x)) ? (((struct ip6_ext *)(x))->ip6e_len + 1) << 3 : 0)

	len += elen(in6p->in6p_outputopts->ip6po_hbh);
	if (in6p->in6p_outputopts->ip6po_rthdr)
		/* dest1 is valid with rthdr only */
		len += elen(in6p->in6p_outputopts->ip6po_dest1);
	len += elen(in6p->in6p_outputopts->ip6po_rthdr);
	len += elen(in6p->in6p_outputopts->ip6po_dest2);
	return len;
#undef elen
}
#ifdef HAVE_NRL_INPCB
# undef in6pcb
# undef in6p_outputopts
#endif
