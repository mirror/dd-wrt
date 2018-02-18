%{

/*
 * The olsr.org Optimized Link-State Routing daemon (olsrd)
 *
 * (c) by the OLSR project
 *
 * See our Git repository to find out who worked on this file
 * and thus is a copyright holder on it.
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * * Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 * * Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in
 *   the documentation and/or other materials provided with the
 *   distribution.
 * * Neither the name of olsr.org, olsrd nor the names of its
 *   contributors may be used to endorse or promote products derived
 *   from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * Visit http://www.olsr.org for more information.
 *
 * If you find this software useful feel free to make a donation
 * to the project. For more information see the website or contact
 * the copyright holders.
 *
 */

#include "olsrd_conf.h"
#include "defs.h"
#include "ipcalc.h"
#include "net_olsr.h"
#include "link_set.h"
#include "olsr.h"
#include "egressTypes.h"
#include "gateway.h"

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <ctype.h>

#define PARSER_DEBUG 1

#if defined PARSER_DEBUG && PARSER_DEBUG
#define PARSER_DEBUG_PRINTF(x, args...)   printf(x, ##args)
#else
#define PARSER_DEBUG_PRINTF(x, args...)   do { } while (0)
#endif

#define SET_IFS_CONF(ifs, ifcnt, field, value) do { \
	for (; ifcnt>0; ifs=ifs->next, ifcnt--) { \
    ifs->cnfi->field = (value); \
    ifs->cnf->field = (value); \
	} \
} while (0)

#define YYSTYPE struct conf_token *

void yyerror(const char *);
int yylex(void);

static int ifs_in_curr_cfg = 0;

static int add_ipv6_addr(YYSTYPE ipaddr_arg, YYSTYPE prefixlen_arg);

static int lq_mult_helper(YYSTYPE ip_addr_arg, YYSTYPE mult_arg)
{
  union olsr_ip_addr addr;
  int i;
  struct olsr_if *walker;

#if defined PARSER_DEBUG && PARSER_DEBUG > 0
  printf("\tLinkQualityMult %s %0.2f\n",
         (ip_addr_arg != NULL) ? ip_addr_arg->string : "any",
         (double)mult_arg->floating);
#endif

  memset(&addr, 0, sizeof(addr));

  if (ip_addr_arg != NULL &&
     inet_pton(olsr_cnf->ip_version, ip_addr_arg->string, &addr) <= 0) {
    fprintf(stderr, "Cannot parse IP address %s.\n", ip_addr_arg->string);
    return -1;
  }

  walker = olsr_cnf->interfaces;

  for (i = 0; i < ifs_in_curr_cfg; i++) {
    struct olsr_lq_mult *mult = malloc(sizeof(*mult));
    if (mult == NULL) {
      fprintf(stderr, "Out of memory (LQ multiplier).\n");
      return -1;
    }

    mult->addr = addr;
    mult->value = (uint32_t)(mult_arg->floating * LINK_LOSS_MULTIPLIER);

    mult->next = walker->cnf->lq_mult;
    walker->cnfi->lq_mult = walker->cnf->lq_mult = mult;
    walker->cnf->orig_lq_mult_cnt++;
    walker->cnfi->orig_lq_mult_cnt=walker->cnf->orig_lq_mult_cnt;

    walker = walker->next;
  }

  if (ip_addr_arg != NULL) {
    free(ip_addr_arg->string);
    free(ip_addr_arg);
  }

  free(mult_arg);

  return 0;
}

static int add_ipv6_addr(YYSTYPE ipaddr_arg, YYSTYPE prefixlen_arg)
{
  union olsr_ip_addr ipaddr;
  PARSER_DEBUG_PRINTF("HNA IPv6 entry: %s/%d\n", ipaddr_arg->string, prefixlen_arg->integer);

  if (olsr_cnf->ip_version != AF_INET6) {
    fprintf(stderr, "IPv6 addresses can only be used if \"IpVersion\" == 6, skipping HNA6.\n");
    olsr_startup_sleep(3);
  }
	else {
	  if(inet_pton(AF_INET6, ipaddr_arg->string, &ipaddr) <= 0) {
      fprintf(stderr, "ihna6entry: Failed converting IP address %s\n", ipaddr_arg->string);
      return 1;
    }

		if (prefixlen_arg->integer > 128) {
			fprintf(stderr, "ihna6entry: Illegal IPv6 prefix length %d\n", prefixlen_arg->integer);
			return 1;
		}

		/* Queue */
		ip_prefix_list_add(&olsr_cnf->hna_entries, &ipaddr, prefixlen_arg->integer);
	}
  free(ipaddr_arg->string);
  free(ipaddr_arg);
  free(prefixlen_arg);

  return 0;
}

%}

%token TOK_SLASH
%token TOK_OPEN
%token TOK_CLOSE

%token TOK_STRING
%token TOK_INTEGER
%token TOK_FLOAT
%token TOK_BOOLEAN

%token TOK_IPV6TYPE

%token TOK_DEBUGLEVEL
%token TOK_IPVERSION
%token TOK_HNA4
%token TOK_HNA6
%token TOK_PLUGIN
%token TOK_INTERFACE_DEFAULTS
%token TOK_INTERFACE
%token TOK_NOINT
%token TOK_TOS
%token TOK_OLSRPORT
%token TOK_RTPROTO
%token TOK_RTTABLE
%token TOK_RTTABLE_DEFAULT
%token TOK_RTTABLE_TUNNEL
%token TOK_RTTABLE_PRIORITY
%token TOK_RTTABLE_DEFAULTOLSR_PRIORITY
%token TOK_RTTABLE_TUNNEL_PRIORITY
%token TOK_RTTABLE_DEFAULT_PRIORITY
%token TOK_WILLINGNESS
%token TOK_IPCCON
%token TOK_FIBMETRIC
%token TOK_FIBMETRICDEFAULT
%token TOK_USEHYST
%token TOK_HYSTSCALE
%token TOK_HYSTUPPER
%token TOK_HYSTLOWER
%token TOK_POLLRATE
%token TOK_NICCHGSPOLLRT
%token TOK_TCREDUNDANCY
%token TOK_MPRCOVERAGE
%token TOK_LQ_LEVEL
%token TOK_LQ_FISH
%token TOK_LQ_AGING
%token TOK_LQ_PLUGIN
%token TOK_LQ_NAT_THRESH
%token TOK_LQ_MULT
%token TOK_CLEAR_SCREEN
%token TOK_PLPARAM
%token TOK_MIN_TC_VTIME
%token TOK_LOCK_FILE
%token TOK_USE_NIIT
%token TOK_SMART_GW
%token TOK_SMART_GW_ALWAYS_REMOVE_SERVER_TUNNEL
%token TOK_SMART_GW_USE_COUNT
%token TOK_SMART_GW_TAKEDOWN_PERCENTAGE
%token TOK_SMART_GW_INSTANCE_ID
%token TOK_SMART_GW_POLICYROUTING_SCRIPT
%token TOK_SMART_GW_EGRESS_IFS
%token TOK_SMART_GW_EGRESS_FILE
%token TOK_SMART_GW_EGRESS_FILE_PERIOD
%token TOK_SMART_GW_STATUS_FILE
%token TOK_SMART_GW_OFFSET_TABLES
%token TOK_SMART_GW_OFFSET_RULES
%token TOK_SMART_GW_ALLOW_NAT
%token TOK_SMART_GW_PERIOD
%token TOK_SMART_GW_STABLECOUNT
%token TOK_SMART_GW_THRESH
%token TOK_SMART_GW_WEIGHT_EXITLINK_UP
%token TOK_SMART_GW_WEIGHT_EXITLINK_DOWN
%token TOK_SMART_GW_WEIGHT_ETX
%token TOK_SMART_GW_DIVIDER_ETX
%token TOK_SMART_GW_MAX_COST_MAX_ETX
%token TOK_SMART_GW_UPLINK
%token TOK_SMART_GW_UPLINK_NAT
%token TOK_SMART_GW_SPEED
%token TOK_SMART_GW_PREFIX
%token TOK_SRC_IP_ROUTES
%token TOK_MAIN_IP
%token TOK_SET_IPFORWARD

