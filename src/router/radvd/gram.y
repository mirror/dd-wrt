/*
 *
 *   Authors:
 *    Pedro Roque		<roque@di.fc.ul.pt>
 *    Lars Fenneberg		<lf@elemental.net>
 *
 *   This software is Copyright 1996-2000 by the above mentioned author(s),
 *   All Rights Reserved.
 *
 *   The license which is distributed with this software in the file COPYRIGHT
 *   applies to this software. If your distribution is missing this file, you
 *   may request it from <reubenhwk@gmail.com>.
 *
 */
%{
#include "config.h"
#include "includes.h"
#include "radvd.h"
#include "defaults.h"

#define YYERROR_VERBOSE 1

#if 0 /* no longer necessary? */
#ifndef HAVE_IN6_ADDR_S6_ADDR
# ifdef __FreeBSD__
#  define s6_addr32 __u6_addr.__u6_addr32
#  define s6_addr16 __u6_addr.__u6_addr16
# endif
#endif
#endif

#define ADD_TO_LL(type, list, value) \
	do { \
		if (iface->list == NULL) \
			iface->list = value; \
		else { \
			type *current = iface->list; \
			while (current->next != NULL) \
				current = current->next; \
			current->next = value; \
		} \
	} while (0)

%}

%token		T_INTERFACE
%token		T_PREFIX
%token		T_ROUTE
%token		T_RDNSS
%token		T_DNSSL
%token		T_CLIENTS
%token		T_LOWPANCO
%token		T_ABRO
%token		T_RASRCADDRESS

%token	<str>	STRING
%token	<num>	NUMBER
%token	<snum>	SIGNEDNUMBER
%token	<dec>	DECIMAL
%token	<num>	SWITCH
%token	<addr>	IPV6ADDR
%token 		INFINITY

%token		T_IgnoreIfMissing
%token		T_AdvSendAdvert
%token		T_MaxRtrAdvInterval
%token		T_MinRtrAdvInterval
%token		T_MinDelayBetweenRAs
%token		T_AdvManagedFlag
%token		T_AdvOtherConfigFlag
%token		T_AdvLinkMTU
%token		T_AdvRAMTU
%token		T_AdvReachableTime
%token		T_AdvRetransTimer
%token		T_AdvCurHopLimit
%token		T_AdvDefaultLifetime
%token		T_AdvDefaultPreference
%token		T_AdvSourceLLAddress

%token		T_AdvOnLink
%token		T_AdvAutonomous
%token		T_AdvValidLifetime
%token		T_AdvPreferredLifetime
%token		T_DeprecatePrefix
%token		T_DecrementLifetimes

%token		T_AdvRouterAddr
%token		T_AdvHomeAgentFlag
%token		T_AdvIntervalOpt
%token		T_AdvHomeAgentInfo

%token		T_Base6Interface
%token		T_Base6to4Interface
%token		T_UnicastOnly
%token		T_AdvRASolicitedUnicast

%token		T_HomeAgentPreference
%token		T_HomeAgentLifetime

%token		T_AdvRoutePreference
%token		T_AdvRouteLifetime
%token		T_RemoveRoute

%token		T_AdvRDNSSPreference
%token		T_AdvRDNSSOpenFlag
%token		T_AdvRDNSSLifetime
%token		T_FlushRDNSS

%token		T_AdvDNSSLLifetime
%token		T_FlushDNSSL

%token		T_AdvMobRtrSupportFlag

%token		T_AdvContextLength
%token		T_AdvContextCompressionFlag
%token		T_AdvContextID
%token		T_AdvLifeTime
%token		T_AdvContextPrefix

%token		T_AdvVersionLow
%token		T_AdvVersionHigh
%token		T_AdvValidLifeTime
%token		T_Adv6LBRaddress

%token		T_BAD_TOKEN

%type	<str>	name
%type	<pinfo> prefixdef
%type	<ainfo> clientslist v6addrlist_clients
%type	<rinfo>	routedef
%type	<rdnssinfo> rdnssdef
%type	<dnsslinfo> dnssldef
%type   <lowpancoinfo> lowpancodef
%type   <abroinfo> abrodef
%type   <num>	number_or_infinity
%type	<rasrcaddressinfo> rasrcaddresslist v6addrlist_rasrcaddress

