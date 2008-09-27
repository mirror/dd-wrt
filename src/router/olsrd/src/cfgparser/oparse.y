%{

/*
 * The olsr.org Optimized Link-State Routing daemon(olsrd)
 * Copyright (c) 2004, Andreas T�nnesen(andreto@olsr.org)
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
#include "../defs.h"
#include "../ipcalc.h"
#include "../net_olsr.h"
#include "../link_set.h"

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>

#define PARSER_DEBUG 0

#if PARSER_DEBUG
#define PARSER_DEBUG_PRINTF(x, ...)   printf(x, ##args)
#else
#define PARSER_DEBUG_PRINTF(x, ...)   do { } while (0)
#endif

#define YYSTYPE struct conf_token *

void yyerror(const char *);
int yylex(void);

static int ifs_in_curr_cfg = 0;

static int lq_mult_helper(YYSTYPE ip_addr_arg, YYSTYPE mult_arg);
static int add_ipv6_addr(YYSTYPE ipaddr_arg, YYSTYPE prefixlen_arg);

static int lq_mult_helper(YYSTYPE ip_addr_arg, YYSTYPE mult_arg)
{
  union olsr_ip_addr addr;
  int i;
  struct olsr_if *walker;

#if PARSER_DEBUG > 0
  printf("\tLinkQualityMult %s %0.2f\n",
         (ip_addr_arg != NULL) ? ip_addr_arg->string : "any",
         mult_arg->floating);
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
    mult->value = (olsr_u32_t)(mult_arg->floating * LINK_LOSS_MULTIPLIER);

    mult->next = walker->cnf->lq_mult;
    walker->cnf->lq_mult = mult;

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
    fprintf(stderr, "IPv6 addresses can only be used if \"IpVersion\" == 6\n");
    return 1;
  }

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

%token TOK_IP6TYPE

%token TOK_DEBUGLEVEL
%token TOK_IPVERSION
%token TOK_HNA4
%token TOK_HNA6
%token TOK_PLUGIN
%token TOK_INTERFACE
%token TOK_NOINT
%token TOK_TOS
%token TOK_RTTABLE
%token TOK_RTTABLE_DEFAULT
%token TOK_WILLINGNESS
%token TOK_IPCCON
%token TOK_FIBMETRIC
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
%token TOK_LQ_DLIMIT
%token TOK_LQ_WSIZE
%token TOK_LQ_AGING
%token TOK_LQ_PLUGIN
%token TOK_LQ_NAT_THRESH
%token TOK_LQ_MULT
%token TOK_CLEAR_SCREEN
%token TOK_PLPARAM

%token TOK_HOSTLABEL
%token TOK_NETLABEL
%token TOK_MAXIPC

%token TOK_IP4BROADCAST
%token TOK_IP6ADDRTYPE
%token TOK_IP6MULTISITE
%token TOK_IP6MULTIGLOBAL
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

%token TOK_IP4_ADDR
%token TOK_IP6_ADDR
%token TOK_DEFAULT

%token TOK_COMMENT

%%

conf:
          | conf block
          | conf stmt
;

stmt:       idebug
          | iipversion
          | fibmetric
          | bnoint
          | atos
          | arttable
          | arttable_default
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
          | alq_dlimit
          | anat_thresh
          | alq_wsize
          | alq_aging
          | bclear_screen
          | vcomment
;

block:      TOK_HNA4 hna4body
          | TOK_HNA6 hna6body
          | TOK_IPCCON ipcbody
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

ifstmts:   | ifstmts ifstmt
;

ifstmt:      vcomment
             | iifweight
             | isetip4br
             | isetip6addrt
             | isetip6mults
             | isetip6multg
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

imaxipc: TOK_MAXIPC TOK_INTEGER
{
  olsr_cnf->ipc_connections = $2->integer;
  free($2);
}
;

ipchost: TOK_HOSTLABEL TOK_IP4_ADDR
{
  union olsr_ip_addr ipaddr;
  PARSER_DEBUG_PRINTF("\tIPC host: %s\n", $2->string);
  
  if (inet_aton($2->string, &ipaddr.v4) == 0) {
    fprintf(stderr, "Failed converting IP address IPC %s\n", $2->string);
    YYABORT;
  }

  ip_prefix_list_add(&olsr_cnf->ipc_nets, &ipaddr, olsr_cnf->maxplen);

  free($2->string);
  free($2);
}
;

ipcnet: TOK_NETLABEL TOK_IP4_ADDR TOK_IP4_ADDR
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
        |       TOK_NETLABEL TOK_IP4_ADDR TOK_SLASH TOK_INTEGER
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
    ifs->cnf->weight.fixed = OLSR_TRUE;

    ifs = ifs->next;
    ifcnt--;
  }

  free($2);
}
;

isetip4br: TOK_IP4BROADCAST TOK_IP4_ADDR
{
  struct in_addr in;
  int ifcnt = ifs_in_curr_cfg;
  struct olsr_if *ifs = olsr_cnf->interfaces;

  PARSER_DEBUG_PRINTF("\tIPv4 broadcast: %s\n", $2->string);

  if (inet_aton($2->string, &in) == 0) {
    fprintf(stderr, "isetip4br: Failed converting IP address %s\n", $2->string);
    YYABORT;
  }

  while (ifcnt) {
    ifs->cnf->ipv4_broadcast.v4 = in;

    ifs = ifs->next;
    ifcnt--;
  }

  free($2->string);
  free($2);
}
;

isetip6addrt: TOK_IP6ADDRTYPE TOK_IP6TYPE
{
  int ifcnt = ifs_in_curr_cfg;
  struct olsr_if *ifs = olsr_cnf->interfaces;

  if ($2->boolean) {
    while (ifcnt) {
      ifs->cnf->ipv6_addrtype = IPV6_ADDR_SITELOCAL;
	  
      ifs = ifs->next;
      ifcnt--;
    }
  } else {
    while (ifcnt) {
      ifs->cnf->ipv6_addrtype = 0;
	  
      ifs = ifs->next;
      ifcnt--;
    }
  }

  free($2);
}
;

isetip6mults: TOK_IP6MULTISITE TOK_IP6_ADDR
{
  struct in6_addr in6;
  int ifcnt = ifs_in_curr_cfg;
  struct olsr_if *ifs = olsr_cnf->interfaces;

  PARSER_DEBUG_PRINTF("\tIPv6 site-local multicast: %s\n", $2->string);

  if (inet_pton(AF_INET6, $2->string, &in6) <= 0) {
    fprintf(stderr, "isetip6mults: Failed converting IP address %s\n", $2->string);
    YYABORT;
  }

  while (ifcnt) {
    ifs->cnf->ipv6_multi_site.v6 = in6;
      
    ifs = ifs->next;
    ifcnt--;
  }

  free($2->string);
  free($2);
}
;


isetip6multg: TOK_IP6MULTIGLOBAL TOK_IP6_ADDR
{
  struct in6_addr in6;
  int ifcnt = ifs_in_curr_cfg;
  struct olsr_if *ifs = olsr_cnf->interfaces;

  PARSER_DEBUG_PRINTF("\tIPv6 global multicast: %s\n", $2->string);

  if (inet_pton(AF_INET6, $2->string, &in6) <= 0) {
    fprintf(stderr, "isetip6multg: Failed converting IP address %s\n", $2->string);
    YYABORT;
  }

  while (ifcnt) {
    //memcpy(&ifs->cnf->ipv6_multi_glbl.v6, &in6, sizeof(struct in6_addr));
    ifs->cnf->ipv6_multi_glbl.v6 = in6;
      
    ifs = ifs->next;
    ifcnt--;
  }

  free($2->string);
  free($2);
}
;
isethelloint: TOK_HELLOINT TOK_FLOAT
{
  int ifcnt = ifs_in_curr_cfg;
  struct olsr_if *ifs = olsr_cnf->interfaces;

  PARSER_DEBUG_PRINTF("\tHELLO interval: %0.2f\n", $2->floating);

  while (ifcnt) {
    ifs->cnf->hello_params.emission_interval = $2->floating;
      
    ifs = ifs->next;
    ifcnt--;
  }

  free($2);
}
;
isethelloval: TOK_HELLOVAL TOK_FLOAT
{
  int ifcnt = ifs_in_curr_cfg;
  struct olsr_if *ifs = olsr_cnf->interfaces;

  PARSER_DEBUG_PRINTF("\tHELLO validity: %0.2f\n", $2->floating);

  while (ifcnt) {
    ifs->cnf->hello_params.validity_time = $2->floating;
      
    ifs = ifs->next;
    ifcnt--;
  }

  free($2);
}
;
isettcint: TOK_TCINT TOK_FLOAT
{
  int ifcnt = ifs_in_curr_cfg;
  struct olsr_if *ifs = olsr_cnf->interfaces;

  PARSER_DEBUG_PRINTF("\tTC interval: %0.2f\n", $2->floating);

  while (ifcnt) {
    ifs->cnf->tc_params.emission_interval = $2->floating;
      
    ifs = ifs->next;
    ifcnt--;
  }
  free($2);
}
;
isettcval: TOK_TCVAL TOK_FLOAT
{
  int ifcnt = ifs_in_curr_cfg;
  struct olsr_if *ifs = olsr_cnf->interfaces;
  
  PARSER_DEBUG_PRINTF("\tTC validity: %0.2f\n", $2->floating);
  while (ifcnt) {
    ifs->cnf->tc_params.validity_time = $2->floating;
      
    ifs = ifs->next;
    ifcnt--;
  }

  free($2);
}
;
isetmidint: TOK_MIDINT TOK_FLOAT
{
  int ifcnt = ifs_in_curr_cfg;
  struct olsr_if *ifs = olsr_cnf->interfaces;


  PARSER_DEBUG_PRINTF("\tMID interval: %0.2f\n", $2->floating);
  while (ifcnt) {
    ifs->cnf->mid_params.emission_interval = $2->floating;
      
    ifs = ifs->next;
    ifcnt--;
  }

  free($2);
}
;
isetmidval: TOK_MIDVAL TOK_FLOAT
{
  int ifcnt = ifs_in_curr_cfg;
  struct olsr_if *ifs = olsr_cnf->interfaces;

  PARSER_DEBUG_PRINTF("\tMID validity: %0.2f\n", $2->floating);
  while (ifcnt) {
    ifs->cnf->mid_params.validity_time = $2->floating;
      
    ifs = ifs->next;
    ifcnt--;
  }

  free($2);
}
;
isethnaint: TOK_HNAINT TOK_FLOAT
{
  int ifcnt = ifs_in_curr_cfg;
  struct olsr_if *ifs = olsr_cnf->interfaces;
  
  PARSER_DEBUG_PRINTF("\tHNA interval: %0.2f\n", $2->floating);
  while (ifcnt) {
    ifs->cnf->hna_params.emission_interval = $2->floating;
      
    ifs = ifs->next;
    ifcnt--;
  }

  free($2);
}
;
isethnaval: TOK_HNAVAL TOK_FLOAT
{
  int ifcnt = ifs_in_curr_cfg;
  struct olsr_if *ifs = olsr_cnf->interfaces;

  PARSER_DEBUG_PRINTF("\tHNA validity: %0.2f\n", $2->floating);
  while (ifcnt) {
    ifs->cnf->hna_params.validity_time = $2->floating;
      
    ifs = ifs->next;
    ifcnt--;
  }

  free($2);
}
;
isetautodetchg: TOK_AUTODETCHG TOK_BOOLEAN
{
  int ifcnt = ifs_in_curr_cfg;
  struct olsr_if *ifs = olsr_cnf->interfaces;

  PARSER_DEBUG_PRINTF("\tAutodetect changes: %s\n", $2->boolean ? "YES" : "NO");
  while (ifcnt) {
    ifs->cnf->autodetect_chg = $2->boolean;
      
    ifs = ifs->next;
    ifcnt--;
  }

  free($2);
}
;

isetlqmult: TOK_LQ_MULT TOK_DEFAULT TOK_FLOAT
{
  if (lq_mult_helper($2, $3) < 0) {
    YYABORT;
  }
}

          | TOK_LQ_MULT TOK_IP4_ADDR TOK_FLOAT
{
  if (lq_mult_helper($2, $3) < 0) {
    YYABORT;
  }
}

          | TOK_LQ_MULT TOK_IP6_ADDR TOK_FLOAT
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
  PARSER_DEBUG_PRINTF("FIBMetric: %d\n", $2->string);
  if (strcmp($2->string, CFG_FIBM_FLAT) == 0) {
      olsr_cnf->fib_metric = FIBM_FLAT;
  } else if (strcmp($2->string, CFG_FIBM_CORRECT) == 0) {
      olsr_cnf->fib_metric = FIBM_CORRECT;
  } else if (strcmp($2->string, CFG_FIBM_APPROX) == 0) {
      olsr_cnf->fib_metric = FIBM_APPROX;
  } else {
    fprintf(stderr, "FIBMetric must be \"%s\", \"%s\", or \"%s\"!\n", CFG_FIBM_FLAT, CFG_FIBM_CORRECT, CFG_FIBM_APPROX);
    YYABORT;
  }
  free($1);
  free($2->string);
  free($2);
}
;

ihna4entry:     TOK_IP4_ADDR TOK_IP4_ADDR
{
  union olsr_ip_addr ipaddr, netmask;

  PARSER_DEBUG_PRINTF("HNA IPv4 entry: %s/%s\n", $1->string, $2->string);

  if (olsr_cnf->ip_version != AF_INET) {
    fprintf(stderr, "IPv4 addresses can only be used if \"IpVersion\" == 4\n");
    YYABORT;
  }

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

  free($1->string);
  free($1);
  free($2->string);
  free($2);
}
        |       TOK_IP4_ADDR TOK_SLASH TOK_INTEGER
{
  union olsr_ip_addr ipaddr, netmask;

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

  free($1->string);
  free($1);
  free($3);
}
;

ihna6entry:     TOK_IP6_ADDR TOK_INTEGER
{
  if (add_ipv6_addr($1, $2)) {
    YYABORT;
  }
}
        |       TOK_IP6_ADDR TOK_SLASH TOK_INTEGER
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
  struct olsr_if *in = malloc(sizeof(*in));
  
  if (in == NULL) {
    fprintf(stderr, "Out of memory(ADD IF)\n");
    YYABORT;
  }

  in->cnf = get_default_if_config();

  if (in->cnf == NULL) {
    fprintf(stderr, "Out of memory(ADD IFRULE)\n");
    YYABORT;
  }

  in->name = $1->string;

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

arttable: TOK_RTTABLE TOK_INTEGER
{
  PARSER_DEBUG_PRINTF("RtTable: %d\n", $2->integer);
  olsr_cnf->rttable = $2->integer;
  free($2);
}
;

arttable_default: TOK_RTTABLE_DEFAULT TOK_INTEGER
{
  PARSER_DEBUG_PRINTF("RtTableDefault: %d\n", $2->integer);
  olsr_cnf->rttable_default = $2->integer;
  free($2);
}
;

awillingness: TOK_WILLINGNESS TOK_INTEGER
{
  PARSER_DEBUG_PRINTF("Willingness: %d\n", $2->integer);
  olsr_cnf->willingness_auto = OLSR_FALSE;
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
  PARSER_DEBUG_PRINTF("Hysteresis Scaling: %0.2f\n", $2->floating);
  free($2);
}
;

fhystupper: TOK_HYSTUPPER TOK_FLOAT
{
  olsr_cnf->hysteresis_param.thr_high = $2->floating;
  PARSER_DEBUG_PRINTF("Hysteresis UpperThr: %0.2f\n", $2->floating);
  free($2);
}
;

fhystlower: TOK_HYSTLOWER TOK_FLOAT
{
  olsr_cnf->hysteresis_param.thr_low = $2->floating;
  PARSER_DEBUG_PRINTF("Hysteresis LowerThr: %0.2f\n", $2->floating);
  free($2);
}
;

fpollrate: TOK_POLLRATE TOK_FLOAT
{
  PARSER_DEBUG_PRINTF("Pollrate %0.2f\n", $2->floating);
  olsr_cnf->pollrate = $2->floating;
  free($2);
}
;

fnicchgspollrt: TOK_NICCHGSPOLLRT TOK_FLOAT
{
  PARSER_DEBUG_PRINTF("NIC Changes Pollrate %0.2f\n", $2->floating);
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

alq_dlimit: TOK_LQ_DLIMIT TOK_INTEGER TOK_FLOAT
{
  PARSER_DEBUG_PRINTF("Link quality dijkstra limit %d, %0.2f\n", $2->integer, $3->floating);
  olsr_cnf->lq_dlimit = $2->integer;
  olsr_cnf->lq_dinter = $3->floating;
  free($2);
}
;

alq_wsize: TOK_LQ_WSIZE TOK_INTEGER
{
  free($2);
}
;

alq_aging: TOK_LQ_AGING TOK_FLOAT
{
  PARSER_DEBUG_PRINTF("Link quality aging factor %f\n", $2->floating);
  olsr_cnf->lq_aging = $2->floating;
  free($2);
}
;

alq_plugin: TOK_LQ_PLUGIN TOK_STRING
{
  olsr_cnf->lq_algorithm = $2->string;
  PARSER_DEBUG_PRINTF("LQ Algorithm: %s\n", $2->string);
  free($2);
}
;

anat_thresh: TOK_LQ_NAT_THRESH TOK_FLOAT
{
  PARSER_DEBUG_PRINTF("NAT threshold %0.2f\n", $2->floating);
  olsr_cnf->lq_nat_thresh = $2->floating;
  free($2);
}
;

bclear_screen: TOK_CLEAR_SCREEN TOK_BOOLEAN
{
  PARSER_DEBUG_PRINTF("Clear screen %s\n", olsr_cnf->clear_screen ? "enabled" : "disabled");
  olsr_cnf->clear_screen = $2->boolean;
  free($2);
}
;

plblock: TOK_PLUGIN TOK_STRING
{
  struct plugin_entry *pe = malloc(sizeof(*pe));
  
  if (pe == NULL) {
    fprintf(stderr, "Out of memory(ADD PL)\n");
    YYABORT;
  }

  pe->name = $2->string;
  pe->params = NULL;
  
  PARSER_DEBUG_PRINTF("Plugin: %s\n", $2->string);

  /* Queue */
  pe->next = olsr_cnf->plugins;
  olsr_cnf->plugins = pe;

  free($2);
}
;

plparam: TOK_PLPARAM TOK_STRING TOK_STRING
{
  struct plugin_param *pp = malloc(sizeof(*pp));
  
  if (pp == NULL) {
    fprintf(stderr, "Out of memory(ADD PP)\n");
    YYABORT;
  }
  
  PARSER_DEBUG_PRINTF("Plugin param key:\"%s\" val: \"%s\"\n", $2->string, $3->string);
  
  pp->key = $2->string;
  pp->value = $3->string;

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