%token TOK_HOSTLABEL
%token TOK_NETLABEL
%token TOK_MAXIPC

%token TOK_IFMODE
%token TOK_IPV4MULTICAST
%token TOK_IP4BROADCAST
%token TOK_IPV4BROADCAST
%token TOK_IPV6MULTICAST
%token TOK_IPV4SRC
%token TOK_IPV6SRC
%token TOK_IFWEIGHT
%token TOK_HELLOINT
%token TOK_HELLOVAL
%token TOK_TCINT
%token TOK_TCVAL
%token TOK_MIDINT
%token TOK_MIDVAL
%token TOK_HNAINT
%token TOK_HNAVAL
%token TOK_AUTODETCHG

%token TOK_IPV4_ADDR
%token TOK_IPV6_ADDR
%token TOK_DEFAULT
%token TOK_AUTO
%token TOK_NONE

%token TOK_COMMENT

%%

conf:
          | conf block
          | conf stmt
;

stmt:       idebug
          | iipversion
          | fibmetric
          | afibmetricdefault
          | bnoint
          | atos
          | aolsrport
          | irtproto
          | irttable
          | irttable_default
          | irttable_tunnel
          | irttable_priority
          | irttable_defaultolsr_priority
          | irttable_tunnel_priority
          | irttable_default_priority
          | awillingness
          | busehyst
          | fhystscale
          | fhystupper
          | fhystlower
          | fpollrate
          | fnicchgspollrt
          | atcredundancy
          | amprcoverage
          | alq_level
          | alq_plugin
          | alq_fish
          | anat_thresh
          | alq_aging
          | bclear_screen
          | vcomment
          | amin_tc_vtime
          | alock_file
          | suse_niit
          | bsmart_gw
          | bsmart_gw_always_remove_server_tunnel
          | ismart_gw_use_count
          | ismart_gw_takedown_percentage
          | ssmart_gw_instance_id
          | ssmart_gw_policyrouting_script
          | ssmart_gw_egress_file
          | ismart_gw_egress_file_period
          | ssmart_gw_status_file
          | ismart_gw_offset_tables
          | ismart_gw_offset_rules
          | bsmart_gw_allow_nat
          | ismart_gw_period
          | asmart_gw_stablecount
          | asmart_gw_thresh
          | asmart_gw_weight_exitlink_up
          | asmart_gw_weight_exitlink_down
          | asmart_gw_weight_etx
          | asmart_gw_divider_etx
          | ssmart_gw_uplink
          | bsmart_gw_uplink_nat
          | ismart_gw_speed
          | ismart_gw_prefix
          | bsrc_ip_routes
          | amain_ip
          | bset_ipforward
          | ssgw_egress_ifs
;

block:      TOK_HNA4 hna4body
          | TOK_HNA6 hna6body
          | TOK_IPCCON ipcbody
          | ifdblock ifdbody
          | ifblock ifbody
          | plblock plbody
;

hna4body:       TOK_OPEN hna4stmts TOK_CLOSE
;

hna4stmts: | hna4stmts hna4stmt
;

hna4stmt:  vcomment
         | ihna4entry
;

hna6body:       TOK_OPEN hna6stmts TOK_CLOSE
;

hna6stmts: | hna6stmts hna6stmt
;

hna6stmt:  vcomment
         | ihna6entry
;

ipcbody:    TOK_OPEN ipcstmts TOK_CLOSE
;

ipcstmts: | ipcstmts ipcstmt
;

ipcstmt:  vcomment
          | imaxipc
          | ipchost
          | ipcnet
;

ifblock:   ifstart ifnicks
;

ifnicks:   | ifnicks ifnick
;

ifbody:     TOK_OPEN ifstmts TOK_CLOSE
;

ifdbody:     TOK_OPEN ifstmts TOK_CLOSE
{
  struct olsr_if *in = olsr_cnf->interfaces;
  printf("\nInterface Defaults");
  /*remove Interface Defaults from Interface list as they are no interface!*/
  olsr_cnf->interfaces = in->next;
  ifs_in_curr_cfg=0;
  /*free interface but keep its config intact?*/
  free(in->cnfi);
  free(in);

}
;

ifstmts:   | ifstmts ifstmt
;

ifstmt:      vcomment
             | iifweight
             | isetifmode
             | TOK_IP4BROADCAST isetipv4mc
             | TOK_IPV4BROADCAST isetipv4mc
             | TOK_IPV4MULTICAST isetipv4mc
             | isetipv6mc
             | isetipv4src
             | isetipv6src
             | isethelloint
             | isethelloval
             | isettcint
             | isettcval
             | isetmidint
             | isetmidval
             | isethnaint
             | isethnaval
             | isetautodetchg
             | isetlqmult
;

plbody:     TOK_OPEN plstmts TOK_CLOSE
;

plstmts:   | plstmts plstmt
;

plstmt:     plparam
          | vcomment
;

ifdblock: TOK_INTERFACE_DEFAULTS
{
  struct olsr_if *in = malloc(sizeof(*in));

  if (in == NULL) {
    fprintf(stderr, "Out of memory(ADD IF)\n");
    YYABORT;
  }

  in->cnf = get_default_if_config();
  in->cnfi = get_default_if_config();

  if (in->cnf == NULL || in->cnfi == NULL) {
    fprintf(stderr, "Out of memory(ADD DEFIFRULE)\n");
    if (in->cnf) {
      free(in->cnf);
    }
    if (in->cnfi) {
      free(in->cnfi);
    }
    free(in);
    YYABORT;
  }

  //should not need a name any more, as we free it on "}" again
  //in->name = strdup(interface_defaults_name);

  olsr_cnf->interface_defaults = in->cnf;

  /* Queue */
  in->next = olsr_cnf->interfaces;
  olsr_cnf->interfaces = in;
  ifs_in_curr_cfg=1;
  
  fflush(stdout);
}
;

imaxipc: TOK_MAXIPC TOK_INTEGER
{
  olsr_cnf->ipc_connections = $2->integer;
  free($2);
}
;

ipchost: TOK_HOSTLABEL TOK_IPV4_ADDR
{
  union olsr_ip_addr ipaddr;
  PARSER_DEBUG_PRINTF("\tIPC host: %s\n", $2->string);
  
  if (inet_pton(AF_INET, $2->string, &ipaddr.v4) == 0) {
    fprintf(stderr, "Failed converting IP address IPC %s\n", $2->string);
    YYABORT;
  }

  ip_prefix_list_add(&olsr_cnf->ipc_nets, &ipaddr, olsr_cnf->maxplen);

  free($2->string);
  free($2);
}
;