%union {
	unsigned int		num;
	int			snum;
	double			dec;
	struct in6_addr		*addr;
	char			*str;
	struct AdvPrefix	*pinfo;
	struct AdvRoute		*rinfo;
	struct AdvRDNSS		*rdnssinfo;
	struct AdvDNSSL		*dnsslinfo;
	struct Clients		*ainfo;
	struct AdvLowpanCo	*lowpancoinfo;
	struct AdvAbro		*abroinfo;
	struct AdvRASrcAddress	*rasrcaddressinfo;
};

%{
extern int num_lines;
static char const * filename;
static struct Interface *iface;
static struct Interface *IfaceList;
static struct AdvPrefix *prefix;
static struct AdvRoute *route;
static struct AdvRDNSS *rdnss;
static struct AdvDNSSL *dnssl;
static struct AdvLowpanCo *lowpanco;
static struct AdvAbro  *abro;
static void cleanup(void);
#define ABORT	do { cleanup(); YYABORT; } while (0);
static void yyerror(char const * msg);
%}

%%


grammar		: grammar ifacedef
		| ifacedef
		;

ifacedef	: ifacehead '{' ifaceparams  '}' ';'
		{
			dlog(LOG_DEBUG, 4, "%s interface definition ok", iface->props.name);

			iface->next = IfaceList;
			IfaceList = iface;

			iface = NULL;
		};

ifacehead	: T_INTERFACE name
		{
			iface = IfaceList;

			while (iface)
			{
				if (!strcmp($2, iface->props.name))
				{
					flog(LOG_ERR, "duplicate interface "
						"definition for %s", $2);
					ABORT;
				}
				iface = iface->next;
			}

			iface = malloc(sizeof(struct Interface));

			if (iface == NULL) {
				flog(LOG_CRIT, "malloc failed: %s", strerror(errno));
				ABORT;
			}

			iface_init_defaults(iface);
			strncpy(iface->props.name, $2, IFNAMSIZ-1);
			iface->props.name[IFNAMSIZ-1] = '\0';
			iface->lineno = num_lines;
		}
		;

name		: STRING
		{
			/* check vality */
			$$ = $1;
		}
		;

ifaceparams 	: ifaceparams ifaceparam /* This is left recursion and won't overrun the stack. */
		| /* empty */
		;

ifaceparam 	: ifaceval
		| prefixdef 	{ ADD_TO_LL(struct AdvPrefix, AdvPrefixList, $1); }
		| clientslist 	{ ADD_TO_LL(struct Clients, ClientList, $1); }
		| routedef 	{ ADD_TO_LL(struct AdvRoute, AdvRouteList, $1); }
		| rdnssdef 	{ ADD_TO_LL(struct AdvRDNSS, AdvRDNSSList, $1); }
		| dnssldef 	{ ADD_TO_LL(struct AdvDNSSL, AdvDNSSLList, $1); }
		| lowpancodef   { ADD_TO_LL(struct AdvLowpanCo, AdvLowpanCoList, $1); }
		| abrodef       { ADD_TO_LL(struct AdvAbro, AdvAbroList, $1); }
		| rasrcaddresslist { ADD_TO_LL(struct AdvRASrcAddress, AdvRASrcAddressList, $1); }
		;

