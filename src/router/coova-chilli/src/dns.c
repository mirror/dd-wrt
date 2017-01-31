/* -*- mode: c; c-basic-offset: 2 -*- */
/*
 * Copyright (C) 2007-2012 David Bird (Coova Technologies) <support@coova.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "chilli.h"

#define antidnstunnel _options.dnsparanoia

extern struct dhcp_t *dhcp;

ssize_t
dns_fullname(char *data, size_t dlen,      /* buffer to store name */
	     uint8_t *res, size_t reslen,  /* current resource */
	     uint8_t *opkt, size_t olen,   /* original packet */
	     int lvl) {
  int ret = 0;
  char *d = data;
  unsigned char l;

  if (lvl >= 15) return -1;

#if(_debug_ > 1)
  if (_options.debug)
    syslog(LOG_DEBUG, "%s(%d): %s dlen=%zd reslen=%zd olen=%zd lvl=%d", __FUNCTION__, __LINE__,
           __FUNCTION__, dlen, reslen, olen, lvl);
#endif

  /* only capture the first name in query */
  if (d && d[0]) d = 0;

  while (reslen-- > 0 && ++ret && (l = *res++) != 0) {

    if ((l & 0xC0) == 0xC0) {
      if (reslen == 0) return -1;
      else {
	unsigned short offset = ((l & ~0xC0) << 8) + *res;

	ret++;

	if (offset > olen) {
          if (_options.debug)
            syslog(LOG_DEBUG, "%s(%d): bad value", __FUNCTION__, __LINE__);
	  return -1;
	}

#if(_debug_ > 1)
        if (_options.debug)
          syslog(LOG_DEBUG, "%s(%d): skip[%d] olen=%zd", __FUNCTION__, __LINE__, offset, olen);
#endif

	if (dns_fullname(d, dlen,
			 opkt + (size_t) offset,
			 olen - (size_t) offset,
			 opkt, olen, lvl+1) < 0)
	  return -1;
	break;
      }
    }

    if (l >= dlen || l >= olen) {
      if (_options.debug)
        syslog(LOG_DEBUG, "%s(%d): bad value %d/%zu/%zu", __FUNCTION__, __LINE__, l, dlen, olen);
      return -1;
    }

#if(_debug_ > 1)
    if (_options.debug)
      syslog(LOG_DEBUG, "%s(%d): part[%.*s] reslen=%zd l=%d dlen=%zd", __FUNCTION__, __LINE__, 
             l, res, reslen, l, dlen);
#endif

    if (d) {
      memcpy(d, res, l);
      d += l;
      dlen -= l;
    }
    res += l;
    reslen -= l;
    ret += l;

    if (d) {
      *d = '.';
      d += 1;
      dlen -= 1;
    }
  }

  if (lvl == 0 && d) {
    int len = strlen((char *)data);
    if (len && len == (d - data) && data[len-1] == '.')
      data[len-1]=0;
  }

  return ret;
}

static void
add_A_to_garden(uint8_t *p) {
  struct in_addr reqaddr;
  pass_through pt;
  memcpy(&reqaddr.s_addr, p, 4);
  memset(&pt, 0, sizeof(pass_through));
  pt.mask.s_addr = 0xffffffff;
  pt.host = reqaddr;
  if (pass_through_add(dhcp->pass_throughs,
		       MAX_PASS_THROUGHS,
		       &dhcp->num_pass_throughs,
		       &pt, 1
#ifdef HAVE_PATRICIA
		       , dhcp->ptree_dyn
#endif
		       ))
    ;
}

int
dns_copy_res(struct dhcp_conn_t *conn, int q,
	     uint8_t **pktp, size_t *left,
	     uint8_t *opkt,  size_t olen,
	     uint8_t *question, size_t qsize,
	     int isReq, int *qmatch, int *modified, int mode) {

#define return_error {                                                  \
    if (_options.debug) syslog(LOG_DEBUG, "%s(%d): failed parsing DNS packet", __FUNCTION__, __LINE__); return -1; }

  uint8_t *p_pkt = *pktp;
  size_t len = *left;

  uint8_t name[PKT_IP_PLEN];
  ssize_t namelen = 0;
  char required = 0;

  uint16_t type;
  #if(_debug_)
  uint16_t class;
  #endif
  uint32_t ttl;
  uint16_t rdlen;

#ifdef ENABLE_IPV6
  uint8_t *pkt_type=0;
#endif
  uint8_t *pkt_ttl=0;

  uint32_t ul;
  uint16_t us;

