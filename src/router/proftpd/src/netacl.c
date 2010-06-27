/*
 * ProFTPD - FTP server daemon
 * Copyright (c) 2003-2010 The ProFTPD Project team
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
 * $Id: netacl.c,v 1.21 2010/02/11 22:35:32 castaglia Exp $
 */

#include "conf.h"

extern int ServerUseReverseDNS;

struct pr_netacl_t {
  pr_netacl_type_t type;
  const char *aclstr;

  char *pattern;
  int negated;
  pr_netaddr_t *addr;
  unsigned int masklen;
};

static const char *trace_channel = "netacl";

pr_netacl_type_t pr_netacl_get_type(pr_netacl_t *acl) {
  return acl->type;
}

/* Returns 1 if there was a positive match, -1 if there was a negative
 * match, -2 if there was an error, and zero if there was no match at all.
 */
int pr_netacl_match(pr_netacl_t *acl, pr_netaddr_t *addr) {
  pool *tmp_pool;

  if (acl == NULL ||
      addr == NULL) {
    errno = EINVAL;
    return -2;
  }

  tmp_pool = make_sub_pool(permanent_pool);

  switch (acl->type) {
    case PR_NETACL_TYPE_ALL:
      pr_trace_msg(trace_channel, 10, "addr '%s' matched rule 'ALL' ('%s')",
        pr_netaddr_get_ipstr(addr), pr_netacl_get_str(tmp_pool, acl));
      destroy_pool(tmp_pool);
      return 1;

    case PR_NETACL_TYPE_NONE:
      pr_trace_msg(trace_channel, 10, "addr '%s' matched rule 'NONE'",
        pr_netaddr_get_ipstr(addr));
      destroy_pool(tmp_pool);
      return -1;

    case PR_NETACL_TYPE_IPMASK:
      pr_trace_msg(trace_channel, 10,
        "checking addr '%s' against IP mask rule '%s'",
        pr_netaddr_get_ipstr(addr), acl->aclstr);

      if (pr_netaddr_ncmp(addr, acl->addr, acl->masklen) == 0) {
        pr_trace_msg(trace_channel, 10, "addr '%s' matched IP mask rule '%s'",
          pr_netaddr_get_ipstr(addr), acl->aclstr);
        destroy_pool(tmp_pool);

        if (acl->negated)
          return -1;

        return 1;

      } else {
        if (acl->negated) {
          destroy_pool(tmp_pool);
          return 1;
        }
      }
      break;

    case PR_NETACL_TYPE_IPMATCH:
      pr_trace_msg(trace_channel, 10,
        "checking addr '%s' against IP address rule '%s'",
        pr_netaddr_get_ipstr(addr), acl->aclstr);

      if (pr_netaddr_cmp(addr, acl->addr) == 0) {
        pr_trace_msg(trace_channel, 10,
          "addr '%s' matched IP address rule '%s'",
          pr_netaddr_get_ipstr(addr), acl->aclstr);
        destroy_pool(tmp_pool);

        if (acl->negated)
          return -1;

        return 1;

      } else {
        if (acl->negated) {
          destroy_pool(tmp_pool);
          return 1;
        }
      }
      break;
 
    case PR_NETACL_TYPE_DNSMATCH:
      pr_trace_msg(trace_channel, 10,
        "checking addr '%s' against DNS name rule '%s'",
        pr_netaddr_get_dnsstr(addr), acl->pattern);

      if (strcmp(pr_netaddr_get_dnsstr(addr), acl->pattern) == 0) {
        pr_trace_msg(trace_channel, 10,
          "addr '%s' (%s) matched DNS name rule '%s'",
          pr_netaddr_get_ipstr(addr), pr_netaddr_get_dnsstr(addr),
          acl->aclstr);
        destroy_pool(tmp_pool);

        if (acl->negated)
          return -1;

        return 1;

      } else {
        if (acl->negated) {
          destroy_pool(tmp_pool);
          return 1;
        }
      }
      break;

    case PR_NETACL_TYPE_IPGLOB:
      pr_trace_msg(trace_channel, 10,
        "checking addr '%s' against IP glob rule '%s'",
        pr_netaddr_get_ipstr(addr), acl->aclstr);

      if (pr_netaddr_fnmatch(addr, acl->pattern,
          PR_NETADDR_MATCH_IP) == TRUE) {
        pr_trace_msg(trace_channel, 10,
          "addr '%s' matched IP glob rule '%s'",
          pr_netaddr_get_ipstr(addr), acl->aclstr);
        destroy_pool(tmp_pool);

        if (acl->negated)
          return -1;

        return 1;

      } else {
        if (acl->negated) {
          destroy_pool(tmp_pool);
          return 1;
        }
      }
      break;

    case PR_NETACL_TYPE_DNSGLOB:
      if (ServerUseReverseDNS) {
        pr_trace_msg(trace_channel, 10,
          "checking addr '%s' against DNS glob rule '%s'",
          pr_netaddr_get_ipstr(addr), acl->pattern);

        if (pr_netaddr_fnmatch(addr, acl->pattern,
            PR_NETADDR_MATCH_DNS) == TRUE) {
          pr_trace_msg(trace_channel, 10,
            "addr '%s' (%s) matched DNS glob rule '%s'",
            pr_netaddr_get_ipstr(addr), pr_netaddr_get_dnsstr(addr),
            acl->aclstr);
          destroy_pool(tmp_pool);

          if (acl->negated)
            return -1;

          return 1;

        } else {
          if (acl->negated) {
            destroy_pool(tmp_pool);
            return 1;
          }
        }

      } else {
        pr_trace_msg(trace_channel, 10,
          "skipping comparing addr '%s' (%s) against DNS glob rule '%s' "
          "because UseReverseDNS is off", pr_netaddr_get_ipstr(addr),
          pr_netaddr_get_dnsstr(addr), acl->aclstr);
      }
      break;
  }

  destroy_pool(tmp_pool);
  return 0;
}