ifaceval	: T_MinRtrAdvInterval NUMBER ';'
		{
			iface->MinRtrAdvInterval = $2;
		}
		| T_MaxRtrAdvInterval NUMBER ';'
		{
			iface->MaxRtrAdvInterval = $2;
		}
		| T_MinDelayBetweenRAs NUMBER ';'
		{
			iface->MinDelayBetweenRAs = $2;
		}
		| T_MinRtrAdvInterval DECIMAL ';'
		{
			iface->MinRtrAdvInterval = $2;
		}
		| T_MaxRtrAdvInterval DECIMAL ';'
		{
			iface->MaxRtrAdvInterval = $2;
		}
		| T_MinDelayBetweenRAs DECIMAL ';'
		{
			iface->MinDelayBetweenRAs = $2;
		}
		| T_IgnoreIfMissing SWITCH ';'
		{
			iface->IgnoreIfMissing = $2;
		}
		| T_AdvSendAdvert SWITCH ';'
		{
			iface->AdvSendAdvert = $2;
		}
		| T_AdvManagedFlag SWITCH ';'
		{
			iface->ra_header_info.AdvManagedFlag = $2;
		}
		| T_AdvOtherConfigFlag SWITCH ';'
		{
			iface->ra_header_info.AdvOtherConfigFlag = $2;
		}
		| T_AdvLinkMTU NUMBER ';'
		{
			iface->AdvLinkMTU = $2;
		}
		| T_AdvRAMTU NUMBER ';'
		{
			iface->AdvRAMTU = $2;
			iface->AdvRAMTU = MAX(MIN_AdvLinkMTU, iface->AdvRAMTU);
			iface->AdvRAMTU = MIN(MAX_AdvLinkMTU, iface->AdvRAMTU);
		}
		| T_AdvReachableTime NUMBER ';'
		{
			iface->ra_header_info.AdvReachableTime = $2;
		}
		| T_AdvRetransTimer NUMBER ';'
		{
			iface->ra_header_info.AdvRetransTimer = $2;
		}
		| T_AdvDefaultLifetime NUMBER ';'
		{
			iface->ra_header_info.AdvDefaultLifetime = $2;
		}
		| T_AdvDefaultPreference SIGNEDNUMBER ';'
		{
			iface->ra_header_info.AdvDefaultPreference = $2;
		}
		| T_AdvCurHopLimit NUMBER ';'
		{
			iface->ra_header_info.AdvCurHopLimit = $2;
		}
		| T_AdvSourceLLAddress SWITCH ';'
		{
			iface->AdvSourceLLAddress = $2;
		}
		| T_AdvIntervalOpt SWITCH ';'
		{
			iface->mipv6.AdvIntervalOpt = $2;
		}
		| T_AdvHomeAgentInfo SWITCH ';'
		{
			iface->mipv6.AdvHomeAgentInfo = $2;
		}
		| T_AdvHomeAgentFlag SWITCH ';'
		{
			iface->ra_header_info.AdvHomeAgentFlag = $2;
		}
		| T_HomeAgentPreference NUMBER ';'
		{
			iface->mipv6.HomeAgentPreference = $2;
		}
		| T_HomeAgentLifetime NUMBER ';'
		{
			iface->mipv6.HomeAgentLifetime = $2;
		}
		| T_UnicastOnly SWITCH ';'
		{
			iface->UnicastOnly = $2;
		}
		| T_AdvRASolicitedUnicast SWITCH ';'
		{
			iface->AdvRASolicitedUnicast = $2;
		}
		| T_AdvMobRtrSupportFlag SWITCH ';'
		{
			iface->mipv6.AdvMobRtrSupportFlag = $2;
		}
		;

clientslist	: T_CLIENTS '{' v6addrlist_clients '}' ';'
		{
			$$ = $3;
		}
		;

v6addrlist_clients	: IPV6ADDR ';'
		{
			struct Clients *new = calloc(1, sizeof(struct Clients));
			if (new == NULL) {
				flog(LOG_CRIT, "calloc failed: %s", strerror(errno));
				ABORT;
			}

			memcpy(&(new->Address), $1, sizeof(struct in6_addr));
			$$ = new;
		}
		| v6addrlist_clients IPV6ADDR ';'
		{
			struct Clients *new = calloc(1, sizeof(struct Clients));
			if (new == NULL) {
				flog(LOG_CRIT, "calloc failed: %s", strerror(errno));
				ABORT;
			}

			memcpy(&(new->Address), $2, sizeof(struct in6_addr));
			new->next = $1;
			$$ = new;
		}
		;

rasrcaddresslist	: T_RASRCADDRESS '{' v6addrlist_rasrcaddress '}' ';'
		{
			$$ = $3;
		}
		;

