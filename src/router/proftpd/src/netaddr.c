/*
 * ProFTPD - FTP server daemon
 * Copyright (c) 2003-2005 The ProFTPD Project team
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307, USA.
 *
 * As a special exemption, The ProFTPD Project team and other respective
 * copyright holders give permission to link this program with OpenSSL, and
 * distribute the resulting executable, without including the source code for
 * OpenSSL in the source distribution.
 */

/* Network address routines
 * $Id: netaddr.c,v 1.1 2006/04/24 11:39:28 honor Exp $
 */

#include "conf.h"

/* Define an IPv4 equivalent of the IN6_IS_ADDR_LOOPBACK macro. */
#undef IN_IS_ADDR_LOOPBACK
#define IN_IS_ADDR_LOOPBACK(a) \
  ((((long int) (a)->s_addr) & 0xff000000) == 0x7f000000)

/* Do reverse DNS lookups? */
static int reverse_dns = 1;

/* Provide replacements for needed functions. */

#if !defined(HAVE_GETNAMEINFO) || defined(PR_USE_GETNAMEINFO)
int pr_getnameinfo(const struct sockaddr *sa, socklen_t salen, char *host,
    size_t hostlen, char *serv, size_t servlen, int flags) {

  struct sockaddr_in *sai = (struct sockaddr_in *) sa;

  if (!sai || sai->sin_family != AF_INET)
    return EAI_FAMILY;

  if (serv != NULL && servlen > (size_t) 1)
    snprintf(serv, servlen, "%lu", (unsigned long) ntohs(sai->sin_port));

  if (host != NULL && hostlen > (size_t) 1) {
    struct hostent *he = NULL;

    if ((flags & NI_NUMERICHOST) == 0 &&
        (he = gethostbyaddr((const char *) &(sai->sin_addr),
          sizeof(sai->sin_addr), AF_INET)) != NULL &&
        he->h_name != NULL &&
        *he->h_name != 0) {

      if (strlen(he->h_name) >= hostlen)
          goto handle_numeric_ip;
      sstrncpy(host, he->h_name, hostlen);

    } else {
      char *ipstr = NULL;

      handle_numeric_ip:
      ipstr = inet_ntoa(sai->sin_addr);
      if (ipstr == NULL)
        return EAI_SYSTEM;

      if (strlen(ipstr) >= hostlen)
        return EAI_FAIL;

      sstrncpy(host, ipstr, hostlen);
    }
  }

  return 0;
}
#endif /* HAVE_GETNAMEINFO or PR_USE_GETNAMEINFO */

#if !defined(HAVE_GETADDRINFO) || defined(PR_USE_GETADDRINFO)
int pr_getaddrinfo(const char *node, const char *service,
    const struct addrinfo *hints, struct addrinfo **res) {

  struct addrinfo *ans = NULL;
  struct sockaddr_in *saddr = NULL;
  const char *proto_name = "tcp";
  int socktype = SOCK_STREAM;
  unsigned short port = 0;

  if (!res)
    return EAI_FAIL;
  *res = NULL;

  ans = malloc(sizeof(struct addrinfo));
  if (ans == NULL)
    return EAI_MEMORY;

  saddr = malloc(sizeof(struct sockaddr_in));
  if (saddr == NULL) {
    free(ans);
    return EAI_MEMORY;
  }

  ans->ai_family = AF_INET;
  ans->ai_addrlen = sizeof *saddr;
  ans->ai_addr = (struct sockaddr *) saddr;
  ans->ai_next = NULL;
  memset(saddr, 0, sizeof(*saddr));
  saddr->sin_family = AF_INET;

  if (hints != NULL) {
    struct protoent *pe = NULL;

    if ((pe = getprotobynumber(hints->ai_protocol)) != NULL &&
         pe->p_name != NULL &&
         *pe->p_name != 0)
      proto_name = pe->p_name;

    if (hints->ai_socktype != 0)
      socktype = hints->ai_socktype;

    else if (strcasecmp(proto_name, "udp") == 0)
      socktype = SOCK_DGRAM;
  }

  if (service != NULL) {
    struct servent *se = NULL;

    if ((se = getservbyname(service, proto_name)) != NULL &&
        se->s_port > 0)
      port = se->s_port;

    else if ((port = (unsigned short) strtoul(service, NULL, 0)) <= 0 ||
        port > 65535)
      port = 0;
  }

  if (hints != NULL &&
      (hints->ai_flags & AI_PASSIVE) != 0)
    saddr->sin_addr.s_addr = htonl(INADDR_ANY);

  if (node != NULL) {
    struct hostent *he = NULL;

    if ((he = gethostbyname(node)) != NULL &&
         he->h_addr_list != NULL &&
         he->h_addr_list[0] != NULL &&
         he->h_length > 0 &&
         he->h_length <= (int) sizeof(saddr->sin_addr))
      memcpy(&saddr->sin_addr, he->h_addr_list[0], he->h_length);
  }

  ans->ai_socktype = socktype;
  saddr->sin_port = htons(port);
  *res = ans;

  return 0;
}