#if(_debug_ > 1)
  if (_options.debug)
    syslog(LOG_DEBUG, "%s(%d): left=%zd olen=%zd qsize=%zd",
           __FUNCTION__, __LINE__, *left, olen, qsize);
#endif

  memset(name, 0, sizeof(name));
  namelen = dns_fullname((char*)name, sizeof(name)-1,
			 p_pkt, len, opkt, olen, 0);

  if (namelen < 0 || namelen > len) return_error;

  p_pkt += namelen;
  len -= namelen;

  if (antidnstunnel && namelen > 128) {
    syslog(LOG_WARNING,"dropping dns for anti-dnstunnel (namelen: %zd)", namelen);
    return -1;
  }

  if (len < 4) return_error;
#ifdef ENABLE_IPV6
  pkt_type = p_pkt;
#endif
  memcpy(&us, p_pkt, sizeof(us));
  type = ntohs(us);
  p_pkt += 2;
  len -= 2;

  memcpy(&us, p_pkt, sizeof(us));
  #if(_debug_)
  class = ntohs(us);
  #endif
  p_pkt += 2;
  len -= 2;

#if(_debug_)
  if (_options.debug)
    syslog(LOG_DEBUG, "%s(%d): It was a dns record type: %d class: %d", __FUNCTION__, __LINE__, type, class);
#endif

  if (q) {
    if (dns_fullname((char *)question, qsize, *pktp, *left, opkt, olen, 0) < 0)
      return_error;

    if (_options.debug)
      syslog(LOG_DEBUG, "%s(%d): DNS: %s", __FUNCTION__, __LINE__, question);

    *pktp = p_pkt;
    *left = len;

    if (!isReq && *qmatch == -1 && _options.uamdomains[0]) {
      int id;

      for (id=0; id < MAX_UAM_DOMAINS && _options.uamdomains[id]; id++) {

	size_t qst_len = strlen((char *)question);
	size_t dom_len = strlen(_options.uamdomains[id]);

#if(_debug_)
        if (_options.debug)
          syslog(LOG_DEBUG, "%s(%d): checking %s [%s]", __FUNCTION__, __LINE__,
                 _options.uamdomains[id], question);
#endif

	if ( qst_len && dom_len &&
	     (
                 /*
                  *  Match if question equals the uamdomain
                  */
                 ( qst_len == dom_len &&
                   !strcmp(_options.uamdomains[id], (char *)question) ) ||
                 /*
                  *  Match if the question is longer than uamdomain,
                  *  and ends with the '.' followed by uamdomain
                  */
                 ( qst_len > dom_len &&
                   (_options.uamdomains[id][0] == '.' ||
                    question[qst_len - dom_len - 1] == '.') &&
                   !strcmp(_options.uamdomains[id],
                           (char *)question + qst_len - dom_len) )
	      ) ) {
#if(_debug_)
          if (_options.debug)
            syslog(LOG_DEBUG, "%s(%d): matched %s [%s]", __FUNCTION__, __LINE__, _options.uamdomains[id], question);
#endif
	  *qmatch = 1;
	  break;
	}
      }
    }

#ifdef ENABLE_UAMDOMAINFILE
    if (!isReq && *qmatch == -1 && _options.uamdomainfile) {
      *qmatch = garden_check_domainfile((char *) question);
    }
#endif

#ifdef ENABLE_IPV6
    if (_options.ipv6) {
      if (isReq && type == 28) {
        if (_options.debug)
          syslog(LOG_DEBUG, "%s(%d): changing AAAA to A request", __FUNCTION__, __LINE__);
	us = 1;
	us = htons(us);
	memcpy(pkt_type, &us, sizeof(us));
	*modified = 1;
      } else if (!isReq && type == 1) {
        if (_options.debug)
          syslog(LOG_DEBUG, "%s(%d): changing A to AAAA response", __FUNCTION__, __LINE__);
	us = 28;
	us = htons(us);
	memcpy(pkt_type, &us, sizeof(us));
	*modified = 1;
      }
    }
#endif

    return 0;
  }

  if (len < 6) return_error;

  pkt_ttl = p_pkt;
  memcpy(&ul, p_pkt, sizeof(ul));
  ttl = ntohl(ul);
  p_pkt += 4;
  len -= 4;

  memcpy(&us, p_pkt, sizeof(us));
  rdlen = ntohs(us);
  p_pkt += 2;
  len -= 2;