v6addrlist_rasrcaddress	: IPV6ADDR ';'
		{
			struct AdvRASrcAddress *new = calloc(1, sizeof(struct AdvRASrcAddress));
			if (new == NULL) {
				flog(LOG_CRIT, "calloc failed: %s", strerror(errno));
				ABORT;
			}

			memcpy(&(new->address), $1, sizeof(struct in6_addr));
			$$ = new;
		}
		| v6addrlist_rasrcaddress IPV6ADDR ';'
		{
			struct AdvRASrcAddress *new = calloc(1, sizeof(struct AdvRASrcAddress));
			if (new == NULL) {
				flog(LOG_CRIT, "calloc failed: %s", strerror(errno));
				ABORT;
			}

			memcpy(&(new->address), $2, sizeof(struct in6_addr));
			new->next = $1;
			$$ = new;
		}
		;

prefixdef	: prefixhead optional_prefixplist ';'
		{
			if (prefix) {

				if (prefix->AdvPreferredLifetime > prefix->AdvValidLifetime)
				{
					flog(LOG_ERR, "AdvValidLifeTime must be "
						"greater than AdvPreferredLifetime in %s, line %d",
						filename, num_lines);
					ABORT;
				}

				if ( prefix->if6[0] )
				{
					if (prefix->PrefixLen != 64) {
						flog(LOG_ERR, "only /64 is allowed with Base6Interface.  %s:%d", filename, num_lines);
						ABORT;
					}
				}
			}
			$$ = prefix;
			prefix = NULL;
		}
		;

prefixhead	: T_PREFIX IPV6ADDR '/' NUMBER
		{
			struct in6_addr zeroaddr;
			memset(&zeroaddr, 0, sizeof(zeroaddr));

#ifndef HAVE_IFADDRS_H	// all-zeros prefix is a way to tell us to get the prefix from the interface config
			if (!memcmp($2, &zeroaddr, sizeof(struct in6_addr))) {
				flog(LOG_WARNING, "invalid all-zeros prefix in %s, line %d", filename, num_lines);
			}
#endif
			prefix = malloc(sizeof(struct AdvPrefix));

			if (prefix == NULL) {
				flog(LOG_CRIT, "malloc failed: %s", strerror(errno));
				ABORT;
			}

			prefix_init_defaults(prefix);

			if ($4 > MAX_PrefixLen)
			{
				flog(LOG_ERR, "invalid prefix length in %s, line %d", filename, num_lines);
				ABORT;
			}

			prefix->PrefixLen = $4;

			memcpy(&prefix->Prefix, $2, sizeof(struct in6_addr));
		}
		;

optional_prefixplist: /* empty */
		| '{' /* somewhat empty */ '}'
		| '{' prefixplist '}'
		;

prefixplist	: prefixplist prefixparms
		| prefixparms
		;

prefixparms	: T_AdvOnLink SWITCH ';'
		{
			if (prefix) {
				prefix->AdvOnLinkFlag = $2;
			}
		}
		| T_AdvAutonomous SWITCH ';'
		{
			if (prefix) {
				prefix->AdvAutonomousFlag = $2;
			}
		}
		| T_AdvRouterAddr SWITCH ';'
		{
			if (prefix) {
				prefix->AdvRouterAddr = $2;
			}
		}
		| T_AdvValidLifetime number_or_infinity ';'
		{
			if (prefix) {
				prefix->AdvValidLifetime = $2;
				prefix->curr_validlft = $2;
			}
		}
		| T_AdvPreferredLifetime number_or_infinity ';'
		{
			if (prefix) {
				prefix->AdvPreferredLifetime = $2;
				prefix->curr_preferredlft = $2;
			}
		}
		| T_DeprecatePrefix SWITCH ';'
		{
			if (prefix) {
				prefix->DeprecatePrefixFlag = $2;
			}
		}
		| T_DecrementLifetimes SWITCH ';'
		{
			if (prefix) {
				prefix->DecrementLifetimesFlag = $2;
			}
		}
		| T_Base6Interface name ';'
		{
#ifndef HAVE_IFADDRS_H
			flog(LOG_ERR, "Base6Interface not supported in %s, line %d", filename, num_lines);
			ABORT;
#else
			if (prefix) {
				dlog(LOG_DEBUG, 4, "using prefixes on interface %s for prefixes on interface %s", $2, iface->props.name);
				strncpy(prefix->if6, $2, IFNAMSIZ-1);
				prefix->if6[IFNAMSIZ-1] = '\0';
			}
#endif
		}

		| T_Base6to4Interface name ';'
		{
#ifndef HAVE_IFADDRS_H
			flog(LOG_ERR, "Base6to4Interface not supported in %s, line %d", filename, num_lines);
			ABORT;
#else
			if (prefix) {
				dlog(LOG_DEBUG, 4, "using interface %s for 6to4 prefixes on interface %s", $2, iface->props.name);
				strncpy(prefix->if6to4, $2, IFNAMSIZ-1);
				prefix->if6to4[IFNAMSIZ-1] = '\0';
			}
#endif
		}
		;

