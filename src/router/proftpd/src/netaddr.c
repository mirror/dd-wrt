/*
 * ProFTPD - FTP server daemon
 * Copyright (c) 2003-2011 The ProFTPD Project team
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
 * Foundation, Inc., 51 Franklin Street, Suite 500, Boston, MA 02110-1335, USA.
 *
 * As a special exemption, The ProFTPD Project team and other respective
 * copyright holders give permission to link this program with OpenSSL, and
 * distribute the resulting executable, without including the source code for
 * OpenSSL in the source distribution.
 */

/* Network address routines
 * $Id: netaddr.c,v 1.79 2011/05/23 21:22:24 castaglia Exp $
 */

#include "conf.h"

/* Define an IPv4 equivalent of the IN6_IS_ADDR_LOOPBACK macro. */
#undef IN_IS_ADDR_LOOPBACK
#define IN_IS_ADDR_LOOPBACK(a) \
  ((((unsigned long int) ntohl((a)->s_addr)) & 0xff000000) == 0x7f000000)

static pr_netaddr_t sess_local_addr;
static int have_sess_local_addr = FALSE;

static pr_netaddr_t sess_remote_addr;
static char sess_remote_name[PR_TUNABLE_BUFFER_SIZE];
static int have_sess_remote_addr = FALSE;

/* Do reverse DNS lookups? */
static int reverse_dns = 1;

/* Use IPv6? */
#ifdef PR_USE_IPV6
static int use_ipv6 = TRUE;
#else
static int use_ipv6 = FALSE;
#endif /* PR_USE_IPV6 */

static char localaddr_str[PR_TUNABLE_BUFFER_SIZE];
static int have_localaddr_str = FALSE;

static pool *netaddr_pool = NULL;
static pr_table_t *netaddr_iptab = NULL;
static pr_table_t *netaddr_dnstab = NULL;

static const char *trace_channel = "dns";

/* Netaddr cache management */
static array_header *netaddr_dnscache_get(pool *p, const char *ip_str) {
  array_header *res = NULL;

  if (netaddr_dnstab) {
    void *v = pr_table_get(netaddr_dnstab, ip_str, NULL);
    if (v) {
      res = v;

      pr_trace_msg(trace_channel, 4,
        "using %d DNS %s from netaddr DNS cache for IP address '%s'",
        res->nelts, res->nelts != 1 ? "names" : "name", ip_str);

      if (p) {
        /* If the caller provided a pool, return a copy of the array. */
        return copy_array_str(p, res);
      }

      return res;
    }
  }

  pr_trace_msg(trace_channel, 12,
    "no DNS names found in netaddr DNS cache for IP address '%s'", ip_str);
  errno = ENOENT;
  return NULL;
}

static void netaddr_dnscache_set(const char *ip_str, const char *dns_name) {
  if (netaddr_dnstab) {
    void *v = NULL;
    array_header *res = NULL;
    int add_list = FALSE;

    res = netaddr_dnscache_get(NULL, ip_str);
    if (res == NULL) {
      /* No existing entries for this IP address yet. */
      res = make_array(netaddr_pool, 1, sizeof(char *));
      add_list = TRUE;

    } else {
      register unsigned int i;
      char **names;

      /* Check for duplicates. */
      names = res->elts;
      for (i = 0; i < res->nelts; i++) {
        if (names[i] != NULL) {
          if (strcmp(names[i], dns_name) == 0) {
            pr_trace_msg(trace_channel, 5,
              "DNS name '%s' for IP address '%s' already stashed in the "
              "netaddr DNS cache", dns_name, ip_str);
            return;
          }
        }
      }
    }

    *((char **) push_array(res)) = pstrdup(netaddr_pool, dns_name);
    v = res;

    if (add_list) { 
      if (pr_table_add(netaddr_dnstab, pstrdup(netaddr_pool, ip_str), v,
          sizeof(array_header *)) < 0) {
        pr_trace_msg(trace_channel, 3,
          "error adding DNS name '%s' for IP address '%s' to the netaddr "
          "DNS cache: %s", dns_name, ip_str, strerror(errno));

      } else {
        pr_trace_msg(trace_channel, 5,
          "stashed DNS name '%s' for IP address '%s' in the netaddr DNS cache",
          dns_name, ip_str);
      }

    } else {
      pr_trace_msg(trace_channel, 5,
        "stashed DNS name '%s' for IP address '%s' in the netaddr DNS cache",
        dns_name, ip_str);
    }
  }

  return;
}

