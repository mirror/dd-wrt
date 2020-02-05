/*
  chronyd/chronyc - Programs for keeping computer clocks accurate.

 **********************************************************************
 * Copyright (C) Richard P. Curnow  1997-2003
 * Copyright (C) Miroslav Lichvar  2009-2011
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 * 
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 * 
 **********************************************************************

  =======================================================================

  Functions to do name to IP address conversion

  */

#include "config.h"

#include "sysincl.h"

#include <netdb.h>
#include <resolv.h>

#include "nameserv.h"
#include "util.h"

/* ================================================== */

static int address_family = IPADDR_UNSPEC;

void
DNS_SetAddressFamily(int family)
{
  address_family = family;
}

DNS_Status 
DNS_Name2IPAddress(const char *name, IPAddr *ip_addrs, int max_addrs)
{
#ifdef HAVE_GETADDRINFO
  struct addrinfo hints, *res, *ai;
  int i, result;

  max_addrs = MIN(max_addrs, DNS_MAX_ADDRESSES);

  memset(&hints, 0, sizeof (hints));

  switch (address_family) {
    case IPADDR_INET4:
      hints.ai_family = AF_INET;
      break;
#ifdef FEAT_IPV6
    case IPADDR_INET6:
      hints.ai_family = AF_INET6;
      break;
#endif
    default:
      hints.ai_family = AF_UNSPEC;
  }
  hints.ai_socktype = SOCK_STREAM;

  result = getaddrinfo(name, NULL, &hints, &res);

  if (result) {
#ifdef FORCE_DNSRETRY
    return DNS_TryAgain;
#else
    return result == EAI_AGAIN ? DNS_TryAgain : DNS_Failure;
#endif
  }

  for (ai = res, i = 0; i < max_addrs && ai != NULL; ai = ai->ai_next) {
    switch (ai->ai_family) {
      case AF_INET:
        if (address_family != IPADDR_UNSPEC && address_family != IPADDR_INET4)
          continue;
        ip_addrs[i].family = IPADDR_INET4;
        ip_addrs[i].addr.in4 = ntohl(((struct sockaddr_in *)ai->ai_addr)->sin_addr.s_addr);
        i++;
        break;
#ifdef FEAT_IPV6
      case AF_INET6:
        if (address_family != IPADDR_UNSPEC && address_family != IPADDR_INET6)
          continue;
        ip_addrs[i].family = IPADDR_INET6;
        memcpy(&ip_addrs[i].addr.in6, &((struct sockaddr_in6 *)ai->ai_addr)->sin6_addr.s6_addr,
               sizeof (ip_addrs->addr.in6));
        i++;
        break;
#endif
    }
  }

  for (; i < max_addrs; i++)
        ip_addrs[i].family = IPADDR_UNSPEC;

  freeaddrinfo(res);

  return !max_addrs || ip_addrs[0].family != IPADDR_UNSPEC ? DNS_Success : DNS_Failure;
#else
  struct hostent *host;
  int i;
  
  if (address_family != IPADDR_UNSPEC && address_family != IPADDR_INET4)
    return DNS_Failure;

  max_addrs = MIN(max_addrs, DNS_MAX_ADDRESSES);

  host = gethostbyname(name);

  if (host == NULL) {
    if (h_errno == TRY_AGAIN)
      return DNS_TryAgain;
  } else {
    if (host->h_addrtype != AF_INET || !host->h_addr_list[0])
      return DNS_Failure;

    for (i = 0; host->h_addr_list[i] && i < max_addrs; i++) {
      ip_addrs[i].family = IPADDR_INET4;
      ip_addrs[i].addr.in4 = ntohl(*(uint32_t *)host->h_addr_list[i]);
    }

    for (; i < max_addrs; i++)
      ip_addrs[i].family = IPADDR_UNSPEC;

    return DNS_Success;
  }

#ifdef FORCE_DNSRETRY
  return DNS_TryAgain;
#else
  return DNS_Failure;
#endif

#endif
}

/* ================================================== */

int
DNS_IPAddress2Name(IPAddr *ip_addr, char *name, int len)
{
  char *result = NULL;

#ifdef FEAT_IPV6
  struct sockaddr_in6 in6;
  socklen_t slen;
  char hbuf[NI_MAXHOST];

  slen = UTI_IPAndPortToSockaddr(ip_addr, 0, (struct sockaddr *)&in6);
  if (!getnameinfo((struct sockaddr *)&in6, slen, hbuf, sizeof (hbuf), NULL, 0, 0))
    result = hbuf;
#else
  struct hostent *host;
  uint32_t addr;

  switch (ip_addr->family) {
    case IPADDR_INET4:
      addr = htonl(ip_addr->addr.in4);
      host = gethostbyaddr((const char *) &addr, sizeof (ip_addr), AF_INET);
      break;
#ifdef FEAT_IPV6
    case IPADDR_INET6:
      host = gethostbyaddr((const void *) ip_addr->addr.in6, sizeof (ip_addr->addr.in6), AF_INET6);
      break;
#endif
    default:
      host = NULL;
  }
  if (host)
    result = host->h_name;
#endif

  if (result == NULL)
    result = UTI_IPToString(ip_addr);
  if (snprintf(name, len, "%s", result) >= len)
    return 0;

  return 1;
}

/* ================================================== */

void
DNS_Reload(void)
{
  res_init();
}

/* ================================================== */