routedef	: routehead '{' optional_routeplist '}' ';'
		{
			$$ = route;
			route = NULL;
		}
		;


routehead	: T_ROUTE IPV6ADDR '/' NUMBER
		{
			route = malloc(sizeof(struct AdvRoute));

			if (route == NULL) {
				flog(LOG_CRIT, "malloc failed: %s", strerror(errno));
				ABORT;
			}

			route_init_defaults(route, iface);

			if ($4 > MAX_PrefixLen)
			{
				flog(LOG_ERR, "invalid route prefix length in %s, line %d", filename, num_lines);
				ABORT;
			}

			route->PrefixLen = $4;

			memcpy(&route->Prefix, $2, sizeof(struct in6_addr));
		}
		;


optional_routeplist: /* empty */
		| routeplist
		;

routeplist	: routeplist routeparms
		| routeparms
		;


routeparms	: T_AdvRoutePreference SIGNEDNUMBER ';'
		{
			route->AdvRoutePreference = $2;
		}
		| T_AdvRouteLifetime number_or_infinity ';'
		{
			route->AdvRouteLifetime = $2;
		}
		| T_RemoveRoute SWITCH ';'
		{
			route->RemoveRouteFlag = $2;
		}
		;

rdnssdef	: rdnsshead '{' optional_rdnssplist '}' ';'
		{
			$$ = rdnss;
			rdnss = NULL;
		}
		;

rdnssaddrs	: rdnssaddrs rdnssaddr
		| rdnssaddr
		;

rdnssaddr	: IPV6ADDR
		{
			if (!rdnss) {
				/* first IP found */
				rdnss = malloc(sizeof(struct AdvRDNSS));

				if (rdnss == NULL) {
					flog(LOG_CRIT, "malloc failed: %s", strerror(errno));
					ABORT;
				}

				rdnss_init_defaults(rdnss, iface);
			}

			switch (rdnss->AdvRDNSSNumber) {
				case 0:
					memcpy(&rdnss->AdvRDNSSAddr1, $1, sizeof(struct in6_addr));
					rdnss->AdvRDNSSNumber++;
					break;
				case 1:
					memcpy(&rdnss->AdvRDNSSAddr2, $1, sizeof(struct in6_addr));
					rdnss->AdvRDNSSNumber++;
					break;
				case 2:
					memcpy(&rdnss->AdvRDNSSAddr3, $1, sizeof(struct in6_addr));
					rdnss->AdvRDNSSNumber++;
					break;
				default:
					flog(LOG_CRIT, "too many addresses in RDNSS section");
					ABORT;
			}

		}
		;

rdnsshead	: T_RDNSS rdnssaddrs
		{
			if (!rdnss) {
				flog(LOG_CRIT, "no address specified in RDNSS section");
				ABORT;
			}
		}
		;

optional_rdnssplist: /* empty */
		| rdnssplist
		;

rdnssplist	: rdnssplist rdnssparms
		| rdnssparms
		;