static pr_netaddr_t *netaddr_ipcache_get(pool *p, const char *name) {
  pr_netaddr_t *res = NULL;

  if (netaddr_iptab) {
    void *v = pr_table_get(netaddr_iptab, name, NULL);
    if (v) {
      res = v;
      pr_trace_msg(trace_channel, 4,
        "using IP address '%s' from netaddr IP cache for name '%s'",
        pr_netaddr_get_ipstr(res), name);

      /* We return a copy of the cache's netaddr_t, if the caller provided
       * a pool for duplication.
       */
      if (p) {
        return pr_netaddr_dup(p, res);
      }

      return res;
    }
  }

  pr_trace_msg(trace_channel, 2,
    "no IP address found in netaddr IP cache for name '%s'", name);
  errno = ENOENT;
  return NULL;
}

static void netaddr_ipcache_set(const char *name, pr_netaddr_t *na) {
  if (netaddr_iptab) {
    int count = 0;
    void *v = NULL;

    /* We store an internal copy of the netaddr_t in the cache. */
    v = pr_netaddr_dup(netaddr_pool, na);

    count = pr_table_exists(netaddr_iptab, name);
    if (count <= 0) {
      if (pr_table_add(netaddr_iptab, pstrdup(netaddr_pool, name), v,
          sizeof(pr_netaddr_t *)) < 0) {
        pr_trace_msg(trace_channel, 3,
          "error adding IP address '%s' for name '%s' to the netaddr "
          "IP cache: %s", pr_netaddr_get_ipstr(na), name,
          strerror(errno));

      } else {
        pr_trace_msg(trace_channel, 5,
          "stashed IP address '%s' for name '%s' in the netaddr IP cache",
          pr_netaddr_get_ipstr(v), name);
      }

    } else {
      if (pr_table_set(netaddr_iptab, pstrdup(netaddr_pool, name), v,
          sizeof(pr_netaddr_t *)) < 0) {
        pr_trace_msg(trace_channel, 3,
          "error setting IP address '%s' for name '%s' in the netaddr "
          "IP cache: %s", pr_netaddr_get_ipstr(na), name, strerror(errno));
      }
    }
  }

  return;
}

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

    if (hints->ai_socktype != 0) {
      socktype = hints->ai_socktype;

    } else if (strncasecmp(proto_name, "udp", 4) == 0) {
      socktype = SOCK_DGRAM;
    }
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

static void *get_v4inaddr(const pr_netaddr_t *na) {

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

/* Validate anything returned from the 'outside', since it's untrusted
 * information.
 */
char *pr_netaddr_validate_dns_str(char *buf) {
  char *p;

  if (buf == NULL) {
    errno = EINVAL;
    return NULL;
  }

  /* Validate anything returned from a DNS. */
  for (p = buf; p && *p; p++) {

    /* Per RFC requirements, these are all that are valid from a DNS. */
    if (!isalnum((int) *p) &&
        *p != '.' &&
        *p != '-'
#ifdef PR_USE_IPV6
        && *p != ':'
#endif /* PR_USE_IPV6 */
        ) {

      /* We set it to _ because we know that's an invalid, yet safe, option
       * for a DNS entry.
       */
      *p = '_';
    }
  }

  return buf;
}

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

  if (na->na_have_ipstr) {
    sstrncpy(dup_na->na_ipstr, na->na_ipstr, sizeof(dup_na->na_ipstr));
    dup_na->na_have_ipstr = 1;
  }

  if (na->na_have_dnsstr) {
    sstrncpy(dup_na->na_dnsstr, na->na_dnsstr, sizeof(dup_na->na_dnsstr));
    dup_na->na_have_dnsstr = 1;
  }

  return dup_na;
}

