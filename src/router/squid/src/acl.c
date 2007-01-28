
/*
 * $Id: acl.c,v 1.318 2007/01/06 17:22:45 hno Exp $
 *
 * DEBUG: section 28    Access Control
 * AUTHOR: Duane Wessels
 *
 * SQUID Web Proxy Cache          http://www.squid-cache.org/
 * ----------------------------------------------------------
 *
 *  Squid is the result of efforts by numerous individuals from
 *  the Internet community; see the CONTRIBUTORS file for full
 *  details.   Many organizations have provided support for Squid's
 *  development; see the SPONSORS file for full details.  Squid is
 *  Copyrighted (C) 2001 by the Regents of the University of
 *  California; see the COPYRIGHT file for full details.  Squid
 *  incorporates software developed and/or copyrighted by other
 *  sources; see the CREDITS file for full details.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *  
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *  
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111, USA.
 *
 */

#include "squid.h"
#include "splay.h"

static void aclParseDomainList(void *curlist);
static void aclParseUserList(void **current);
static void aclParseIpList(void *curlist);
static void aclParseIntlist(void *curlist);
static void aclParseWordList(void *curlist);
static void aclParseProtoList(void *curlist);
static void aclParseMethodList(void *curlist);
static void aclParseTimeSpec(void *curlist);
static void aclParsePortRange(void *curlist);
static void aclDestroyTimeList(acl_time_data * data);
static void aclDestroyIntRange(intrange *);
static void aclLookupProxyAuthStart(aclCheck_t * checklist);
static void aclLookupProxyAuthDone(void *data, char *result);
static struct _acl *aclFindByName(const char *name);
static int aclMatchAcl(struct _acl *, aclCheck_t *);
static int aclMatchTime(acl_time_data * data, time_t when);
static int aclMatchUser(void *proxyauth_acl, char *user);
static int aclMatchIp(void *dataptr, struct in_addr c);
static int aclMatchDomainList(void *dataptr, const char *);
static int aclMatchIntegerRange(intrange * data, int i);
static int aclMatchWordList(wordlist *, const char *);
static void aclParseUserMaxIP(void *data);
static void aclDestroyUserMaxIP(void *data);
static wordlist *aclDumpUserMaxIP(void *data);
static int aclMatchUserMaxIP(void *, auth_user_request_t *, struct in_addr);
static void aclParseHeader(void *data);
static void aclDestroyHeader(void *data);
static squid_acl aclStrToType(const char *s);
static int decode_addr(const char *, struct in_addr *);
static void aclCheck(aclCheck_t * checklist);
static void aclCheckCallback(aclCheck_t * checklist, allow_t answer);
#if USE_IDENT
static IDCB aclLookupIdentDone;
#endif
static IPH aclLookupDstIPDone;
static IPH aclLookupDstIPforASNDone;
static FQDNH aclLookupSrcFQDNDone;
static FQDNH aclLookupDstFQDNDone;
static EAH aclLookupExternalDone;
static wordlist *aclDumpIpList(void *);
static wordlist *aclDumpDomainList(void *data);
static wordlist *aclDumpTimeSpecList(acl_time_data *);
static wordlist *aclDumpRegexList(relist * data);
static wordlist *aclDumpIntlistList(intlist * data);
static wordlist *aclDumpIntRangeList(intrange * data);
static wordlist *aclDumpProtoList(intlist * data);
static wordlist *aclDumpMethodList(intlist * data);
static SPLAYCMP aclIpAddrNetworkCompare;
static SPLAYCMP aclIpNetworkCompare;
static SPLAYCMP aclHostDomainCompare;
static SPLAYCMP aclDomainCompare;
static SPLAYWALKEE aclDumpIpListWalkee;
static SPLAYWALKEE aclDumpDomainListWalkee;
static SPLAYFREE aclFreeIpData;

#if USE_ARP_ACL
static void aclParseArpList(void *curlist);
static int decode_eth(const char *asc, char *eth);
static int aclMatchArp(void *dataptr, struct in_addr c);
static wordlist *aclDumpArpList(void *);
static SPLAYCMP aclArpCompare;
static SPLAYWALKEE aclDumpArpListWalkee;
#endif
#if USE_SSL
static void aclParseCertList(void *curlist);
static int aclMatchUserCert(void *data, aclCheck_t *);
static int aclMatchCACert(void *data, aclCheck_t *);
static wordlist *aclDumpCertList(void *data);
static void aclDestroyCertList(void *data);
#endif

static int aclCacheMatchAcl(dlink_list * cache, squid_acl acltype, void *data, char *MatchParam);

static squid_acl
aclStrToType(const char *s)
{
    if (!strcmp(s, "src"))
	return ACL_SRC_IP;
    if (!strcmp(s, "dst"))
	return ACL_DST_IP;
    if (!strcmp(s, "myip"))
	return ACL_MY_IP;
    if (!strcmp(s, "domain"))
	return ACL_DST_DOMAIN;
    if (!strcmp(s, "dstdomain"))
	return ACL_DST_DOMAIN;
    if (!strcmp(s, "srcdomain"))
	return ACL_SRC_DOMAIN;
    if (!strcmp(s, "dstdom_regex"))
	return ACL_DST_DOM_REGEX;
    if (!strcmp(s, "srcdom_regex"))
	return ACL_SRC_DOM_REGEX;
    if (!strcmp(s, "time"))
	return ACL_TIME;
    if (!strcmp(s, "pattern"))
	return ACL_URLPATH_REGEX;
    if (!strcmp(s, "urlpath_regex"))
	return ACL_URLPATH_REGEX;
    if (!strcmp(s, "url_regex"))
	return ACL_URL_REGEX;
    if (!strcmp(s, "port"))
	return ACL_URL_PORT;
    if (!strcmp(s, "myport"))
	return ACL_MY_PORT;
    if (!strcmp(s, "maxconn"))
	return ACL_MAXCONN;
#if USE_IDENT
    if (!strcmp(s, "ident"))
	return ACL_IDENT;
    if (!strcmp(s, "ident_regex"))
	return ACL_IDENT_REGEX;
#endif
    if (!strncmp(s, "type", 5))
	return ACL_TYPE;
    if (!strncmp(s, "proto", 5))
	return ACL_PROTO;
    if (!strcmp(s, "method"))
	return ACL_METHOD;
    if (!strcmp(s, "browser"))
	return ACL_BROWSER;
    if (!strcmp(s, "referer_regex"))
	return ACL_REFERER_REGEX;
    if (!strcmp(s, "referrer_regex"))
	return ACL_REFERER_REGEX;
    if (!strcmp(s, "proxy_auth"))
	return ACL_PROXY_AUTH;
    if (!strcmp(s, "proxy_auth_regex"))
	return ACL_PROXY_AUTH_REGEX;
    if (!strcmp(s, "src_as"))
	return ACL_SRC_ASN;
    if (!strcmp(s, "dst_as"))
	return ACL_DST_ASN;
#if SQUID_SNMP
    if (!strcmp(s, "snmp_community"))
	return ACL_SNMP_COMMUNITY;
#endif
#if SRC_RTT_NOT_YET_FINISHED
    if (!strcmp(s, "src_rtt"))
	return ACL_NETDB_SRC_RTT;
#endif
#if USE_ARP_ACL
    if (!strcmp(s, "arp"))
	return ACL_SRC_ARP;
#endif
    if (!strcmp(s, "req_mime_type"))
	return ACL_REQ_MIME_TYPE;
    if (!strcmp(s, "rep_mime_type"))
	return ACL_REP_MIME_TYPE;
    if (!strcmp(s, "rep_header"))
	return ACL_REP_HEADER;
    if (!strcmp(s, "req_header"))
	return ACL_REQ_HEADER;
    if (!strcmp(s, "max_user_ip"))
	return ACL_MAX_USER_IP;
    if (!strcmp(s, "external"))
	return ACL_EXTERNAL;
    if (!strcmp(s, "urllogin"))
	return ACL_URLLOGIN;
#if USE_SSL
    if (!strcmp(s, "user_cert"))
	return ACL_USER_CERT;
    if (!strcmp(s, "ca_cert"))
	return ACL_CA_CERT;
#endif
    if (!strcmp(s, "urlgroup"))
	return ACL_URLGROUP;
    if (!strcmp(s, "ext_user"))
	return ACL_EXTUSER;
    if (!strcmp(s, "ext_user_regex"))
	return ACL_EXTUSER_REGEX;
    return ACL_NONE;
}

const char *
aclTypeToStr(squid_acl type)
{
    if (type == ACL_SRC_IP)
	return "src";
    if (type == ACL_DST_IP)
	return "dst";
    if (type == ACL_MY_IP)
	return "myip";
    if (type == ACL_DST_DOMAIN)
	return "dstdomain";
    if (type == ACL_SRC_DOMAIN)
	return "srcdomain";
    if (type == ACL_DST_DOM_REGEX)
	return "dstdom_regex";
    if (type == ACL_SRC_DOM_REGEX)
	return "srcdom_regex";
    if (type == ACL_TIME)
	return "time";
    if (type == ACL_URLPATH_REGEX)
	return "urlpath_regex";
    if (type == ACL_URL_REGEX)
	return "url_regex";
    if (type == ACL_URL_PORT)
	return "port";
    if (type == ACL_MY_PORT)
	return "myport";
    if (type == ACL_MAXCONN)
	return "maxconn";
#if USE_IDENT
    if (type == ACL_IDENT)
	return "ident";
    if (type == ACL_IDENT_REGEX)
	return "ident_regex";
#endif
    if (type == ACL_TYPE)
	return "type";
    if (type == ACL_PROTO)
	return "proto";
    if (type == ACL_METHOD)
	return "method";
    if (type == ACL_BROWSER)
	return "browser";
    if (type == ACL_REFERER_REGEX)
	return "referer_regex";
    if (type == ACL_PROXY_AUTH)
	return "proxy_auth";
    if (type == ACL_PROXY_AUTH_REGEX)
	return "proxy_auth_regex";
    if (type == ACL_SRC_ASN)
	return "src_as";
    if (type == ACL_DST_ASN)
	return "dst_as";
#if SQUID_SNMP
    if (type == ACL_SNMP_COMMUNITY)
	return "snmp_community";
#endif
#if SRC_RTT_NOT_YET_FINISHED
    if (type == ACL_NETDB_SRC_RTT)
	return "src_rtt";
#endif
#if USE_ARP_ACL
    if (type == ACL_SRC_ARP)
	return "arp";
#endif
    if (type == ACL_REQ_MIME_TYPE)
	return "req_mime_type";
    if (type == ACL_REP_MIME_TYPE)
	return "rep_mime_type";
    if (type == ACL_REP_HEADER)
	return "rep_header";
    if (type == ACL_REQ_HEADER)
	return "req_header";
    if (type == ACL_MAX_USER_IP)
	return "max_user_ip";
    if (type == ACL_EXTERNAL)
	return "external";
    if (type == ACL_URLLOGIN)
	return "urllogin";
#if USE_SSL
    if (type == ACL_USER_CERT)
	return "user_cert";
    if (type == ACL_CA_CERT)
	return "ca_cert";
#endif
    if (type == ACL_URLGROUP)
	return "urlgroup";
    if (type == ACL_EXTUSER)
	return "ext_user";
    if (type == ACL_EXTUSER_REGEX)
	return "ext_user_regex";
    return "ERROR";
}

static acl *
aclFindByName(const char *name)
{
    acl *a;
    for (a = Config.aclList; a; a = a->next)
	if (!strcasecmp(a->name, name))
	    return a;
    return NULL;
}

static void
aclParseIntlist(void *curlist)
{
    intlist **Tail;
    intlist *q = NULL;
    char *t = NULL;
    for (Tail = curlist; *Tail; Tail = &((*Tail)->next));
    while ((t = strtokFile())) {
	q = memAllocate(MEM_INTLIST);
	q->i = xatoi(t);
	*(Tail) = q;
	Tail = &q->next;
    }
}

static void
aclParsePortRange(void *curlist)
{
    intrange **Tail;
    char *a;
    for (Tail = curlist; *Tail; Tail = &((*Tail)->next));
    while ((a = strtokFile())) {
	char *b = strchr(a, '-');
	unsigned short port1, port2;
	if (b)
	    *b++ = '\0';
	port1 = xatos(a);
	if (b)
	    port2 = xatos(b);
	else
	    port2 = port1;
	if (port2 >= port1) {
	    intrange *q = xcalloc(1, sizeof(intrange));
	    q->i = port1;
	    q->j = port2;
	    *(Tail) = q;
	    Tail = &q->next;
	} else {
	    debug(28, 0) ("aclParsePortRange: Invalid port value\n");
	    self_destruct();
	}
    }
}

static void
aclParseProtoList(void *curlist)
{
    intlist **Tail;
    intlist *q = NULL;
    char *t = NULL;
    protocol_t protocol;
    for (Tail = curlist; *Tail; Tail = &((*Tail)->next));
    while ((t = strtokFile())) {
	protocol = urlParseProtocol(t);
	q = memAllocate(MEM_INTLIST);
	q->i = (int) protocol;
	*(Tail) = q;
	Tail = &q->next;
    }
}

static void
aclParseMethodList(void *curlist)
{
    intlist **Tail;
    intlist *q = NULL;
    char *t = NULL;
    for (Tail = curlist; *Tail; Tail = &((*Tail)->next));
    while ((t = strtokFile())) {
	q = memAllocate(MEM_INTLIST);
	q->i = (int) urlParseMethod(t);
	if (q->i == METHOD_NONE)
	    self_destruct();
	*(Tail) = q;
	Tail = &q->next;
    }
}

static void
aclParseType(void *current)
{
    acl_request_type **p = current;
    acl_request_type *type;
    char *t = NULL;
    if (!*p)
	*p = memAllocate(MEM_ACL_REQUEST_TYPE);
    type = *p;
    while ((t = strtokFile())) {
	if (strcmp(t, "accelerated") == 0) {
	    type->accelerated = 1;
	    continue;
	}
	if (strcmp(t, "accel") == 0) {
	    type->accelerated = 1;
	    continue;
	}
	if (strcmp(t, "transparent") == 0) {
	    type->transparent = 1;
	    continue;
	}
	if (strcmp(t, "internal") == 0) {
	    type->internal = 1;
	    continue;
	}
	if (strcmp(t, "auth") == 0) {
	    type->internal = 1;
	    continue;
	}
	fatalf("unknown acl type argument '%s'\n", t);
    }
}

/*
 * Decode a ascii representation (asc) of a IP adress, and place
 * adress and netmask information in addr and mask.
 * This function should NOT be called if 'asc' is a hostname!
 */
static int
decode_addr(const char *asc, struct in_addr *addr)
{
    int a1 = 0, a2 = 0, a3 = 0, a4 = 0;

    switch (sscanf(asc, "%d.%d.%d.%d", &a1, &a2, &a3, &a4)) {
    case 4:			/* a dotted quad */
	if (!safe_inet_addr(asc, addr)) {
	    debug(28, 0) ("decode_addr: unsafe IP address: '%s'\n", asc);
	    self_destruct();
	}
	break;
    case 1:			/* a significant bits value for a mask */
	if (a1 >= 0 && a1 < 33) {
	    addr->s_addr = a1 ? htonl(0xfffffffful << (32 - a1)) : 0;
	    break;
	}
    default:
	debug(28, 0) ("decode_addr: Invalid IP address '%s'\n", asc);
	return 0;		/* This is not valid address */
    }

    return 1;
}


#define SCAN_ACL1       "%[0123456789.]-%[0123456789.]/%[0123456789.]"
#define SCAN_ACL2       "%[0123456789.]-%[0123456789.]%c"
#define SCAN_ACL3       "%[0123456789.]/%[0123456789.]"
#define SCAN_ACL4       "%[0123456789.]%c"