ipcnet: TOK_NETLABEL TOK_IPV4_ADDR TOK_IPV4_ADDR
{
  union olsr_ip_addr ipaddr, netmask;

  PARSER_DEBUG_PRINTF("\tIPC net: %s/%s\n", $2->string, $3->string);
  
  if (inet_pton(AF_INET, $2->string, &ipaddr.v4) == 0) {
    fprintf(stderr, "Failed converting IP net IPC %s\n", $2->string);
    YYABORT;
  }

  if (inet_pton(AF_INET, $3->string, &netmask.v4) == 0) {
    fprintf(stderr, "Failed converting IP mask IPC %s\n", $3->string);
    YYABORT;
  }

  ip_prefix_list_add(&olsr_cnf->ipc_nets, &ipaddr, olsr_netmask_to_prefix(&netmask));

  free($2->string);
  free($2);
  free($3->string);
  free($3);
}
        |       TOK_NETLABEL TOK_IPV4_ADDR TOK_SLASH TOK_INTEGER
{
  union olsr_ip_addr ipaddr;

  PARSER_DEBUG_PRINTF("\tIPC net: %s/%s\n", $2->string, $3->string);
  
  if (inet_pton(AF_INET, $2->string, &ipaddr.v4) == 0) {
    fprintf(stderr, "Failed converting IP net IPC %s\n", $2->string);
    YYABORT;
  }

  if ($4->integer > olsr_cnf->maxplen) {
    fprintf(stderr, "ipcnet: Prefix len %u > %d is not allowed!\n", $4->integer, olsr_cnf->maxplen);
    YYABORT;
  }

  ip_prefix_list_add(&olsr_cnf->ipc_nets, &ipaddr, $4->integer);

  free($2->string);
  free($2);
  free($4);
}
;

iifweight:       TOK_IFWEIGHT TOK_INTEGER
{
  int ifcnt = ifs_in_curr_cfg;
  struct olsr_if *ifs = olsr_cnf->interfaces;

  PARSER_DEBUG_PRINTF("Fixed willingness: %d\n", $2->integer);

  while (ifcnt) {
    ifs->cnf->weight.value = $2->integer;
    ifs->cnf->weight.fixed = true;
    ifs->cnfi->weight.value = $2->integer;
    ifs->cnfi->weight.fixed = true;

    ifs = ifs->next;
    ifcnt--;
  }

  free($2);
}
;

isetifmode: TOK_IFMODE TOK_STRING
{
  int ifcnt = ifs_in_curr_cfg;
  struct olsr_if *ifs = olsr_cnf->interfaces;
	int mode = (strcmp($2->string, "ether") == 0)?IF_MODE_ETHER:IF_MODE_MESH;

  PARSER_DEBUG_PRINTF("\tMode: %s\n", $2->string);

	SET_IFS_CONF(ifs, ifcnt, mode, mode);
	
  free($2->string);
  free($2);
}
;

/* called if prepended with TOK_IPV4MULTICAST TOK_IP4BROADCAST TOK_IPV4BROADCAST */
isetipv4mc: TOK_IPV4_ADDR
{
  struct in_addr in;
  int ifcnt = ifs_in_curr_cfg;
  struct olsr_if *ifs = olsr_cnf->interfaces;

  PARSER_DEBUG_PRINTF("\tIPv4 broadcast: %s\n", $1->string);

  if (inet_pton(AF_INET, $1->string, &in) == 0) {
    fprintf(stderr, "isetipv4br: Failed converting IP address %s\n", $1->string);
    YYABORT;
  }

	SET_IFS_CONF(ifs, ifcnt, ipv4_multicast.v4, in);

  free($1->string);
  free($1);
}
;

isetipv6mc: TOK_IPV6MULTICAST TOK_IPV6_ADDR
{
  struct in6_addr in6;
  int ifcnt = ifs_in_curr_cfg;
  struct olsr_if *ifs = olsr_cnf->interfaces;

  PARSER_DEBUG_PRINTF("\tIPv6 multicast: %s\n", $2->string);

  if (inet_pton(AF_INET6, $2->string, &in6) <= 0) {
    fprintf(stderr, "isetipv6mc: Failed converting IP address %s\n", $2->string);
    YYABORT;
  }

	SET_IFS_CONF(ifs, ifcnt, ipv6_multicast.v6, in6);

  free($2->string);
  free($2);
}
;

isetipv4src: TOK_IPV4SRC TOK_IPV4_ADDR
{
  struct in_addr in;
  int ifcnt = ifs_in_curr_cfg;
  struct olsr_if *ifs = olsr_cnf->interfaces;

  PARSER_DEBUG_PRINTF("\tIPv4 src: %s\n", $2->string);

  if (inet_pton(AF_INET, $2->string, &in) == 0) {
    fprintf(stderr, "isetipv4src: Failed converting IP address %s\n", $2->string);
    YYABORT;
  }

	SET_IFS_CONF(ifs, ifcnt, ipv4_src.v4, in);

  free($2->string);
  free($2);
}
;

isetipv6src: TOK_IPV6SRC TOK_IPV6_ADDR TOK_SLASH TOK_INTEGER
{
  struct olsr_ip_prefix pr6;
  int ifcnt = ifs_in_curr_cfg;
  struct olsr_if *ifs = olsr_cnf->interfaces;

  PARSER_DEBUG_PRINTF("\tIPv6 src prefix: %s/%d\n", $2->string, $4->integer);

  if (inet_pton(AF_INET6, $2->string, &pr6.prefix.v6) <= 0) {
    fprintf(stderr, "isetipv6src: Failed converting IP address %s\n", $2->string);
    YYABORT;
  }
  if ($4->integer > 128) {
    fprintf(stderr, "isetipv6src: Illegal Prefixlength %d\n", $4->integer);
    YYABORT;
  }
  pr6.prefix_len = $4->integer;

	SET_IFS_CONF(ifs, ifcnt, ipv6_src, pr6);

  free($2->string);
  free($2);
}
        | TOK_IPV6SRC TOK_IPV6_ADDR
{
  struct olsr_ip_prefix pr6;
  int ifcnt = ifs_in_curr_cfg;
  struct olsr_if *ifs = olsr_cnf->interfaces;

  PARSER_DEBUG_PRINTF("\tIPv6 src prefix: %s/%d\n", $2->string, 128);

  if (inet_pton(AF_INET6, $2->string, &pr6.prefix.v6) <= 0) {
    fprintf(stderr, "isetipv6src: Failed converting IP address %s\n", $2->string);
    YYABORT;
  }
  pr6.prefix_len = 128;

  SET_IFS_CONF(ifs, ifcnt, ipv6_src, pr6);

  free($2->string);
  free($2);
}
;