pr_netaddr_t *pr_netaddr_get_addr(pool *p, const char *name,
    array_header **addrs) {

  struct sockaddr_in v4;
  pr_netaddr_t *na = NULL;
  int res;

  if (p == NULL ||
      name == NULL) {
    errno = EINVAL;
    return NULL;
  }

  pr_trace_msg(trace_channel, 10, "resolving name '%s' to IP address",
    name);

  /* First, check our cache to see if this name has already been
   * resolved.  We only want to use the cache, though, if the caller did not
   * provide the `addrs' pointer, indicating that the caller wants to know
   * about any additional addresses for the given name.  The netaddr cache
   * is a simple cache, hidden from callers, and thus is unable to populate
   * that `addrs' pointer if the name is in the cache.
   */
  if (addrs == NULL) {
    na = netaddr_ipcache_get(p, name);
    if (na) {
      return na;
    }
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
  if (use_ipv6) {
    struct sockaddr_in6 v6;
    memset(&v6, 0, sizeof(v6));
    v6.sin6_family = AF_INET6;

# ifdef SIN6_LEN
    v6.sin6_len = sizeof(struct sockaddr_in6);
# endif /* SIN6_LEN */

    res = pr_inet_pton(AF_INET6, name, &v6.sin6_addr);
    if (res > 0) {
      int xerrno = errno;

      pr_netaddr_set_family(na, AF_INET6);
      pr_netaddr_set_sockaddr(na, (struct sockaddr *) &v6);
      if (addrs)
        *addrs = NULL;

      pr_trace_msg(trace_channel, 7, "'%s' resolved to IPv6 address %s", name,
        pr_netaddr_get_ipstr(na));

      netaddr_ipcache_set(name, na);
      netaddr_ipcache_set(pr_netaddr_get_ipstr(na), na);

      errno = xerrno;
      return na;
    }
  }
#endif /* PR_USE_IPV6 */

  memset(&v4, 0, sizeof(v4));
  v4.sin_family = AF_INET;

# ifdef SIN_LEN
  v4.sin_len = sizeof(struct sockaddr_in);
# endif /* SIN_LEN */

  res = pr_inet_pton(AF_INET, name, &v4.sin_addr);
  if (res > 0) {
    int xerrno = errno;

    pr_netaddr_set_family(na, AF_INET);
    pr_netaddr_set_sockaddr(na, (struct sockaddr *) &v4);
    if (addrs)
      *addrs = NULL;

    pr_trace_msg(trace_channel, 7, "'%s' resolved to IPv4 address %s", name,
      pr_netaddr_get_ipstr(na));

    netaddr_ipcache_set(name, na);
    netaddr_ipcache_set(pr_netaddr_get_ipstr(na), na);

    errno = xerrno;
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
    hints.ai_protocol = IPPROTO_TCP;

    pr_trace_msg(trace_channel, 7,
      "attempting to resolve '%s' to IPv4 address via DNS", name);
    gai_res = pr_getaddrinfo(name, NULL, &hints, &info);
    if (gai_res != 0) {
      int xerrno = errno;

      if (gai_res != EAI_SYSTEM) {
        pr_trace_msg(trace_channel, 1, "IPv4 getaddrinfo '%s' error: %s",
          name, pr_gai_strerror(gai_res));

      } else {
        pr_trace_msg(trace_channel, 1,
          "IPv4 getaddrinfo '%s' system error: [%d] %s", name,
          xerrno, strerror(xerrno));
      }

      errno = xerrno;
      return NULL;
    }

    if (info) {
      /* Copy the first returned addr into na, as the return value. */
      pr_netaddr_set_family(na, info->ai_family);
      pr_netaddr_set_sockaddr(na, info->ai_addr);

      pr_trace_msg(trace_channel, 7, "resolved '%s' to %s address %s", name,
        info->ai_family == AF_INET ? "IPv4" : "IPv6",
        pr_netaddr_get_ipstr(na));

      netaddr_ipcache_set(name, na);
      netaddr_ipcache_set(pr_netaddr_get_ipstr(na), na);

      pr_freeaddrinfo(info);
    }

#ifdef PR_USE_IPV6
    if (use_ipv6 && addrs) {
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

      memset(&hints, 0, sizeof(hints));

      hints.ai_family = AF_INET6;
      hints.ai_socktype = SOCK_STREAM;
      hints.ai_protocol = IPPROTO_TCP;

      pr_trace_msg(trace_channel, 7,
        "attempting to resolve '%s' to IPv6 address via DNS", name);
      gai_res = pr_getaddrinfo(name, NULL, &hints, &info);
      if (gai_res != 0) {
        int xerrno = errno;

        if (gai_res != EAI_SYSTEM) {
          pr_trace_msg(trace_channel, 1, "IPv6 getaddrinfo '%s' error: %s",
            name, pr_gai_strerror(gai_res));

        } else {
          pr_trace_msg(trace_channel, 1, 
            "IPv6 getaddrinfo '%s' system error: [%d] %s", name,
            xerrno, strerror(xerrno));
        }

        errno = xerrno;
        return na;
      }

      if (info) {
        pr_netaddr_t **elt;

        *addrs = make_array(p, 0, sizeof(pr_netaddr_t *));
        elt = push_array(*addrs);

        *elt = pcalloc(p, sizeof(pr_netaddr_t));
        pr_netaddr_set_family(*elt, info->ai_family);
        pr_netaddr_set_sockaddr(*elt, info->ai_addr);

        pr_trace_msg(trace_channel, 7, "resolved '%s' to %s address %s", name,
          info->ai_family == AF_INET ? "IPv4" : "IPv6",
          pr_netaddr_get_ipstr(*elt));

        pr_freeaddrinfo(info);
      }
    }
#endif /* PR_USE_IPV6 */

    return na;
  }

  pr_trace_msg(trace_channel, 8, "failed to resolve '%s' to an IP address",
    name);
  errno = ENOENT;
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
      if (use_ipv6) {
        na->na_addr.v6.sin6_family = AF_INET6;
        break;
      }
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
      if (use_ipv6)
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
      if (use_ipv6)
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
      if (use_ipv6)
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
      if (use_ipv6) {
        memcpy(&(na->na_addr.v6), addr, sizeof(struct sockaddr_in6));
        return 0;
      }
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
      if (use_ipv6) {
        na->na_addr.v6.sin6_family = AF_INET6;
#ifdef SIN6_LEN
        na->na_addr.v6.sin6_len = sizeof(struct sockaddr_in6);
#endif /* SIN6_LEN */
        memcpy(&na->na_addr.v6.sin6_addr, &in6addr_any, sizeof(struct in6_addr));
        return 0;
      }
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
      if (use_ipv6)
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
      if (use_ipv6)
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
      if (use_ipv6) {
        na->na_addr.v6.sin6_port = port;
        return 0;
      }
#endif /* PR_USE_IPV6 */
  }

  errno = EPERM;
  return -1;
}

