//==========================================================================
//
//      src/sys/netinet6/in6_gif.c
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

/*	$KAME: in6_gif.c,v 1.88 2001/12/21 03:32:34 itojun Exp $	*/

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

/* define it if you want to use encap_attach_func (it helps *BSD merge) */
/*#define USE_ENCAPCHECK*/

#include <sys/param.h>
#include <sys/socket.h>
#include <sys/sockio.h>
#include <sys/mbuf.h>
#include <sys/errno.h>
#if !(defined(__FreeBSD__) && __FreeBSD__ >= 3)
#include <sys/ioctl.h>
#endif
#include <sys/queue.h>
#include <sys/protosw.h>

#if defined(__FreeBSD__) && __FreeBSD__ >= 3
#include <sys/malloc.h>
#endif

#include <net/if.h>
#include <net/route.h>

#include <netinet/in.h>
#include <netinet/in_systm.h>
#ifdef INET
#include <netinet/ip.h>
#endif
#include <netinet/ip_encap.h>
#ifdef INET6
#include <netinet/ip6.h>
#include <netinet6/ip6_var.h>
#include <netinet6/in6_gif.h>
#include <netinet6/in6_var.h>
#endif
#include <netinet6/ip6protosw.h>
#include <netinet/ip_ecn.h>
#ifdef __OpenBSD__
#include <netinet/ip_ipsp.h>
#endif

#include <net/if_gif.h>

#ifdef __OpenBSD__
#include "bridge.h"
#endif

static int gif_validate6 __P((const struct ip6_hdr *, struct gif_softc *,
	struct ifnet *));
static int in6_gif_rtcachettl = 300; /* XXX see in_gif.c */

int	ip6_gif_hlim = GIF_HLIM;

extern struct domain inet6domain;
struct ip6protosw in6_gif_protosw =
{ SOCK_RAW,	&inet6domain,	0/* IPPROTO_IPV[46] */,	PR_ATOMIC|PR_ADDR,
  in6_gif_input, rip6_output,	in6_gif_ctlinput, rip6_ctloutput,
#if defined(__FreeBSD__) && __FreeBSD__ >= 3
  0,
#else
  rip6_usrreq,
#endif
  0,            0,              0,              0,
#if defined(__FreeBSD__) && __FreeBSD__ >= 3
  &rip6_usrreqs
#endif
};

extern LIST_HEAD(, gif_softc) gif_softc_list;

#ifndef offsetof
#define offsetof(s, e) ((int)&((s *)0)->e)
#endif

int
in6_gif_output(ifp, family, m)
	struct ifnet *ifp;
	int family; /* family of the packet to be encapsulate. */
	struct mbuf *m;
{
#ifdef __OpenBSD__
	struct gif_softc *sc = (struct gif_softc*)ifp;
        struct sockaddr_in6 *dst = (struct sockaddr_in6 *)&sc->gif_ro6.ro_dst;
	struct sockaddr_in6 *sin6_src = (struct sockaddr_in6 *)sc->gif_psrc;
	struct sockaddr_in6 *sin6_dst = (struct sockaddr_in6 *)sc->gif_pdst;
	struct tdb tdb;
	struct xformsw xfs;
	int error;
	int hlen, poff;
	struct mbuf *mp;
	long time_second;

	if (sin6_src == NULL || sin6_dst == NULL ||
	    sin6_src->sin6_family != AF_INET6 ||
	    sin6_dst->sin6_family != AF_INET6) {
		m_freem(m);
		return EAFNOSUPPORT;
	}

	/* setup dummy tdb.  it highly depends on ipip_output() code. */
	bzero(&tdb, sizeof(tdb));
	bzero(&xfs, sizeof(xfs));
	tdb.tdb_src.sin6.sin6_family = AF_INET6;
	tdb.tdb_src.sin6.sin6_len = sizeof(struct sockaddr_in6);
	tdb.tdb_src.sin6.sin6_addr = sin6_src->sin6_addr;
	tdb.tdb_dst.sin6.sin6_family = AF_INET6;
	tdb.tdb_dst.sin6.sin6_len = sizeof(struct sockaddr_in6);
	tdb.tdb_dst.sin6.sin6_addr = sin6_dst->sin6_addr;
	tdb.tdb_xform = &xfs;
	xfs.xf_type = -1;	/* not XF_IP4 */