isethelloint: TOK_HELLOINT TOK_FLOAT
{
  int ifcnt = ifs_in_curr_cfg;
  struct olsr_if *ifs = olsr_cnf->interfaces;

  PARSER_DEBUG_PRINTF("\tHELLO interval: %0.2f\n", (double)$2->floating);

	SET_IFS_CONF(ifs, ifcnt, hello_params.emission_interval, $2->floating);

  free($2);
}
;
isethelloval: TOK_HELLOVAL TOK_FLOAT
{
  int ifcnt = ifs_in_curr_cfg;
  struct olsr_if *ifs = olsr_cnf->interfaces;

  PARSER_DEBUG_PRINTF("\tHELLO validity: %0.2f\n", (double)$2->floating);

	SET_IFS_CONF(ifs, ifcnt, hello_params.validity_time, $2->floating);

  free($2);
}
;
isettcint: TOK_TCINT TOK_FLOAT
{
  int ifcnt = ifs_in_curr_cfg;
  struct olsr_if *ifs = olsr_cnf->interfaces;

  PARSER_DEBUG_PRINTF("\tTC interval: %0.2f\n", (double)$2->floating);

	SET_IFS_CONF(ifs, ifcnt, tc_params.emission_interval, $2->floating);

  free($2);
}
;
isettcval: TOK_TCVAL TOK_FLOAT
{
  int ifcnt = ifs_in_curr_cfg;
  struct olsr_if *ifs = olsr_cnf->interfaces;
  
  PARSER_DEBUG_PRINTF("\tTC validity: %0.2f\n", (double)$2->floating);
  
 SET_IFS_CONF(ifs, ifcnt, tc_params.validity_time, $2->floating);

  free($2);
}
;
isetmidint: TOK_MIDINT TOK_FLOAT
{
  int ifcnt = ifs_in_curr_cfg;
  struct olsr_if *ifs = olsr_cnf->interfaces;


  PARSER_DEBUG_PRINTF("\tMID interval: %0.2f\n", (double)$2->floating);
  
  SET_IFS_CONF(ifs, ifcnt, mid_params.emission_interval, $2->floating);

  free($2);
}
;
isetmidval: TOK_MIDVAL TOK_FLOAT
{
  int ifcnt = ifs_in_curr_cfg;
  struct olsr_if *ifs = olsr_cnf->interfaces;

  PARSER_DEBUG_PRINTF("\tMID validity: %0.2f\n", (double)$2->floating);
  
  SET_IFS_CONF(ifs, ifcnt, mid_params.validity_time, $2->floating);

  free($2);
}
;
isethnaint: TOK_HNAINT TOK_FLOAT
{
  int ifcnt = ifs_in_curr_cfg;
  struct olsr_if *ifs = olsr_cnf->interfaces;
  
  PARSER_DEBUG_PRINTF("\tHNA interval: %0.2f\n", (double)$2->floating);

  SET_IFS_CONF(ifs, ifcnt, hna_params.emission_interval, $2->floating);

  free($2);
}
;
isethnaval: TOK_HNAVAL TOK_FLOAT
{
  int ifcnt = ifs_in_curr_cfg;
  struct olsr_if *ifs = olsr_cnf->interfaces;

  PARSER_DEBUG_PRINTF("\tHNA validity: %0.2f\n", (double)$2->floating);

  SET_IFS_CONF(ifs, ifcnt, hna_params.validity_time, $2->floating);

  free($2);
}
;
isetautodetchg: TOK_AUTODETCHG TOK_BOOLEAN
{
  int ifcnt = ifs_in_curr_cfg;
  struct olsr_if *ifs = olsr_cnf->interfaces;

  PARSER_DEBUG_PRINTF("\tAutodetect changes: %s\n", $2->boolean ? "YES" : "NO");

  SET_IFS_CONF(ifs, ifcnt, autodetect_chg, $2->boolean);

  free($2);
}
;

isetlqmult: TOK_LQ_MULT TOK_DEFAULT TOK_FLOAT
{
  if (lq_mult_helper($2, $3) < 0) {
    YYABORT;
  }
}

          | TOK_LQ_MULT TOK_IPV4_ADDR TOK_FLOAT
{
  if (lq_mult_helper($2, $3) < 0) {
    YYABORT;
  }
}

          | TOK_LQ_MULT TOK_IPV6_ADDR TOK_FLOAT
{
  if (lq_mult_helper($2, $3) < 0) {
    YYABORT;
  }
}
;

idebug:       TOK_DEBUGLEVEL TOK_INTEGER
{
  olsr_cnf->debug_level = $2->integer;
  PARSER_DEBUG_PRINTF("Debug level: %d\n", olsr_cnf->debug_level);
  free($2);
}
;


iipversion:    TOK_IPVERSION TOK_INTEGER
{
  if ($2->integer == 4) {
    olsr_cnf->ip_version = AF_INET;
    olsr_cnf->ipsize = sizeof(struct in_addr);
    olsr_cnf->maxplen = 32;
  } else if ($2->integer == 6) {
    olsr_cnf->ip_version = AF_INET6;
    olsr_cnf->ipsize = sizeof(struct in6_addr);
    olsr_cnf->maxplen = 128;
  } else {
    fprintf(stderr, "IPversion must be 4 or 6!\n");
    YYABORT;
  }

  PARSER_DEBUG_PRINTF("IpVersion: %d\n", $2->integer);
  free($2);
}
;

fibmetric:    TOK_FIBMETRIC TOK_STRING
{
  int i;
  PARSER_DEBUG_PRINTF("FIBMetric: %s\n", $2->string);
  for (i=0; i<FIBM_CNT; i++) {
    if (strcmp($2->string, FIB_METRIC_TXT[i]) == 0) {
      olsr_cnf->fib_metric = i;
      break;
    }
  }
  if (i == FIBM_CNT) {
    fprintf(stderr, "Bad FIBMetric value: %s\n", $2->string);
    YYABORT;
  }
  free($1);
  free($2->string);
  free($2);
}
;

afibmetricdefault: TOK_FIBMETRICDEFAULT TOK_INTEGER
{
  PARSER_DEBUG_PRINTF("FIBMetricDefault: %d\n", $2->integer);
  olsr_cnf->fib_metric_default = $2->integer;
  free($2);
}
;

ihna4entry:     TOK_IPV4_ADDR TOK_IPV4_ADDR
{
  union olsr_ip_addr ipaddr, netmask;

  if (olsr_cnf->ip_version == AF_INET6) {
    fprintf(stderr, "IPv4 addresses can only be used if \"IpVersion\" == 4, skipping HNA.\n");
    olsr_startup_sleep(3);
  }
  else {
    PARSER_DEBUG_PRINTF("HNA IPv4 entry: %s/%s\n", $1->string, $2->string);

    if (inet_pton(AF_INET, $1->string, &ipaddr.v4) <= 0) {
      fprintf(stderr, "ihna4entry: Failed converting IP address %s\n", $1->string);
      YYABORT;
    }
    if (inet_pton(AF_INET, $2->string, &netmask.v4) <= 0) {
      fprintf(stderr, "ihna4entry: Failed converting IP address %s\n", $1->string);
      YYABORT;
    }

    /* check that the given IP address is actually a network address */
    if ((ipaddr.v4.s_addr & ~netmask.v4.s_addr) != 0) {
      fprintf(stderr, "ihna4entry: The ipaddress \"%s\" is not a network address!\n", $1->string);
      YYABORT;
    }

    /* Queue */
    ip_prefix_list_add(&olsr_cnf->hna_entries, &ipaddr, olsr_netmask_to_prefix(&netmask));
  }
  free($1->string);
  free($1);
  free($2->string);
  free($2);
}
        |       TOK_IPV4_ADDR TOK_SLASH TOK_INTEGER
{
  union olsr_ip_addr ipaddr, netmask;

  if (olsr_cnf->ip_version == AF_INET6) {
    fprintf(stderr, "IPv4 addresses can only be used if \"IpVersion\" == 4, skipping HNA.\n");
    olsr_startup_sleep(3);
  }
  else {
    PARSER_DEBUG_PRINTF("HNA IPv4 entry: %s/%d\n", $1->string, $3->integer);

    if (inet_pton(AF_INET, $1->string, &ipaddr.v4) <= 0) {
      fprintf(stderr, "ihna4entry: Failed converting IP address %s\n", $1->string);
      YYABORT;
    }
    if ($3->integer > olsr_cnf->maxplen) {
      fprintf(stderr, "ihna4entry: Prefix len %u > %d is not allowed!\n", $3->integer, olsr_cnf->maxplen);
      YYABORT;
    }

    /* check that the given IP address is actually a network address */
    olsr_prefix_to_netmask(&netmask, $3->integer);
    if ((ipaddr.v4.s_addr & ~netmask.v4.s_addr) != 0) {
      fprintf(stderr, "ihna4entry: The ipaddress \"%s\" is not a network address!\n", $1->string);
      YYABORT;
    }

    /* Queue */
    ip_prefix_list_add(&olsr_cnf->hna_entries, &ipaddr, $3->integer);
  }
  free($1->string);
  free($1);
  free($3);
}
;