int pr_netaddr_cmp(const pr_netaddr_t *na1, const pr_netaddr_t *na2) {
  pool *tmp_pool = NULL;
  pr_netaddr_t *a, *b;
  int res;

  if (na1 && !na2)
    return 1;

  if (!na1 && na2)
    return -1;

  if (!na1 && !na2)
    return 0;

  if (pr_netaddr_get_family(na1) != pr_netaddr_get_family(na2)) {

    /* Cannot compare addresses from different families, unless one
     * of the netaddrs has an AF_INET family, and the other has an
     * AF_INET6 family AND is an IPv4-mapped IPv6 address.
     */

    if (pr_netaddr_is_v4mappedv6(na1) != TRUE &&
        pr_netaddr_is_v4mappedv6(na2) != TRUE) {
      errno = EINVAL;
      return -1;
    }

    if (pr_netaddr_is_v4mappedv6(na1) == TRUE) {
      tmp_pool = make_sub_pool(permanent_pool);

      pr_trace_msg(trace_channel, 5, "addr '%s' is an IPv4-mapped IPv6 address",
        pr_netaddr_get_ipstr((pr_netaddr_t *) na1));

      /* This case means that na1 is an IPv4-mapped IPv6 address, and
       * na2 is an IPv4 address.
       */
      a = pr_netaddr_v6tov4(tmp_pool, na1);
      b = (pr_netaddr_t *) na2;

      pr_trace_msg(trace_channel, 6, "comparing IPv4 address '%s' against "
        "IPv4-mapped IPv6 address '%s'", pr_netaddr_get_ipstr(b),
        pr_netaddr_get_ipstr(a));

    } else if (pr_netaddr_is_v4mappedv6(na2) == TRUE) {
      tmp_pool = make_sub_pool(permanent_pool);

      pr_trace_msg(trace_channel, 5, "addr '%s' is an IPv4-mapped IPv6 address",
        pr_netaddr_get_ipstr((pr_netaddr_t *) na2));

      /* This case means that na is an IPv4 address, and na2 is an
       * IPv4-mapped IPv6 address.
       */
      a = (pr_netaddr_t *) na1;
      b = pr_netaddr_v6tov4(tmp_pool, na2);

      pr_trace_msg(trace_channel, 6, "comparing IPv4 address '%s' against "
        "IPv4-mapped IPv6 address '%s'", pr_netaddr_get_ipstr(a),
        pr_netaddr_get_ipstr(b));

    } else {
      a = (pr_netaddr_t *) na1;
      b = (pr_netaddr_t *) na2;
    }

  } else {
    a = (pr_netaddr_t *) na1;
    b = (pr_netaddr_t *) na2;
  }

  switch (pr_netaddr_get_family(a)) {
    case AF_INET:
      res = memcmp(&a->na_addr.v4.sin_addr, &b->na_addr.v4.sin_addr,
        sizeof(struct in_addr));

      if (res != 0) {
        pr_trace_msg(trace_channel, 4, "addr %s does not match addr %s",
          pr_netaddr_get_ipstr(a), pr_netaddr_get_ipstr(b));
      }

      if (tmp_pool) {
        destroy_pool(tmp_pool);
        tmp_pool = NULL;
      }

      return res;

#ifdef PR_USE_IPV6
    case AF_INET6:
      if (use_ipv6) {
        res = memcmp(&a->na_addr.v6.sin6_addr, &b->na_addr.v6.sin6_addr,
          sizeof(struct in6_addr));

        if (res != 0) {
          pr_trace_msg(trace_channel, 4, "addr %s does not match addr %s",
            pr_netaddr_get_ipstr(a), pr_netaddr_get_ipstr(b));
        }

        if (tmp_pool) {
          destroy_pool(tmp_pool);
          tmp_pool = NULL;
        }

        return res;
      }
#endif /* PR_USE_IPV6 */
  }

  if (tmp_pool)
    destroy_pool(tmp_pool);

  errno = EPERM;
  return -1;
}

