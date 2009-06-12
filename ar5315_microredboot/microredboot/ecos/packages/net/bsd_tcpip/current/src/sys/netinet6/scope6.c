//==========================================================================
//
//      src/sys/netinet6/scope6.c
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

/*	$KAME: scope6.c,v 1.27 2001/12/18 02:23:45 itojun Exp $	*/

/*
 * Copyright (C) 2000 WIDE Project.
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

#include <sys/param.h>
#include <sys/malloc.h>
#include <sys/mbuf.h>
#include <sys/socket.h>
#include <sys/queue.h>

#include <net/route.h>
#include <net/if.h>

#include <netinet/in.h>

#include <netinet6/in6_var.h>
#include <netinet6/scope6_var.h>

#ifdef ENABLE_DEFAULT_SCOPE
int ip6_use_defzone = 1;
#else
int ip6_use_defzone = 0;
#endif

struct scope6_id {
	/*
	 * 16 is correspondent to 4bit multicast scope field.
	 * i.e. from node-local to global with some reserved/unassigned types.
	 */
	u_int32_t s6id_list[16];
};
static size_t if_indexlim = 8;
struct scope6_id *scope6_ids = NULL;

void
scope6_ifattach(ifp)
	struct ifnet *ifp;
{
#ifdef __NetBSD__
	int s = splsoftnet();
#else
	int s = splnet();
#endif

	/*
	 * We have some arrays that should be indexed by if_index.
	 * since if_index will grow dynamically, they should grow too.
	 */
	if (scope6_ids == NULL || if_index >= if_indexlim) {
		size_t n;
		caddr_t q;

		while (if_index >= if_indexlim)
			if_indexlim <<= 1;

		/* grow scope index array */
		n = if_indexlim * sizeof(struct scope6_id);
		/* XXX: need new malloc type? */
		q = (caddr_t)malloc(n, M_IFADDR, M_WAITOK);
		bzero(q, n);
		if (scope6_ids) {
			bcopy((caddr_t)scope6_ids, q, n/2);
			free((caddr_t)scope6_ids, M_IFADDR);
		}
		scope6_ids = (struct scope6_id *)q;
	}

#define SID scope6_ids[ifp->if_index]

	/* don't initialize if called twice */
	if (SID.s6id_list[IPV6_ADDR_SCOPE_LINKLOCAL]) {
		splx(s);
		return;
	}

	/*
	 * XXX: IPV6_ADDR_SCOPE_xxx macros are not standard.
	 * Should we rather hardcode here?
	 */
	SID.s6id_list[IPV6_ADDR_SCOPE_INTFACELOCAL] = ifp->if_index;
	SID.s6id_list[IPV6_ADDR_SCOPE_LINKLOCAL] = ifp->if_index;
#ifdef MULTI_SCOPE
	/* by default, we don't care about scope boundary for these scopes. */
	SID.s6id_list[IPV6_ADDR_SCOPE_SITELOCAL] = 1;
	SID.s6id_list[IPV6_ADDR_SCOPE_ORGLOCAL] = 1;
#endif
#undef SID

	splx(s);
}

int
scope6_set(ifp, idlist)
	struct ifnet *ifp;
	u_int32_t *idlist;
{
	int i, s;
	int error = 0;

	if (scope6_ids == NULL)	/* paranoid? */
		return(EINVAL);

	/*
	 * XXX: We need more consistency checks of the relationship among
	 * scopes (e.g. an organization should be larger than a site).
	 */

	/*
	 * TODO(XXX): after setting, we should reflect the changes to
	 * interface addresses, routing table entries, PCB entries... 
	 */

#ifdef __NetBSD__
	s = splsoftnet();
#else
	s = splnet();
#endif

	for (i = 0; i < 16; i++) {
		if (idlist[i] &&
		    idlist[i] != scope6_ids[ifp->if_index].s6id_list[i]) {
			/*
			 * An interface zone ID must be the corresponding
			 * interface index by definition.
			 */
			if (i == IPV6_ADDR_SCOPE_INTFACELOCAL &&
			    idlist[i] != ifp->if_index) {
				splx(s);
				return(EINVAL);
			}

			if (i == IPV6_ADDR_SCOPE_LINKLOCAL &&
			    idlist[i] > if_index) {
				/*
				 * XXX: theoretically, there should be no
				 * relationship between link IDs and interface
				 * IDs, but we check the consistency for
				 * safety in later use.
				 */
				splx(s);
				return(EINVAL);
			}

			/*
			 * XXX: we must need lots of work in this case,
			 * but we simply set the new value in this initial
			 * implementation.
			 */
			scope6_ids[ifp->if_index].s6id_list[i] = idlist[i];
		}
	}
	splx(s);

	return(error);
}