void pr_freeaddrinfo(struct addrinfo *ai) {
  if (!ai)
    return;

  if (ai->ai_addr != NULL) {
    free(ai->ai_addr);
    ai->ai_addr = NULL;
  }
  free(ai);
}
#endif /* HAVE_GETADDRINFO or PR_USE_GETADDRINFO */

#if !defined(HAVE_INET_NTOP)
const char *pr_inet_ntop(int af, const void *src, char *dst, size_t len) {
  char *res;

  if (af != AF_INET) {
    errno = EAFNOSUPPORT;
    return NULL;
  }

  res = inet_ntoa(*((struct in_addr *) src));
  if (res == NULL)
    return NULL;

  memcpy(dst, res, len);
  return dst;
}
#endif /* !HAVE_INET_NTOP */

#if !defined(HAVE_INET_PTON)
int pr_inet_pton(int af, const char *src, void *dst) {
  unsigned long res;

  if (af != AF_INET) {
    errno = EAFNOSUPPORT;
    return -1;
  }

  /* inet_aton(3) would be better. However, it is not ubiquitous.  */
  res = inet_addr(src);
  if (res == INADDR_NONE ||
      res == 0)
    return 0;

  memcpy(dst, &res, sizeof(res));
  return 1;
}
#endif /* !HAVE_INET_PTON */

#ifdef HAVE_GETHOSTBYNAME2
static void *get_v4inaddr(pr_netaddr_t *na) {

  /* This function is specifically for IPv4 clients (when gethostbyname2(2) is
   * present) that have an IPv4-mapped IPv6 address, when performing reverse
   * DNS checks.  This function is called iff the given netaddr object is
   * indeed an IPv4-mapped IPv6 address.  IPv6 address have 128 bits in their
   * sin6_addr field.  For IPv4-mapped IPv6 addresses, the relevant 32 bits
   * are the last of those 128 bits (or, alternatively, the last 4 bytes of
   * those 16 bytes); hence the read of 12 bytes after the start of the
   * sin6_addr pointer.
   */

  return (((char *) pr_netaddr_get_inaddr(na)) + 12);
}
#endif /* HAVE_GETHOSTBYNAME2 */

int pr_netaddr_set_reverse_dns(int enable) {
  int old_enable = reverse_dns;
  reverse_dns = enable;
  return old_enable;
}

pr_netaddr_t *pr_netaddr_alloc(pool *p) {
  if (!p) {
    errno = EINVAL;
    return NULL;
  }

  return pcalloc(p, sizeof(pr_netaddr_t));
}

void pr_netaddr_clear(pr_netaddr_t *na) {
  if (!na)
    return;

  memset(na, 0, sizeof(pr_netaddr_t));
}

pr_netaddr_t *pr_netaddr_dup(pool *p, pr_netaddr_t *na) {
  pr_netaddr_t *dup_na;

  if (!p || !na) {
    errno = EINVAL;
    return NULL;
  }

  dup_na = pr_netaddr_alloc(p);

  pr_netaddr_set_family(dup_na, pr_netaddr_get_family(na));
  pr_netaddr_set_sockaddr(dup_na, pr_netaddr_get_sockaddr(na));  

  return dup_na;
}