	switch (family) {
#ifdef INET
	case AF_INET:
	    {
		if (m->m_len < sizeof(struct ip)) {
			m = m_pullup(m, sizeof(struct ip));
			if (!m)
				return ENOBUFS;
		}
		hlen = (mtod(m, struct ip *)->ip_hl) << 2;
		poff = offsetof(struct ip, ip_p);
		break;
	    }
#endif
#ifdef INET6
	case AF_INET6:
	    {
		hlen = sizeof(struct ip6_hdr);
		poff = offsetof(struct ip6_hdr, ip6_nxt);
		break;
	    }
#endif
#if NBRIDGE > 0
	case AF_LINK:
		break;
#endif /* NBRIDGE */
	default:
#ifdef DEBUG
		printf("in6_gif_output: warning: unknown family %d passed\n",
			family);
#endif
		m_freem(m);
		return EAFNOSUPPORT;
	}
	
#if NBRIDGE > 0
	if (family == AF_LINK) {
	        mp = NULL;
		error = etherip_output(m, &tdb, &mp, 0, 0);
		if (error)
		        return error;
		else if (mp == NULL)
		        return EFAULT;

		m = mp;
		goto sendit;
	}
#endif /* NBRIDGE */

	/* encapsulate into IPv6 packet */
	mp = NULL;
	error = ipip_output(m, &tdb, &mp, hlen, poff);
	if (error)
	        return error;
	else if (mp == NULL)
	        return EFAULT;

	m = mp;

#if NBRIDGE > 0
 sendit:
#endif /* NBRIDGE */
	/*
	 * compare address family just for safety.  other validity checks
	 * are made in in6_selectsrc() called from ip6_output().
	 */
	if (sc->gif_ro6.ro_rt && (dst->sin6_family != sin6_dst->sin6_family ||
				  sc->rtcache_expire == 0 ||
				  time_second >= sc->rtcache_expire)) {
		RTFREE(sc->gif_ro6.ro_rt);
		sc->gif_ro6.ro_rt = NULL;
	}

#ifdef IPV6_MINMTU
	/*
	 * force fragmentation to minimum MTU, to avoid path MTU discovery.
	 * it is too painful to ask for resend of inner packet, to achieve
	 * path MTU discovery for encapsulated packets.
	 */
	error = ip6_output(m, 0, &sc->gif_ro6, IPV6_MINMTU, 0, NULL);
#else
	error = ip6_output(m, 0, &sc->gif_ro6, 0, 0, NULL);
#endif

	if (sc->gif_ro6.ro_rt && time_second >= sc->rtcache_expire)
		sc->rtcache_expire = time_second + in6_gif_rtcachettl;

	return(error);
#else	/* !__OpenBSD__ */
	struct gif_softc *sc = (struct gif_softc*)ifp;
	struct sockaddr_in6 *dst = (struct sockaddr_in6 *)&sc->gif_ro6.ro_dst;
	struct sockaddr_in6 *sin6_src = (struct sockaddr_in6 *)sc->gif_psrc;
	struct sockaddr_in6 *sin6_dst = (struct sockaddr_in6 *)sc->gif_pdst;
	struct ip6_hdr *ip6;
	int proto, error;
	u_int8_t itos, otos;
#if !(defined(__FreeBSD__) && __FreeBSD__ >= 3)
	long time_second;
#endif

	if (sin6_src == NULL || sin6_dst == NULL ||
	    sin6_src->sin6_family != AF_INET6 ||
	    sin6_dst->sin6_family != AF_INET6) {
		m_freem(m);
		return EAFNOSUPPORT;
	}

	switch (family) {
#ifdef INET
	case AF_INET:
	    {
		struct ip *ip;

		proto = IPPROTO_IPV4;
		if (m->m_len < sizeof(*ip)) {
			m = m_pullup(m, sizeof(*ip));
			if (!m)
				return ENOBUFS;
		}
		ip = mtod(m, struct ip *);
		itos = ip->ip_tos;
		break;
	    }
#endif
#ifdef INET6
	case AF_INET6:
	    {
		struct ip6_hdr *ip6;
		proto = IPPROTO_IPV6;
		if (m->m_len < sizeof(*ip6)) {
			m = m_pullup(m, sizeof(*ip6));
			if (!m)
				return ENOBUFS;
		}
		ip6 = mtod(m, struct ip6_hdr *);
		itos = (ntohl(ip6->ip6_flow) >> 20) & 0xff;
		break;
	    }
#endif
#if defined(__NetBSD__) && defined(ISO)
	case AF_ISO:
		proto = IPPROTO_EON;
		itos = 0;
		break;
#endif
	default:
#ifdef DEBUG
		printf("in6_gif_output: warning: unknown family %d passed\n",
			family);
#endif
		m_freem(m);
		return EAFNOSUPPORT;
	}
	