rdnssparms	: T_AdvRDNSSPreference NUMBER ';'
		{
			flog(LOG_WARNING, "ignoring deprecated RDNSS preference");
		}
		| T_AdvRDNSSOpenFlag SWITCH ';'
		{
			flog(LOG_WARNING, "ignoring deprecated RDNSS open flag");
		}
		| T_AdvRDNSSLifetime number_or_infinity ';'
		{
			if ($2 > 2*(iface->MaxRtrAdvInterval))
				flog(LOG_WARNING, "warning: AdvRDNSSLifetime <= 2*MaxRtrAdvInterval would allow stale DNS servers to be deleted faster");
			if ($2 < iface->MaxRtrAdvInterval && $2 != 0) {
				flog(LOG_ERR, "AdvRDNSSLifetime must be at least MaxRtrAdvInterval");
				rdnss->AdvRDNSSLifetime = iface->MaxRtrAdvInterval;
			} else {
				rdnss->AdvRDNSSLifetime = $2;
			}
			if ($2 > 2*(iface->MaxRtrAdvInterval))
				flog(LOG_WARNING, "warning: (%s:%d) AdvRDNSSLifetime <= 2*MaxRtrAdvInterval would allow stale DNS servers to be deleted faster", filename, num_lines);

			rdnss->AdvRDNSSLifetime = $2;
		}
		| T_FlushRDNSS SWITCH ';'
		{
			rdnss->FlushRDNSSFlag = $2;
		}
		;

dnssldef	: dnsslhead '{' optional_dnsslplist '}' ';'
		{
			$$ = dnssl;
			dnssl = NULL;
		}
		;

dnsslsuffixes	: dnsslsuffixes dnsslsuffix
		| dnsslsuffix
		;

dnsslsuffix	: STRING
		{
			char *ch;
			for (ch = $1;*ch != '\0';ch++) {
				if (*ch >= 'A' && *ch <= 'Z')
					continue;
				if (*ch >= 'a' && *ch <= 'z')
					continue;
				if (*ch >= '0' && *ch <= '9')
					continue;
				if (*ch == '-' || *ch == '.')
					continue;

				flog(LOG_CRIT, "invalid domain suffix specified");
				ABORT;
			}

			if (!dnssl) {
				/* first domain found */
				dnssl = malloc(sizeof(struct AdvDNSSL));

				if (dnssl == NULL) {
					flog(LOG_CRIT, "malloc failed: %s", strerror(errno));
					ABORT;
				}

				dnssl_init_defaults(dnssl, iface);
			}

			dnssl->AdvDNSSLNumber++;
			dnssl->AdvDNSSLSuffixes =
				realloc(dnssl->AdvDNSSLSuffixes,
					dnssl->AdvDNSSLNumber * sizeof(char*));
			if (dnssl->AdvDNSSLSuffixes == NULL) {
				flog(LOG_CRIT, "realloc failed: %s", strerror(errno));
				ABORT;
			}

			dnssl->AdvDNSSLSuffixes[dnssl->AdvDNSSLNumber - 1] = strdup($1);
		}
		;

dnsslhead	: T_DNSSL dnsslsuffixes
		{
			if (!dnssl) {
				flog(LOG_CRIT, "no domain specified in DNSSL section");
				ABORT;
			}
		}
		;

optional_dnsslplist: /* empty */
		| dnsslplist
		;

dnsslplist	: dnsslplist dnsslparms
		| dnsslparms
		;


dnsslparms	: T_AdvDNSSLLifetime number_or_infinity ';'
		{
			if ($2 > 2*(iface->MaxRtrAdvInterval))
				flog(LOG_WARNING, "warning: AdvDNSSLLifetime <= 2*MaxRtrAdvInterval would allow stale DNS suffixes to be deleted faster");
			if ($2 < iface->MaxRtrAdvInterval && $2 != 0) {
				flog(LOG_ERR, "AdvDNSSLLifetime must be at least MaxRtrAdvInterval");
				dnssl->AdvDNSSLLifetime = iface->MaxRtrAdvInterval;
			} else {
				dnssl->AdvDNSSLLifetime = $2;
			}

		}
		| T_FlushDNSSL SWITCH ';'
		{
			dnssl->FlushDNSSLFlag = $2;
		}
		;

lowpancodef 	: lowpancohead  '{' optional_lowpancoplist '}' ';'
		{
			$$ = lowpanco;
			lowpanco = NULL;
		}
		;