int pr_netaddr_ncmp(const pr_netaddr_t *na1, const pr_netaddr_t *na2,
    unsigned int bitlen) {
  pool *tmp_pool = NULL;
  pr_netaddr_t *a, *b;
  unsigned int nbytes, nbits;
  const unsigned char *in1, *in2;

  if (na1 && !na2)
    return 1;

  if (!na1 && na2)
    return -1;

  if (!na1 && !na2)
    return 0;

  if (pr_netaddr_get_family(na1) != pr_netaddr_get_family(na2)) {

    /* Cannot compare addresses from different families, unless one
     * of the netaddrs has an AF_INET family, and the other has an
     * AF_INET6 family AND is an IPv4-mapped IPv6 address.
     */

    if (pr_netaddr_is_v4mappedv6(na1) != TRUE &&
        pr_netaddr_is_v4mappedv6(na2) != TRUE) {
      errno = EINVAL;
      return -1;
    }

    if (pr_netaddr_is_v4mappedv6(na1) == TRUE) {
      tmp_pool = make_sub_pool(permanent_pool);

      /* This case means that na1 is an IPv4-mapped IPv6 address, and
       * na2 is an IPv4 address.
       */
      a = pr_netaddr_v6tov4(tmp_pool, na1);
      b = (pr_netaddr_t *) na2;

      pr_trace_msg(trace_channel, 6, "comparing IPv4 address '%s' against "
        "IPv4-mapped IPv6 address '%s'", pr_netaddr_get_ipstr(b),
        pr_netaddr_get_ipstr(a));

    } else if (pr_netaddr_is_v4mappedv6(na2) == TRUE) {
      tmp_pool = make_sub_pool(permanent_pool);

      /* This case means that na is an IPv4 address, and na2 is an
       * IPv4-mapped IPv6 address.
       */
      a = (pr_netaddr_t *) na1;
      b = pr_netaddr_v6tov4(tmp_pool, na2);

      pr_trace_msg(trace_channel, 6, "comparing IPv4 address '%s' against "
        "IPv4-mapped IPv6 address '%s'", pr_netaddr_get_ipstr(a),
        pr_netaddr_get_ipstr(b));

    } else {
      a = (pr_netaddr_t *) na1;
      b = (pr_netaddr_t *) na2;
    }

  } else {
    a = (pr_netaddr_t *) na1;
    b = (pr_netaddr_t *) na2;
  }

  switch (pr_netaddr_get_family(a)) {
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
      if (use_ipv6) {
        /* Make sure that the given number of bits is not more than supported
         * for IPv6 addresses (128).
         */
        if (bitlen > 128) {
          errno = EINVAL;
          return -1;
        }

        break;
      }
    }
#endif /* PR_USE_IPV6 */

    default:
      errno = EPERM;
      return -1;
  }

  /* Retrieve pointers to the contained in_addrs. */
  in1 = (const unsigned char *) pr_netaddr_get_inaddr(a);
  in2 = (const unsigned char *) pr_netaddr_get_inaddr(b);

  /* Determine the number of bytes, and leftover bits, in the given
   * bit length.
   */
  nbytes = bitlen / 8;
  nbits = bitlen % 8;

  /* Compare bytes, using memcmp(3), first. */
  if (nbytes > 0) {
    int res = memcmp(in1, in2, nbytes);

    /* No need to continue comparing the addresses if they differ already. */
    if (res != 0) {
      if (tmp_pool)
        destroy_pool(tmp_pool);

      return res;
    }
  }

  /* Next, compare the remaining bits in the addresses. */
  if (nbits > 0) {
    unsigned char mask;

    /* Get the bytes in the addresses that have not yet been compared. */
    unsigned char in1byte = in1[nbytes];
    unsigned char in2byte = in2[nbytes];

    /* Build up a mask covering the bits left to be checked. */
    mask = (0xff << (8 - nbits)) & 0xff;

    if ((in1byte & mask) > (in2byte & mask)) {
      if (tmp_pool)
        destroy_pool(tmp_pool);

      return 1;
    }

    if ((in1byte & mask) < (in2byte & mask)) {
      if (tmp_pool)
        destroy_pool(tmp_pool);

      return -1;
    }
  }

  if (tmp_pool)
    destroy_pool(tmp_pool);

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

    if (pr_fnmatch(pattern, dnsstr, match_flags) == 0) {
      pr_trace_msg(trace_channel, 6, "DNS name '%s' matches pattern '%s'",
        dnsstr, pattern);
      return TRUE;
    }
  }

  if (flags & PR_NETADDR_MATCH_IP) {
    const char *ipstr = pr_netaddr_get_ipstr(na);

    if (pr_fnmatch(pattern, ipstr, match_flags) == 0) {
      pr_trace_msg(trace_channel, 6, "IP address '%s' matches pattern '%s'",
        ipstr, pattern);
      return TRUE;
    }

    /* If the address is an IPv4-mapped IPv6 address, get the IPv4 address
     * and try to match that against the configured glob pattern.
     */
    if (pr_netaddr_is_v4mappedv6(na) == TRUE) {
      pool *tmp_pool;
      pr_netaddr_t *a;

      pr_trace_msg(trace_channel, 5, "addr '%s' is an IPv4-mapped IPv6 address",
        ipstr);

      tmp_pool = make_sub_pool(permanent_pool);
      a = pr_netaddr_v6tov4(tmp_pool, na);

      ipstr = pr_netaddr_get_ipstr(a);

      if (pr_fnmatch(pattern, ipstr, match_flags) == 0) {
        pr_trace_msg(trace_channel, 6, "IP address '%s' matches pattern '%s'",
          ipstr, pattern);

        destroy_pool(tmp_pool);
        return TRUE;
      }

      destroy_pool(tmp_pool);
    }
  }

  pr_trace_msg(trace_channel, 4, "addr %s does not match pattern '%s'",
    pr_netaddr_get_ipstr(na), pattern);
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
    if (res != EAI_SYSTEM) {
      pr_log_pri(PR_LOG_INFO, "getnameinfo error: %s", pr_gai_strerror(res));

    } else {
      pr_log_pri(PR_LOG_INFO, "getnameinfo system error: [%d] %s",
        errno, strerror(errno));
    }

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
  pr_netaddr_t *cache = NULL;

  if (!na) {
    errno = EINVAL;
    return NULL;
  }

  cache = netaddr_ipcache_get(NULL, pr_netaddr_get_ipstr(na));
  if (cache &&
      cache->na_have_dnsstr) {
    memset(na->na_dnsstr, '\0', sizeof(na->na_dnsstr));
    sstrncpy(na->na_dnsstr, cache->na_dnsstr, sizeof(na->na_dnsstr));
    na->na_have_dnsstr = TRUE;

    return na->na_dnsstr;
  }

  /* If this pr_netaddr_t has already been resolved to an DNS string, return the
   * cached string.
   */
  if (na->na_have_dnsstr)
    return na->na_dnsstr;

  if (reverse_dns) {
    int res = 0;

    pr_trace_msg(trace_channel, 3,
      "verifying DNS name for IP address %s via reverse DNS lookup",
      pr_netaddr_get_ipstr(na));

    memset(buf, '\0', sizeof(buf));
    res = pr_getnameinfo(pr_netaddr_get_sockaddr(na),
      pr_netaddr_get_sockaddr_len(na), buf, sizeof(buf), NULL, 0, NI_NAMEREQD);
    buf[sizeof(buf)-1] = '\0';

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
        if (hent->h_name != NULL) {
          netaddr_dnscache_set(pr_netaddr_get_ipstr(na), hent->h_name);
        }

        pr_trace_msg(trace_channel, 10,
          "checking addresses associated with host '%s'",
          hent->h_name ? hent->h_name : "(null)");

        switch (hent->h_addrtype) {
          case AF_INET:
            if (family == AF_INET) {

              for (checkaddr = hent->h_addr_list; *checkaddr; ++checkaddr) {
                if (memcmp(*checkaddr, inaddr, hent->h_length) == 0) {
                  char **alias;

                  for (alias = hent->h_aliases; *alias; ++alias) {
                    if (hent->h_name) {
                      pr_trace_msg(trace_channel, 10,
                        "host '%s' has alias '%s'", hent->h_name, *alias);
                      netaddr_ipcache_set(*alias, na);
                      netaddr_dnscache_set(pr_netaddr_get_ipstr(na), *alias);
                    }
                  }

                  ok = TRUE;
                  break;
                }
              }
            } 
            break;

#ifdef PR_USE_IPV6
          case AF_INET6:
            if (use_ipv6 && family == AF_INET6) {

              for (checkaddr = hent->h_addr_list; *checkaddr; ++checkaddr) {
                if (memcmp(*checkaddr, inaddr, hent->h_length) == 0) {
                  char **alias;

                  for (alias = hent->h_aliases; *alias; ++alias) {
                    if (hent->h_name) {
                      pr_trace_msg(trace_channel, 10,
                        "host '%s' has alias '%s'", hent->h_name, *alias);
                      netaddr_ipcache_set(*alias, na);
                      netaddr_dnscache_set(pr_netaddr_get_ipstr(na), *alias);
                    }
                  }

                  ok = TRUE;
                  break;
                }
              }
            } 
            break;
#endif /* PR_USE_IPV6 */
        }

        if (ok) {
          name = buf;
          pr_trace_msg(trace_channel, 8,
            "using DNS name '%s' for IP address '%s'", name,
            pr_netaddr_get_ipstr(na));

        } else {
          name = NULL;
          pr_trace_msg(trace_channel, 8,
            "unable to verify any DNS names for IP address '%s'",
            pr_netaddr_get_ipstr(na));
        }

      } else {
        pr_log_debug(DEBUG1, "notice: unable to resolve '%s': %s", buf,
          hstrerror(errno));
      }
    }

  } else {
    pr_log_debug(DEBUG10,
      "UseReverseDNS off, returning IP address instead of DNS name");
  }

  if (name) {
    name = pr_netaddr_validate_dns_str(name);

  } else {
    name = (char *) pr_netaddr_get_ipstr(na);
  }

  /* Copy the string into the pr_netaddr_t cache as well, so we only
   * have to do this once for this pr_netaddr_t.
   */
  memset(na->na_dnsstr, '\0', sizeof(na->na_dnsstr));
  sstrncpy(na->na_dnsstr, name, sizeof(na->na_dnsstr));
  na->na_have_dnsstr = TRUE;

  /* Update the netaddr object in the cache with the resolved DNS names. */
  netaddr_ipcache_set(name, na);
  netaddr_ipcache_set(pr_netaddr_get_ipstr(na), na);

  return na->na_dnsstr;
}