int
scope6_get(ifp, idlist)
	struct ifnet *ifp;
	u_int32_t *idlist;
{
	if (scope6_ids == NULL)	/* paranoid? */
		return(EINVAL);

	bcopy(scope6_ids[ifp->if_index].s6id_list, idlist,
	      sizeof(scope6_ids[ifp->if_index].s6id_list));

	return(0);
}


/*
 * Get a scope of the address. Node-local, link-local, site-local or global.
 */
int
in6_addrscope(addr)
	struct in6_addr *addr;
{
	int scope;

	if (addr->s6_addr[0] == 0xfe) {
		scope = addr->s6_addr[1] & 0xc0;

		switch (scope) {
		case 0x80:
			return IPV6_ADDR_SCOPE_LINKLOCAL;
			break;
		case 0xc0:
			return IPV6_ADDR_SCOPE_SITELOCAL;
			break;
		default:
			return IPV6_ADDR_SCOPE_GLOBAL; /* just in case */
			break;
		}
	}


	if (addr->s6_addr[0] == 0xff) {
		scope = addr->s6_addr[1] & 0x0f;

		/*
		 * due to other scope such as reserved,
		 * return scope doesn't work.
		 */
		switch (scope) {
		case IPV6_ADDR_SCOPE_INTFACELOCAL:
			return IPV6_ADDR_SCOPE_INTFACELOCAL;
			break;
		case IPV6_ADDR_SCOPE_LINKLOCAL:
			return IPV6_ADDR_SCOPE_LINKLOCAL;
			break;
		case IPV6_ADDR_SCOPE_SITELOCAL:
			return IPV6_ADDR_SCOPE_SITELOCAL;
			break;
		default:
			return IPV6_ADDR_SCOPE_GLOBAL;
			break;
		}
	}

	/*
	 * Regard loopback and unspecified addresses as global, since
	 * they have no ambiguity.
	 */
	if (bcmp(&in6addr_loopback, addr, sizeof(addr) - 1) == 0) {
		if (addr->s6_addr[15] == 1) /* loopback */
			return IPV6_ADDR_SCOPE_LINKLOCAL;
		if (addr->s6_addr[15] == 0) /* unspecified */
			return IPV6_ADDR_SCOPE_GLOBAL; /* XXX: correct? */
	}

	return IPV6_ADDR_SCOPE_GLOBAL;
}

/*
 * When we introduce the "4+28" split semantics in sin6_scope_id,
 * a 32bit integer is not enough to tell a large ID from an error (-1).
 * So, we intentionally use a large type as the return value.
 */
int64_t
in6_addr2zoneid(ifp, addr)
	struct ifnet *ifp;	/* must not be NULL */
	struct in6_addr *addr;	/* must not be NULL */
{
	int scope;

#ifdef DIAGNOSTIC
	if (scope6_ids == NULL) { /* should not happen */
		panic("in6_addr2zoneid: scope array is NULL");
		/* NOTREACHED */
	}
	if (ifp->if_index >= if_indexlim) {
		panic("in6_addr2zoneid: invalid interface");
		/* NOTREACHED */
	}
#endif

	/*
	 * special case: the loopback address can only belong to a loopback
	 * interface.
	 */
	if (IN6_IS_ADDR_LOOPBACK(addr)) {
		if (!(ifp->if_flags & IFF_LOOPBACK))
			return(-1);
		else
			return(0); /* there's no ambiguity */
	}

	scope = in6_addrscope(addr);

#define SID scope6_ids[ifp->if_index]
	switch (scope) {
	case IPV6_ADDR_SCOPE_INTFACELOCAL: /* should be interface index */
		return(SID.s6id_list[IPV6_ADDR_SCOPE_INTFACELOCAL]);

	case IPV6_ADDR_SCOPE_LINKLOCAL:
		return(SID.s6id_list[IPV6_ADDR_SCOPE_LINKLOCAL]);

	case IPV6_ADDR_SCOPE_SITELOCAL:
		return(SID.s6id_list[IPV6_ADDR_SCOPE_SITELOCAL]);

	case IPV6_ADDR_SCOPE_ORGLOCAL:
		return(SID.s6id_list[IPV6_ADDR_SCOPE_ORGLOCAL]);

	default:
		return(0);	/* XXX: treat as global. */
	}
#undef SID
}