static acl_ip_data *
aclParseIpData(const char *t)
{
    LOCAL_ARRAY(char, addr1, 256);
    LOCAL_ARRAY(char, addr2, 256);
    LOCAL_ARRAY(char, mask, 256);
    acl_ip_data *q = memAllocate(MEM_ACL_IP_DATA);
    acl_ip_data *r;
    acl_ip_data **Q;
    struct hostent *hp;
    char **x;
    char c;
    debug(28, 5) ("aclParseIpData: %s\n", t);
    if (!strcasecmp(t, "all")) {
	q->addr1.s_addr = 0;
	q->addr2.s_addr = 0;
	q->mask.s_addr = 0;
	return q;
    }
    q->mask.s_addr = no_addr.s_addr;	/* 255.255.255.255 */
    if (sscanf(t, SCAN_ACL1, addr1, addr2, mask) == 3) {
	(void) 0;
    } else if (sscanf(t, SCAN_ACL2, addr1, addr2, &c) == 2) {
	mask[0] = '\0';
    } else if (sscanf(t, SCAN_ACL3, addr1, mask) == 2) {
	addr2[0] = '\0';
    } else if (sscanf(t, SCAN_ACL4, addr1, &c) == 1) {
	addr2[0] = '\0';
	mask[0] = '\0';
    } else if (sscanf(t, "%[^/]/%s", addr1, mask) == 2) {
	addr2[0] = '\0';
    } else if (sscanf(t, "%s", addr1) == 1) {
	/*
	 * Note, must use plain gethostbyname() here because at startup
	 * ipcache hasn't been initialized
	 */
	if ((hp = gethostbyname(addr1)) == NULL) {
	    debug(28, 0) ("aclParseIpData: Bad host/IP: '%s'\n", t);
	    safe_free(q);
	    return NULL;
	}
	Q = &q;
	for (x = hp->h_addr_list; x != NULL && *x != NULL; x++) {
	    if ((r = *Q) == NULL)
		r = *Q = memAllocate(MEM_ACL_IP_DATA);
	    xmemcpy(&r->addr1.s_addr, *x, sizeof(r->addr1.s_addr));
	    r->addr2.s_addr = 0;
	    r->mask.s_addr = no_addr.s_addr;	/* 255.255.255.255 */
	    Q = &r->next;
	    debug(28, 3) ("%s --> %s\n", addr1, inet_ntoa(r->addr1));
	}
	return q;
    } else {
	debug(28, 0) ("aclParseIpData: Bad host/IP: '%s'\n", t);
	safe_free(q);
	return NULL;
    }
    /* Decode addr1 */
    if (!decode_addr(addr1, &q->addr1)) {
	debug(28, 0) ("%s line %d: %s\n",
	    cfg_filename, config_lineno, config_input_line);
	debug(28, 0) ("aclParseIpData: Ignoring invalid IP acl entry: unknown first address '%s'\n", addr1);
	safe_free(q);
	return NULL;
    }
    /* Decode addr2 */
    if (*addr2 && !decode_addr(addr2, &q->addr2)) {
	debug(28, 0) ("%s line %d: %s\n",
	    cfg_filename, config_lineno, config_input_line);
	debug(28, 0) ("aclParseIpData: Ignoring invalid IP acl entry: unknown second address '%s'\n", addr2);
	safe_free(q);
	return NULL;
    }
    /* Decode mask */
    if (*mask && !decode_addr(mask, &q->mask)) {
	debug(28, 0) ("%s line %d: %s\n",
	    cfg_filename, config_lineno, config_input_line);
	debug(28, 0) ("aclParseIpData: Ignoring invalid IP acl entry: unknown netmask '%s'\n", mask);
	safe_free(q);
	return NULL;
    }
    if ((q->addr1.s_addr & q->mask.s_addr) != q->addr1.s_addr ||
	(q->addr2.s_addr & q->mask.s_addr) != q->addr2.s_addr)
	debug(28, 0) ("aclParseIpData: WARNING: Netmask masks away part of the specified IP in '%s'\n", t);
    q->addr1.s_addr &= q->mask.s_addr;
    q->addr2.s_addr &= q->mask.s_addr;
    /* 1.2.3.4/255.255.255.0  --> 1.2.3.0 */
    return q;
}

/******************/
/* aclParseIpList */
/******************/

static void
aclParseIpList(void *curlist)
{
    char *t = NULL;
    splayNode **Top = curlist;
    acl_ip_data *q = NULL;
    while ((t = strtokFile())) {
	acl_ip_data *next;
	for (q = aclParseIpData(t); q != NULL; q = next) {
	    next = q->next;
	    *Top = splay_insert(q, *Top, aclIpNetworkCompare);
	    if (splayLastResult == 0)
		xfree(q);
	}
    }
}

static void
aclParseTimeSpec(void *curlist)
{
    acl_time_data *q = NULL;
    acl_time_data **Tail;
    int h1, m1, h2, m2;
    char *t = NULL;
    long weekbits = 0;
    for (Tail = curlist; *Tail; Tail = &((*Tail)->next));
    while ((t = strtokFile())) {
	if (*t < '0' || *t > '9') {
	    /* assume its day-of-week spec */
	    while (*t) {
		switch (*t++) {
		case 'S':
		    weekbits |= ACL_SUNDAY;
		    break;
		case 'M':
		    weekbits |= ACL_MONDAY;
		    break;
		case 'T':
		    weekbits |= ACL_TUESDAY;
		    break;
		case 'W':
		    weekbits |= ACL_WEDNESDAY;
		    break;
		case 'H':
		    weekbits |= ACL_THURSDAY;
		    break;
		case 'F':
		    weekbits |= ACL_FRIDAY;
		    break;
		case 'A':
		    weekbits |= ACL_SATURDAY;
		    break;
		case 'D':
		    weekbits |= ACL_WEEKDAYS;
		    break;
		case '-':
		    /* ignore placeholder */
		    break;
		default:
		    debug(28, 0) ("%s line %d: %s\n",
			cfg_filename, config_lineno, config_input_line);
		    debug(28, 0) ("aclParseTimeSpec: Bad Day '%c'\n", t[-1]);
		    self_destruct();
		    break;
		}
	    }
	} else {
	    /* assume its time-of-day spec */
	    if ((sscanf(t, "%d:%d-%d:%d", &h1, &m1, &h2, &m2) < 4) || (!((h1 >= 0 && h1 < 24) && ((h2 >= 0 && h2 < 24) || (h2 == 24 && m2 == 0)) && (m1 >= 0 && m1 < 60) && (m2 >= 0 && m2 < 60)))) {
		debug(28, 0) ("aclParseTimeSpec: ERROR: Bad time range '%s'\n", t);
		self_destruct();
	    }
	    q = memAllocate(MEM_ACL_TIME_DATA);
	    q->start = h1 * 60 + m1;
	    q->stop = h2 * 60 + m2;
	    q->weekbits = weekbits;
	    weekbits = 0;
	    if (q->start > q->stop) {
		debug(28, 0) ("aclParseTimeSpec: ERROR: Reversed time range in"
		    "%s line %d: %s\n",
		    cfg_filename, config_lineno, config_input_line);
		self_destruct();
	    }
	    if (q->weekbits == 0)
		q->weekbits = ACL_ALLWEEK;
	    *(Tail) = q;
	    Tail = &q->next;
	}
    }
    if (weekbits) {
	q = memAllocate(MEM_ACL_TIME_DATA);
	q->start = 0 * 60 + 0;
	q->stop = 24 * 60 + 0;
	q->weekbits = weekbits;
	*(Tail) = q;
	Tail = &q->next;
    }
}

void
aclParseRegexList(void *curlist)
{
    relist **Tail;
    relist *q = NULL;
    char *t = NULL;
    regex_t comp;
    int errcode;
    int flags = REG_EXTENDED | REG_NOSUB;
    for (Tail = curlist; *Tail; Tail = &((*Tail)->next));
    while ((t = strtokFile())) {
	if (strcmp(t, "-i") == 0) {
	    flags |= REG_ICASE;
	    continue;
	}
	if (strcmp(t, "+i") == 0) {
	    flags &= ~REG_ICASE;
	    continue;
	}
	if ((errcode = regcomp(&comp, t, flags)) != 0) {
	    char errbuf[256];
	    regerror(errcode, &comp, errbuf, sizeof errbuf);
	    debug(28, 0) ("%s line %d: %s\n",
		cfg_filename, config_lineno, config_input_line);
	    debug(28, 0) ("aclParseRegexList: Invalid regular expression '%s': %s\n",
		t, errbuf);
	    continue;
	}
	q = memAllocate(MEM_RELIST);
	q->pattern = xstrdup(t);
	q->regex = comp;
	*(Tail) = q;
	Tail = &q->next;
    }
}

static void
aclParseHeader(void *data)
{
    char *t;
    acl_hdr_data **hd = data;
    acl_hdr_data *q;

    t = strtokFile();
    if (NULL == t) {
	debug(28, 0) ("%s line %d: %s\n", cfg_filename, config_lineno, config_input_line);
	debug(28, 0) ("aclParseHeader: No data defined '%s'\n", t);
	return;
    }
    q = xcalloc(1, sizeof(acl_hdr_data));
    q->hdr_name = xstrdup(t);
    q->hdr_id = httpHeaderIdByNameDef(t, strlen(t));
    aclParseRegexList(&q->reglist);
    if (!q->reglist) {
	debug(28, 0) ("%s line %d: %s\n", cfg_filename, config_lineno, config_input_line);
	debug(28, 0) ("aclParseHeader: No pattern defined '%s'\n", t);
	aclDestroyHeader(&q);
	return;
    }
    while (*hd)
	hd = &(*hd)->next;
    *hd = q;
}

static int
aclMatchHeader(acl_hdr_data * hdrs, const HttpHeader * hdr)
{
    acl_hdr_data *hd;
    for (hd = hdrs; hd; hd = hd->next) {
	int ret;
	String header;
	if (hd->hdr_id != -1)
	    header = httpHeaderGetStrOrList(hdr, hd->hdr_id);
	else
	    header = httpHeaderGetByName(hdr, hd->hdr_name);
	if (!strBuf(header))
	    continue;
	ret = aclMatchRegex(hd->reglist, strBuf(header));
	stringClean(&header);
	if (ret)
	    return 1;
    }
    return 0;
}

void
aclDestroyHeader(void *data)
{
    acl_hdr_data **acldata = data;
    while (*acldata) {
	acl_hdr_data *q = *acldata;
	*acldata = q->next;
	if (q->reglist)
	    aclDestroyRegexList(q->reglist);
	safe_free(q);
    }
}

static wordlist *
aclDumpHeader(acl_hdr_data * hd)
{
    wordlist *W = NULL;
    while (hd) {
	MemBuf mb;
	relist *data;
	memBufDefInit(&mb);
	memBufPrintf(&mb, "%s", hd->hdr_name);
	for (data = hd->reglist; data; data = data->next) {
	    memBufPrintf(&mb, " %s", data->pattern);
	}
	wordlistAdd(&W, mb.buf);
	memBufClean(&mb);
	hd = hd->next;
    }
    return W;
}

static void
aclParseWordList(void *curlist)
{
    char *t = NULL;
    while ((t = strtokFile()))
	wordlistAdd(curlist, t);
}

static void
aclParseUserList(void **current)
{
    char *t = NULL;
    acl_user_data *data;
    splayNode *Top = NULL;

    debug(28, 2) ("aclParseUserList: parsing user list\n");
    if (*current == NULL) {
	debug(28, 3) ("aclParseUserList: current is null. Creating\n");
	*current = memAllocate(MEM_ACL_USER_DATA);
    }
    t = strtokFile();
    if (!t) {
	debug(28, 2) ("aclParseUserList: No data defined\n");
	return;
    }
    debug(28, 5) ("aclParseUserList: First token is %s\n", t);
    data = *current;
    Top = data->names;
    if (strcmp("-i", t) == 0) {
	debug(28, 5) ("aclParseUserList: Going case-insensitive\n");
	data->flags.case_insensitive = 1;
    } else if (strcmp("REQUIRED", t) == 0) {
	debug(28, 5) ("aclParseUserList: REQUIRED-type enabled\n");
	data->flags.required = 1;
    } else {
	if (data->flags.case_insensitive)
	    Tolower(t);
	t = xstrdup(t);
	Top = splay_insert(t, Top, (SPLAYCMP *) strcmp);
	if (splayLastResult == 0)
	    xfree(t);
    }
    debug(28, 3) ("aclParseUserList: Case-insensitive-switch is %d\n",
	data->flags.case_insensitive);
    /* we might inherit from a previous declaration */

    debug(28, 4) ("aclParseUserList: parsing user list\n");
    while ((t = strtokFile())) {
	debug(28, 6) ("aclParseUserList: Got token: %s\n", t);
	if (data->flags.case_insensitive)
	    Tolower(t);
	t = xstrdup(t);
	Top = splay_insert(t, Top, (SPLAYCMP *) strcmp);
	if (splayLastResult == 0)
	    xfree(t);
    }
    data->names = Top;
}


/**********************/
/* aclParseDomainList */
/**********************/

static void
aclParseDomainList(void *curlist)
{
    char *t = NULL;
    splayNode **Top = curlist;
    while ((t = strtokFile())) {
	Tolower(t);
	t = xstrdup(t);
	*Top = splay_insert(t, *Top, aclDomainCompare);
	if (splayLastResult == 0)
	    safe_free(t);
    }
}

#if USE_SSL
static void
aclParseCertList(void *curlist)
{
    acl_cert_data **datap = curlist;
    splayNode **Top;
    char *t;
    char *attribute = strtokFile();
    if (!attribute)
	self_destruct();
    if (*datap) {
	if (strcasecmp((*datap)->attribute, attribute) != 0)
	    self_destruct();
    } else {
	*datap = memAllocate(MEM_ACL_CERT_DATA);
	(*datap)->attribute = xstrdup(attribute);
    }
    Top = &(*datap)->values;
    while ((t = strtokFile())) {
	t = xstrdup(t);
	*Top = splay_insert(t, *Top, aclDomainCompare);
	if (splayLastResult == 0)
	    safe_free(t);
    }
}

static int
aclMatchUserCert(void *data, aclCheck_t * checklist)
{
    acl_cert_data *cert_data = data;
    const char *value;
    SSL *ssl = fd_table[checklist->conn->fd].ssl;

    if (!ssl)
	return 0;
    value = sslGetUserAttribute(ssl, cert_data->attribute);
    if (!value)
	return 0;
    cert_data->values = splay_splay(value, cert_data->values, (SPLAYCMP *) strcmp);
    return !splayLastResult;
}

static int
aclMatchCACert(void *data, aclCheck_t * checklist)
{
    acl_cert_data *cert_data = data;
    const char *value;
    SSL *ssl = fd_table[checklist->conn->fd].ssl;

    if (!ssl)
	return 0;
    value = sslGetCAAttribute(ssl, cert_data->attribute);
    if (!value)
	return 0;
    cert_data->values = splay_splay(value, cert_data->values, (SPLAYCMP *) strcmp);
    return !splayLastResult;
}

static void
aclDestroyCertList(void *curlist)
{
    acl_cert_data **datap = curlist;
    if (!*datap)
	return;
    splay_destroy((*datap)->values, xfree);
    memFree(*datap, MEM_ACL_CERT_DATA);
    *datap = NULL;
}

static void
aclDumpCertListWalkee(void *node_data, void *outlist)
{
    /* outlist is really a wordlist ** */
    wordlistAdd(outlist, node_data);
}

static wordlist *
aclDumpCertList(void *curlist)
{
    acl_cert_data *data = curlist;
    wordlist *wl = NULL;
    wordlistAdd(&wl, data->attribute);
    if (data->values)
	splay_walk(data->values, aclDumpCertListWalkee, &wl);
    return wl;
}
#endif