ihna6entry:     TOK_IPV6_ADDR TOK_INTEGER
{
  if (add_ipv6_addr($1, $2)) {
    YYABORT;
  }
}
        |       TOK_IPV6_ADDR TOK_SLASH TOK_INTEGER
{
  if (add_ipv6_addr($1, $3)) {
    YYABORT;
  }
}
;

ifstart: TOK_INTERFACE
{
  PARSER_DEBUG_PRINTF("setting ifs_in_curr_cfg = 0\n");
  ifs_in_curr_cfg = 0;
}
;

ifnick: TOK_STRING
{
  struct olsr_if *in, *last;
  in = olsr_cnf->interfaces;
  last = NULL;
  while (in != NULL) {
    if (strcmp(in->name, $1->string) == 0) {
      free ($1->string);
      break;
    }
    last = in;
    in = in->next;
  }

  if (in != NULL) {
    /* remove old interface from list to add it later at the beginning */
    if (last) {
      last->next = in->next;
    }
    else {
      olsr_cnf->interfaces = in->next;
    }
  }
  else {
    in = malloc(sizeof(*in));
    if (in == NULL) {
      fprintf(stderr, "Out of memory(ADD IF)\n");
      YYABORT;
    }
    memset(in, 0, sizeof(*in));

    in->cnf = malloc(sizeof(*in->cnf));
    if (in->cnf == NULL) {
      fprintf(stderr, "Out of memory(ADD IFRULE)\n");
      free(in);
      YYABORT;
    }
    memset(in->cnf, 0x00, sizeof(*in->cnf));

    in->cnfi = malloc(sizeof(*in->cnfi));
    if (in->cnfi == NULL) {
      fprintf(stderr, "Out of memory(ADD IFRULE)\n");
      free (in->cnf);
      free(in);
      YYABORT;
    }
    memset(in->cnfi, 0xFF, sizeof(*in->cnfi));
    in->cnfi->orig_lq_mult_cnt=0;

    in->name = $1->string;
  }
  /* Queue */
  in->next = olsr_cnf->interfaces;
  olsr_cnf->interfaces = in;
  ifs_in_curr_cfg++;
  free($1);
}
;

bnoint: TOK_NOINT TOK_BOOLEAN
{
  PARSER_DEBUG_PRINTF("Noint set to %d\n", $2->boolean);
  olsr_cnf->allow_no_interfaces = $2->boolean;
  free($2);
}
;

atos: TOK_TOS TOK_INTEGER
{
  PARSER_DEBUG_PRINTF("TOS: %d\n", $2->integer);
  olsr_cnf->tos = $2->integer;
  free($2);

}
;

aolsrport: TOK_OLSRPORT TOK_INTEGER
{
  PARSER_DEBUG_PRINTF("OlsrPort: %d\n", $2->integer);
  olsr_cnf->olsrport = $2->integer;
  free($2);
}
;

irtproto: TOK_RTPROTO TOK_INTEGER
{
  PARSER_DEBUG_PRINTF("RtProto: %d\n", $2->integer);
  olsr_cnf->rt_proto = $2->integer;
  free($2);
}
;

irttable: TOK_RTTABLE TOK_INTEGER
{
  PARSER_DEBUG_PRINTF("RtTable: %d\n", $2->integer);
  olsr_cnf->rt_table = $2->integer;
  free($2);
}
       | TOK_RTTABLE TOK_AUTO
{
  PARSER_DEBUG_PRINTF("RtTable: auto\n");
  olsr_cnf->rt_table = DEF_RT_AUTO;
  free($2);
}
;

irttable_default: TOK_RTTABLE_DEFAULT TOK_INTEGER
{
  PARSER_DEBUG_PRINTF("RtTableDefault: %d\n", $2->integer);
  olsr_cnf->rt_table_default = $2->integer;
  free($2);
}
       | TOK_RTTABLE_DEFAULT TOK_AUTO
{
  PARSER_DEBUG_PRINTF("RtTableDefault: auto\n");
  olsr_cnf->rt_table_default = DEF_RT_AUTO;
  free($2);
}
;

irttable_tunnel: TOK_RTTABLE_TUNNEL TOK_INTEGER
{
  PARSER_DEBUG_PRINTF("RtTableTunnel: %d\n", $2->integer);
  olsr_cnf->rt_table_tunnel = $2->integer;
  free($2);
}
       | TOK_RTTABLE_TUNNEL TOK_AUTO
{
  PARSER_DEBUG_PRINTF("RtTableTunnel: auto\n");
  olsr_cnf->rt_table_tunnel = DEF_RT_AUTO;
  free($2);
}
;

irttable_priority: TOK_RTTABLE_PRIORITY TOK_INTEGER
{
  PARSER_DEBUG_PRINTF("RtTablePriority: %d\n", $2->integer);
  olsr_cnf->rt_table_pri = $2->integer;
  free($2);
}
        | TOK_RTTABLE_PRIORITY TOK_AUTO
{
  PARSER_DEBUG_PRINTF("RtTablePriority: auto\n");
  olsr_cnf->rt_table_pri = DEF_RT_AUTO;
  free($2);
}
        | TOK_RTTABLE_PRIORITY TOK_NONE
{
  PARSER_DEBUG_PRINTF("RtTablePriority: none\n");
  olsr_cnf->rt_table_pri = DEF_RT_NONE;
  free($2);
}
;

irttable_default_priority: TOK_RTTABLE_DEFAULT_PRIORITY TOK_INTEGER
{
  PARSER_DEBUG_PRINTF("RtTableDefaultPriority: %d\n", $2->integer);
  olsr_cnf->rt_table_default_pri = $2->integer;
  free($2);
}
        | TOK_RTTABLE_DEFAULT_PRIORITY TOK_AUTO
{
  PARSER_DEBUG_PRINTF("RtTableDefaultPriority: auto\n");
  olsr_cnf->rt_table_default_pri = DEF_RT_AUTO;
  free($2);
}
        | TOK_RTTABLE_DEFAULT_PRIORITY TOK_NONE
{
  PARSER_DEBUG_PRINTF("RtTableDefaultPriority: none\n");
  olsr_cnf->rt_table_default_pri = DEF_RT_NONE;
  free($2);
}
;

irttable_tunnel_priority: TOK_RTTABLE_TUNNEL_PRIORITY TOK_INTEGER
{
  PARSER_DEBUG_PRINTF("RtTableTunnelPriority: %d\n", $2->integer);
  olsr_cnf->rt_table_tunnel_pri = $2->integer;
  free($2);
}
        | TOK_RTTABLE_TUNNEL_PRIORITY TOK_AUTO
{
  PARSER_DEBUG_PRINTF("RtTableTunnelPriority: auto\n");
  olsr_cnf->rt_table_tunnel_pri = DEF_RT_AUTO;
  free($2);
}
        | TOK_RTTABLE_TUNNEL_PRIORITY TOK_NONE
{
  PARSER_DEBUG_PRINTF("RtTableTunnelPriority: none\n");
  olsr_cnf->rt_table_tunnel_pri = DEF_RT_NONE;
  free($2);
}
;