pr_netaddr_t *pr_netaddr_get_addr(pool *p, const char *name,
    array_header **addrs) {

  struct sockaddr_in v4;
#ifdef PR_USE_IPV6
  struct sockaddr_in6 v6;
#endif /* PR_USE_IPV6 */
  pr_netaddr_t *na = NULL;
  int res;

  if (p == NULL || name == NULL) {
    errno = EINVAL;
    return NULL;
  }

  /* Attempt to translate the given name into a pr_netaddr_t using
   * pr_inet_pton() first.
   *
   * First, if IPv6 support is enabled, we try to translate the name using
   * pr_inet_pton(AF_INET6) on the hopes that the given string is a valid
   * representation of an IPv6 address.  If that fails, or if IPv6 support
   * is not enabled, we try with pr_inet_pton(AF_INET).  If that fails, we
   * assume that the given name is a DNS name, and we call pr_getaddrinfo().
   */

  na = (pr_netaddr_t *) pcalloc(p, sizeof(pr_netaddr_t));

#ifdef PR_USE_IPV6
  memset(&v6, 0, sizeof(v6));
  v6.sin6_family = AF_INET6;

# ifdef SIN6_LEN
  v6.sin6_len = sizeof(struct sockaddr_in6);
# endif /* SIN6_LEN */

  res = pr_inet_pton(AF_INET6, name, &v6.sin6_addr);
  if (res > 0) {
    pr_netaddr_set_family(na, AF_INET6);
    pr_netaddr_set_sockaddr(na, (struct sockaddr *) &v6);
    if (addrs)
      *addrs = NULL;

    pr_log_debug(DEBUG10, "'%s' resolved to IPv6 address %s", name,
      pr_netaddr_get_ipstr(na));
    return na;
  }
#endif /* PR_USE_IPV6 */

  memset(&v4, 0, sizeof(v4));
  v4.sin_family = AF_INET;

# ifdef SIN_LEN
  v4.sin_len = sizeof(struct sockaddr_in);
# endif /* SIN_LEN */

  res = pr_inet_pton(AF_INET, name, &v4.sin_addr);
  if (res > 0) {
    pr_netaddr_set_family(na, AF_INET);
    pr_netaddr_set_sockaddr(na, (struct sockaddr *) &v4);
    if (addrs)
      *addrs = NULL;

    pr_log_debug(DEBUG10, "'%s' resolved to IPv4 address %s", name,
      pr_netaddr_get_ipstr(na));
    return na;

  } else if (res == 0) {

    /* If pr_inet_pton() returns 0, it means that name does not represent a
     * valid network address in the specified address family.  Usually,
     * this means that name is actually a DNS name, not an IP address
     * string.  So we treat it as a DNS name, and use getaddrinfo(3) to
     * resolve that name to its IP address(es).
     */

    struct addrinfo hints, *info = NULL;
    int gai_res = 0;

    memset(&hints, 0, sizeof(hints));

    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    pr_log_debug(DEBUG10,
      "attempting to resolve '%s' to IPv4 address via DNS", name);
    gai_res = pr_getaddrinfo(name, NULL, &hints, &info);
    if (gai_res != 0) {
      pr_log_pri(PR_LOG_INFO, "getaddrinfo '%s' error: %s", name,
        gai_res != EAI_SYSTEM ? pr_gai_strerror(gai_res) : strerror(errno));
      return NULL;
    }

    if (info) {
      /* Copy the first returned addr into na, as the return value. */
      pr_netaddr_set_family(na, info->ai_family);
      pr_netaddr_set_sockaddr(na, info->ai_addr);

      pr_log_debug(DEBUG10, "resolved '%s' to %s address %s", name,
        info->ai_family == AF_INET ? "IPv4" : "IPv6",
        pr_netaddr_get_ipstr(na));

      pr_freeaddrinfo(info);
    }

#ifdef PR_USE_IPV6
    if (addrs) {
      /* Do the call again, this time for IPv6 addresses.
       *
       * We make two separate getaddrinfo(3) calls, rather than one
       * with a hint of AF_UNSPEC, because of certain bugs where the use
       * of AF_UNSPEC does not function as advertised.  (I suspect this
       * bug was caused by proftpd's calling pattern, but as I could
       * not track it down, and as there are reports of AF_UNSPEC not
       * being as fast as AF_INET/AF_INET6, it just seemed easier to
       * do it this way.)
       */

      gai_res = 0;

      memset(&hints, 0, sizeof(hints));

      hints.ai_family = AF_INET6;
      hints.ai_socktype = SOCK_STREAM;

      pr_log_debug(DEBUG10,
        "attempting to resolve '%s' to IPv6 address via DNS", name);
      gai_res = pr_getaddrinfo(name, NULL, &hints, &info);
      if (gai_res != 0) {
        pr_log_pri(PR_LOG_INFO, "getaddrinfo '%s' error: %s", name,
          gai_res != EAI_SYSTEM ? pr_gai_strerror(gai_res) : strerror(errno));
        return na;
      }

      if (info) {
        pr_netaddr_t **elt;

        *addrs = make_array(p, 0, sizeof(pr_netaddr_t *));
        elt = push_array(*addrs);

        *elt = pcalloc(p, sizeof(pr_netaddr_t));
        pr_netaddr_set_family(*elt, info->ai_family);
        pr_netaddr_set_sockaddr(*elt, info->ai_addr);

        pr_log_debug(DEBUG10, "resolved '%s' to %s address %s", name,
          info->ai_family == AF_INET ? "IPv4" : "IPv6",
          pr_netaddr_get_ipstr(*elt));

        pr_freeaddrinfo(info);
      }
    }
#endif /* PR_USE_IPV6 */

    return na;
  }

  pr_log_debug(DEBUG10, "failed to resolve '%s' to an IP address", name);
  return NULL;
}