pr_netacl_t *pr_netacl_create(pool *p, char *aclstr) {
  pr_netacl_t *acl;
  char *cp, *aclstr_dup;

  if (p == NULL ||
      aclstr == NULL) {
    errno = EINVAL;
    return NULL;
  }

  if (strlen(aclstr) == 0) {
    errno = EINVAL;
    return NULL;
  }

  /* Parse the given rule into a netacl object. */
  acl = pcalloc(p, sizeof(pr_netacl_t));

  aclstr_dup = pstrdup(p, aclstr);

  if (strcasecmp(aclstr, "all") == 0) {
    aclstr_dup = pstrdup(p, "all");
    acl->type = PR_NETACL_TYPE_ALL;

  } else if (strcasecmp(aclstr, "none") == 0) {
    aclstr_dup = pstrdup(p, "none");
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
        if (pr_netaddr_use_ipv6()) {
          if (acl->masklen > 128) {
            errno = EINVAL;
            return NULL;

          } else if (pr_netaddr_is_v4mappedv6(acl->addr) == TRUE &&
                     acl->masklen > 32) {
            /* The admin may be trying to use IPv6-style masks on IPv4-mapped
             * IPv6 addresses, which of course will not work as expected.
             * If the mask is 32 bits or more, warn the admin.
             */
            pr_log_pri(PR_LOG_WARNING, "warning: possibly using IPv6-style netmask on IPv4-mapped IPv6 address, which will not work as expected");
            pr_trace_msg(trace_channel, 1, "possibly using IPv6-style netmask on IPv4-mapped IPv6 address (%s), which will not work as expected", aclstr);

            break;
          }
        }
      }
#endif /* PR_USE_IPV6 */

      default:
        break;
    }