irttable_defaultolsr_priority: TOK_RTTABLE_DEFAULTOLSR_PRIORITY TOK_INTEGER
{
  PARSER_DEBUG_PRINTF("RtTableDefaultOlsrPriority: %d\n", $2->integer);
  olsr_cnf->rt_table_defaultolsr_pri = $2->integer;
  free($2);
}
        | TOK_RTTABLE_DEFAULTOLSR_PRIORITY TOK_AUTO
{
  PARSER_DEBUG_PRINTF("RtTableDefaultOlsrPriority: auto\n");
  olsr_cnf->rt_table_defaultolsr_pri = DEF_RT_AUTO;
  free($2);
}
        | TOK_RTTABLE_DEFAULTOLSR_PRIORITY TOK_NONE
{
  PARSER_DEBUG_PRINTF("RtTableDefaultOlsrPriority: none\n");
  olsr_cnf->rt_table_defaultolsr_pri = DEF_RT_NONE;
  free($2);
}
;

awillingness: TOK_WILLINGNESS TOK_INTEGER
{
  PARSER_DEBUG_PRINTF("Willingness: %d\n", $2->integer);
  olsr_cnf->willingness_auto = false;
  olsr_cnf->willingness = $2->integer;
  free($2);
}
;

busehyst: TOK_USEHYST TOK_BOOLEAN
{
  olsr_cnf->use_hysteresis = $2->boolean;
  PARSER_DEBUG_PRINTF("Hysteresis %s\n", olsr_cnf->use_hysteresis ? "enabled" : "disabled");
  free($2);
}
;

fhystscale: TOK_HYSTSCALE TOK_FLOAT
{
  olsr_cnf->hysteresis_param.scaling = $2->floating;
  PARSER_DEBUG_PRINTF("Hysteresis Scaling: %0.2f\n", (double)$2->floating);
  free($2);
}
;

fhystupper: TOK_HYSTUPPER TOK_FLOAT
{
  olsr_cnf->hysteresis_param.thr_high = $2->floating;
  PARSER_DEBUG_PRINTF("Hysteresis UpperThr: %0.2f\n", (double)$2->floating);
  free($2);
}
;

fhystlower: TOK_HYSTLOWER TOK_FLOAT
{
  olsr_cnf->hysteresis_param.thr_low = $2->floating;
  PARSER_DEBUG_PRINTF("Hysteresis LowerThr: %0.2f\n", (double)$2->floating);
  free($2);
}
;

fpollrate: TOK_POLLRATE TOK_FLOAT
{
  PARSER_DEBUG_PRINTF("Pollrate %0.2f\n", (double)$2->floating);
  olsr_cnf->pollrate = $2->floating;
  free($2);
}
;

fnicchgspollrt: TOK_NICCHGSPOLLRT TOK_FLOAT
{
  PARSER_DEBUG_PRINTF("NIC Changes Pollrate %0.2f\n", (double)$2->floating);
  olsr_cnf->nic_chgs_pollrate = $2->floating;
  free($2);
}
;

atcredundancy: TOK_TCREDUNDANCY TOK_INTEGER
{
  PARSER_DEBUG_PRINTF("TC redundancy %d\n", $2->integer);
  olsr_cnf->tc_redundancy = $2->integer;
  free($2);
}
;

amprcoverage: TOK_MPRCOVERAGE TOK_INTEGER
{
  PARSER_DEBUG_PRINTF("MPR coverage %d\n", $2->integer);
  olsr_cnf->mpr_coverage = $2->integer;
  free($2);
}
;

alq_level: TOK_LQ_LEVEL TOK_INTEGER
{
  PARSER_DEBUG_PRINTF("Link quality level %d\n", $2->integer);
  olsr_cnf->lq_level = $2->integer;
  free($2);
}
;

alq_fish: TOK_LQ_FISH TOK_INTEGER
{
  PARSER_DEBUG_PRINTF("Link quality fish eye %d\n", $2->integer);
  olsr_cnf->lq_fish = $2->integer;
  free($2);
}
;

alq_aging: TOK_LQ_AGING TOK_FLOAT
{
  PARSER_DEBUG_PRINTF("Link quality aging factor %f\n", (double)$2->floating);
  olsr_cnf->lq_aging = $2->floating;
  free($2);
}
;

amin_tc_vtime: TOK_MIN_TC_VTIME TOK_FLOAT
{
  PARSER_DEBUG_PRINTF("Minimum TC validity time %f\n", (double)$2->floating);
  olsr_cnf->min_tc_vtime = $2->floating;
  free($2);
}
;

alock_file: TOK_LOCK_FILE TOK_STRING
{
  PARSER_DEBUG_PRINTF("Lock file %s\n", $2->string);
  if (olsr_cnf->lock_file) free(olsr_cnf->lock_file);
  olsr_cnf->lock_file = $2->string;
  free($2);
}
;
alq_plugin: TOK_LQ_PLUGIN TOK_STRING
{
  if (olsr_cnf->lq_algorithm) free(olsr_cnf->lq_algorithm);
  olsr_cnf->lq_algorithm = $2->string;
  PARSER_DEBUG_PRINTF("LQ Algorithm: %s\n", $2->string);
  free($2);
}
;

anat_thresh: TOK_LQ_NAT_THRESH TOK_FLOAT
{
  PARSER_DEBUG_PRINTF("NAT threshold %0.2f\n", (double)$2->floating);
  olsr_cnf->lq_nat_thresh = $2->floating;
  free($2);
}
;

bclear_screen: TOK_CLEAR_SCREEN TOK_BOOLEAN
{
  PARSER_DEBUG_PRINTF("Clear screen %s\n", $2->boolean ? "enabled" : "disabled");
  olsr_cnf->clear_screen = $2->boolean;
  free($2);
}
;

suse_niit: TOK_USE_NIIT TOK_BOOLEAN
{
  PARSER_DEBUG_PRINTF("Use NIIT ip translation: %s\n", $2->boolean ? "enabled" : "disabled");
  olsr_cnf->use_niit = $2->boolean;
  free($2);
}
;

bsmart_gw: TOK_SMART_GW TOK_BOOLEAN
{
	PARSER_DEBUG_PRINTF("Smart gateway system: %s\n", $2->boolean ? "enabled" : "disabled");
	olsr_cnf->smart_gw_active = $2->boolean;
	free($2);
}
;

bsmart_gw_always_remove_server_tunnel: TOK_SMART_GW_ALWAYS_REMOVE_SERVER_TUNNEL TOK_BOOLEAN
{
	PARSER_DEBUG_PRINTF("Smart gateway always remove server tunnel: %s\n", $2->boolean ? "enabled" : "disabled");
	olsr_cnf->smart_gw_always_remove_server_tunnel = $2->boolean;
	free($2);
}
;

ismart_gw_use_count: TOK_SMART_GW_USE_COUNT TOK_INTEGER
{
  PARSER_DEBUG_PRINTF("Smart gateway use count: %d\n", $2->integer);
  olsr_cnf->smart_gw_use_count = $2->integer;
  free($2);
}
;