int pr_netaddr_get_family(const pr_netaddr_t *na) {
  if (!na) {
    errno = EINVAL;
    return -1;
  }

  return na->na_family;
}

int pr_netaddr_set_family(pr_netaddr_t *na, int family) {
  if (!na) {
    errno = EINVAL;
    return -1;
  }

  /* Set the family member of the appropriate sockaddr struct. */
  switch (family) {
    case AF_INET:
      na->na_addr.v4.sin_family = AF_INET;
      break;

#ifdef PR_USE_IPV6
    case AF_INET6:
      na->na_addr.v6.sin6_family = AF_INET6;
      break;
#endif /* PR_USE_IPV6 */

    default:
#ifdef EAFNOSUPPORT
      errno = EAFNOSUPPORT;
#else
      errno = EINVAL;
#endif
      return -1;
  }

  na->na_family = family;
  return 0;
}

size_t pr_netaddr_get_sockaddr_len(const pr_netaddr_t *na) {
  if (!na) {
    errno = EINVAL;
    return -1;
  }

  switch (pr_netaddr_get_family(na)) {
    case AF_INET:
      return sizeof(struct sockaddr_in);
 
#ifdef PR_USE_IPV6
    case AF_INET6:
      return sizeof(struct sockaddr_in6);
#endif /* PR_USE_IPV6 */   
  }

  errno = EPERM;
  return -1;
}

size_t pr_netaddr_get_inaddr_len(const pr_netaddr_t *na) {
  if (!na) {
    errno = EINVAL;
    return -1;
  }

  switch (pr_netaddr_get_family(na)) {
    case AF_INET:
      return sizeof(struct in_addr);

#ifdef PR_USE_IPV6
    case AF_INET6:
      return sizeof(struct in6_addr);
#endif /* PR_USE_IPV6 */
  }

  errno = EPERM;
  return -1;
}

struct sockaddr *pr_netaddr_get_sockaddr(const pr_netaddr_t *na) {
  if (!na) {
    errno = EINVAL;
    return NULL;
  }

  switch (pr_netaddr_get_family(na)) {
    case AF_INET:
      return (struct sockaddr *) &na->na_addr.v4;

#ifdef PR_USE_IPV6
    case AF_INET6:
      return (struct sockaddr *) &na->na_addr.v6;
#endif /* PR_USE_IPV6 */
  }

  errno = EPERM;
  return NULL;
}