void
aclParseAclLine(acl ** head)
{
    /* we're already using strtok() to grok the line */
    char *t = NULL;
    acl *A = NULL;
    LOCAL_ARRAY(char, aclname, ACL_NAME_SZ);
    squid_acl acltype;
    int new_acl = 0;

    /* snarf the ACL name */
    if ((t = strtok(NULL, w_space)) == NULL) {
	debug(28, 0) ("aclParseAclLine: missing ACL name.\n");
	self_destruct();
    }
    if (strlen(t) >= ACL_NAME_SZ) {
	debug(28, 0) ("aclParseAclLine: ACL name '%s' too long. Max %d characters allowed\n", t, ACL_NAME_SZ - 1);
	self_destruct();
    }
    xstrncpy(aclname, t, ACL_NAME_SZ);
    /* snarf the ACL type */
    if ((t = strtok(NULL, w_space)) == NULL) {
	debug(28, 0) ("aclParseAclLine: missing ACL type.\n");
	self_destruct();
	return;
    }
    if ((acltype = aclStrToType(t)) == ACL_NONE) {
	debug(28, 0) ("aclParseAclLine: Invalid ACL type '%s'\n", t);
	self_destruct();
	return;
    }
    if ((A = aclFindByName(aclname)) == NULL) {
	debug(28, 3) ("aclParseAclLine: Creating ACL '%s'\n", aclname);
	A = memAllocate(MEM_ACL);
	xstrncpy(A->name, aclname, ACL_NAME_SZ);
	A->type = acltype;
	A->cfgline = xstrdup(config_input_line);
	new_acl = 1;
    } else {
	if (acltype != A->type) {
	    debug(28, 0) ("aclParseAclLine: ACL '%s' already exists with different type.\n", A->name);
	    self_destruct();
	    return;
	}
	debug(28, 3) ("aclParseAclLine: Appending to '%s'\n", aclname);
	new_acl = 0;
    }
    /*
     * Here we set AclMatchedName in case we need to use it in a
     * warning message in aclDomainCompare().
     */
    AclMatchedName = aclname;	/* ugly */
    switch (A->type) {
    case ACL_SRC_IP:
    case ACL_DST_IP:
    case ACL_MY_IP:
	aclParseIpList(&A->data);
	break;
    case ACL_SRC_DOMAIN:
    case ACL_DST_DOMAIN:
	aclParseDomainList(&A->data);
	break;
    case ACL_TIME:
	aclParseTimeSpec(&A->data);
	break;
    case ACL_URL_REGEX:
    case ACL_URLLOGIN:
    case ACL_URLPATH_REGEX:
    case ACL_BROWSER:
    case ACL_REFERER_REGEX:
    case ACL_SRC_DOM_REGEX:
    case ACL_DST_DOM_REGEX:
    case ACL_REQ_MIME_TYPE:
    case ACL_REP_MIME_TYPE:
	aclParseRegexList(&A->data);
	break;
    case ACL_REP_HEADER:
    case ACL_REQ_HEADER:
	aclParseHeader(&A->data);
	break;
    case ACL_SRC_ASN:
    case ACL_MAXCONN:
    case ACL_DST_ASN:
	aclParseIntlist(&A->data);
	break;
    case ACL_MAX_USER_IP:
	aclParseUserMaxIP(&A->data);
	break;
#if SRC_RTT_NOT_YET_FINISHED
    case ACL_NETDB_SRC_RTT:
	aclParseIntlist(&A->data);
	break;
#endif
    case ACL_URL_PORT:
    case ACL_MY_PORT:
	aclParsePortRange(&A->data);
	break;
#if USE_IDENT
    case ACL_IDENT:
	aclParseUserList(&A->data);
	break;
    case ACL_IDENT_REGEX:
	aclParseRegexList(&A->data);
	break;
#endif
    case ACL_TYPE:
	aclParseType(&A->data);
	break;
    case ACL_PROTO:
	aclParseProtoList(&A->data);
	break;
    case ACL_METHOD:
	aclParseMethodList(&A->data);
	break;
    case ACL_PROXY_AUTH:
	if (authenticateSchemeCount() == 0) {
	    debug(28, 0) ("Invalid Proxy Auth ACL '%s' "
		"because no authentication schemes were compiled.\n", A->cfgline);
	    self_destruct();
	} else if (authenticateActiveSchemeCount() == 0) {
	    debug(28, 0) ("Invalid Proxy Auth ACL '%s' "
		"because no authentication schemes are fully configured.\n", A->cfgline);
	    self_destruct();
	} else {
	    aclParseUserList(&A->data);
	}
	break;
    case ACL_PROXY_AUTH_REGEX:
	if (authenticateSchemeCount() == 0) {
	    debug(28, 0) ("Invalid Proxy Auth ACL '%s' "
		"because no authentication schemes were compiled.\n", A->cfgline);
	    self_destruct();
	} else if (authenticateActiveSchemeCount() == 0) {
	    debug(28, 0) ("Invalid Proxy Auth ACL '%s' "
		"because no authentication schemes are fully configured.\n", A->cfgline);
	    self_destruct();
	} else {
	    aclParseRegexList(&A->data);
	}
	break;
#if SQUID_SNMP
    case ACL_SNMP_COMMUNITY:
	aclParseWordList(&A->data);
	break;
#endif
#if USE_ARP_ACL
    case ACL_SRC_ARP:
	aclParseArpList(&A->data);
	break;
#endif
    case ACL_EXTERNAL:
	aclParseExternal(&A->data, A->name);
	break;
    case ACL_URLGROUP:
	aclParseWordList(&A->data);
	break;
#if USE_SSL
    case ACL_USER_CERT:
    case ACL_CA_CERT:
	aclParseCertList(&A->data);
	break;
#endif
    case ACL_EXTUSER:
	aclParseUserList(&A->data);
	break;
    case ACL_EXTUSER_REGEX:
	aclParseRegexList(&A->data);
	break;
    case ACL_NONE:
    case ACL_ENUM_MAX:
	fatal("Bad ACL type");
	break;
    }
    /*
     * Clear AclMatchedName from our temporary hack
     */
    AclMatchedName = NULL;	/* ugly */
    if (!new_acl)
	return;
    if (A->data == NULL) {
	debug(28, 0) ("aclParseAclLine: WARNING: empty ACL: %s\n",
	    A->cfgline);
    }
    /* append */
    while (*head)
	head = &(*head)->next;
    *head = A;
}

/* does name lookup, returns page_id */
err_type
aclGetDenyInfoPage(acl_deny_info_list ** head, const char *name, int redirect_allowed)
{
    acl_deny_info_list *A = NULL;

    for (A = *head; A; A = A->next) {
	acl_name_list *L = NULL;
	if (!redirect_allowed && strchr(A->err_page_name, ':'))
	    continue;
	for (L = A->acl_list; L; L = L->next) {
	    if (!strcmp(name, L->name))
		return A->err_page_id;
	}
    }
    return ERR_NONE;
}

/* does name lookup, returns if it is a proxy_auth acl */
int
aclIsProxyAuth(const char *name)
{
    acl *a;
    if (NULL == name)
	return 0;
    a = aclFindByName(name);
    if (a == NULL)
	return 0;
    switch (a->type) {
    case ACL_PROXY_AUTH:
    case ACL_PROXY_AUTH_REGEX:
	return 1;
    case ACL_EXTERNAL:
	return externalAclRequiresAuth(a->data);
    default:
	return 0;
    }
}


/* maex@space.net (05.09.96)
 *    get the info for redirecting "access denied" to info pages
 *      TODO (probably ;-)
 *      currently there is no optimization for
 *      - more than one deny_info line with the same url
 *      - a check, whether the given acl really is defined
 *      - a check, whether an acl is added more than once for the same url
 */

void
aclParseDenyInfoLine(acl_deny_info_list ** head)
{
    char *t = NULL;
    acl_deny_info_list *A = NULL;
    acl_deny_info_list *B = NULL;
    acl_deny_info_list **T = NULL;
    acl_name_list *L = NULL;
    acl_name_list **Tail = NULL;

    /* first expect a page name */
    if ((t = strtok(NULL, w_space)) == NULL) {
	debug(28, 0) ("%s line %d: %s\n",
	    cfg_filename, config_lineno, config_input_line);
	debug(28, 0) ("aclParseDenyInfoLine: missing 'error page' parameter.\n");
	return;
    }
    A = memAllocate(MEM_ACL_DENY_INFO_LIST);
    A->err_page_id = errorReservePageId(t);
    A->err_page_name = xstrdup(t);
    A->next = (acl_deny_info_list *) NULL;
    /* next expect a list of ACL names */
    Tail = &A->acl_list;
    while ((t = strtok(NULL, w_space))) {
	L = memAllocate(MEM_ACL_NAME_LIST);
	xstrncpy(L->name, t, ACL_NAME_SZ);
	*Tail = L;
	Tail = &L->next;
    }
    if (A->acl_list == NULL) {
	debug(28, 0) ("%s line %d: %s\n",
	    cfg_filename, config_lineno, config_input_line);
	debug(28, 0) ("aclParseDenyInfoLine: deny_info line contains no ACL's, skipping\n");
	memFree(A, MEM_ACL_DENY_INFO_LIST);
	return;
    }
    for (B = *head, T = head; B; T = &B->next, B = B->next);	/* find the tail */
    *T = A;
}

void
aclParseAccessLine(acl_access ** head)
{
    char *t = NULL;
    acl_access *A = NULL;
    acl_access *B = NULL;
    acl_access **T = NULL;

    /* first expect either 'allow' or 'deny' */
    if ((t = strtok(NULL, w_space)) == NULL) {
	debug(28, 0) ("%s line %d: %s\n",
	    cfg_filename, config_lineno, config_input_line);
	debug(28, 0) ("aclParseAccessLine: missing 'allow' or 'deny'.\n");
	return;
    }
    A = cbdataAlloc(acl_access);

    if (!strcmp(t, "allow"))
	A->allow = 1;
    else if (!strcmp(t, "deny"))
	A->allow = 0;
    else {
	debug(28, 0) ("%s line %d: %s\n",
	    cfg_filename, config_lineno, config_input_line);
	debug(28, 0) ("aclParseAccessLine: expecting 'allow' or 'deny', got '%s'.\n", t);
	cbdataFree(A);
	return;
    }
    aclParseAclList(&A->acl_list);
    if (A->acl_list == NULL) {
	debug(28, 0) ("%s line %d: %s\n",
	    cfg_filename, config_lineno, config_input_line);
	debug(28, 0) ("aclParseAccessLine: Access line contains no ACL's, skipping\n");
	cbdataFree(A);
	return;
    }
    A->cfgline = xstrdup(config_input_line);
    /* Append to the end of this list */
    for (B = *head, T = head; B; T = &B->next, B = B->next);
    *T = A;
    /* We lock _acl_access structures in aclCheck() */
}

void
aclParseAclList(acl_list ** head)
{
    acl_list *L = NULL;
    acl_list **Tail = head;	/* sane name in the use below */
    acl *a = NULL;
    char *t;

    /* next expect a list of ACL names, possibly preceeded
     * by '!' for negation */
    while ((t = strtok(NULL, w_space))) {
	L = memAllocate(MEM_ACL_LIST);
	L->op = 1;		/* defaults to non-negated */
	if (*t == '!') {
	    /* negated ACL */
	    L->op = 0;
	    t++;
	}
	debug(28, 3) ("aclParseAccessLine: looking for ACL name '%s'\n", t);
	a = aclFindByName(t);
	if (a == NULL) {
	    debug(28, 0) ("ACL name '%s' not defined!\n", t);
	    self_destruct();
	    memFree(L, MEM_ACL_LIST);
	    continue;
	}
	L->acl = a;
	*Tail = L;
	Tail = &L->next;
    }
}

/**************/
/* aclMatchIp */
/**************/

static int
aclMatchIp(void *dataptr, struct in_addr c)
{
    splayNode **Top = dataptr;
    acl_ip_data x;
    /*
     * aclIpAddrNetworkCompare() takes two acl_ip_data pointers as
     * arguments, so we must create a fake one for the client's IP
     * address, and use a /32 netmask.  However, the current code
     * probably only accesses the addr1 element of this argument,
     * so it might be possible to leave addr2 and mask unset.
     * XXX Could eliminate these repetitive assignments with a
     * static structure.
     */
    x.addr1 = c;
    x.addr2 = any_addr;
    x.mask = no_addr;
    x.next = NULL;
    *Top = splay_splay(&x, *Top, aclIpAddrNetworkCompare);
    debug(28, 3) ("aclMatchIp: '%s' %s\n",
	inet_ntoa(c), splayLastResult ? "NOT found" : "found");
    return !splayLastResult;
}

/**********************/
/* aclMatchDomainList */
/**********************/

static int
aclMatchDomainList(void *dataptr, const char *host)
{
    splayNode **Top = dataptr;
    if (host == NULL)
	return 0;
    debug(28, 3) ("aclMatchDomainList: checking '%s'\n", host);
    *Top = splay_splay(host, *Top, aclHostDomainCompare);
    debug(28, 3) ("aclMatchDomainList: '%s' %s\n",
	host, splayLastResult ? "NOT found" : "found");
    return !splayLastResult;
}

int
aclMatchRegex(relist * data, const char *word)
{
    relist *first, *prev;
    if (word == NULL)
	return 0;
    debug(28, 3) ("aclMatchRegex: checking '%s'\n", word);
    first = data;
    prev = NULL;
    while (data) {
	debug(28, 3) ("aclMatchRegex: looking for '%s'\n", data->pattern);
	if (regexec(&data->regex, word, 0, 0, 0) == 0) {
	    if (prev != NULL) {
		/* shift the element just found to the second position
		 * in the list */
		prev->next = data->next;
		data->next = first->next;
		first->next = data;
	    }
	    debug(28, 2) ("aclMatchRegex: match '%s' found in '%s'\n", data->pattern, word);
	    return 1;
	}
	prev = data;
	data = data->next;
    }
    return 0;
}

static int
aclMatchUser(void *proxyauth_acl, char *user)
{
    acl_user_data *data = (acl_user_data *) proxyauth_acl;
    splayNode *Top = data->names;

    debug(28, 7) ("aclMatchUser: user is %s, case_insensitive is %d\n",
	user, data->flags.case_insensitive);
    debug(28, 8) ("Top is %p, Top->data is %s\n", Top,
	(char *) (Top != NULL ? (Top)->data : "Unavailable"));

    if (user == NULL || strcmp(user, "-") == 0)
	return 0;

    if (data->flags.required) {
	debug(28, 7) ("aclMatchUser: user REQUIRED and auth-info present.\n");
	return 1;
    }
    if (data->flags.case_insensitive)
	Top = splay_splay(user, Top, (SPLAYCMP *) strcasecmp);
    else
	Top = splay_splay(user, Top, (SPLAYCMP *) strcmp);
    /* Top=splay_splay(user,Top,(SPLAYCMP *)dumping_strcmp); */
    debug(28, 7) ("aclMatchUser: returning %d,Top is %p, Top->data is %s\n",
	!splayLastResult, Top, (char *) (Top ? Top->data : "Unavailable"));
    data->names = Top;
    return !splayLastResult;
}

/* ACL result caching routines */

/*
 * we lookup an acl's cached results, and if we cannot find the acl being 
 * checked we check it and cache the result. This function is deliberatly 
 * generic to support caching of multiple acl types (but it needs to be more
 * generic still....
 * The Match Param and the cache MUST be tied together by the calling routine.
 * You have been warned :-]
 * Also only Matchxxx that are of the form (void *, void *) can be used.
 * probably some ugly overloading _could_ be done but I'll leave that as an
 * exercise for the reader. Note that caching of time based acl's is not
 * wise due to no expiry occuring to the cache entries until the user expires
 * or a reconfigure takes place. 
 * RBC
 */