	/* prepend new IP header */
	M_PREPEND(m, sizeof(struct ip6_hdr), M_DONTWAIT);
	if (m && m->m_len < sizeof(struct ip6_hdr))
		m = m_pullup(m, sizeof(struct ip6_hdr));
	if (m == NULL)
		return ENOBUFS;

	ip6 = mtod(m, struct ip6_hdr *);
	ip6->ip6_flow	= 0;
	ip6->ip6_vfc	&= ~IPV6_VERSION_MASK;
	ip6->ip6_vfc	|= IPV6_VERSION;
	ip6->ip6_plen	= htons((u_short)m->m_pkthdr.len);
	ip6->ip6_nxt	= proto;
	ip6->ip6_hlim	= ip6_gif_hlim;
	ip6->ip6_src	= sin6_src->sin6_addr;
	/* bidirectional configured tunnel mode */
	if (!IN6_IS_ADDR_UNSPECIFIED(&sin6_dst->sin6_addr))
		ip6->ip6_dst = sin6_dst->sin6_addr;
	else  {
		m_freem(m);
		return ENETUNREACH;
	}
	if (ifp->if_flags & IFF_LINK1)
		ip_ecn_ingress(ECN_ALLOWED, &otos, &itos);
	else
		ip_ecn_ingress(ECN_NOCARE, &otos, &itos);
	ip6->ip6_flow &= ~ntohl(0xff00000);
	ip6->ip6_flow |= htonl((u_int32_t)otos << 20);

#if !(defined(__FreeBSD__) && __FreeBSD__ >= 3)
	time_second = time.tv_sec;
#endif
	/*
	 * compare address family just for safety.  other validity checks
	 * are made in in6_selectsrc() called from ip6_output().
	 */
	if (sc->gif_ro6.ro_rt && (dst->sin6_family != sin6_dst->sin6_family ||
				  sc->rtcache_expire == 0 ||
				  time_second >= sc->rtcache_expire)) {
		/*
		 * If the cached route is not valid or has expired,
		 * clear the stale cache and let ip6_output make a new cached
		 * route.
		 */
		RTFREE(sc->gif_ro6.ro_rt);
		sc->gif_ro6.ro_rt = NULL;
	}

#ifdef IPV6_MINMTU
	/*
	 * force fragmentation to minimum MTU, to avoid path MTU discovery.
	 * it is too painful to ask for resend of inner packet, to achieve
	 * path MTU discovery for encapsulated packets.
	 */
	error = ip6_output(m, 0, &sc->gif_ro6, IPV6_MINMTU, 0, NULL);
#else
	error = ip6_output(m, 0, &sc->gif_ro6, 0, 0, NULL);
#endif

	/*
	 * if a (new) cached route has been created in ip6_output(), extend
	 * the expiration time.
	 */
	if (sc->gif_ro6.ro_rt && time_second >= sc->rtcache_expire)
		sc->rtcache_expire = time_second + in6_gif_rtcachettl;

	return(error);
#endif	/* __OpenBSD__ */
}