#if(_debug_ > 1)
  if (_options.debug)
    syslog(LOG_DEBUG, "%s(%d): -> w ttl: %d rdlength: %d/%zd", __FUNCTION__, __LINE__, ttl, rdlen, len);
#endif

  if (*qmatch == 1 && ttl > _options.uamdomain_ttl) {
#if(_debug_)
    if (_options.debug)
      syslog(LOG_DEBUG, "%s(%d): Rewriting DNS ttl from %d to %d", __FUNCTION__, __LINE__,
             (int) ttl, _options.uamdomain_ttl);
#endif
    ul = _options.uamdomain_ttl;
    ul = htonl(ul);
    memcpy(pkt_ttl, &ul, sizeof(ul));
    *modified = 1;
  }

  if (len < rdlen) return_error;

  /*
   *  dns records
   */

  switch (type) {

    default:
      if (_options.debug)
        syslog(LOG_DEBUG, "%s(%d): Record type %d", __FUNCTION__, __LINE__, type);
      return_error;
      break;

    case 1:
#if(_debug_ > 1)
      if (_options.debug)
        syslog(LOG_DEBUG, "%s(%d): A record", __FUNCTION__, __LINE__);
#endif
      required = 1;

#ifdef ENABLE_MDNS
      if (mode == DNS_MDNS_MODE) {
        size_t offset;
        for (offset=0; offset < rdlen; offset += 4) {
          struct in_addr reqaddr;
          memcpy(&reqaddr.s_addr, p_pkt+offset, 4);
#if(_debug_)
          if (_options.debug)
            syslog(LOG_DEBUG, "%s(%d): mDNS %s = %s", __FUNCTION__, __LINE__, name, inet_ntoa(reqaddr));
#endif
        }
        break;
      }
#endif

      if (*qmatch == 1) {
        size_t offset;
        for (offset=0; offset < rdlen; offset += 4) {
          add_A_to_garden(p_pkt+offset);
        }
      }
      break;

    case 2: 
      if (_options.debug)
        syslog(LOG_DEBUG, "%s(%d): NS record", __FUNCTION__, __LINE__);
      required = 1;
      break;
    case 5: 
      if (_options.debug)
        syslog(LOG_DEBUG, "%s(%d): CNAME record %s", __FUNCTION__, __LINE__, name);
      required = 1;
      break;
    case 6: 
      if (_options.debug)
        syslog(LOG_DEBUG, "%s(%d): SOA record", __FUNCTION__, __LINE__);
      break;

    case 12:
      if (_options.debug)
        syslog(LOG_DEBUG, "%s(%d): PTR record", __FUNCTION__, __LINE__);
      break;
    case 15:
      if (_options.debug)
        syslog(LOG_DEBUG, "%s(%d): MX record", __FUNCTION__, __LINE__);
      required = 1;
      break;

    case 16:/* TXT */
      if (_options.debug)
        syslog(LOG_DEBUG, "%s(%d): TXT record %d", __FUNCTION__, __LINE__, rdlen);
      if (_options.debug) {
        char *txt = (char *)p_pkt;
        int txtlen = rdlen;
        while (txtlen-- > 0) {
          uint8_t l = *txt++;
          if (l == 0) break;
          if (_options.debug)
            syslog(LOG_DEBUG, "%s(%d): Text: %.*s", __FUNCTION__, __LINE__, (int) l, txt);
          txt += l;
          txtlen -= l;
        }
      }
      break;

    case 28:
      if (_options.debug)
        syslog(LOG_DEBUG, "%s(%d): AAAA record", __FUNCTION__, __LINE__);
      required = 1;
      break;
    case 29:
      if (_options.debug)
        syslog(LOG_DEBUG, "%s(%d): LOC record", __FUNCTION__, __LINE__);
      break;
    case 33:
      if (_options.debug)
        syslog(LOG_DEBUG, "%s(%d): SRV record", __FUNCTION__, __LINE__);
      break;
    case 41:
      if (_options.debug)
        syslog(LOG_DEBUG, "%s(%d): EDNS OPT pseudorecord", __FUNCTION__, __LINE__);
      break;
    case 47:
      if (_options.debug)
        syslog(LOG_DEBUG, "%s(%d): NSEC record", __FUNCTION__, __LINE__);
      break;
  }

  if (antidnstunnel && !required) {
    syslog(LOG_WARNING, "dropping dns for anti-dnstunnel (type %d: length %d)",
           type, rdlen);
    return -1;
  }

  p_pkt += rdlen;
  len -= rdlen;

  *pktp = p_pkt;
  *left = len;

  return 0;
}
