/*
 * ProFTPD - FTP server daemon
 * Copyright (c) 2003 The ProFTPD Project team
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
 * As a special exemption, the ProFTPD Project team and other respective
 * copyright holders give permission to link this program with OpenSSL, and
 * distribute the resulting executable, without including the source code for
 * OpenSSL in the source distribution.
 */

/* Network ACL routines
 * $Id: netacl.c,v 1.1 2006/04/24 11:39:28 honor Exp $
 */

#include "conf.h"

extern int ServerUseReverseDNS;

struct pr_netacl_t {
  pr_netacl_type_t type;

  char *pattern;
  int negated;
  pr_netaddr_t *addr;
  unsigned int masklen;
};

pr_netacl_type_t pr_netacl_get_type(pr_netacl_t *acl) {
  return acl->type;
}

int pr_netacl_match(pr_netacl_t *acl, pr_netaddr_t *addr) {

  if (!acl || !addr)
    return -2;

  switch (acl->type) {
    case PR_NETACL_TYPE_ALL:
      pr_log_debug(DEBUG10, "NetACL: matched 'ALL'");
      return 1;

    case PR_NETACL_TYPE_NONE:
      pr_log_debug(DEBUG10, "NetACL: matched 'NONE'");
      return -1;

    case PR_NETACL_TYPE_IPMASK:
      if (pr_netaddr_ncmp(addr, acl->addr, acl->masklen) == 0) {
        pr_log_debug(DEBUG10, "NetACL: matched IP/mask");
        return 1;
      }
      break;

    case PR_NETACL_TYPE_IPMATCH:
      if (pr_netaddr_cmp(addr, acl->addr) == 0) {
        pr_log_debug(DEBUG10, "NetACL: matched IP address");
        return 1;
      }
      break;
 
    case PR_NETACL_TYPE_DNSMATCH:
      if (strcmp(pr_netaddr_get_dnsstr(addr), acl->pattern) == 0) {
        pr_log_debug(DEBUG10, "NetACL: matched DNS name");
        return 1;
      }
      break;

    case PR_NETACL_TYPE_IPGLOB:
      if (pr_netaddr_fnmatch(addr, acl->pattern,
          PR_NETADDR_MATCH_IP) == TRUE) {
        pr_log_debug(DEBUG10, "NetACL: matched IP glob pattern");
        return 1;
      }
      break;

    case PR_NETACL_TYPE_DNSGLOB:
      if (ServerUseReverseDNS &&
          pr_netaddr_fnmatch(addr, acl->pattern,
            PR_NETADDR_MATCH_DNS) == TRUE) {
        pr_log_debug(DEBUG10, "NetACL: matched DNS glob pattern");
        return 1;
      }
      break;

    default:
     break; 
  }

  return 0;
}

pr_netacl_t *pr_netacl_create(pool *p, char *aclstr) {
  pr_netacl_t *acl;
  char *cp;

  if (!p || !aclstr) {
    errno = EINVAL;
    return NULL;
  }

  /* Parse the given rule into a netacl object. */
  acl = pcalloc(p, sizeof(pr_netacl_t));

  if (strcasecmp(aclstr, "all") == 0) {
    acl->type = PR_NETACL_TYPE_ALL;

  } else if (strcasecmp(aclstr, "none") == 0) {
    acl->type = PR_NETACL_TYPE_NONE;

  } else if ((cp = strchr(aclstr, '/')) != NULL) {
    char *tmp;

    acl->type = PR_NETACL_TYPE_IPMASK;
    *cp = '\0';

    /* Check if the given rule is negated. */
    if (*aclstr == '!') {
      acl->negated = TRUE;
      aclstr++;
    }

    /* We have some type of IP/mask, either IPv4 or IPv6.  We know that colons
     * will only appear in IPv6 addresses, so...
     */

    if (strspn(aclstr, "0123456789ABCDEFabcdef.:") != strlen(aclstr)) {
      errno = EINVAL;
      return NULL;
    }

    acl->addr = pr_netaddr_get_addr(p, aclstr, NULL);
    if (!acl->addr)
      return NULL;

    /* Determine what the given bitmask is. */
    acl->masklen = strtol(cp + 1, &tmp, 10);

    if (tmp && *tmp) {
      /* Invalid bitmask syntax. */
      errno = EINVAL;
      return NULL;
    }

    *cp = '/';

    /* Make sure the given mask length is appropriate for the address. */
    switch (pr_netaddr_get_family(acl->addr)) {
      case AF_INET: {
        /* Make sure that the given number of bits is not more than supported
         * for IPv4 addresses (32).
         */
        if (acl->masklen > 32) {
          errno = EINVAL;
          return NULL;
        }

        break;
      }

#ifdef PR_USE_IPV6
      case AF_INET6: {
        /* Make sure that the given number of bits is not more than supported
         * for IPv6 addresses (128).
         */
        if (acl->masklen > 128) {
          errno = EINVAL;
          return NULL;
        }

        break;
      }
#endif /* PR_USE_IPV6 */

      default:
        break;
    }
    
  } else if (strspn(aclstr, "0123456789ABCDEFabcdef.:") != strlen(aclstr)) {

    /* Check if the given rule is negated. */
    if (*aclstr == '!') {
      acl->negated = TRUE;
      aclstr++;
    }

    /* If there are any glob characters (e.g. '{', '[', '*', or '?'), or if the
     * first character is a '.', then treat the rule as a glob.
     */
    if (strpbrk(aclstr, "{[*?")) {
      acl->type = PR_NETACL_TYPE_DNSGLOB;
      acl->pattern = pstrdup(p, aclstr);

    } else if (*aclstr == '.') {
      acl->type = PR_NETACL_TYPE_DNSGLOB;
      acl->pattern = pstrcat(p, "*", aclstr, NULL);

    } else {
      acl->type = PR_NETACL_TYPE_DNSMATCH;
      acl->pattern = pstrdup(p, aclstr);
    }

  } else {

    /* Check if the given rule is negated. */
    if (*aclstr == '!') {
      acl->negated = TRUE;
      aclstr++;
    }

    /* If the last character is a '.', then treat the rule as a glob. */
    if (aclstr[strlen(aclstr)-1] == '.') {
      acl->type = PR_NETACL_TYPE_IPGLOB;
      acl->pattern = pstrcat(p, aclstr, "*", NULL);

    } else {
      acl->type = PR_NETACL_TYPE_IPMATCH;
      acl->addr = pr_netaddr_get_addr(p, aclstr, NULL);

      if (!acl->addr) 
        return NULL;
    }
  }

  return acl;
}

pr_netacl_t *pr_netacl_dup(pool *p, pr_netacl_t *acl) {
  pr_netacl_t *acl2;

  if (!p || !acl) {
    errno = EINVAL;
    return NULL;
  }

  acl2 = pcalloc(p, sizeof(pr_netacl_t));

  /* A simple memcpy(3) won't suffice; we need a deep copy. */
  acl2->type = acl->type;

  if (acl->pattern)
    acl2->pattern = pstrdup(p, acl->pattern);

  acl2->negated = acl->negated;

  if (acl->addr) {
    acl2->addr = pr_netaddr_alloc(p);

    pr_netaddr_set_family(acl2->addr, pr_netaddr_get_family(acl->addr));
    pr_netaddr_set_sockaddr(acl2->addr, pr_netaddr_get_sockaddr(acl->addr));
  }

  acl2->masklen = acl->masklen;

  return acl2;
}

int pr_netacl_get_negated(pr_netacl_t *acl) {
  if (!acl) {
    errno = EINVAL;
    return -1;
  }

  return acl->negated;
}