void
scope6_setdefault(ifp)
	struct ifnet *ifp;	/* note that this might be NULL */
{
	/*
	 * Currently, this function just set the default "interfaces"
	 * and "links" according to the given interface.
	 * We might eventually have to separate the notion of "link" from
	 * "interface" and provide a user interface to set the default.
	 */
	if (ifp) {
		scope6_ids[0].s6id_list[IPV6_ADDR_SCOPE_INTFACELOCAL] =
			ifp->if_index;
		scope6_ids[0].s6id_list[IPV6_ADDR_SCOPE_LINKLOCAL] =
			ifp->if_index;
	}
	else {
		scope6_ids[0].s6id_list[IPV6_ADDR_SCOPE_INTFACELOCAL] = 0;
		scope6_ids[0].s6id_list[IPV6_ADDR_SCOPE_LINKLOCAL] = 0;
	}
}

int
scope6_get_default(idlist)
	u_int32_t *idlist;
{
	if (scope6_ids == NULL)	/* paranoid? */
		return(EINVAL);

	bcopy(scope6_ids[0].s6id_list, idlist,
	      sizeof(scope6_ids[0].s6id_list));

	return(0);
}

u_int32_t
scope6_addr2default(addr)
	struct in6_addr *addr;
{
	/*
	 * special case: The loopback address should be considered as
	 * link-local, but there's no ambiguity in the syntax. 
	 */
	if (IN6_IS_ADDR_LOOPBACK(addr))
		return(0);

	return(scope6_ids[0].s6id_list[in6_addrscope(addr)]);
}

/*
 * XXX: some applications assume the kernel guesses a correct zone ID when the
 * outgoing interface is explicitly specified, and omit specifying the ID.
 * This is a bad manner, but we handle such cases anyway.
 * Note that this function intentionally overrides the argument SIN6.
 */
int
scope6_setzoneid(ifp, sin6)
	struct ifnet *ifp;
	struct sockaddr_in6 *sin6;
{
	int64_t zoneid;
	int error;

	zoneid = in6_addr2zoneid(ifp, &sin6->sin6_addr);
				 
	if (zoneid < 0)
		return(ENXIO);
	if (zoneid) {
		sin6->sin6_scope_id = zoneid;
		if ((error = in6_embedscope(&sin6->sin6_addr, sin6)) != 0)
			return(error);
	}

	return(0);
}	

/*
 * Check if the zone ID in SIN6 is valid according to the scope of the
 * address.  If DFEFAULTOK is true and the zone ID is unspecified, fill the
 * default value for the scope type in the ID field.
 * This function also embeds the zone ID into the 128bit address field if
 * necessary.  This format is internally used in the kernel.
 * As a result, it is ensured that SIN6 has a valid scope zone ID and the
 * address part is converted to the internal form.
 *
 * The caller must expect that SIN6 can be modified within this function;
 * if it is not desired to override the original value a local copy must be
 * made in advance.
 */
int
scope6_check_id(sin6, defaultok)
	struct sockaddr_in6 *sin6;
	int defaultok;
{
	u_int32_t zoneid;
	struct in6_addr *in6 = &sin6->sin6_addr;
	struct ifnet *ifp;

	if ((zoneid = sin6->sin6_scope_id) != 0) {
		/*
		 * At this moment, we only check interface-local and
		 * link-local scope IDs, and use interface indices as the
		 * zone IDs assuming a one-to-one mapping between interfaces
		 * and links.
		 * XXX: in6_embedscope() below does the same check (in case
		 * of !SCOPEDROUTING).  We should eventually centralize the
		 * check in this function.
		 */
		if (IN6_IS_SCOPE_LINKLOCAL(in6) ||
		    IN6_IS_ADDR_MC_INTFACELOCAL(in6)) {
			if (if_index < zoneid)
				return(ENXIO);
#if defined(__FreeBSD__) && __FreeBSD__ >= 5
			ifp = ifnet_byindex(zoneid);
#else
			ifp = ifindex2ifnet[zoneid];
#endif
			if (ifp == NULL) /* XXX: this can happen for some OS */
				return(ENXIO);
		}
	} else if (defaultok)
		sin6->sin6_scope_id = scope6_addr2default(in6);

	/* KAME hack: embed scopeid */
	if (in6_embedscope(in6, sin6) != 0)
		return(EINVAL);

	return(0);
}