ismart_gw_takedown_percentage: TOK_SMART_GW_TAKEDOWN_PERCENTAGE TOK_INTEGER
{
  PARSER_DEBUG_PRINTF("Smart gateway takedown percentage: %d\n", $2->integer);
  olsr_cnf->smart_gw_takedown_percentage = $2->integer;
  free($2);
}
;

ssmart_gw_instance_id: TOK_SMART_GW_INSTANCE_ID TOK_STRING
{
  PARSER_DEBUG_PRINTF("Smart gateway instance id: %s\n", $2->string);
  if (olsr_cnf->smart_gw_instance_id) free(olsr_cnf->smart_gw_instance_id);
  olsr_cnf->smart_gw_instance_id = $2->string;
  free($2);
}
;

ssmart_gw_policyrouting_script: TOK_SMART_GW_POLICYROUTING_SCRIPT TOK_STRING
{
  PARSER_DEBUG_PRINTF("Smart gateway policy routing script: %s\n", $2->string);
  if (olsr_cnf->smart_gw_policyrouting_script) free(olsr_cnf->smart_gw_policyrouting_script);
  olsr_cnf->smart_gw_policyrouting_script = $2->string;
  free($2);
}
;

ssgw_egress_ifs:   TOK_SMART_GW_EGRESS_IFS sgw_egress_ifs
;

sgw_egress_ifs:   | sgw_egress_ifs sgw_egress_if
;

sgw_egress_if: TOK_STRING
{
  struct sgw_egress_if *in, *previous, *last;
  char * str = $1->string;
  char *end;

  /* Trim leading space */
  while(isspace(*str)) {
    str++;
  }

  /* Trim trailing space */
  end = &str[strlen(str) - 1];
  while((end > str) && isspace(*end)) {
    end--;
  }

  /* Write new null terminator */
  end[1] = '\0';

  if(*str == '\0') {
    PARSER_DEBUG_PRINTF("Smart gateway egress interface: <empty> (skipped)\n");
  } else {
    PARSER_DEBUG_PRINTF("Smart gateway egress interface: %s\n", str);

    in = olsr_cnf->smart_gw_egress_interfaces;
    previous = NULL;
    while (in != NULL) {
      if (strcmp(in->name, str) == 0) {
        free ($1->string);
        break;
      }
      previous = in;
      in = in->next;
    }

    if (in != NULL) {
      /* remove old interface from list to add it later at the end */
      if (previous) {
        previous->next = in->next;
      }
      else {
        olsr_cnf->smart_gw_egress_interfaces = in->next;
      }
      in->next = NULL;
    }
    else {
      /* interface in not in the list: create a new entry to add it later at the end */
      in = malloc(sizeof(*in));
      if (in == NULL) {
        fprintf(stderr, "Out of memory(ADD IF)\n");
        YYABORT;
      }
      memset(in, 0, sizeof(*in));

      in->name = strdup(str);
      free ($1->string);
    }

    last = olsr_cnf->smart_gw_egress_interfaces;
    while (last && last->next) {
      last = last->next;
    }

    /* Add to the end of the list */
    if (!last) {
      olsr_cnf->smart_gw_egress_interfaces = in;
    } else {
      last->next = in;
    }
    free($1);
  }
}
;

ssmart_gw_egress_file: TOK_SMART_GW_EGRESS_FILE TOK_STRING
{
  PARSER_DEBUG_PRINTF("Smart gateway egress file: %s\n", $2->string);
  if (olsr_cnf->smart_gw_egress_file) free(olsr_cnf->smart_gw_egress_file);
  olsr_cnf->smart_gw_egress_file = $2->string;
  free($2);
}
;

ismart_gw_egress_file_period: TOK_SMART_GW_EGRESS_FILE_PERIOD TOK_INTEGER
{
  PARSER_DEBUG_PRINTF("Smart gateway egress file period: %d\n", $2->integer);
  olsr_cnf->smart_gw_egress_file_period = $2->integer;
  free($2);
}
;

ssmart_gw_status_file: TOK_SMART_GW_STATUS_FILE TOK_STRING
{
  PARSER_DEBUG_PRINTF("Smart gateway status file: %s\n", $2->string);
  if (olsr_cnf->smart_gw_status_file) free(olsr_cnf->smart_gw_status_file);
  olsr_cnf->smart_gw_status_file = $2->string;
  free($2);
}
;

ismart_gw_offset_tables: TOK_SMART_GW_OFFSET_TABLES TOK_INTEGER
{
  PARSER_DEBUG_PRINTF("Smart gateway tables offset: %d\n", $2->integer);
  olsr_cnf->smart_gw_offset_tables = $2->integer;
  free($2);
}
;

ismart_gw_offset_rules: TOK_SMART_GW_OFFSET_RULES TOK_INTEGER
{
  PARSER_DEBUG_PRINTF("Smart gateway rules offset: %d\n", $2->integer);
  olsr_cnf->smart_gw_offset_rules = $2->integer;
  free($2);
}
;

bsmart_gw_allow_nat: TOK_SMART_GW_ALLOW_NAT TOK_BOOLEAN
{
	PARSER_DEBUG_PRINTF("Smart gateway allow client nat: %s\n", $2->boolean ? "yes" : "no");
	olsr_cnf->smart_gw_allow_nat = $2->boolean;
	free($2);
}
;

ismart_gw_period: TOK_SMART_GW_PERIOD TOK_INTEGER
{
  PARSER_DEBUG_PRINTF("Smart gateway period: %d\n", $2->integer);
  olsr_cnf->smart_gw_period = $2->integer;
  free($2);
}
;

asmart_gw_stablecount: TOK_SMART_GW_STABLECOUNT TOK_INTEGER
{
  PARSER_DEBUG_PRINTF("Smart gateway stablecount: %d\n", $2->integer);
  olsr_cnf->smart_gw_stablecount = $2->integer;
  free($2);
}
;

asmart_gw_thresh: TOK_SMART_GW_THRESH TOK_INTEGER
{
  PARSER_DEBUG_PRINTF("Smart gateway threshold: %d\n", $2->integer);
  olsr_cnf->smart_gw_thresh = $2->integer;
  free($2);
}
;

asmart_gw_weight_exitlink_up: TOK_SMART_GW_WEIGHT_EXITLINK_UP TOK_INTEGER
{
  PARSER_DEBUG_PRINTF("Smart gateway exitlink uplink weight: %d\n", $2->integer);
  olsr_cnf->smart_gw_weight_exitlink_up = $2->integer;
  free($2);
}
;

asmart_gw_weight_exitlink_down: TOK_SMART_GW_WEIGHT_EXITLINK_DOWN TOK_INTEGER
{
  PARSER_DEBUG_PRINTF("Smart gateway exitlink downlink weight: %d\n", $2->integer);
  olsr_cnf->smart_gw_weight_exitlink_down = $2->integer;
  free($2);
}
;

asmart_gw_weight_etx: TOK_SMART_GW_WEIGHT_ETX TOK_INTEGER
{
  PARSER_DEBUG_PRINTF("Smart gateway ETX weight: %d\n", $2->integer);
  olsr_cnf->smart_gw_weight_etx = $2->integer;
  free($2);
}
;

asmart_gw_divider_etx: TOK_SMART_GW_DIVIDER_ETX TOK_INTEGER
{
  PARSER_DEBUG_PRINTF("Smart gateway ETX divider: %d\n", $2->integer);
  olsr_cnf->smart_gw_divider_etx = $2->integer;
  free($2);
}
;