int pr_netaddr_set_sockaddr(pr_netaddr_t *na, struct sockaddr *addr) {
  if (!na || !addr) {
    errno = EINVAL;
    return -1;
  }

  memset(&na->na_addr, 0, sizeof(na->na_addr));
  switch (na->na_family) {
    case AF_INET:
      memcpy(&(na->na_addr.v4), addr, sizeof(struct sockaddr_in));
      return 0;

#ifdef PR_USE_IPV6
    case AF_INET6:
      memcpy(&(na->na_addr.v6), addr, sizeof(struct sockaddr_in6));
      return 0;
#endif /* PR_USE_IPV6 */
  }

  errno = EPERM;
  return -1;
}

int pr_netaddr_set_sockaddr_any(pr_netaddr_t *na) {
  if (!na) {
    errno = EINVAL;
    return -1;
  }

  switch (pr_netaddr_get_family(na)) {
    case AF_INET: {
      struct in_addr in4addr_any;
      in4addr_any.s_addr = htonl(INADDR_ANY);
      na->na_addr.v4.sin_family = AF_INET;
#ifdef SIN_LEN
      na->na_addr.v4.sin_len = sizeof(struct sockaddr_in);
#endif /* SIN_LEN */
      memcpy(&na->na_addr.v4.sin_addr, &in4addr_any, sizeof(struct in_addr));
      return 0;
    }

#ifdef PR_USE_IPV6
    case AF_INET6:
      na->na_addr.v6.sin6_family = AF_INET6;
#ifdef SIN6_LEN
      na->na_addr.v6.sin6_len = sizeof(struct sockaddr_in6);
#endif /* SIN6_LEN */
      memcpy(&na->na_addr.v6.sin6_addr, &in6addr_any, sizeof(struct in6_addr));
      return 0;
#endif /* PR_USE_IPV6 */
  }

  errno = EPERM;
  return -1;
}

void *pr_netaddr_get_inaddr(const pr_netaddr_t *na) {
  if (!na) {
    errno = EINVAL;
    return NULL;
  }

  switch (pr_netaddr_get_family(na)) {
    case AF_INET:
      return (void *) &na->na_addr.v4.sin_addr;

#ifdef PR_USE_IPV6
    case AF_INET6:
      return (void *) &na->na_addr.v6.sin6_addr;
#endif /* PR_USE_IPV6 */
  }

  errno = EPERM;
  return NULL;
}

unsigned int pr_netaddr_get_port(const pr_netaddr_t *na) {
  if (!na) {
    errno = EINVAL;
    return 0;
  }

  switch (pr_netaddr_get_family(na)) {
    case AF_INET:
      return na->na_addr.v4.sin_port;

#ifdef PR_USE_IPV6
    case AF_INET6:
      return na->na_addr.v6.sin6_port;
#endif /* PR_USE_IPV6 */
  }

  errno = EPERM;
  return 0;
}

int pr_netaddr_set_port(pr_netaddr_t *na, unsigned int port) {
  if (!na) {
    errno = EINVAL;
    return -1;
  }

  switch (pr_netaddr_get_family(na)) {
    case AF_INET:
      na->na_addr.v4.sin_port = port;
      return 0;

#ifdef PR_USE_IPV6
    case AF_INET6:
      na->na_addr.v6.sin6_port = port;
      return 0;
#endif /* PR_USE_IPV6 */
  }

  errno = EPERM;
  return 0;
}

int pr_netaddr_cmp(const pr_netaddr_t *na1, const pr_netaddr_t *na2) {
  if (na1 && !na2)
    return 1;

  if (!na1 && na2)
    return -1;

  if (!na1 && !na2)
    return 0;

  if (pr_netaddr_get_family(na1) != pr_netaddr_get_family(na2)) {
    /* Cannot compare addresses from different families. */
    errno = EINVAL;
    return -1;
  }

  switch (pr_netaddr_get_family(na1)) {
    case AF_INET:
      return memcmp(&na1->na_addr.v4.sin_addr, &na2->na_addr.v4.sin_addr,
        sizeof(struct in_addr));

#ifdef PR_USE_IPV6
    case AF_INET6:
      return memcmp(&na1->na_addr.v6.sin6_addr, &na2->na_addr.v6.sin6_addr,
        sizeof(struct in6_addr));
#endif /* PR_USE_IPV6 */
  }

  errno = EPERM;
  return -1;
}