array_header *pr_netaddr_get_dnsstr_list(pool *p, pr_netaddr_t *na) {
  array_header *res;

  if (p == NULL ||
      na == NULL) {
    errno = EINVAL;
    return NULL;
  }

  if (!reverse_dns) {
    /* If UseReverseDNS is off, then we won't have any names that we trust.
     * So return an empty list.
     */
    return make_array(p, 0, sizeof(char *));
  }

  res = netaddr_dnscache_get(p, pr_netaddr_get_ipstr(na));
  if (res == NULL) {
    res = make_array(p, 0, sizeof(char *));
  }

  return res;
}

/* Return the hostname (wrapper for gethostname(2), except returns FQDN). */
const char *pr_netaddr_get_localaddr_str(pool *p) {
  char buf[256];

  if (p == NULL) {
    errno = EINVAL;
    return NULL;
  }

  if (have_localaddr_str) {
    return pr_netaddr_validate_dns_str(pstrdup(p, localaddr_str));
  }

  memset(buf, '\0', sizeof(buf));
  if (gethostname(buf, sizeof(buf)-1) != -1) {
    struct hostent *host;

    buf[sizeof(buf)-1] = '\0';

    /* Note: this may need to be gethostbyname2() on systems that provide
     * that function, for it is possible that the configured hostname for
     * a machine only resolves to an IPv6 address.
     */
    host = gethostbyname(buf);
    if (host)
      return pr_netaddr_validate_dns_str(pstrdup(p, host->h_name));

    return pr_netaddr_validate_dns_str(pstrdup(p, buf));
  }

  pr_trace_msg(trace_channel, 1, "gethostname(2) error: %s", strerror(errno));
  return NULL;
}