asmart_gw_divider_etx: TOK_SMART_GW_MAX_COST_MAX_ETX TOK_INTEGER
{
  PARSER_DEBUG_PRINTF("Smart gateway max cost max ETX: %d\n", $2->integer);
  olsr_cnf->smart_gw_path_max_cost_etx_max = $2->integer;
  free($2);
}
;

ssmart_gw_uplink: TOK_SMART_GW_UPLINK TOK_STRING
{
	PARSER_DEBUG_PRINTF("Smart gateway uplink: %s\n", $2->string);
	if (strcasecmp($2->string, GW_UPLINK_TXT[GW_UPLINK_NONE]) == 0) {
		olsr_cnf->smart_gw_type = GW_UPLINK_NONE;
	}
	else if (strcasecmp($2->string, GW_UPLINK_TXT[GW_UPLINK_IPV4]) == 0) {
		olsr_cnf->smart_gw_type = GW_UPLINK_IPV4;
	}
	else if (strcasecmp($2->string, GW_UPLINK_TXT[GW_UPLINK_IPV6]) == 0) {
		olsr_cnf->smart_gw_type = GW_UPLINK_IPV6;
	}
	else if (strcasecmp($2->string, GW_UPLINK_TXT[GW_UPLINK_IPV46]) == 0) {
		olsr_cnf->smart_gw_type = GW_UPLINK_IPV46;
	}
	else {
		fprintf(stderr, "Bad gateway uplink type: %s\n", $2->string);
		YYABORT;
	}
	free($2);
}
;

ismart_gw_speed: TOK_SMART_GW_SPEED TOK_INTEGER TOK_INTEGER
{
	PARSER_DEBUG_PRINTF("Smart gateway speed: %u uplink/%u downlink kbit/s\n", $2->integer, $3->integer);
	smartgw_set_uplink(olsr_cnf, $2->integer);
	smartgw_set_downlink(olsr_cnf, $3->integer);
	free($2);
	free($3);
}
;

bsmart_gw_uplink_nat: TOK_SMART_GW_UPLINK_NAT TOK_BOOLEAN
{
	PARSER_DEBUG_PRINTF("Smart gateway uplink nat: %s\n", $2->boolean ? "yes" : "no");
	olsr_cnf->smart_gw_uplink_nat = $2->boolean;
	free($2);
}
;

ismart_gw_prefix: TOK_SMART_GW_PREFIX TOK_IPV6_ADDR TOK_INTEGER
{
  PARSER_DEBUG_PRINTF("Smart gateway prefix: %s %u\n", $2->string, $3->integer);
	if (inet_pton(olsr_cnf->ip_version, $2->string, &olsr_cnf->smart_gw_prefix.prefix) == 0) {
	  fprintf(stderr, "Bad IP part of gateway prefix: %s\n", $2->string);
    YYABORT;
  }
	olsr_cnf->smart_gw_prefix.prefix_len = (uint8_t)$3->integer;
	
	free($2);
	free($3);
}
        |       TOK_SMART_GW_PREFIX TOK_IPV6_ADDR TOK_SLASH TOK_INTEGER
{
	PARSER_DEBUG_PRINTF("Smart gateway prefix: %s %u\n", $2->string, $4->integer);
	if (inet_pton(olsr_cnf->ip_version, $2->string, &olsr_cnf->smart_gw_prefix.prefix) == 0) {
	  fprintf(stderr, "Bad IP part of gateway prefix: %s\n", $2->string);
    YYABORT;
  }
	olsr_cnf->smart_gw_prefix.prefix_len = (uint8_t)$4->integer;
	
	free($2);
	free($4);
}
;

bsrc_ip_routes: TOK_SRC_IP_ROUTES TOK_BOOLEAN
{
	PARSER_DEBUG_PRINTF("Use originator for routes src-ip: %s\n", $2->boolean ? "yes" : "no");
	if (olsr_cnf->ip_version != AF_INET) {
          fprintf(stderr, "Source ip routes not possible with IPV6\n");
          YYABORT;
	}
	else olsr_cnf->use_src_ip_routes = $2->boolean;
	free($2);
}
;

amain_ip: TOK_MAIN_IP TOK_IPV4_ADDR
{
  PARSER_DEBUG_PRINTF("Fixed Main IP: %s\n", $2->string);
  
  if (olsr_cnf->ip_version != AF_INET
      || inet_pton(olsr_cnf->ip_version, $2->string, &olsr_cnf->main_addr) != 1) {
    fprintf(stderr, "Bad main IP: %s\n", $2->string);
    YYABORT;
  }
  else olsr_cnf->unicast_src_ip = olsr_cnf->main_addr;
  free($2);
}
        |       TOK_MAIN_IP TOK_IPV6_ADDR
{
  PARSER_DEBUG_PRINTF("Fixed Main IP: %s\n", $2->string);
  
  if (olsr_cnf->ip_version != AF_INET6
      || inet_pton(olsr_cnf->ip_version, $2->string, &olsr_cnf->main_addr) != 1) {
    fprintf(stderr, "Bad main IP: %s\n", $2->string);
    YYABORT;
  }
  free($2);
}
;

bset_ipforward: TOK_SET_IPFORWARD TOK_BOOLEAN
{
  PARSER_DEBUG_PRINTF("Set IP-Forward procfile variable: %s\n", $2->boolean ? "yes" : "no");
  olsr_cnf->set_ip_forward = $2->boolean;
  free($2);
}
;


plblock: TOK_PLUGIN TOK_STRING
{
  struct plugin_entry *pe, *last;
  
  pe = olsr_cnf->plugins;
  last = NULL;
  while (pe != NULL) {
    if (strcmp(pe->name, $2->string) == 0) {
      free ($2->string);
      break;
    }
    last = pe;
    pe = pe->next;
  }

  if (pe != NULL) {
    /* remove old plugin from list to add it later at the beginning */
    if (last) {
      last->next = pe->next;
    }
    else {
      olsr_cnf->plugins = pe->next;
    }
  }
  else {
    pe = malloc(sizeof(*pe));

    if (pe == NULL) {
      fprintf(stderr, "Out of memory(ADD PL)\n");
      YYABORT;
    }

    pe->name = $2->string;
    pe->params = NULL;

    PARSER_DEBUG_PRINTF("Plugin: %s\n", $2->string);
  }
  
  /* Queue */
  pe->next = olsr_cnf->plugins;
  olsr_cnf->plugins = pe;

  free($2);
}
;

plparam: TOK_PLPARAM TOK_STRING TOK_STRING
{
  struct plugin_param *pp = malloc(sizeof(*pp));
  char *p;
  
  if (pp == NULL) {
    fprintf(stderr, "Out of memory(ADD PP)\n");
    YYABORT;
  }
  
  PARSER_DEBUG_PRINTF("Plugin param key:\"%s\" val: \"%s\"\n", $2->string, $3->string);
  
  pp->key = $2->string;
  pp->value = $3->string;

  /* Lower-case the key */
  for (p = pp->key; *p; p++) {
    *p = tolower(*p);
  }

  /* Queue */
  pp->next = olsr_cnf->plugins->params;
  olsr_cnf->plugins->params = pp;

  free($2);
  free($3);
}
;

vcomment:       TOK_COMMENT
{
    //PARSER_DEBUG_PRINTF("Comment\n");
}
;



%%

void yyerror (const char *string)
{
  fprintf(stderr, "Config line %d: %s\n", current_line, string);
}