static int
aclCacheMatchAcl(dlink_list * cache, squid_acl acltype, void *data,
    char *MatchParam)
{
    int matchrv;
    acl_proxy_auth_match_cache *auth_match;
    dlink_node *link;
    link = cache->head;
    while (link) {
	auth_match = link->data;
	if (auth_match->acl_data == data) {
	    debug(28, 4) ("aclCacheMatchAcl: cache hit on acl '%p'\n", data);
	    return auth_match->matchrv;
	}
	link = link->next;
    }
    auth_match = NULL;
    /* match the user in the acl. They are not cached. */
    switch (acltype) {
    case ACL_PROXY_AUTH:
	matchrv = aclMatchUser(data, MatchParam);
	break;
    case ACL_PROXY_AUTH_REGEX:
	matchrv = aclMatchRegex(data, MatchParam);
	break;
    default:
	/* This is a fatal to ensure that aclCacheMatchAcl calls are _only_
	 * made for supported acl types */
	fatal("aclCacheMatchAcl: unknown or unexpected ACL type");
	return 0;		/* NOTREACHED */
    }
    auth_match = memAllocate(MEM_ACL_PROXY_AUTH_MATCH);
    auth_match->matchrv = matchrv;
    auth_match->acl_data = data;
    dlinkAddTail(auth_match, &auth_match->link, cache);
    return matchrv;
}

void
aclCacheMatchFlush(dlink_list * cache)
{
    acl_proxy_auth_match_cache *auth_match;
    dlink_node *link, *tmplink;
    link = cache->head;
    while (link) {
	auth_match = link->data;
	tmplink = link;
	link = link->next;
	dlinkDelete(tmplink, cache);
	memFree(auth_match, MEM_ACL_PROXY_AUTH_MATCH);
    }
}

/* aclMatchProxyAuth can return two exit codes:
 * 0 : Authorisation for this ACL failed. (Did not match)
 * 1 : Authorisation OK. (Matched)
 */
static int
aclMatchProxyAuth(void *data, auth_user_request_t * auth_user_request,
    aclCheck_t * checklist, squid_acl acltype)
{
    /* checklist is used to register user name when identified, nothing else */
    /* General program flow in proxy_auth acls
     * 1. Consistency checks: are we getting sensible data
     * 2. Call the authenticate* functions to establish a authenticated user
     * 4. look up the username in acltype (and cache the result against the 
     *     username
     */

    /* for completeness */
    authenticateAuthUserRequestLock(auth_user_request);

    /* consistent parameters ? */
    assert(authenticateUserAuthenticated(auth_user_request));
    /* this ACL check completed */
    authenticateAuthUserRequestUnlock(auth_user_request);
    /* check to see if we have matched the user-acl before */
    return aclCacheMatchAcl(&auth_user_request->auth_user->proxy_match_cache,
	acltype, data, authenticateUserRequestUsername(auth_user_request));
}

CBDATA_TYPE(acl_user_ip_data);

void
aclParseUserMaxIP(void *data)
{
    acl_user_ip_data **acldata = data;
    char *t = NULL;
    CBDATA_INIT_TYPE(acl_user_ip_data);
    if (*acldata) {
	debug(28, 1) ("Attempting to alter already set User max IP acl\n");
	return;
    }
    *acldata = cbdataAlloc(acl_user_ip_data);
    t = strtokFile();
    if (!t)
	goto error;
    debug(28, 5) ("aclParseUserMaxIP: First token is %s\n", t);
    if (strcmp("-s", t) == 0) {
	debug(28, 5) ("aclParseUserMaxIP: Going strict\n");
	(*acldata)->flags.strict = 1;
	t = strtokFile();
	if (!t)
	    goto error;
    }
    (*acldata)->max = xatoi(t);
    debug(28, 5) ("aclParseUserMaxIP: Max IP address's %d\n", (int) (*acldata)->max);
    return;
  error:
    self_destruct();
}

void
aclDestroyUserMaxIP(void *data)
{
    acl_user_ip_data **acldata = data;
    if (*acldata)
	cbdataFree(*acldata);
    *acldata = NULL;
}

wordlist *
aclDumpUserMaxIP(void *data)
{
    acl_user_ip_data *acldata = data;
    wordlist *W = NULL;
    char buf[128];
    if (acldata->flags.strict)
	wordlistAdd(&W, "-s");
    snprintf(buf, sizeof(buf), "%lu", (unsigned long int) acldata->max);
    wordlistAdd(&W, buf);
    return W;
}

/*
 * aclMatchUserMaxIP - check for users logging in from multiple IP's 
 * 0 : No match
 * 1 : Match 
 */
int
aclMatchUserMaxIP(void *data, auth_user_request_t * auth_user_request,
    struct in_addr src_addr)
{
/*
 * the logic for flush the ip list when the limit is hit vs keep
 * it sorted in most recent access order and just drop the oldest
 * one off is currently undecided
 */
    acl_user_ip_data *acldata = data;

    if (authenticateAuthUserRequestIPCount(auth_user_request) <= acldata->max)
	return 0;

    debug(28, 1) ("aclMatchUserMaxIP: user '%s' tries to use too many IP addresses (max %d allowed)!\n", authenticateUserRequestUsername(auth_user_request), acldata->max);

    /* this is a match */
    if (acldata->flags.strict) {
	/*
	 * simply deny access - the user name is already associated with
	 * the request 
	 */
	/* remove _this_ ip, as it is the culprit for going over the limit */
	authenticateAuthUserRequestRemoveIp(auth_user_request, src_addr);
	debug(28, 4) ("aclMatchUserMaxIP: Denying access in strict mode\n");
    } else {
	/*
	 * non-strict - remove some/all of the cached entries 
	 * ie to allow the user to move machines easily
	 */
	authenticateAuthUserRequestClearIp(auth_user_request);
	debug(28, 4) ("aclMatchUserMaxIP: Denying access in non-strict mode - flushing the user ip cache\n");
    }

    return 1;
}

static void
aclLookupProxyAuthStart(aclCheck_t * checklist)
{
    auth_user_request_t *auth_user_request;
    /* make sure someone created auth_user_request for us */
    assert(checklist->auth_user_request != NULL);
    auth_user_request = checklist->auth_user_request;

    assert(authenticateValidateUser(auth_user_request));
    authenticateStart(auth_user_request, aclLookupProxyAuthDone, checklist);
}

static int
aclMatchInteger(intlist * data, int i)
{
    intlist *first, *prev;
    first = data;
    prev = NULL;
    while (data) {
	if (data->i == i) {
	    if (prev != NULL) {
		/* shift the element just found to the second position
		 * in the list */
		prev->next = data->next;
		data->next = first->next;
		first->next = data;
	    }
	    return 1;
	}
	prev = data;
	data = data->next;
    }
    return 0;
}

static int
aclMatchIntegerRange(intrange * data, int i)
{
    intrange *first, *prev;
    first = data;
    prev = NULL;
    while (data) {
	if (i < data->i) {
	    (void) 0;
	} else if (i > data->j) {
	    (void) 0;
	} else {
	    /* matched */
	    if (prev != NULL) {
		/* shift the element just found to the second position
		 * in the list */
		prev->next = data->next;
		data->next = first->next;
		first->next = data;
	    }
	    return 1;
	}
	prev = data;
	data = data->next;
    }
    return 0;
}

static int
aclMatchTime(acl_time_data * data, time_t when)
{
    static time_t last_when = 0;
    static struct tm tm;
    time_t t;
    assert(data != NULL);
    if (when != last_when) {
	last_when = when;
	xmemcpy(&tm, localtime(&when), sizeof(struct tm));
    }
    t = (time_t) (tm.tm_hour * 60 + tm.tm_min);
    debug(28, 3) ("aclMatchTime: checking %d in %d-%d, weekbits=%x\n",
	(int) t, (int) data->start, (int) data->stop, data->weekbits);

    while (data) {
	if (t >= data->start && t <= data->stop && (data->weekbits & (1 << tm.tm_wday)))
	    return 1;
	data = data->next;
    }
    return 0;
}

static int
aclMatchWordList(wordlist * w, const char *word)
{
    debug(28, 3) ("aclMatchWordList: looking for '%s'\n", word);
    while (w != NULL) {
	debug(28, 3) ("aclMatchWordList: checking '%s'\n", w->key);
	if (!strcmp(w->key, word))
	    return 1;
	w = w->next;
    }
    return 0;
}

static int
aclMatchType(acl_request_type * type, request_t * request)
{
    if (type->accelerated && request->flags.accelerated)
	return 1;
    if (type->transparent && request->flags.transparent)
	return 1;
    if (type->internal && request->flags.internal)
	return 1;
    return 0;
}

int
aclAuthenticated(aclCheck_t * checklist)
{
    request_t *r = checklist->request;
    http_hdr_type headertype;
    if (NULL == r) {
	return -1;
    } else if (r->flags.accelerated) {
	/* WWW authorization on accelerated requests */
	headertype = HDR_AUTHORIZATION;
    } else if (r->flags.transparent) {
	debug(28, 1) ("aclAuthenticated: authentication not applicable on transparently intercepted requests.\n");
	return -1;
    } else {
	/* Proxy authorization on proxy requests */
	headertype = HDR_PROXY_AUTHORIZATION;
    }
    /* get authed here */
    /* Note: this fills in checklist->auth_user_request when applicable (auth incomplete) */
    switch (authenticateTryToAuthenticateAndSetAuthUser(&checklist->auth_user_request, headertype, checklist->request, checklist->conn, checklist->src_addr)) {
    case AUTH_ACL_CANNOT_AUTHENTICATE:
	debug(28, 4) ("aclMatchAcl: returning  0 user authenticated but not authorised.\n");
	return 0;
    case AUTH_AUTHENTICATED:
	if (checklist->auth_user_request) {
	    authenticateAuthUserRequestUnlock(checklist->auth_user_request);
	    checklist->auth_user_request = NULL;
	}
	return 1;
	break;
    case AUTH_ACL_HELPER:
	debug(28, 4) ("aclMatchAcl: returning 0 sending credentials to helper.\n");
	checklist->state[ACL_PROXY_AUTH] = ACL_LOOKUP_NEEDED;
	return -1;
    case AUTH_ACL_CHALLENGE:
	debug(28, 4) ("aclMatchAcl: returning 0 sending authentication challenge.\n");
	checklist->state[ACL_PROXY_AUTH] = ACL_PROXY_AUTH_NEEDED;
	return -1;
    default:
	fatal("unexpected authenticateAuthenticate reply\n");
	return -1;
    }
}