int pr_netaddr_set_localaddr_str(const char *addr_str) {
  if (addr_str == NULL) {
    errno = EINVAL;
    return -1;
  }

  memset(localaddr_str, '\0', sizeof(localaddr_str));
  sstrncpy(localaddr_str, addr_str, sizeof(localaddr_str));
  have_localaddr_str = TRUE;
  return 0;
}

int pr_netaddr_is_loopback(const pr_netaddr_t *na) {
  if (na == NULL) {
    errno = EINVAL;
    return -1;
  }

  switch (pr_netaddr_get_family(na)) {
    case AF_INET:
      return IN_IS_ADDR_LOOPBACK(
        (struct in_addr *) pr_netaddr_get_inaddr(na));

#ifdef PR_USE_IPV6
    case AF_INET6:
      if (pr_netaddr_is_v4mappedv6(na) == TRUE) {
        pool *tmp_pool;
        pr_netaddr_t *v4na;
        int res;

        tmp_pool = make_sub_pool(permanent_pool);
        v4na = pr_netaddr_v6tov4(tmp_pool, na);

        res = pr_netaddr_is_loopback(v4na);
        destroy_pool(tmp_pool);

        return res;
      }

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
uint32_t pr_netaddr_get_addrno(const pr_netaddr_t *na) {
  if (na == NULL) {
    errno = EINVAL;
    return 0;
  }

  switch (pr_netaddr_get_family(na)) {
    case AF_INET:
      return (uint32_t) na->na_addr.v4.sin_addr.s_addr;

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
      errno = ENOENT;
      return 0;
#endif
    }
#endif /* PR_USE_IPV6 */
  }

  errno = EPERM;
  return 0;
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
    case AF_INET6: {
      int res;

      if (!use_ipv6) {
        errno = EINVAL;
        return -1;
      }

# ifndef LINUX
      res = IN6_IS_ADDR_V4MAPPED(
        (struct in6_addr *) pr_netaddr_get_inaddr(na));
# else
      res = IN6_IS_ADDR_V4MAPPED(
        ((struct in6_addr *) pr_netaddr_get_inaddr(na))->s6_addr32);
# endif
      return res;
    }
#endif /* PR_USE_IPV6 */
  }

  errno = EPERM;
  return -1;
}