int pr_netaddr_ncmp(const pr_netaddr_t *na1, const pr_netaddr_t *na2,
    unsigned int bitlen) {
  unsigned int nbytes, nbits;
  const unsigned char *in1, *in2;

  if (na1 && !na2)
    return 1;

  if (!na1 && na2)
    return -1;

  if (!na1 && !na2)
    return 0;

  if (pr_netaddr_get_family(na1) != pr_netaddr_get_family(na2)) {
    /* Cannot compare addresses from different families. */
    errno = EINVAL;
    return -1;
  }

  switch (pr_netaddr_get_family(na1)) {
    case AF_INET: {
      /* Make sure that the given number of bits is not more than supported
       * for IPv4 addresses (32).
       */
      if (bitlen > 32) {
        errno = EINVAL;
        return -1;
      }

      break;
    }

#ifdef PR_USE_IPV6
    case AF_INET6: {
      /* Make sure that the given number of bits is not more than supported
       * for IPv6 addresses (128).
       */
      if (bitlen > 128) {
        errno = EINVAL;
        return -1;
      }

      break;
    }
#endif /* PR_USE_IPV6 */

    default:
      errno = EPERM;
      return -1;
  }

  /* Retrieve pointers to the contained in_addrs. */
  in1 = (const unsigned char *) pr_netaddr_get_inaddr(na1);
  in2 = (const unsigned char *) pr_netaddr_get_inaddr(na2);

  /* Determine the number of bytes, and leftover bits, in the given
   * bit length.
   */
  nbytes = bitlen / 8;
  nbits = bitlen % 8;

  /* Compare bytes, using memcmp(3), first. */
  if (nbytes > 0) {
    int res = memcmp(in1, in2, nbytes);

    /* No need to continue comparing the addresses if they differ already. */
    if (res != 0)
      return res;
  }

  /* Next, compare the remaining bits in the addresses. */
  if (nbits > 0) {
    unsigned char mask;

    /* Get the bytes in the addresses that have not yet been compared. */
    unsigned char in1byte = in1[nbytes];
    unsigned char in2byte = in2[nbytes];

    /* Build up a mask covering the bits left to be checked. */
    mask = (0xff << (8 - nbits)) & 0xff;

    if ((in1byte & mask) > (in2byte & mask))
      return 1;

    if ((in1byte & mask) < (in2byte & mask))
      return -1;
  }

  /* If we've made it this far, the addresses match, for the given bit
   * length.
   */
  return 0;
}

int pr_netaddr_fnmatch(pr_netaddr_t *na, const char *pattern, int flags) {

  /* Note: I'm still not sure why proftpd bundles an fnmatch(3)
   * implementation rather than using the system library's implementation.
   * Needs looking into.
   *
   * The FNM_CASEFOLD flag is a GNU extension; perhaps the bundled
   * implementation was added to make that flag available on other platforms.
   */
  int match_flags = PR_FNM_NOESCAPE|PR_FNM_CASEFOLD;

  if (!na || !pattern) {
    errno = EINVAL;
    return -1;
  }

  if (flags & PR_NETADDR_MATCH_DNS) {
    const char *dnsstr = pr_netaddr_get_dnsstr(na);

    pr_log_debug(DEBUG6, "comparing DNS name '%s' to pattern '%s'", dnsstr,
      pattern);
    if (pr_fnmatch(pattern, dnsstr, match_flags) == 0)
      return TRUE;
  }

  if (flags & PR_NETADDR_MATCH_IP) {
    const char *ipstr = pr_netaddr_get_ipstr(na);

    pr_log_debug(DEBUG6, "comparing IP address '%s' to pattern '%s'", ipstr,
      pattern);
    if (pr_fnmatch(pattern, ipstr, match_flags) == 0)
      return TRUE;
  }

  return FALSE;
}