#ifdef PR_USE_IPV6
  } else if (pr_netaddr_use_ipv6() &&
             strspn(aclstr, "0123456789ABCDEFabcdef.:!") != strlen(aclstr)) {
#else
  } else if (strspn(aclstr, "0123456789.!") != strlen(aclstr)) {
#endif /* PR_USE_IPV6 */

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

  } else if (strchr(aclstr, '.') == NULL) {

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

    /* If the last character is a '.', then treat the rule as an IP glob. */
    if (aclstr[strlen(aclstr)-1] == '.') {
      acl->type = PR_NETACL_TYPE_IPGLOB;
      acl->pattern = pstrcat(p, aclstr, "*", NULL);

    } else {
      register unsigned int i;
      int use_dns = FALSE;

      /* If there are only digits and periods, it's an IP match.  Otherwise,
       * it is a DNS glob (if there are glob characters) or a DNS match.
       */
      for (i = 0; i < strlen(aclstr); i++) {
        if (aclstr[i] == '.') {
          continue;
        }

        if (isdigit((int) aclstr[i]) == 0) {
          /* Not a digit. */
          use_dns = TRUE;
          break;
        }
      }

      if (!use_dns) {
        acl->type = PR_NETACL_TYPE_IPMATCH;
        acl->addr = pr_netaddr_get_addr(p, aclstr, NULL);

        if (!acl->addr) 
           return NULL;

      } else {

        /* If there are any glob characters (e.g. '{', '[', '*', or '?'), or
         * if the first character is a '.', then treat the rule as a glob.
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
      }
    }
  }

  acl->aclstr = aclstr_dup;
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

  if (acl->aclstr)
    acl2->aclstr = pstrdup(p, acl->aclstr);

  return acl2;
}

int pr_netacl_get_negated(pr_netacl_t *acl) {
  if (!acl) {
    errno = EINVAL;
    return -1;
  }

  return acl->negated;
}

const char *pr_netacl_get_str(pool *p, pr_netacl_t *acl) {
  char *res = "";

  if (!p || !acl) {
    errno = EINVAL;
    return NULL;
  }


  switch (acl->type) {
    case PR_NETACL_TYPE_ALL:
      res = pstrcat(p, res, acl->aclstr, NULL);
      res = pstrcat(p, res, " <all>", NULL);
      return res;

    case PR_NETACL_TYPE_NONE:
      res = pstrcat(p, res, acl->aclstr, NULL);
      res = pstrcat(p, res, " <none>", NULL);
      return res;

    case PR_NETACL_TYPE_IPMASK: {
      char masklenstr[64];

      res = pstrcat(p, res, acl->aclstr, NULL);
      memset(masklenstr, '\0', sizeof(masklenstr));
      snprintf(masklenstr, sizeof(masklenstr)-1, "%u", acl->masklen);
      res = pstrcat(p, res, " <IP address mask, ", masklenstr, "-bit mask",
        NULL);
      break;
    }

    case PR_NETACL_TYPE_IPMATCH:
      res = pstrcat(p, res, acl->aclstr, NULL);
      res = pstrcat(p, res, " <IP address match", NULL);
      break;

    case PR_NETACL_TYPE_DNSMATCH:
      res = pstrcat(p, res, acl->aclstr, NULL);
      res = pstrcat(p, res, " <DNS hostname match", NULL);
      break;

    case PR_NETACL_TYPE_IPGLOB:
      res = pstrcat(p, res, acl->pattern, NULL);
      res = pstrcat(p, res, " <IP address glob", NULL);
      break;

    case PR_NETACL_TYPE_DNSGLOB:
      res = pstrcat(p, res, acl->pattern, NULL);
      res = pstrcat(p, res, " <DNS hostname glob", NULL);
      break;
  }

  if (!acl->negated)
    res = pstrcat(p, res, ">", NULL);
  else
    res = pstrcat(p, res, ", inverted>", NULL);

  return res;
}