static int
aclMatchAcl(acl * ae, aclCheck_t * checklist)
{
    request_t *r = checklist->request;
    const ipcache_addrs *ia = NULL;
    const char *fqdn = NULL;
    char *esc_buf;
    const char *header;
    const char *browser;
    int k, ti;
    if (!ae)
	return 0;
    switch (ae->type) {
    case ACL_BROWSER:
    case ACL_REFERER_REGEX:
    case ACL_DST_ASN:
    case ACL_DST_DOMAIN:
    case ACL_DST_DOM_REGEX:
    case ACL_DST_IP:
    case ACL_MAX_USER_IP:
    case ACL_METHOD:
    case ACL_TYPE:
    case ACL_PROTO:
    case ACL_PROXY_AUTH:
    case ACL_PROXY_AUTH_REGEX:
    case ACL_REP_MIME_TYPE:
    case ACL_REQ_MIME_TYPE:
    case ACL_REP_HEADER:
    case ACL_REQ_HEADER:
    case ACL_URLPATH_REGEX:
    case ACL_URL_PORT:
    case ACL_URL_REGEX:
    case ACL_URLLOGIN:
	/* These ACL types require checklist->request */
	if (NULL == r) {
	    debug(28, 1) ("WARNING: '%s' ACL is used but there is no"
		" HTTP request -- access denied.\n", ae->name);
	    return 0;
	}
	break;
    default:
	break;
    }
    debug(28, 3) ("aclMatchAcl: checking '%s'\n", ae->cfgline);
    switch (ae->type) {
    case ACL_SRC_IP:
	return aclMatchIp(&ae->data, checklist->src_addr);
	/* NOTREACHED */
    case ACL_MY_IP:
	return aclMatchIp(&ae->data, checklist->my_addr);
	/* NOTREACHED */
    case ACL_DST_IP:
	ia = ipcache_gethostbyname(r->host, IP_LOOKUP_IF_MISS);
	if (ia) {
	    for (k = 0; k < (int) ia->count; k++) {
		if (aclMatchIp(&ae->data, ia->in_addrs[k]))
		    return 1;
	    }
	    return 0;
	} else if (checklist->state[ACL_DST_IP] == ACL_LOOKUP_NONE) {
	    debug(28, 3) ("aclMatchAcl: Can't yet compare '%s' ACL for '%s'\n",
		ae->name, r->host);
	    checklist->state[ACL_DST_IP] = ACL_LOOKUP_NEEDED;
	    return 0;
	} else {
	    return 0;
	}
	/* NOTREACHED */
    case ACL_DST_DOMAIN:
	if (aclMatchDomainList(&ae->data, r->host))
	    return 1;
	if ((ia = ipcacheCheckNumeric(r->host)) == NULL)
	    return 0;
	fqdn = fqdncache_gethostbyaddr(ia->in_addrs[0], FQDN_LOOKUP_IF_MISS);
	if (fqdn)
	    return aclMatchDomainList(&ae->data, fqdn);
	if (checklist->state[ACL_DST_DOMAIN] == ACL_LOOKUP_NONE) {
	    debug(28, 3) ("aclMatchAcl: Can't yet compare '%s' ACL for '%s'\n",
		ae->name, inet_ntoa(ia->in_addrs[0]));
	    checklist->state[ACL_DST_DOMAIN] = ACL_LOOKUP_NEEDED;
	    return 0;
	}
	return aclMatchDomainList(&ae->data, "none");
	/* NOTREACHED */
    case ACL_SRC_DOMAIN:
	fqdn = fqdncache_gethostbyaddr(checklist->src_addr, FQDN_LOOKUP_IF_MISS);
	if (fqdn) {
	    return aclMatchDomainList(&ae->data, fqdn);
	} else if (checklist->state[ACL_SRC_DOMAIN] == ACL_LOOKUP_NONE) {
	    debug(28, 3) ("aclMatchAcl: Can't yet compare '%s' ACL for '%s'\n",
		ae->name, inet_ntoa(checklist->src_addr));
	    checklist->state[ACL_SRC_DOMAIN] = ACL_LOOKUP_NEEDED;
	    return 0;
	}
	return aclMatchDomainList(&ae->data, "none");
	/* NOTREACHED */
    case ACL_DST_DOM_REGEX:
	if (aclMatchRegex(ae->data, r->host))
	    return 1;
	if ((ia = ipcacheCheckNumeric(r->host)) == NULL)
	    return 0;
	fqdn = fqdncache_gethostbyaddr(ia->in_addrs[0], FQDN_LOOKUP_IF_MISS);
	if (fqdn)
	    return aclMatchRegex(ae->data, fqdn);
	if (checklist->state[ACL_DST_DOMAIN] == ACL_LOOKUP_NONE) {
	    debug(28, 3) ("aclMatchAcl: Can't yet compare '%s' ACL for '%s'\n",
		ae->name, inet_ntoa(ia->in_addrs[0]));
	    checklist->state[ACL_DST_DOMAIN] = ACL_LOOKUP_NEEDED;
	    return 0;
	}
	return aclMatchRegex(ae->data, "none");
	/* NOTREACHED */
    case ACL_SRC_DOM_REGEX:
	fqdn = fqdncache_gethostbyaddr(checklist->src_addr, FQDN_LOOKUP_IF_MISS);
	if (fqdn) {
	    return aclMatchRegex(ae->data, fqdn);
	} else if (checklist->state[ACL_SRC_DOMAIN] == ACL_LOOKUP_NONE) {
	    debug(28, 3) ("aclMatchAcl: Can't yet compare '%s' ACL for '%s'\n",
		ae->name, inet_ntoa(checklist->src_addr));
	    checklist->state[ACL_SRC_DOMAIN] = ACL_LOOKUP_NEEDED;
	    return 0;
	}
	return aclMatchRegex(ae->data, "none");
	/* NOTREACHED */
    case ACL_TIME:
	return aclMatchTime(ae->data, squid_curtime);
	/* NOTREACHED */
    case ACL_URLPATH_REGEX:
	esc_buf = xstrdup(strBuf(r->urlpath));
	rfc1738_unescape(esc_buf);
	k = aclMatchRegex(ae->data, esc_buf);
	safe_free(esc_buf);
	return k;
	/* NOTREACHED */
    case ACL_URL_REGEX:
	esc_buf = xstrdup(urlCanonical(r));
	rfc1738_unescape(esc_buf);
	k = aclMatchRegex(ae->data, esc_buf);
	safe_free(esc_buf);
	return k;
    case ACL_URLLOGIN:
	esc_buf = xstrdup(r->login);
	rfc1738_unescape(esc_buf);
	k = aclMatchRegex(ae->data, esc_buf);
	safe_free(esc_buf);
	return k;
	/* NOTREACHED */
    case ACL_MAXCONN:
	k = clientdbEstablished(checklist->src_addr, 0);
	return ((k > ((intlist *) ae->data)->i) ? 1 : 0);
	/* NOTREACHED */
    case ACL_URL_PORT:
	return aclMatchIntegerRange(ae->data, (int) r->port);
	/* NOTREACHED */
    case ACL_MY_PORT:
	return aclMatchIntegerRange(ae->data, (int) checklist->my_port);
	/* NOTREACHED */
#if USE_IDENT
    case ACL_IDENT:
	if (checklist->rfc931[0]) {
	    return aclMatchUser(ae->data, checklist->rfc931);
	} else if (checklist->conn && checklist->conn->rfc931[0]) {
	    return aclMatchUser(ae->data, checklist->conn->rfc931);
	} else {
	    checklist->state[ACL_IDENT] = ACL_LOOKUP_NEEDED;
	    return 0;
	}
	/* NOTREACHED */
    case ACL_IDENT_REGEX:
	if (checklist->rfc931[0]) {
	    return aclMatchRegex(ae->data, checklist->rfc931);
	} else if (checklist->conn && checklist->conn->rfc931[0]) {
	    return aclMatchRegex(ae->data, checklist->conn->rfc931);
	} else {
	    checklist->state[ACL_IDENT] = ACL_LOOKUP_NEEDED;
	    return 0;
	}
	/* NOTREACHED */
#endif
    case ACL_TYPE:
	return aclMatchType(ae->data, r);
    case ACL_PROTO:
	return aclMatchInteger(ae->data, r->protocol);
	/* NOTREACHED */
    case ACL_METHOD:
	return aclMatchInteger(ae->data, r->method);
	/* NOTREACHED */
    case ACL_BROWSER:
	browser = httpHeaderGetStr(&checklist->request->header, HDR_USER_AGENT);
	if (NULL == browser)
	    return 0;
	return aclMatchRegex(ae->data, browser);
	/* NOTREACHED */
    case ACL_REFERER_REGEX:
	header = httpHeaderGetStr(&checklist->request->header, HDR_REFERER);
	if (NULL == header)
	    return 0;
	return aclMatchRegex(ae->data, header);
	/* NOTREACHED */
    case ACL_PROXY_AUTH:
    case ACL_PROXY_AUTH_REGEX:
	if ((ti = aclAuthenticated(checklist)) != 1)
	    return ti;
	ti = aclMatchProxyAuth(ae->data, r->auth_user_request,
	    checklist, ae->type);
	return ti;
	/* NOTREACHED */
    case ACL_MAX_USER_IP:
	if ((ti = aclAuthenticated(checklist)) != 1)
	    return ti;
	ti = aclMatchUserMaxIP(ae->data, r->auth_user_request,
	    checklist->src_addr);
	return ti;
	/* NOTREACHED */
#if SQUID_SNMP
    case ACL_SNMP_COMMUNITY:
	return aclMatchWordList(ae->data, checklist->snmp_community);
	/* NOTREACHED */
#endif
    case ACL_SRC_ASN:
	return asnMatchIp(ae->data, checklist->src_addr);
	/* NOTREACHED */
    case ACL_DST_ASN:
	ia = ipcache_gethostbyname(r->host, IP_LOOKUP_IF_MISS);
	if (ia) {
	    for (k = 0; k < (int) ia->count; k++) {
		if (asnMatchIp(ae->data, ia->in_addrs[k]))
		    return 1;
	    }
	    return 0;
	} else if (checklist->state[ACL_DST_ASN] == ACL_LOOKUP_NONE) {
	    debug(28, 3) ("asnMatchAcl: Can't yet compare '%s' ACL for '%s'\n",
		ae->name, r->host);
	    checklist->state[ACL_DST_ASN] = ACL_LOOKUP_NEEDED;
	} else {
	    return asnMatchIp(ae->data, no_addr);
	}
	return 0;
	/* NOTREACHED */
#if USE_ARP_ACL
    case ACL_SRC_ARP:
	return aclMatchArp(&ae->data, checklist->src_addr);
	/* NOTREACHED */
#endif
    case ACL_REQ_MIME_TYPE:
	header = httpHeaderGetStr(&checklist->request->header,
	    HDR_CONTENT_TYPE);
	if (NULL == header)
	    header = "";
	return aclMatchRegex(ae->data, header);
	/* NOTREACHED */
    case ACL_REP_MIME_TYPE:
	if (!checklist->reply)
	    return 0;
	header = httpHeaderGetStr(&checklist->reply->header, HDR_CONTENT_TYPE);
	if (NULL == header)
	    header = "";
	return aclMatchRegex(ae->data, header);
	/* NOTREACHED */
    case ACL_REP_HEADER:
	if (!checklist->reply)
	    return 0;
	return aclMatchHeader(ae->data, &checklist->reply->header);
	/* NOTREACHED */
    case ACL_REQ_HEADER:
	return aclMatchHeader(ae->data, &checklist->request->header);
	/* NOTREACHED */
    case ACL_EXTERNAL:
	return aclMatchExternal(ae->data, checklist);
	/* NOTREACHED */
    case ACL_URLGROUP:
	if (!checklist->request->urlgroup)
	    return 0;
	return aclMatchWordList(ae->data, checklist->request->urlgroup);
	/* NOTREACHED */
#if USE_SSL
    case ACL_USER_CERT:
	return aclMatchUserCert(ae->data, checklist);
	/* NOTREACHED */
    case ACL_CA_CERT:
	return aclMatchCACert(ae->data, checklist);
	/* NOTREACHED */
#endif
    case ACL_EXTUSER:
	if (checklist->request->extacl_user) {
	    return aclMatchUser(ae->data, (char *) checklist->request->extacl_user);
	} else {
	    return -1;
	}
	/* NOTREACHED */
    case ACL_EXTUSER_REGEX:
	if (checklist->request->extacl_user) {
	    return aclMatchRegex(ae->data, checklist->request->extacl_user);
	} else {
	    return -1;
	}
	/* NOTREACHED */
    case ACL_NONE:
    case ACL_ENUM_MAX:
	break;
    }
    debug(28, 0) ("aclMatchAcl: '%s' has bad type %d\n",
	ae->name, ae->type);
    return 0;
}

int
aclMatchAclList(const acl_list * list, aclCheck_t * checklist)
{
    while (list) {
	int answer;
	checklist->current_acl = list->acl;
	AclMatchedName = list->acl->name;
	debug(28, 3) ("aclMatchAclList: checking %s%s\n",
	    list->op ? null_string : "!", list->acl->name);
	answer = aclMatchAcl(list->acl, checklist);
#if NOT_SURE_THIS_IS_GOOD
	/* This will make access denied if an acl cannot be evaluated.
	 * Normally Squid will just continue to the next rule
	 */
	if (answer < 0) {
	    debug(28, 3) ("aclMatchAclList: failure. returning -1\n");
	    return -1;
	}
#endif
	if (answer != list->op) {
	    debug(28, 3) ("aclMatchAclList: no match, returning 0\n");
	    return 0;
	}
	list = list->next;
    }
    debug(28, 3) ("aclMatchAclList: returning 1\n");
    return 1;
}

static void
aclCheckCleanup(aclCheck_t * checklist)
{
    /* Cleanup temporary stuff used by the ACL checking */
    if (checklist->extacl_entry) {
	cbdataUnlock(checklist->extacl_entry);
	checklist->extacl_entry = NULL;
    }
    /* During reconfigure or if authentication is used in aclCheckFast without
     * first being authenticated in http_access we can end up not finishing call
     * sequences into the auth code. In such case we must make sure to forget
     * the authentication state completely
     */
    if (checklist->auth_user_request) {
	authenticateAuthUserRequestUnlock(checklist->auth_user_request);
	checklist->auth_user_request = NULL;
	if (checklist->request) {
	    if (checklist->request->auth_user_request) {
		authenticateAuthUserRequestUnlock(checklist->request->auth_user_request);
		checklist->request->auth_user_request = NULL;
	    }
	}
	/* it might have been connection based */
	if (checklist->conn) {
	    if (checklist->conn->auth_user_request) {
		authenticateAuthUserRequestUnlock(checklist->conn->auth_user_request);
		checklist->conn->auth_user_request = NULL;
	    }
	    assert(checklist->request);
	    checklist->conn->auth_type = AUTH_BROKEN;
	}
    }
    checklist->current_acl = NULL;
}

int
aclCheckFast(const acl_access * A, aclCheck_t * checklist)
{
    allow_t allow = ACCESS_DENIED;
    int answer;
    debug(28, 5) ("aclCheckFast: list: %p\n", A);
    while (A) {
	allow = A->allow;
	answer = aclMatchAclList(A->acl_list, checklist);
	if (answer) {
	    if (answer < 0)
		return ACCESS_DENIED;
	    aclCheckCleanup(checklist);
	    return allow == ACCESS_ALLOWED;
	}
	A = A->next;
    }
    debug(28, 5) ("aclCheckFast: no matches, returning: %d\n", allow == ACCESS_DENIED);
    aclCheckCleanup(checklist);
    return allow == ACCESS_DENIED;
}

static void
aclCheck(aclCheck_t * checklist)
{
    allow_t allow = ACCESS_DENIED;
    const acl_access *A;
    int match;
    ipcache_addrs *ia;
    while ((A = checklist->access_list) != NULL) {
	/*
	 * If the _acl_access is no longer valid (i.e. its been
	 * freed because of a reconfigure), then bail on this
	 * access check.  For now, return ACCESS_DENIED.
	 */
	if (!cbdataValid(A)) {
	    cbdataUnlock(A);
	    checklist->access_list = NULL;
	    break;
	}
	debug(28, 3) ("aclCheck: checking '%s'\n", A->cfgline);
	allow = A->allow;
	match = aclMatchAclList(A->acl_list, checklist);
	if (match == -1)
	    allow = ACCESS_DENIED;
	if (checklist->state[ACL_DST_IP] == ACL_LOOKUP_NEEDED) {
	    checklist->state[ACL_DST_IP] = ACL_LOOKUP_PENDING;
	    ipcache_nbgethostbyname(checklist->request->host,
		aclLookupDstIPDone, checklist);
	    return;
	} else if (checklist->state[ACL_DST_ASN] == ACL_LOOKUP_NEEDED) {
	    checklist->state[ACL_DST_ASN] = ACL_LOOKUP_PENDING;
	    ipcache_nbgethostbyname(checklist->request->host,
		aclLookupDstIPforASNDone, checklist);
	    return;
	} else if (checklist->state[ACL_SRC_DOMAIN] == ACL_LOOKUP_NEEDED) {
	    checklist->state[ACL_SRC_DOMAIN] = ACL_LOOKUP_PENDING;
	    fqdncache_nbgethostbyaddr(checklist->src_addr,
		aclLookupSrcFQDNDone, checklist);
	    return;
	} else if (checklist->state[ACL_DST_DOMAIN] == ACL_LOOKUP_NEEDED) {
	    ia = ipcacheCheckNumeric(checklist->request->host);
	    if (ia == NULL) {
		checklist->state[ACL_DST_DOMAIN] = ACL_LOOKUP_DONE;
		return;
	    }
	    checklist->dst_addr = ia->in_addrs[0];
	    checklist->state[ACL_DST_DOMAIN] = ACL_LOOKUP_PENDING;
	    fqdncache_nbgethostbyaddr(checklist->dst_addr,
		aclLookupDstFQDNDone, checklist);
	    return;
	} else if (checklist->state[ACL_PROXY_AUTH] == ACL_LOOKUP_NEEDED) {
	    debug(28, 3)
		("aclCheck: checking password via authenticator\n");
	    aclLookupProxyAuthStart(checklist);
	    checklist->state[ACL_PROXY_AUTH] = ACL_LOOKUP_PENDING;
	    return;
	} else if (checklist->state[ACL_PROXY_AUTH] == ACL_PROXY_AUTH_NEEDED) {
	    /* Client is required to resend the request with correct authentication
	     * credentials. (This may be part of a stateful auth protocol.
	     * The request is denied.
	     */
	    debug(28, 6) ("aclCheck: requiring Proxy Auth header.\n");
	    allow = ACCESS_REQ_PROXY_AUTH;
	    match = -1;
	}
#if USE_IDENT
	else if (checklist->state[ACL_IDENT] == ACL_LOOKUP_NEEDED) {
	    debug(28, 3) ("aclCheck: Doing ident lookup\n");
	    if (cbdataValid(checklist->conn)) {
		identStart(&checklist->conn->me, &checklist->conn->peer,
		    aclLookupIdentDone, checklist);
		checklist->state[ACL_IDENT] = ACL_LOOKUP_PENDING;
		return;
	    } else {
		debug(28, 1) ("aclCheck: Can't start ident lookup. No client connection\n");
		cbdataUnlock(checklist->conn);
		checklist->conn = NULL;
		allow = ACCESS_DENIED;
		match = -1;
	    }
	}
#endif
	else if (checklist->state[ACL_EXTERNAL] == ACL_LOOKUP_NEEDED) {
	    acl *acl = checklist->current_acl;
	    assert(acl->type == ACL_EXTERNAL);
	    externalAclLookup(checklist, acl->data, aclLookupExternalDone, checklist);
	    return;
	}
	/*
	 * We are done with this _acl_access entry.  Either the request
	 * is allowed, denied, requires authentication, or we move on to
	 * the next entry.
	 */
	if (match) {
	    debug(28, 3) ("aclCheck: match found, returning %d\n", allow);
	    cbdataUnlock(A);
	    checklist->access_list = NULL;
	    aclCheckCallback(checklist, allow);
	    return;
	}
	checklist->access_list = A->next;
	/*
	 * Lock the next _acl_access entry
	 */
	if (A->next)
	    cbdataLock(A->next);
	cbdataUnlock(A);
    }
    debug(28, 3) ("aclCheck: NO match found, returning %d\n", allow != ACCESS_DENIED ? ACCESS_DENIED : ACCESS_ALLOWED);
    aclCheckCallback(checklist, allow != ACCESS_DENIED ? ACCESS_DENIED : ACCESS_ALLOWED);
}

void
aclChecklistFree(aclCheck_t * checklist)
{
    if (checklist->request)
	requestUnlink(checklist->request);
    checklist->request = NULL;
    if (checklist->conn) {
	cbdataUnlock(checklist->conn);
	checklist->conn = NULL;
    }
    if (checklist->access_list) {
	cbdataUnlock(checklist->access_list);
	checklist->access_list = NULL;
    }
    if (checklist->callback_data) {
	cbdataUnlock(checklist->callback_data);
	checklist->callback_data = NULL;
    }
    aclCheckCleanup(checklist);
    cbdataFree(checklist);
}