const char *pr_netaddr_get_ipstr(pr_netaddr_t *na) {
#ifdef PR_USE_IPV6
  char buf[INET6_ADDRSTRLEN];
#else
  char buf[INET_ADDRSTRLEN];
#endif /* PR_USE_IPV6 */
  int res = 0;
  
  if (!na) {
    errno = EINVAL;
    return NULL;
  }

  /* If this pr_netaddr_t has already been resolved to an IP string, return the
   * cached string.
   */
  if (na->na_have_ipstr)
    return na->na_ipstr;

  memset(buf, '\0', sizeof(buf));
  res = pr_getnameinfo(pr_netaddr_get_sockaddr(na),
    pr_netaddr_get_sockaddr_len(na), buf, sizeof(buf), NULL, 0, NI_NUMERICHOST);

  if (res != 0) {
    pr_log_pri(PR_LOG_NOTICE, "getnameinfo error: %s",
      res != EAI_SYSTEM ? pr_gai_strerror(res) : strerror(errno));
    return NULL;
  }

  /* Copy the string into the pr_netaddr_t cache as well, so we only
   * have to do this once for this pr_netaddr_t.
   */
  memset(na->na_ipstr, '\0', sizeof(na->na_ipstr));
  sstrncpy(na->na_ipstr, buf, sizeof(na->na_ipstr));
  na->na_have_ipstr = TRUE;

  return na->na_ipstr;
}

/* This differs from pr_netaddr_get_ipstr() in that pr_netaddr_get_ipstr()
 * returns a string of the numeric form of the given network address, whereas
 * this function returns a string of the DNS name (if present).
 */
const char *pr_netaddr_get_dnsstr(pr_netaddr_t *na) {
  char *name = NULL;
  char buf[256];

  if (!na) {
    errno = EINVAL;
    return NULL;
  }

  /* If this pr_netaddr_t has already been resolved to an DNS string, return the
   * cached string.
   */
  if (na->na_have_dnsstr)
    return na->na_dnsstr;

  if (reverse_dns) {
    int res = 0;

    memset(buf, '\0', sizeof(buf));
    res = pr_getnameinfo(pr_netaddr_get_sockaddr(na),
      pr_netaddr_get_sockaddr_len(na), buf, sizeof(buf), NULL, 0, NI_NAMEREQD);

    if (res == 0) {
      char **checkaddr;
      struct hostent *hent = NULL;
      unsigned char ok = FALSE;
      int family = pr_netaddr_get_family(na);
      void *inaddr = pr_netaddr_get_inaddr(na);
    
#ifdef HAVE_GETHOSTBYNAME2
      if (pr_netaddr_is_v4mappedv6(na) == TRUE) {
        family = AF_INET;
        inaddr = get_v4inaddr(na);
      }

      hent = gethostbyname2(buf, family);
#else
      hent = gethostbyname(buf);
#endif /* HAVE_GETHOSTBYNAME2 */

      if (hent != NULL) {
        switch (hent->h_addrtype) {
          case AF_INET:
            if (family == AF_INET) {
              for (checkaddr = hent->h_addr_list; *checkaddr; ++checkaddr) {
                if (memcmp(*checkaddr, inaddr, hent->h_length) == 0) {
                  ok = TRUE;
                  break;
                }
              }
            } 
            break;

#ifdef PR_USE_IPV6
          case AF_INET6:
            if (family == AF_INET6) {
              for (checkaddr = hent->h_addr_list; *checkaddr; ++checkaddr) {
                if (memcmp(*checkaddr, inaddr, hent->h_length) == 0) {
                  ok = TRUE;
                  break;
                }
              }
            } 
            break;
#endif /* PR_USE_IPV6 */
        }

        name = ok ? buf : NULL;

      } else
        pr_log_debug(DEBUG1, "notice: unable to resolve '%s': %s", buf,
          hstrerror(errno));
    }

  } else
    pr_log_debug(DEBUG10,
      "UseReverseDNS off, returning IP address instead of DNS name");

  if (!name)
    name = (char *) pr_netaddr_get_ipstr(na);

  name = pr_inet_validate(name);

  /* Copy the string into the pr_netaddr_t cache as well, so we only
   * have to do this once for this pr_netaddr_t.
   */
  memset(na->na_dnsstr, '\0', sizeof(na->na_dnsstr));
  sstrncpy(na->na_dnsstr, name, sizeof(na->na_dnsstr));
  na->na_have_dnsstr = TRUE;

  return na->na_dnsstr;
}