int in6_gif_input(mp, offp, proto)
	struct mbuf **mp;
	int *offp, proto;
{
	struct mbuf *m = *mp;
	struct ifnet *gifp = NULL;
	struct ip6_hdr *ip6;
#ifndef __OpenBSD__
	int af = 0;
	u_int32_t otos;
#endif

	ip6 = mtod(m, struct ip6_hdr *);

	gifp = (struct ifnet *)encap_getarg(m);

	if (gifp == NULL || (gifp->if_flags & IFF_UP) == 0) {
		m_freem(m);
		ip6stat.ip6s_nogif++;
		return IPPROTO_DONE;
	}
#ifndef USE_ENCAPCHECK
	if (!gif_validate6(ip6, (struct gif_softc *)gifp, m->m_pkthdr.rcvif)) {
		m_freem(m);
		ip6stat.ip6s_nogif++;
		return IPPROTO_DONE;
	}
#endif

#ifdef __OpenBSD__
	m->m_pkthdr.rcvif = gifp;
	gifp->if_ipackets++;
	gifp->if_ibytes += m->m_pkthdr.len;
	ipip_input(m, *offp, gifp);
	return IPPROTO_DONE;
#else
	otos = ip6->ip6_flow;
	m_adj(m, *offp);

	switch (proto) {
#ifdef INET
	case IPPROTO_IPV4:
	    {
		struct ip *ip;
		u_int8_t otos8;
		af = AF_INET;
		otos8 = (ntohl(otos) >> 20) & 0xff;
		if (m->m_len < sizeof(*ip)) {
			m = m_pullup(m, sizeof(*ip));
			if (!m)
				return IPPROTO_DONE;
		}
		ip = mtod(m, struct ip *);
		if (gifp->if_flags & IFF_LINK1)
			ip_ecn_egress(ECN_ALLOWED, &otos8, &ip->ip_tos);
		else
			ip_ecn_egress(ECN_NOCARE, &otos8, &ip->ip_tos);
		break;
	    }
#endif /* INET */
#ifdef INET6
	case IPPROTO_IPV6:
	    {
		struct ip6_hdr *ip6;
		af = AF_INET6;
		if (m->m_len < sizeof(*ip6)) {
			m = m_pullup(m, sizeof(*ip6));
			if (!m)
				return IPPROTO_DONE;
		}
		ip6 = mtod(m, struct ip6_hdr *);
		if (gifp->if_flags & IFF_LINK1)
			ip6_ecn_egress(ECN_ALLOWED, &otos, &ip6->ip6_flow);
		else
			ip6_ecn_egress(ECN_NOCARE, &otos, &ip6->ip6_flow);
		break;
	    }
#endif
#if defined(__NetBSD__) && defined(ISO)
	case IPPROTO_EON:
		af = AF_ISO;
		break;
#endif
	default:
		ip6stat.ip6s_nogif++;
		m_freem(m);
		return IPPROTO_DONE;
	}
		
	gif_input(m, af, gifp);
	return IPPROTO_DONE;
#endif
}

/*
 * validate outer address.
 */
static int
gif_validate6(ip6, sc, ifp)
	const struct ip6_hdr *ip6;
	struct gif_softc *sc;
	struct ifnet *ifp;
{
	struct sockaddr_in6 *src, *dst;

	src = (struct sockaddr_in6 *)sc->gif_psrc;
	dst = (struct sockaddr_in6 *)sc->gif_pdst;

	/* check for address match */
	if (!IN6_ARE_ADDR_EQUAL(&src->sin6_addr, &ip6->ip6_dst) ||
	    !IN6_ARE_ADDR_EQUAL(&dst->sin6_addr, &ip6->ip6_src))
		return 0;

	/* martian filters on outer source - done in ip6_input */

	/* ingress filters on outer source */
	if ((sc->gif_if.if_flags & IFF_LINK2) == 0 && ifp) {
		struct sockaddr_in6 sin6;
		struct rtentry *rt;

		bzero(&sin6, sizeof(sin6));
		sin6.sin6_family = AF_INET6;
		sin6.sin6_len = sizeof(struct sockaddr_in6);
		sin6.sin6_addr = ip6->ip6_src;
		/* XXX scopeid */
#ifdef __FreeBSD__
		rt = rtalloc1((struct sockaddr *)&sin6, 0, 0UL);
#else
		rt = rtalloc1((struct sockaddr *)&sin6, 0);
#endif
		if (!rt || rt->rt_ifp != ifp) {
#if 0
			log(LOG_WARNING, "%s: packet from %s dropped "
			    "due to ingress filter\n", if_name(&sc->gif_if),
			    ip6_sprintf(&sin6.sin6_addr));
#endif
			if (rt)
				rtfree(rt);
			return 0;
		}
		rtfree(rt);
	}

	return 128 * 2;
}

/*
 * we know that we are in IFF_UP, outer address available, and outer family
 * matched the physical addr family.  see gif_encapcheck().
 */