lowpancohead	: T_LOWPANCO
		{
			lowpanco = malloc(sizeof(struct AdvLowpanCo));

			if (lowpanco == NULL) {
				flog(LOG_CRIT, "malloc failed: %s", strerror(errno));
				ABORT;
			}

			memset(lowpanco, 0, sizeof(struct AdvLowpanCo));
		}
		;

optional_lowpancoplist:
		| lowpancoplist
		;

lowpancoplist	: lowpancoplist lowpancoparms
		| lowpancoparms
		;

lowpancoparms 	: T_AdvContextLength NUMBER ';'
		{
			lowpanco->ContextLength = $2;
		}
		| T_AdvContextCompressionFlag SWITCH ';'
		{
			lowpanco->ContextCompressionFlag = $2;
		}
		| T_AdvContextID NUMBER ';'
		{
			lowpanco->AdvContextID = $2;
		}
		| T_AdvLifeTime NUMBER ';'
		{
			lowpanco->AdvLifeTime = $2;
		}
		;

abrodef		: abrohead  '{' optional_abroplist '}' ';'
		{
			$$ = abro;
			abro = NULL;
		}
		;

abrohead	: abrohead_new | abrohead_dep

abrohead_new	: T_ABRO IPV6ADDR
		{
			abro = malloc(sizeof(struct AdvAbro));

			if (abro == NULL) {
				flog(LOG_CRIT, "malloc failed: %s", strerror(errno));
				ABORT;
			}

			memset(abro, 0, sizeof(struct AdvAbro));
			memcpy(&abro->LBRaddress, $2, sizeof(struct in6_addr));
		}
		;

abrohead_dep	: T_ABRO IPV6ADDR '/' NUMBER
		{
			flog(LOG_WARNING
				, "%s:%d abro prefix length deprecated, remove trailing '/%d'"
				, filename
				, num_lines
				, $4
			);
			abro = malloc(sizeof(struct AdvAbro));

			if (abro == NULL) {
				flog(LOG_CRIT, "malloc failed: %s", strerror(errno));
				ABORT;
			}

			memset(abro, 0, sizeof(struct AdvAbro));
			memcpy(&abro->LBRaddress, $2, sizeof(struct in6_addr));
		}
		;

optional_abroplist:
		| abroplist
		;

abroplist	: abroplist abroparms
		| abroparms
		;

abroparms	: T_AdvVersionLow NUMBER ';'
		{
			abro->Version[1] = $2;
		}
		| T_AdvVersionHigh NUMBER ';'
		{
			abro->Version[0] = $2;
		}
		| T_AdvValidLifeTime NUMBER ';'
		{
			abro->ValidLifeTime = $2;
		}
		;

number_or_infinity	: NUMBER
			{
				$$ = $1;
			}
			| INFINITY
			{
				$$ = (uint32_t)~0;
			}
			;

%%

static void cleanup(void)
{
	if (iface) {
		free_ifaces(iface);
		iface = 0;
	}

	if (prefix) {
		free(prefix);
		prefix = 0;
	}

	if (route) {
		free(route);
		route = 0;
	}

	if (rdnss) {
		free(rdnss);
		rdnss = 0;
	}

	if (dnssl) {
		int i;
		for (i = 0;i < dnssl->AdvDNSSLNumber;i++)
			free(dnssl->AdvDNSSLSuffixes[i]);
		free(dnssl->AdvDNSSLSuffixes);
		free(dnssl);
		dnssl = 0;
	}

	if (lowpanco) {
		free(lowpanco);
		lowpanco = 0;
	}

	if (abro) {
		free(abro);
		abro = 0;
	}
}

struct Interface * readin_config(char const *path)
{
	IfaceList = 0;
	FILE * in = fopen(path, "r");
	if (in) {
		filename = path;
		num_lines = 1;
		iface = 0;

		yyset_in(in);
		if (yyparse() != 0) {
			free_ifaces(iface);
			iface = 0;
			IfaceList = 0;
		} else {
			dlog(LOG_DEBUG, 1, "config file, %s, syntax ok", path);
		}
		yylex_destroy();
		fclose(in);
	}

	return IfaceList;
}

static void yyerror(char const * msg)
{
	fprintf(stderr, "%s:%d error: %s\n",
		filename,
		num_lines,
		msg);
}