/* Return the hostname (wrapper for gethostname(2), except returns FQDN). */
const char *pr_netaddr_get_localaddr_str(pool *p) {
  char buf[256] = {'\0'};
  struct hostent *host;

  if (gethostname(buf, sizeof(buf)-1) != -1) {
    buf[sizeof(buf)-1] = '\0';

    /* Note: this may need to be gethostbyname2() on systems that provide
     * that function, for it is possible that the configured hostname for
     * a machine only resolves to an IPv6 address.
     */
    host = gethostbyname(buf);

    if (host)
      return pr_inet_validate(pstrdup(p, host->h_name));

    return pr_inet_validate(pstrdup(p, buf));
  }

  return NULL;
}

int pr_netaddr_loopback(const pr_netaddr_t *na) {
  if (!na) {
    errno = EINVAL;
    return -1;
  }

  switch (pr_netaddr_get_family(na)) {
    case AF_INET:
      return IN_IS_ADDR_LOOPBACK(
        (struct in_addr *) pr_netaddr_get_inaddr(na));

#ifdef PR_USE_IPV6
    case AF_INET6:

      /* XXX *sigh* Different platforms implement the IN6_IS_ADDR macros
       * differently.  For example, on Linux, those macros expect to operate
       * on s6_addr32, while on Solaris, the macros operate on struct in6_addr.
       * Certain Drafts define the macros to work on struct in6_addr *, as
       * Solaris does, so Linux may have it wrong.  Tentative research on
       * Google shows some BSD netinet6/in6.h headers that define these
       * macros in terms of struct in6_addr *, so I'll go with that for now.
       * Joy. =P
       */
# ifndef LINUX
      return IN6_IS_ADDR_LOOPBACK(
        (struct in6_addr *) pr_netaddr_get_inaddr(na));
# else
      return IN6_IS_ADDR_LOOPBACK(
        ((struct in6_addr *) pr_netaddr_get_inaddr(na))->s6_addr32);
# endif
#endif /* PR_USE_IPV6 */
  }

  return FALSE;
}

/* A slightly naughty function that should go away. It relies too much on
 * knowledge of the internal structures of struct in_addr, struct in6_addr.
 */
unsigned int pr_netaddr_get_addrno(const pr_netaddr_t *na) {
  if (!na) {
    errno = EINVAL;
    return -1;
  }

  switch (pr_netaddr_get_family(na)) {
    case AF_INET:
      return na->na_addr.v4.sin_addr.s_addr;

#ifdef PR_USE_IPV6
    case AF_INET6: {

      /* Linux defines s6_addr32 in its netinet/in.h header.
       * FreeBSD defines s6_addr32 in KAME's netinet6/in6.h header.
       * Solaris defines s6_addr32 in its netinet/in.h header, but only
       * for kernel builds.
       */
#if 0
      int *addrs = ((struct sockaddr_in6 *) pr_netaddr_get_inaddr(na))->s6_addr32;
      return addrs[0];
#else
      return 0;
#endif
    }
#endif /* PR_USE_IPV6 */
  }

  errno = EPERM;
  return -1;
}

int pr_netaddr_is_v4mappedv6(const pr_netaddr_t *na) {
  if (!na) {
    errno = EINVAL;
    return -1;
  }

  switch (pr_netaddr_get_family(na)) {
    case AF_INET:

      /* This function tests only IPv6 addresses, not IPv4 addresses. */
      errno = EINVAL;
      return -1;

#ifdef PR_USE_IPV6
    case AF_INET6:

# ifndef LINUX
      return IN6_IS_ADDR_V4MAPPED(
        (struct in6_addr *) pr_netaddr_get_inaddr(na));
# else
      return IN6_IS_ADDR_V4MAPPED(
        ((struct in6_addr *) pr_netaddr_get_inaddr(na))->s6_addr32);
# endif
#endif /* PR_USE_IPV6 */
  }

  errno = EPERM;
  return -1;
}