pr_netaddr_t *pr_netaddr_v6tov4(pool *p, const pr_netaddr_t *na) {
  pr_netaddr_t *res;

  if (p == NULL ||
      na == NULL) {
    errno = EINVAL;
    return NULL;
  }

  if (pr_netaddr_is_v4mappedv6(na) != TRUE) {
    errno = EPERM;
    return NULL;
  }

  res = pr_netaddr_alloc(p);
  pr_netaddr_set_family(res, AF_INET);
  pr_netaddr_set_port(res, pr_netaddr_get_port(na));
  memcpy(&res->na_addr.v4.sin_addr, get_v4inaddr(na), sizeof(struct in_addr));

  return res;
}

pr_netaddr_t *pr_netaddr_get_sess_local_addr(void) {
  if (have_sess_local_addr) {
    return &sess_local_addr;
  }

  errno = ENOENT;
  return NULL;
}

pr_netaddr_t *pr_netaddr_get_sess_remote_addr(void) {
  if (have_sess_remote_addr) {
    return &sess_remote_addr;
  }

  errno = ENOENT;
  return NULL;
}

const char *pr_netaddr_get_sess_remote_name(void) {
  if (have_sess_remote_addr) {
    return sess_remote_name;
  }

  errno = ENOENT;
  return NULL;
}

void pr_netaddr_set_sess_addrs(void) {
  pr_netaddr_set_family(&sess_local_addr,
    pr_netaddr_get_family(session.c->local_addr));
  pr_netaddr_set_sockaddr(&sess_local_addr,
    pr_netaddr_get_sockaddr(session.c->local_addr));
  have_sess_local_addr = TRUE;

  pr_netaddr_set_family(&sess_remote_addr,
    pr_netaddr_get_family(session.c->remote_addr));
  pr_netaddr_set_sockaddr(&sess_remote_addr,
    pr_netaddr_get_sockaddr(session.c->remote_addr));

  memset(sess_remote_name, '\0', sizeof(sess_remote_name));
  sstrncpy(sess_remote_name, session.c->remote_name, sizeof(sess_remote_name));
  have_sess_remote_addr = TRUE;
}

unsigned char pr_netaddr_use_ipv6(void) {
  if (use_ipv6)
    return TRUE;

  return FALSE;
}

void pr_netaddr_disable_ipv6(void) {
#ifdef PR_USE_IPV6
  use_ipv6 = 0;
#endif /* PR_USE_IPV6 */
}

void pr_netaddr_enable_ipv6(void) {
#ifdef PR_USE_IPV6
  use_ipv6 = 1;
#endif /* PR_USE_IPV6 */
}

void pr_netaddr_clear_cache(void) {
  if (netaddr_iptab) {
    pr_trace_msg(trace_channel, 5, "emptying netaddr IP cache");
    (void) pr_table_empty(netaddr_iptab);
    (void) pr_table_free(netaddr_iptab);

    /* Allocate a fresh table. */
    netaddr_iptab = pr_table_alloc(netaddr_pool, 0);
  }

  if (netaddr_dnstab) {
    pr_trace_msg(trace_channel, 5, "emptying netaddr DNS cache");
    (void) pr_table_empty(netaddr_dnstab);
    (void) pr_table_free(netaddr_dnstab);

    /* Allocate a fresh table. */
    netaddr_dnstab = pr_table_alloc(netaddr_pool, 0);
  }
}

void init_netaddr(void) {
  if (netaddr_pool) {
    pr_netaddr_clear_cache();
    destroy_pool(netaddr_pool);
    netaddr_pool = NULL;
  }

  netaddr_pool = make_sub_pool(permanent_pool);
  pr_pool_tag(netaddr_pool, "Netaddr API");

  netaddr_iptab = pr_table_alloc(netaddr_pool, 0);
  netaddr_dnstab = pr_table_alloc(netaddr_pool, 0);
}