static void
aclCheckCallback(aclCheck_t * checklist, allow_t answer)
{
    debug(28, 3) ("aclCheckCallback: answer=%d\n", answer);
    aclCheckCleanup(checklist);
    if (cbdataValid(checklist->callback_data))
	checklist->callback(answer, checklist->callback_data);
    cbdataUnlock(checklist->callback_data);
    checklist->callback = NULL;
    checklist->callback_data = NULL;
    aclChecklistFree(checklist);
}

#if USE_IDENT
static void
aclLookupIdentDone(const char *ident, void *data)
{
    aclCheck_t *checklist = data;
    if (ident) {
	xstrncpy(checklist->rfc931, ident, USER_IDENT_SZ);
#if DONT
	xstrncpy(checklist->request->authuser, ident, USER_IDENT_SZ);
#endif
    } else {
	xstrncpy(checklist->rfc931, dash_str, USER_IDENT_SZ);
    }
    /*
     * Cache the ident result in the connection, to avoid redoing ident lookup
     * over and over on persistent connections
     */
    if (cbdataValid(checklist->conn) && !checklist->conn->rfc931[0])
	xstrncpy(checklist->conn->rfc931, checklist->rfc931, USER_IDENT_SZ);
    aclCheck(checklist);
}
#endif

static void
aclLookupDstIPDone(const ipcache_addrs * ia, void *data)
{
    aclCheck_t *checklist = data;
    checklist->state[ACL_DST_IP] = ACL_LOOKUP_DONE;
    aclCheck(checklist);
}

static void
aclLookupDstIPforASNDone(const ipcache_addrs * ia, void *data)
{
    aclCheck_t *checklist = data;
    checklist->state[ACL_DST_ASN] = ACL_LOOKUP_DONE;
    aclCheck(checklist);
}

static void
aclLookupSrcFQDNDone(const char *fqdn, void *data)
{
    aclCheck_t *checklist = data;
    checklist->state[ACL_SRC_DOMAIN] = ACL_LOOKUP_DONE;
    aclCheck(checklist);
}

static void
aclLookupDstFQDNDone(const char *fqdn, void *data)
{
    aclCheck_t *checklist = data;
    checklist->state[ACL_DST_DOMAIN] = ACL_LOOKUP_DONE;
    aclCheck(checklist);
}

static void
aclLookupProxyAuthDone(void *data, char *result)
{
    aclCheck_t *checklist = data;
    checklist->state[ACL_PROXY_AUTH] = ACL_LOOKUP_DONE;
    if (result != NULL)
	fatal("AclLookupProxyAuthDone: Old code floating around somewhere.\nMake clean and if that doesn't work, report a bug to the squid developers.\n");
    if (!authenticateValidateUser(checklist->auth_user_request) || checklist->conn == NULL) {
	/* credentials could not be checked either way
	 * restart the whole process */
	/* OR the connection was closed, there's no way to continue */
	authenticateAuthUserRequestUnlock(checklist->auth_user_request);
	checklist->auth_user_request = NULL;
	if (checklist->conn) {
	    if (checklist->conn->auth_user_request) {
		authenticateAuthUserRequestUnlock(checklist->conn->auth_user_request);
		checklist->conn->auth_user_request = NULL;
	    }
	    checklist->conn->auth_type = AUTH_BROKEN;
	}
    }
    aclCheck(checklist);
}

static void
aclLookupExternalDone(void *data, void *result)
{
    aclCheck_t *checklist = data;
    checklist->state[ACL_EXTERNAL] = ACL_LOOKUP_DONE;
    checklist->extacl_entry = result;
    cbdataLock(checklist->extacl_entry);
    aclCheck(checklist);
}

aclCheck_t *
aclChecklistCreate(const acl_access * A, request_t * request, const char *ident)
{
    int i;
    aclCheck_t *checklist;
    checklist = cbdataAlloc(aclCheck_t);
    checklist->access_list = A;
    /*
     * aclCheck() makes sure checklist->access_list is a valid
     * pointer, so lock it.
     */
    cbdataLock(A);
    if (request != NULL) {
	checklist->request = requestLink(request);
#if FOLLOW_X_FORWARDED_FOR
	if (Config.onoff.acl_uses_indirect_client) {
	    checklist->src_addr = request->indirect_client_addr;
	} else
#endif /* FOLLOW_X_FORWARDED_FOR */
	    checklist->src_addr = request->client_addr;
	checklist->my_addr = request->my_addr;
	checklist->my_port = request->my_port;
    }
    for (i = 0; i < ACL_ENUM_MAX; i++)
	checklist->state[i] = ACL_LOOKUP_NONE;
#if USE_IDENT
    if (ident)
	xstrncpy(checklist->rfc931, ident, USER_IDENT_SZ);
#endif
    checklist->auth_user_request = NULL;
    return checklist;
}

void
aclNBCheck(aclCheck_t * checklist, PF * callback, void *callback_data)
{
    checklist->callback = callback;
    checklist->callback_data = callback_data;
    cbdataLock(callback_data);
    aclCheck(checklist);
}







/*********************/
/* Destroy functions */
/*********************/

static void
aclDestroyTimeList(acl_time_data * data)
{
    acl_time_data *next = NULL;
    for (; data; data = next) {
	next = data->next;
	memFree(data, MEM_ACL_TIME_DATA);
    }
}

void
aclDestroyRegexList(relist * data)
{
    relist *next = NULL;
    for (; data; data = next) {
	next = data->next;
	regfree(&data->regex);
	safe_free(data->pattern);
	memFree(data, MEM_RELIST);
    }
}

static void
aclFreeIpData(void *p)
{
    memFree(p, MEM_ACL_IP_DATA);
}

static void
aclFreeUserData(void *data)
{
    acl_user_data *d = data;
    if (d->names)
	splay_destroy(d->names, xfree);
    memFree(d, MEM_ACL_USER_DATA);
}

static void
aclDestroyType(acl_request_type * type)
{
    memFree(type, MEM_ACL_REQUEST_TYPE);
}

void
aclDestroyAcls(acl ** head)
{
    acl *a = NULL;
    acl *next = NULL;
    for (a = *head; a; a = next) {
	next = a->next;
	debug(28, 3) ("aclDestroyAcls: '%s'\n", a->cfgline);
	switch (a->type) {
	case ACL_SRC_IP:
	case ACL_DST_IP:
	case ACL_MY_IP:
	    splay_destroy(a->data, aclFreeIpData);
	    break;
#if USE_ARP_ACL
	case ACL_SRC_ARP:
#endif
	case ACL_DST_DOMAIN:
	case ACL_SRC_DOMAIN:
	    splay_destroy(a->data, xfree);
	    break;
#if SQUID_SNMP
	case ACL_SNMP_COMMUNITY:
	    wordlistDestroy((wordlist **) (void *) &a->data);
	    break;
#endif
#if USE_IDENT
	case ACL_IDENT:
	    aclFreeUserData(a->data);
	    break;
#endif
	case ACL_PROXY_AUTH:
	case ACL_EXTUSER:
	    aclFreeUserData(a->data);
	    break;
	case ACL_TIME:
	    aclDestroyTimeList(a->data);
	    break;
#if USE_IDENT
	case ACL_IDENT_REGEX:
#endif
	case ACL_PROXY_AUTH_REGEX:
	case ACL_URL_REGEX:
	case ACL_URLLOGIN:
	case ACL_URLPATH_REGEX:
	case ACL_BROWSER:
	case ACL_REFERER_REGEX:
	case ACL_SRC_DOM_REGEX:
	case ACL_DST_DOM_REGEX:
	case ACL_REP_MIME_TYPE:
	case ACL_REQ_MIME_TYPE:
	case ACL_EXTUSER_REGEX:
	    aclDestroyRegexList(a->data);
	    break;
	case ACL_TYPE:
	    aclDestroyType(a->data);
	    break;
	case ACL_REP_HEADER:
	case ACL_REQ_HEADER:
	    aclDestroyHeader(a->data);
	    break;
	case ACL_PROTO:
	case ACL_METHOD:
	case ACL_SRC_ASN:
	case ACL_DST_ASN:
#if SRC_RTT_NOT_YET_FINISHED
	case ACL_NETDB_SRC_RTT:
#endif
	case ACL_MAXCONN:
	    intlistDestroy((intlist **) (void *) &a->data);
	    break;
	case ACL_MAX_USER_IP:
	    aclDestroyUserMaxIP(&a->data);
	    break;
	case ACL_URL_PORT:
	case ACL_MY_PORT:
	    aclDestroyIntRange(a->data);
	    break;
	case ACL_EXTERNAL:
	    aclDestroyExternal(&a->data);
	    break;
	case ACL_URLGROUP:
	    wordlistDestroy((wordlist **) (void *) &a->data);
	    break;
#if USE_SSL
	case ACL_USER_CERT:
	case ACL_CA_CERT:
	    aclDestroyCertList(&a->data);
	    break;
#endif
	case ACL_NONE:
	case ACL_ENUM_MAX:
	    debug(28, 1) ("aclDestroyAcls: no case for ACL type %d\n", a->type);
	    break;
	}
	safe_free(a->cfgline);
	memFree(a, MEM_ACL);
    }
    *head = NULL;
}

void
aclDestroyAclList(acl_list ** head)
{
    acl_list *l;
    for (l = *head; l; l = *head) {
	*head = l->next;
	memFree(l, MEM_ACL_LIST);
    }
}

void
aclDestroyAccessList(acl_access ** list)
{
    acl_access *l = NULL;
    acl_access *next = NULL;
    for (l = *list; l; l = next) {
	debug(28, 3) ("aclDestroyAccessList: '%s'\n", l->cfgline);
	next = l->next;
	aclDestroyAclList(&l->acl_list);
	safe_free(l->cfgline);
	cbdataFree(l);
    }
    *list = NULL;
}

/* maex@space.net (06.09.1996)
 *    destroy an _acl_deny_info_list */

void
aclDestroyDenyInfoList(acl_deny_info_list ** list)
{
    acl_deny_info_list *a = NULL;
    acl_deny_info_list *a_next = NULL;
    acl_name_list *l = NULL;
    acl_name_list *l_next = NULL;

    for (a = *list; a; a = a_next) {
	for (l = a->acl_list; l; l = l_next) {
	    l_next = l->next;
	    safe_free(l);
	}
	a_next = a->next;
	xfree(a->err_page_name);
	memFree(a, MEM_ACL_DENY_INFO_LIST);
    }
    *list = NULL;
}

static void
aclDestroyIntRange(intrange * list)
{
    intrange *w = NULL;
    intrange *n = NULL;
    for (w = list; w; w = n) {
	n = w->next;
	safe_free(w);
    }
}

/* general compare functions, these are used for tree search algorithms
 * so they return <0, 0 or >0 */

/* compare two domains */

static int
aclDomainCompare(const void *a, const void *b)
{
    const char *d1;
    const char *d2;
    int ret;
    d1 = b;
    d2 = a;
    ret = aclHostDomainCompare(d1, d2);
    if (ret != 0) {
	d1 = a;
	d2 = b;
	ret = aclHostDomainCompare(d1, d2);
    }
    if (ret == 0) {
	debug(28, 0) ("WARNING: '%s' is a subdomain of '%s'\n", d1, d2);
	debug(28, 0) ("WARNING: because of this '%s' is ignored to keep splay tree searching predictable\n", (char *) a);
	debug(28, 0) ("WARNING: You should probably remove '%s' from the ACL named '%s'\n", d1, AclMatchedName);
    }
    return ret;
}

/* compare a host and a domain */

static int
aclHostDomainCompare(const void *a, const void *b)
{
    const char *h = a;
    const char *d = b;
    return matchDomainName(h, d);
}

/*
 * aclIpDataToStr - print/format an acl_ip_data structure for
 * debugging output.
 */
static void
aclIpDataToStr(const acl_ip_data * ip, char *buf, int len)
{
    char b1[20];
    char b2[20];
    char b3[20];
    snprintf(b1, 20, "%s", inet_ntoa(ip->addr1));
    if (ip->addr2.s_addr != any_addr.s_addr)
	snprintf(b2, 20, "-%s", inet_ntoa(ip->addr2));
    else
	b2[0] = '\0';
    if (ip->mask.s_addr != no_addr.s_addr)
	snprintf(b3, 20, "/%s", inet_ntoa(ip->mask));
    else
	b3[0] = '\0';
    snprintf(buf, len, "%s%s%s", b1, b2, b3);
}

/*
 * aclIpNetworkCompare2 - The guts of the comparison for IP ACLs.
 * The first argument (a) is a "host" address, i.e. the IP address
 * of a cache client.  The second argument (b) is a "network" address
 * that might have a subnet and/or range.  We mask the host address
 * bits with the network subnet mask.
 */
static int
aclIpNetworkCompare2(const acl_ip_data * p, const acl_ip_data * q)
{
    struct in_addr A = p->addr1;
    const struct in_addr B = q->addr1;
    const struct in_addr C = q->addr2;
    int rc = 0;
    A.s_addr &= q->mask.s_addr;	/* apply netmask */
    if (C.s_addr == 0) {	/* single address check */
	if (ntohl(A.s_addr) > ntohl(B.s_addr))
	    rc = 1;
	else if (ntohl(A.s_addr) < ntohl(B.s_addr))
	    rc = -1;
	else
	    rc = 0;
    } else {			/* range address check */
	if (ntohl(A.s_addr) > ntohl(C.s_addr))
	    rc = 1;
	else if (ntohl(A.s_addr) < ntohl(B.s_addr))
	    rc = -1;
	else
	    rc = 0;
    }
    return rc;
}

/*
 * aclIpNetworkCompare - Compare two acl_ip_data entries.  Strictly
 * used by the splay insertion routine.  It emits a warning if it
 * detects a "collision" or overlap that would confuse the splay
 * sorting algorithm.  Much like aclDomainCompare.
 */
static int
aclIpNetworkCompare(const void *a, const void *b)
{
    const acl_ip_data *n1;
    const acl_ip_data *n2;
    int ret;
    n1 = b;
    n2 = a;
    ret = aclIpNetworkCompare2(n1, n2);
    if (ret != 0) {
	n1 = a;
	n2 = b;
	ret = aclIpNetworkCompare2(n1, n2);
    }
    if (ret == 0) {
	char buf_n1[60];
	char buf_n2[60];
	char buf_a[60];
	aclIpDataToStr(n1, buf_n1, 60);
	aclIpDataToStr(n2, buf_n2, 60);
	aclIpDataToStr((acl_ip_data *) a, buf_a, 60);
	debug(28, 0) ("WARNING: '%s' is a subnetwork of "
	    "'%s'\n", buf_n1, buf_n2);
	debug(28, 0) ("WARNING: because of this '%s' is ignored "
	    "to keep splay tree searching predictable\n", buf_a);
	debug(28, 0) ("WARNING: You should probably remove '%s' "
	    "from the ACL named '%s'\n", buf_n1, AclMatchedName);
    }
    return ret;
}

/*
 * aclIpAddrNetworkCompare - The comparison function used for ACL
 * matching checks.  The first argument (a) is a "host" address,
 * i.e.  the IP address of a cache client.  The second argument (b)
 * is an entry in some address-based access control element.  This
 * function is called via aclMatchIp() and the splay library.
 */
static int
aclIpAddrNetworkCompare(const void *a, const void *b)
{
    return aclIpNetworkCompare2(a, b);
}

static void
aclDumpUserListWalkee(void *node_data, void *outlist)
{
    /* outlist is really a wordlist ** */
    wordlistAdd(outlist, node_data);
}