int
gif_encapcheck6(m, off, proto, arg)
	const struct mbuf *m;
	int off;
	int proto;
	void *arg;
{
	struct ip6_hdr ip6;
	struct gif_softc *sc;
	struct ifnet *ifp;

	/* sanity check done in caller */
	sc = (struct gif_softc *)arg;

	/* LINTED const cast */
	m_copydata((struct mbuf *)m, 0, sizeof(ip6), (caddr_t)&ip6);
	ifp = ((m->m_flags & M_PKTHDR) != 0) ? m->m_pkthdr.rcvif : NULL;

	return gif_validate6(&ip6, sc, ifp);
}

int
in6_gif_attach(sc)
	struct gif_softc *sc;
{
#ifndef USE_ENCAPCHECK
	struct sockaddr_in6 mask6;

	bzero(&mask6, sizeof(mask6));
	mask6.sin6_len = sizeof(struct sockaddr_in6);
	mask6.sin6_addr.s6_addr32[0] = mask6.sin6_addr.s6_addr32[1] = 
	    mask6.sin6_addr.s6_addr32[2] = mask6.sin6_addr.s6_addr32[3] = ~0;

	if (!sc->gif_psrc || !sc->gif_pdst)
		return EINVAL;
	sc->encap_cookie6 = encap_attach(AF_INET6, -1, sc->gif_psrc,
	    (struct sockaddr *)&mask6, sc->gif_pdst, (struct sockaddr *)&mask6,
	    (struct protosw *)&in6_gif_protosw, sc);
#else
	sc->encap_cookie6 = encap_attach_func(AF_INET6, -1, gif_encapcheck,
	    (struct protosw *)&in6_gif_protosw, sc);
#endif
	if (sc->encap_cookie6 == NULL)
		return EEXIST;
	return 0;
}

int
in6_gif_detach(sc)
	struct gif_softc *sc;
{
	int error;

	error = encap_detach(sc->encap_cookie6);
	if (error == 0)
		sc->encap_cookie6 = NULL;
	return error;
}

void
in6_gif_ctlinput(cmd, sa, d)
	int cmd;
	struct sockaddr *sa;
	void *d;
{
	struct gif_softc *sc;
	struct ip6ctlparam *ip6cp = NULL;
	struct mbuf *m;
	struct ip6_hdr *ip6;
	int off;
	void *cmdarg;
	const struct sockaddr_in6 *sa6_src = NULL;
	struct sockaddr_in6 *dst6;

	if (sa->sa_family != AF_INET6 ||
	    sa->sa_len != sizeof(struct sockaddr_in6))
		return;

	if ((unsigned)cmd >= PRC_NCMDS)
		return;
	if (cmd == PRC_HOSTDEAD)
		d = NULL;
	else if (inet6ctlerrmap[cmd] == 0)
		return;

	/* if the parameter is from icmp6, decode it. */
	if (d != NULL) {
		ip6cp = (struct ip6ctlparam *)d;
		m = ip6cp->ip6c_m;
		ip6 = ip6cp->ip6c_ip6;
		off = ip6cp->ip6c_off;
		cmdarg = ip6cp->ip6c_cmdarg;
		sa6_src = ip6cp->ip6c_src;
	} else {
		m = NULL;
		ip6 = NULL;
		cmdarg = NULL;
		sa6_src = &sa6_any;
	}

	if (!ip6)
		return;

	/*
	 * for now we don't care which type it was, just flush the route cache.
	 * XXX slow.  sc (or sc->encap_cookie6) should be passed from
	 * ip_encap.c.
	 */
	for (sc = LIST_FIRST(&gif_softc_list); sc;
	     sc = LIST_NEXT(sc, gif_list)) {
		if ((sc->gif_if.if_flags & IFF_RUNNING) == 0)
			continue;
		if (sc->gif_psrc->sa_family != AF_INET6)
			continue;
		if (!sc->gif_ro6.ro_rt)
			continue;

		dst6 = (struct sockaddr_in6 *)&sc->gif_ro6.ro_dst;
		/* XXX scope */
		if (IN6_ARE_ADDR_EQUAL(&ip6->ip6_dst, &dst6->sin6_addr)) {
			/* flush route cache */
			RTFREE(sc->gif_ro6.ro_rt);
			sc->gif_ro6.ro_rt = NULL;
		}
	}
}