static wordlist *
aclDumpUserList(acl_user_data * data)
{
    wordlist *wl = NULL;
    if (data->flags.case_insensitive)
	wordlistAdd(&wl, "-i");
    /* damn this is VERY inefficient for long ACL lists... filling
     * a wordlist this way costs Sum(1,N) iterations. For instance
     * a 1000-elements list will be filled in 499500 iterations.
     */
    if (data->flags.required)
	wordlistAdd(&wl, "REQUIRED");
    else if (data->names)
	splay_walk(data->names, aclDumpUserListWalkee, &wl);
    return wl;
}

static void
aclDumpIpListWalkee(void *node, void *state)
{
    acl_ip_data *ip = node;
    MemBuf mb;
    wordlist **W = state;
    memBufDefInit(&mb);
    memBufPrintf(&mb, "%s", inet_ntoa(ip->addr1));
    if (ip->addr2.s_addr != any_addr.s_addr)
	memBufPrintf(&mb, "-%s", inet_ntoa(ip->addr2));
    if (ip->mask.s_addr != no_addr.s_addr)
	memBufPrintf(&mb, "/%s", inet_ntoa(ip->mask));
    wordlistAdd(W, mb.buf);
    memBufClean(&mb);
}

static wordlist *
aclDumpIpList(void *data)
{
    wordlist *w = NULL;
    splay_walk(data, aclDumpIpListWalkee, &w);
    return w;
}

static void
aclDumpDomainListWalkee(void *node, void *state)
{
    char *domain = node;
    wordlistAdd(state, domain);
}

static wordlist *
aclDumpDomainList(void *data)
{
    wordlist *w = NULL;
    splay_walk(data, aclDumpDomainListWalkee, &w);
    return w;
}

static wordlist *
aclDumpTimeSpecList(acl_time_data * t)
{
    wordlist *W = NULL;
    char buf[128];
    while (t != NULL) {
	snprintf(buf, sizeof(buf), "%c%c%c%c%c%c%c %02d:%02d-%02d:%02d",
	    t->weekbits & ACL_SUNDAY ? 'S' : '-',
	    t->weekbits & ACL_MONDAY ? 'M' : '-',
	    t->weekbits & ACL_TUESDAY ? 'T' : '-',
	    t->weekbits & ACL_WEDNESDAY ? 'W' : '-',
	    t->weekbits & ACL_THURSDAY ? 'H' : '-',
	    t->weekbits & ACL_FRIDAY ? 'F' : '-',
	    t->weekbits & ACL_SATURDAY ? 'A' : '-',
	    t->start / 60, t->start % 60, t->stop / 60, t->stop % 60);
	wordlistAdd(&W, buf);
	t = t->next;
    }
    return W;
}

static wordlist *
aclDumpRegexList(relist * data)
{
    wordlist *W = NULL;
    while (data != NULL) {
	wordlistAdd(&W, data->pattern);
	data = data->next;
    }
    return W;
}

static wordlist *
aclDumpIntlistList(intlist * data)
{
    wordlist *W = NULL;
    char buf[32];
    while (data != NULL) {
	snprintf(buf, sizeof(buf), "%d", data->i);
	wordlistAdd(&W, buf);
	data = data->next;
    }
    return W;
}

static wordlist *
aclDumpIntRangeList(intrange * data)
{
    wordlist *W = NULL;
    char buf[32];
    while (data != NULL) {
	if (data->i == data->j)
	    snprintf(buf, sizeof(buf), "%d", data->i);
	else
	    snprintf(buf, sizeof(buf), "%d-%d", data->i, data->j);
	wordlistAdd(&W, buf);
	data = data->next;
    }
    return W;
}

static wordlist *
aclDumpProtoList(intlist * data)
{
    wordlist *W = NULL;
    while (data != NULL) {
	wordlistAdd(&W, ProtocolStr[data->i]);
	data = data->next;
    }
    return W;
}

static wordlist *
aclDumpMethodList(intlist * data)
{
    wordlist *W = NULL;
    while (data != NULL) {
	wordlistAdd(&W, RequestMethodStr[data->i]);
	data = data->next;
    }
    return W;
}

static wordlist *
aclDumpType(acl_request_type * type)
{
    wordlist *W = NULL;
    if (type->accelerated)
	wordlistAdd(&W, "accelerated");
    if (type->internal)
	wordlistAdd(&W, "internal");
    if (type->transparent)
	wordlistAdd(&W, "transparent");
    return W;
}

wordlist *
aclDumpGeneric(const acl * a)
{
    debug(28, 3) ("aclDumpGeneric: %s type %d\n", a->name, a->type);
    switch (a->type) {
    case ACL_SRC_IP:
    case ACL_DST_IP:
    case ACL_MY_IP:
	return aclDumpIpList(a->data);
    case ACL_SRC_DOMAIN:
    case ACL_DST_DOMAIN:
	return aclDumpDomainList(a->data);
#if SQUID_SNMP
    case ACL_SNMP_COMMUNITY:
	return wordlistDup(a->data);
#endif
#if USE_IDENT
    case ACL_IDENT:
	return aclDumpUserList(a->data);
    case ACL_IDENT_REGEX:
	return aclDumpRegexList(a->data);
#endif
    case ACL_PROXY_AUTH:
	return aclDumpUserList(a->data);
    case ACL_TIME:
	return aclDumpTimeSpecList(a->data);
    case ACL_PROXY_AUTH_REGEX:
    case ACL_URL_REGEX:
    case ACL_URLLOGIN:
    case ACL_URLPATH_REGEX:
    case ACL_BROWSER:
    case ACL_REFERER_REGEX:
    case ACL_SRC_DOM_REGEX:
    case ACL_DST_DOM_REGEX:
    case ACL_REQ_MIME_TYPE:
    case ACL_REP_MIME_TYPE:
	return aclDumpRegexList(a->data);
    case ACL_REQ_HEADER:
    case ACL_REP_HEADER:
	return aclDumpHeader(a->data);
    case ACL_SRC_ASN:
    case ACL_MAXCONN:
    case ACL_DST_ASN:
	return aclDumpIntlistList(a->data);
    case ACL_MAX_USER_IP:
	return aclDumpUserMaxIP(a->data);
    case ACL_URL_PORT:
    case ACL_MY_PORT:
	return aclDumpIntRangeList(a->data);
    case ACL_TYPE:
	return aclDumpType(a->data);
    case ACL_PROTO:
	return aclDumpProtoList(a->data);
    case ACL_METHOD:
	return aclDumpMethodList(a->data);
#if USE_ARP_ACL
    case ACL_SRC_ARP:
	return aclDumpArpList(a->data);
#endif
    case ACL_EXTERNAL:
	return aclDumpExternal(a->data);
    case ACL_URLGROUP:
	return wordlistDup(a->data);
#if USE_SSL
    case ACL_USER_CERT:
    case ACL_CA_CERT:
	return aclDumpCertList(a->data);
#endif
    case ACL_EXTUSER:
	return aclDumpUserList(a->data);
    case ACL_EXTUSER_REGEX:
	return aclDumpRegexList(a->data);
    case ACL_NONE:
    case ACL_ENUM_MAX:
	break;
    }
    debug(28, 1) ("aclDumpGeneric: no case for ACL type %d\n", a->type);
    return NULL;
}

/*
 * This function traverses all ACL elements referenced
 * by an access list (presumably 'http_access').   If 
 * it finds a PURGE method ACL, then it returns TRUE,
 * otherwise FALSE.
 */
int
aclPurgeMethodInUse(acl_access * a)
{
    acl_list *b;
    for (; a; a = a->next) {
	for (b = a->acl_list; b; b = b->next) {
	    if (ACL_METHOD != b->acl->type)
		continue;
	    if (aclMatchInteger(b->acl->data, METHOD_PURGE))
		return 1;
	}
    }
    return 0;
}


#if USE_ARP_ACL
/* ==== BEGIN ARP ACL SUPPORT ============================================= */

/*
 * From:    dale@server.ctam.bitmcnit.bryansk.su (Dale)
 * To:      wessels@nlanr.net
 * Subject: Another Squid patch... :)
 * Date:    Thu, 04 Dec 1997 19:55:01 +0300
 * ============================================================================
 * 
 * Working on setting up a proper firewall for a network containing some
 * Win'95 computers at our Univ, I've discovered that some smart students
 * avoid the restrictions easily just changing their IP addresses in Win'95
 * Contol Panel... It has been getting boring, so I took Squid-1.1.18
 * sources and added a new acl type for hard-wired access control:
 * 
 * acl <name> arp <Ethernet address> ...
 * 
 * For example,
 * 
 * acl students arp 00:00:21:55:ed:22 00:00:21:ff:55:38
 *
 * NOTE: Linux code by David Luyer <luyer@ucs.uwa.edu.au>.
 *       Original (BSD-specific) code no longer works.
 *       Solaris code by R. Gancarz <radekg@solaris.elektrownia-lagisza.com.pl>
 */

#ifdef _SQUID_WIN32_
#ifdef _SQUID_CYGWIN_
#include <windows.h>
#endif

struct arpreq {
    struct sockaddr arp_pa;	/* protocol address */
    struct sockaddr arp_ha;	/* hardware address */
    int arp_flags;		/* flags */
};

#include <Iphlpapi.h>
#else
#ifdef _SQUID_SOLARIS_
#include <sys/sockio.h>
#else
/* SG - 25 Jul 2006
 * Workaround needed to allow the build of ARP acl on OpenBSD.
 * 
 * Some defines, like
 * #define free +
 * are used in squid.h to block misuse of standard malloc routines
 * where the Squid versions should be used. This pollutes the C/C++
 * token namespace crashing any structures or classes having members
 * of the same names.
 */
#if defined(_SQUID_NETBSD_) || defined(_SQUID_OPENBSD_)
#undef free
#endif
#include <sys/sysctl.h>
#endif
#ifdef _SQUID_LINUX_
#include <net/if_arp.h>
#include <sys/ioctl.h>
#else
#include <net/if_dl.h>
#include <net/route.h>
#endif
#include <net/if.h>
#if defined(_SQUID_FREEBSD_) || defined(_SQUID_NETBSD_) || defined(_SQUID_OPENBSD_)
#include <net/if_arp.h>
#endif
#if HAVE_NETINET_IF_ETHER_H
#include <netinet/if_ether.h>
#endif
#endif

/*
 * Decode an ascii representation (asc) of an ethernet adress, and place
 * it in eth[6].
 */
static int
decode_eth(const char *asc, char *eth)
{
    int a1 = 0, a2 = 0, a3 = 0, a4 = 0, a5 = 0, a6 = 0;
    if (sscanf(asc, "%x:%x:%x:%x:%x:%x", &a1, &a2, &a3, &a4, &a5, &a6) != 6) {
	debug(28, 0) ("decode_eth: Invalid ethernet address '%s'\n", asc);
	return 0;		/* This is not valid address */
    }
    eth[0] = (u_char) a1;
    eth[1] = (u_char) a2;
    eth[2] = (u_char) a3;
    eth[3] = (u_char) a4;
    eth[4] = (u_char) a5;
    eth[5] = (u_char) a6;
    return 1;
}

static acl_arp_data *
aclParseArpData(const char *t)
{
    LOCAL_ARRAY(char, eth, 256);
    acl_arp_data *q = xcalloc(1, sizeof(acl_arp_data));
    debug(28, 5) ("aclParseArpData: %s\n", t);
    if (sscanf(t, "%[0-9a-fA-F:]", eth) != 1) {
	debug(28, 0) ("aclParseArpData: Bad ethernet address: '%s'\n", t);
	safe_free(q);
	return NULL;
    }
    if (!decode_eth(eth, q->eth)) {
	debug(28, 0) ("%s line %d: %s\n",
	    cfg_filename, config_lineno, config_input_line);
	debug(28, 0) ("aclParseArpData: Ignoring invalid ARP acl entry: can't parse '%s'\n", eth);
	safe_free(q);
	return NULL;
    }
    return q;
}


/*******************/
/* aclParseArpList */
/*******************/
static void
aclParseArpList(void *curlist)
{
    char *t = NULL;
    splayNode **Top = curlist;
    acl_arp_data *q = NULL;
    while ((t = strtokFile())) {
	if ((q = aclParseArpData(t)) == NULL)
	    continue;
	*Top = splay_insert(q, *Top, aclArpCompare);
	if (splayLastResult == 0)
	    safe_free(q);
    }
}

/***************/
/* aclMatchArp */
/***************/
static int
aclMatchArp(void *dataptr, struct in_addr c)
{
#if defined(_SQUID_LINUX_)
    struct arpreq arpReq;
    struct sockaddr_in ipAddr;
    char ifbuffer[sizeof(struct ifreq) * 64];
    struct ifconf ifc;
    struct ifreq *ifr;
    int offset;
    splayNode **Top = dataptr;
    /*
     * The linux kernel 2.2 maintains per interface ARP caches and
     * thus requires an interface name when doing ARP queries.
     * 
     * The older 2.0 kernels appear to use a unified ARP cache,
     * and require an empty interface name
     * 
     * To support both, we attempt the lookup with a blank interface
     * name first. If that does not succeed, the try each interface
     * in turn
     */
    /*
     * Set up structures for ARP lookup with blank interface name
     */
    ipAddr.sin_family = AF_INET;
    ipAddr.sin_port = 0;
    ipAddr.sin_addr = c;
    memset(&arpReq, '\0', sizeof(arpReq));
    xmemcpy(&arpReq.arp_pa, &ipAddr, sizeof(struct sockaddr_in));
    /* Query ARP table */
    if (ioctl(HttpSockets[0], SIOCGARP, &arpReq) != -1) {
	/* Skip non-ethernet interfaces */
	if (arpReq.arp_ha.sa_family != ARPHRD_ETHER) {
	    return 0;
	}
	debug(28, 4) ("Got address %02x:%02x:%02x:%02x:%02x:%02x\n",
	    arpReq.arp_ha.sa_data[0] & 0xff, arpReq.arp_ha.sa_data[1] & 0xff,
	    arpReq.arp_ha.sa_data[2] & 0xff, arpReq.arp_ha.sa_data[3] & 0xff,
	    arpReq.arp_ha.sa_data[4] & 0xff, arpReq.arp_ha.sa_data[5] & 0xff);
	/* Do lookup */
	*Top = splay_splay(&arpReq.arp_ha.sa_data, *Top, aclArpCompare);
	debug(28, 3) ("aclMatchArp: '%s' %s\n",
	    inet_ntoa(c), splayLastResult ? "NOT found" : "found");
	return (0 == splayLastResult);
    }
    /* lookup list of interface names */
    ifc.ifc_len = sizeof(ifbuffer);
    ifc.ifc_buf = ifbuffer;
    if (ioctl(HttpSockets[0], SIOCGIFCONF, &ifc) < 0) {
	debug(28, 1) ("Attempt to retrieve interface list failed: %s\n",
	    xstrerror());
	return 0;
    }
    if (ifc.ifc_len > sizeof(ifbuffer)) {
	debug(28, 1) ("Interface list too long - %d\n", ifc.ifc_len);
	return 0;
    }
    /* Attempt ARP lookup on each interface */
    offset = 0;
    while (offset < ifc.ifc_len) {
	ifr = (struct ifreq *) (ifbuffer + offset);
	offset += sizeof(*ifr);
	/* Skip loopback and aliased interfaces */
	if (0 == strncmp(ifr->ifr_name, "lo", 2))
	    continue;
	if (NULL != strchr(ifr->ifr_name, ':'))
	    continue;
	debug(28, 4) ("Looking up ARP address for %s on %s\n", inet_ntoa(c),
	    ifr->ifr_name);
	/* Set up structures for ARP lookup */
	ipAddr.sin_family = AF_INET;
	ipAddr.sin_port = 0;
	ipAddr.sin_addr = c;
	memset(&arpReq, '\0', sizeof(arpReq));
	xmemcpy(&arpReq.arp_pa, &ipAddr, sizeof(struct sockaddr_in));
	strncpy(arpReq.arp_dev, ifr->ifr_name, sizeof(arpReq.arp_dev) - 1);
	arpReq.arp_dev[sizeof(arpReq.arp_dev) - 1] = '\0';
	/* Query ARP table */
	if (-1 == ioctl(HttpSockets[0], SIOCGARP, &arpReq)) {
	    /*
	     * Query failed.  Do not log failed lookups or "device
	     * not supported"
	     */
	    if (ENXIO == errno)
		(void) 0;
	    else if (ENODEV == errno)
		(void) 0;
	    else
		debug(28, 1) ("ARP query failed: %s: %s\n",
		    ifr->ifr_name, xstrerror());
	    continue;
	}
	/* Skip non-ethernet interfaces */
	if (arpReq.arp_ha.sa_family != ARPHRD_ETHER)
	    continue;
	debug(28, 4) ("Got address %02x:%02x:%02x:%02x:%02x:%02x on %s\n",
	    arpReq.arp_ha.sa_data[0] & 0xff,
	    arpReq.arp_ha.sa_data[1] & 0xff,
	    arpReq.arp_ha.sa_data[2] & 0xff,
	    arpReq.arp_ha.sa_data[3] & 0xff,
	    arpReq.arp_ha.sa_data[4] & 0xff,
	    arpReq.arp_ha.sa_data[5] & 0xff, ifr->ifr_name);
	/* Do lookup */
	*Top = splay_splay(&arpReq.arp_ha.sa_data, *Top, aclArpCompare);
	/* Return if match, otherwise continue to other interfaces */
	if (0 == splayLastResult) {
	    debug(28, 3) ("aclMatchArp: %s found on %s\n",
		inet_ntoa(c), ifr->ifr_name);
	    return 1;
	}
	/*
	 * Should we stop looking here? Can the same IP address
	 * exist on multiple interfaces?
	 */
    }
#elif defined(_SQUID_SOLARIS_)
    struct arpreq arpReq;
    struct sockaddr_in ipAddr;
    splayNode **Top = dataptr;
    /*
     * Set up structures for ARP lookup with blank interface name
     */
    ipAddr.sin_family = AF_INET;
    ipAddr.sin_port = 0;
    ipAddr.sin_addr = c;
    memset(&arpReq, '\0', sizeof(arpReq));
    xmemcpy(&arpReq.arp_pa, &ipAddr, sizeof(struct sockaddr_in));
    /* Query ARP table */
    if (ioctl(HttpSockets[0], SIOCGARP, &arpReq) != -1) {
	/*
	 *  Solaris (at least 2.6/x86) does not use arp_ha.sa_family -
	 * it returns 00:00:00:00:00:00 for non-ethernet media 
	 */
	if (arpReq.arp_ha.sa_data[0] == 0 &&
	    arpReq.arp_ha.sa_data[1] == 0 &&
	    arpReq.arp_ha.sa_data[2] == 0 &&
	    arpReq.arp_ha.sa_data[3] == 0 &&
	    arpReq.arp_ha.sa_data[4] == 0 && arpReq.arp_ha.sa_data[5] == 0)
	    return 0;
	debug(28, 4) ("Got address %02x:%02x:%02x:%02x:%02x:%02x\n",
	    arpReq.arp_ha.sa_data[0] & 0xff, arpReq.arp_ha.sa_data[1] & 0xff,
	    arpReq.arp_ha.sa_data[2] & 0xff, arpReq.arp_ha.sa_data[3] & 0xff,
	    arpReq.arp_ha.sa_data[4] & 0xff, arpReq.arp_ha.sa_data[5] & 0xff);
	/* Do lookup */
	*Top = splay_splay(&arpReq.arp_ha.sa_data, *Top, aclArpCompare);
	debug(28, 3) ("aclMatchArp: '%s' %s\n",
	    inet_ntoa(c), splayLastResult ? "NOT found" : "found");
	return (0 == splayLastResult);
    }
#elif defined(_SQUID_FREEBSD_) || defined(_SQUID_NETBSD_) || defined(_SQUID_OPENBSD_)

    struct arpreq arpReq;
    struct sockaddr_in ipAddr;
    splayNode **Top = dataptr;

    int mib[6];
    size_t needed;
    char *lim, *buf, *next;
    struct rt_msghdr *rtm;
    struct sockaddr_inarp *sin;
    struct sockaddr_dl *sdl;

    /*
     * Set up structures for ARP lookup with blank interface name
     */
    ipAddr.sin_family = AF_INET;
    ipAddr.sin_port = 0;
    ipAddr.sin_addr = c;
    memset(&arpReq, '\0', sizeof(arpReq));
    xmemcpy(&arpReq.arp_pa, &ipAddr, sizeof(struct sockaddr_in));

    /* Query ARP table */
    mib[0] = CTL_NET;
    mib[1] = PF_ROUTE;
    mib[2] = 0;
    mib[3] = AF_INET;
    mib[4] = NET_RT_FLAGS;
    mib[5] = RTF_LLINFO;
    if (sysctl(mib, 6, NULL, &needed, NULL, 0) < 0) {
	debug(28, 0) ("Can't estimate ARP table size!\n");
	return 0;
    }
    if ((buf = xmalloc(needed)) == NULL) {
	debug(28, 0) ("Can't allocate temporary ARP table!\n");
	return 0;
    }
    if (sysctl(mib, 6, buf, &needed, NULL, 0) < 0) {
	debug(28, 0) ("Can't retrieve ARP table!\n");
	xfree(buf);
	return 0;
    }
    lim = buf + needed;
    for (next = buf; next < lim; next += rtm->rtm_msglen) {
	rtm = (struct rt_msghdr *) next;
	sin = (struct sockaddr_inarp *) (rtm + 1);
#define ROUNDUP(a) \
        ((a) > 0 ? (1 + (((a) - 1) | (sizeof(long) - 1))) : sizeof(long))
	sdl = (struct sockaddr_dl *) ((char *) sin + ROUNDUP(sin->sin_len));
	if (c.s_addr == sin->sin_addr.s_addr) {
	    if (sdl->sdl_alen) {
		arpReq.arp_ha.sa_len = sizeof(struct sockaddr);
		arpReq.arp_ha.sa_family = AF_UNSPEC;
		memcpy(arpReq.arp_ha.sa_data, LLADDR(sdl), sdl->sdl_alen);
	    }
	}
    }
    xfree(buf);
    if (arpReq.arp_ha.sa_data[0] == 0 && arpReq.arp_ha.sa_data[1] == 0 &&
	arpReq.arp_ha.sa_data[2] == 0 && arpReq.arp_ha.sa_data[3] == 0 &&
	arpReq.arp_ha.sa_data[4] == 0 && arpReq.arp_ha.sa_data[5] == 0)
	return 0;
    debug(28, 4) ("Got address %02x:%02x:%02x:%02x:%02x:%02x\n",
	arpReq.arp_ha.sa_data[0] & 0xff, arpReq.arp_ha.sa_data[1] & 0xff,
	arpReq.arp_ha.sa_data[2] & 0xff, arpReq.arp_ha.sa_data[3] & 0xff,
	arpReq.arp_ha.sa_data[4] & 0xff, arpReq.arp_ha.sa_data[5] & 0xff);
    /* Do lookup */
    *Top = splay_splay(&arpReq.arp_ha.sa_data, *Top, aclArpCompare);
    debug(28, 3) ("aclMatchArp: '%s' %s\n",
	inet_ntoa(c), splayLastResult ? "NOT found" : "found");
    return (0 == splayLastResult);
#elif defined(_SQUID_WIN32_)

    DWORD dwNetTable = 0;
    DWORD ipNetTableLen = 0;
    PMIB_IPNETTABLE NetTable = NULL;
    DWORD i;
    splayNode **Top = dataptr;
    struct arpreq arpReq;

    memset(&arpReq, '\0', sizeof(arpReq));
    /* Get size of Windows ARP table */
    if (GetIpNetTable(NetTable, &ipNetTableLen, FALSE) != ERROR_INSUFFICIENT_BUFFER) {
	debug(28, 0) ("Can't estimate ARP table size!\n");
	return 0;
    }
    /* Allocate space for ARP table and assign pointers */
    if ((NetTable = (PMIB_IPNETTABLE) xmalloc(ipNetTableLen)) == NULL) {
	debug(28, 0) ("Can't allocate temporary ARP table!\n");
	return 0;
    }
    /* Get actual ARP table */
    if ((dwNetTable = GetIpNetTable(NetTable, &ipNetTableLen, FALSE)) != NO_ERROR) {
	debug(28, 0) ("Can't retrieve ARP table!\n");
	xfree(NetTable);
	return 0;
    }
    /* Find MAC address from net table */
    for (i = 0; i < NetTable->dwNumEntries; i++) {
	if ((c.s_addr == NetTable->table[i].dwAddr) && (NetTable->table[i].dwType > 2)) {
	    arpReq.arp_ha.sa_family = AF_UNSPEC;
	    memcpy(arpReq.arp_ha.sa_data, NetTable->table[i].bPhysAddr, NetTable->table[i].dwPhysAddrLen);
	}
    }
    xfree(NetTable);
    if (arpReq.arp_ha.sa_data[0] == 0 && arpReq.arp_ha.sa_data[1] == 0 &&
	arpReq.arp_ha.sa_data[2] == 0 && arpReq.arp_ha.sa_data[3] == 0 &&
	arpReq.arp_ha.sa_data[4] == 0 && arpReq.arp_ha.sa_data[5] == 0)
	return 0;
    debug(28, 4) ("Got address %02x:%02x:%02x:%02x:%02x:%02x\n",
	arpReq.arp_ha.sa_data[0] & 0xff, arpReq.arp_ha.sa_data[1] & 0xff,
	arpReq.arp_ha.sa_data[2] & 0xff, arpReq.arp_ha.sa_data[3] & 0xff,
	arpReq.arp_ha.sa_data[4] & 0xff, arpReq.arp_ha.sa_data[5] & 0xff);

    /* Do lookup */
    *Top = splay_splay(&arpReq.arp_ha.sa_data, *Top, aclArpCompare);

    debug(28, 3) ("aclMatchArp: '%s' %s\n",
	inet_ntoa(c), splayLastResult ? "NOT found" : "found");
    return (0 == splayLastResult);
#else
    WRITE ME;
#endif
    /*
     * Address was not found on any interface
     */
    debug(28, 3) ("aclMatchArp: %s NOT found\n", inet_ntoa(c));
    return 0;
}

static int
aclArpCompare(const void *a, const void *b)
{
#if defined(_SQUID_LINUX_)
    const unsigned short *d1 = a;
    const unsigned short *d2 = b;
    if (d1[0] != d2[0])
	return (d1[0] > d2[0]) ? 1 : -1;
    if (d1[1] != d2[1])
	return (d1[1] > d2[1]) ? 1 : -1;
    if (d1[2] != d2[2])
	return (d1[2] > d2[2]) ? 1 : -1;
#elif defined(_SQUID_SOLARIS_)
    const unsigned char *d1 = a;
    const unsigned char *d2 = b;
    if (d1[0] != d2[0])
	return (d1[0] > d2[0]) ? 1 : -1;
    if (d1[1] != d2[1])
	return (d1[1] > d2[1]) ? 1 : -1;
    if (d1[2] != d2[2])
	return (d1[2] > d2[2]) ? 1 : -1;
    if (d1[3] != d2[3])
	return (d1[3] > d2[3]) ? 1 : -1;
    if (d1[4] != d2[4])
	return (d1[4] > d2[4]) ? 1 : -1;
    if (d1[5] != d2[5])
	return (d1[5] > d2[5]) ? 1 : -1;
#elif defined(_SQUID_FREEBSD_) || defined(_SQUID_OPENBSD_) || defined(_SQUID_NETBSD_)
    const unsigned char *d1 = a;
    const unsigned char *d2 = b;
    if (d1[0] != d2[0])
	return (d1[0] > d2[0]) ? 1 : -1;
    if (d1[1] != d2[1])
	return (d1[1] > d2[1]) ? 1 : -1;
    if (d1[2] != d2[2])
	return (d1[2] > d2[2]) ? 1 : -1;
    if (d1[3] != d2[3])
	return (d1[3] > d2[3]) ? 1 : -1;
    if (d1[4] != d2[4])
	return (d1[4] > d2[4]) ? 1 : -1;
    if (d1[5] != d2[5])
	return (d1[5] > d2[5]) ? 1 : -1;
#elif defined(_SQUID_WIN32_)
    const unsigned char *d1 = a;
    const unsigned char *d2 = b;
    if (d1[0] != d2[0])
	return (d1[0] > d2[0]) ? 1 : -1;
    if (d1[1] != d2[1])
	return (d1[1] > d2[1]) ? 1 : -1;
    if (d1[2] != d2[2])
	return (d1[2] > d2[2]) ? 1 : -1;
    if (d1[3] != d2[3])
	return (d1[3] > d2[3]) ? 1 : -1;
    if (d1[4] != d2[4])
	return (d1[4] > d2[4]) ? 1 : -1;
    if (d1[5] != d2[5])
	return (d1[5] > d2[5]) ? 1 : -1;
#else
    WRITE ME;
#endif
    return 0;
}

#if UNUSED_CODE
/**********************************************************************
* This is from the pre-splay-tree code for BSD
* I suspect the Linux approach will work on most O/S and be much
* better - <luyer@ucs.uwa.edu.au>
***********************************************************************
static int
checkARP(u_long ip, char *eth)
{
    int mib[6] =
    {CTL_NET, PF_ROUTE, 0, AF_INET, NET_RT_FLAGS, RTF_LLINFO};
    size_t needed;
    char *buf, *next, *lim;
    struct rt_msghdr *rtm;
    struct sockaddr_inarp *sin;
    struct sockaddr_dl *sdl;
    if (sysctl(mib, 6, NULL, &needed, NULL, 0) < 0) {
	debug(28, 0) ("Can't estimate ARP table size!\n");
	return 0;
    }
    if ((buf = xmalloc(needed)) == NULL) {
	debug(28, 0) ("Can't allocate temporary ARP table!\n");
	return 0;
    }
    if (sysctl(mib, 6, buf, &needed, NULL, 0) < 0) {
	debug(28, 0) ("Can't retrieve ARP table!\n");
	xfree(buf);
	return 0;
    }
    lim = buf + needed;
    for (next = buf; next < lim; next += rtm->rtm_msglen) {
	rtm = (struct rt_msghdr *) next;
	sin = (struct sockaddr_inarp *) (rtm + 1);
	sdl = (struct sockaddr_dl *) (sin + 1);
	if (sin->sin_addr.s_addr == ip) {
	    if (sdl->sdl_alen)
		if (!memcmp(LLADDR(sdl), eth, 6)) {
		    xfree(buf);
		    return 1;
		}
	    break;
	}
    }
    xfree(buf);
    return 0;
}
**********************************************************************/
#endif

static void
aclDumpArpListWalkee(void *node, void *state)
{
    acl_arp_data *arp = node;
    wordlist **W = state;
    static char buf[24];
    while (*W != NULL)
	W = &(*W)->next;
    snprintf(buf, sizeof(buf), "%02x:%02x:%02x:%02x:%02x:%02x",
	arp->eth[0] & 0xff, arp->eth[1] & 0xff,
	arp->eth[2] & 0xff, arp->eth[3] & 0xff,
	arp->eth[4] & 0xff, arp->eth[5] & 0xff);
    wordlistAdd(state, buf);
}

static wordlist *
aclDumpArpList(void *data)
{
    wordlist *w = NULL;
    splay_walk(data, aclDumpArpListWalkee, &w);
    return w;
}

/* ==== END ARP ACL SUPPORT =============================================== */
#endif /* USE_ARP_ACL */
