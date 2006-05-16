%{

/*
 * The olsr.org Optimized Link-State Routing daemon(olsrd)
 * Copyright (c) 2004, Andreas Tønnesen(andreto@olsr.org)
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
 * $Id: oparse.y,v 1.28 2005/11/17 04:25:44 tlopatic Exp $
 */


#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>

#include "olsrd_conf.h"

#define PARSER_DEBUG 0

#define YYSTYPE struct conf_token *

void yyerror(char *);
int yylex(void);

static int ifs_in_curr_cfg = 0;

static int lq_mult_helper(YYSTYPE ip_addr_arg, YYSTYPE mult_arg);

static int lq_mult_helper(YYSTYPE ip_addr_arg, YYSTYPE mult_arg)
{
  union olsr_ip_addr addr;
  int i;
  struct olsr_if *walker;
  struct olsr_lq_mult *mult;

#if PARSER_DEBUG > 0
  printf("\tLinkQualityMult %s %0.2f\n",
         (ip_addr_arg != NULL) ? ip_addr_arg->string : "any",
         mult_arg->floating);
#endif

  memset(&addr, 0, sizeof (union olsr_ip_addr));

  if(ip_addr_arg != NULL &&
     inet_pton(cnf->ip_version, ip_addr_arg->string, &addr) < 0)
  {
    fprintf(stderr, "Cannot parse IP address %s.\n", ip_addr_arg->string);
    return -1;
  }

  walker = cnf->interfaces;

  for (i = 0; i < ifs_in_curr_cfg; i++)
  {
    mult = malloc(sizeof (struct olsr_lq_mult));

    if (mult == NULL)
    {
      fprintf(stderr, "Out of memory (LQ multiplier).\n");
      return -1;
    }

    memcpy(&mult->addr, &addr, sizeof (union olsr_ip_addr));
    mult->val = mult_arg->floating;

    mult->next = walker->cnf->lq_mult;
    walker->cnf->lq_mult = mult;

    walker = walker->next;
  }

  if (ip_addr_arg != NULL)
  {
    free(ip_addr_arg->string);
    free(ip_addr_arg);
  }

  free(mult_arg);

  return 0;
}
%}

%token TOK_OPEN
%token TOK_CLOSE
%token TOK_SEMI

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
%token TOK_WILLINGNESS
%token TOK_IPCCON
%token TOK_USEHYST
%token TOK_HYSTSCALE
%token TOK_HYSTUPPER
%token TOK_HYSTLOWER
%token TOK_POLLRATE
%token TOK_TCREDUNDANCY
%token TOK_MPRCOVERAGE
%token TOK_LQ_LEVEL
%token TOK_LQ_FISH
%token TOK_LQ_DLIMIT
%token TOK_LQ_WSIZE
%token TOK_LQ_MULT
%token TOK_CLEAR_SCREEN
%token TOK_PLNAME
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
          | bnoint
          | atos
          | awillingness
          | busehyst
          | fhystscale
          | fhystupper
          | fhystlower
          | fpollrate
          | atcredundancy
          | amprcoverage
          | alq_level
          | alq_fish
          | alq_dlimit
          | alq_wsize
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
  cnf->ipc_connections = $2->integer;

  cnf->open_ipc = cnf->ipc_connections ? OLSR_TRUE : OLSR_FALSE;

  free($2);
}
;

ipchost: TOK_HOSTLABEL TOK_IP4_ADDR
{
  struct in_addr in;
  struct ipc_host *ipch;

  if(PARSER_DEBUG) printf("\tIPC host: %s\n", $2->string);
  
  if(inet_aton($2->string, &in) == 0)
    {
      fprintf(stderr, "Failed converting IP address IPC %s\n", $2->string);
      return -1;
    }

  ipch = malloc(sizeof(struct ipc_host));
  ipch->host.v4 = in.s_addr;

  ipch->next = cnf->ipc_hosts;
  cnf->ipc_hosts = ipch;

  free($2->string);
  free($2);

}
;

ipcnet: TOK_NETLABEL TOK_IP4_ADDR TOK_IP4_ADDR
{
  struct in_addr in1, in2;
  struct ipc_net *ipcn;

  if(PARSER_DEBUG) printf("\tIPC net: %s/%s\n", $2->string, $3->string);
  
  if(inet_aton($2->string, &in1) == 0)
    {
      fprintf(stderr, "Failed converting IP net IPC %s\n", $2->string);
      return -1;
    }

  if(inet_aton($3->string, &in2) == 0)
    {
      fprintf(stderr, "Failed converting IP mask IPC %s\n", $3->string);
      return -1;
    }

  ipcn = malloc(sizeof(struct ipc_net));
  ipcn->net.v4 = in1.s_addr;
  ipcn->mask.v4 = in2.s_addr;

  ipcn->next = cnf->ipc_nets;
  cnf->ipc_nets = ipcn;

  free($2->string);
  free($2);
  free($3->string);
  free($3);

}
;

iifweight:       TOK_IFWEIGHT TOK_INTEGER
{
  int ifcnt = ifs_in_curr_cfg;
  struct olsr_if *ifs = cnf->interfaces;

  if(PARSER_DEBUG) printf("Fixed willingness: %d\n", $2->integer);

  while(ifcnt)
    {
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
  struct olsr_if *ifs = cnf->interfaces;

  if(PARSER_DEBUG) printf("\tIPv4 broadcast: %s\n", $2->string);

  if(inet_aton($2->string, &in) == 0)
    {
      fprintf(stderr, "Failed converting IP address %s\n", $2->string);
      return -1;
    }

  while(ifcnt)
    {
      ifs->cnf->ipv4_broadcast.v4 = in.s_addr;

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
  struct olsr_if *ifs = cnf->interfaces;

  if($2->boolean)
    {
      while(ifcnt)
	{
	  ifs->cnf->ipv6_addrtype = IPV6_ADDR_SITELOCAL;
	  
	  ifs = ifs->next;
	  ifcnt--;
	}
    }
  else
    {
      while(ifcnt)
	{
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
  struct olsr_if *ifs = cnf->interfaces;

  if(PARSER_DEBUG) printf("\tIPv6 site-local multicast: %s\n", $2->string);

  if(inet_pton(AF_INET6, $2->string, &in6) < 0)
    {
      fprintf(stderr, "Failed converting IP address %s\n", $2->string);
      return -1;
    }

  while(ifcnt)
    {
      memcpy(&ifs->cnf->ipv6_multi_site.v6, &in6, sizeof(struct in6_addr));
      
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
  struct olsr_if *ifs = cnf->interfaces;

  if(PARSER_DEBUG) printf("\tIPv6 global multicast: %s\n", $2->string);

  if(inet_pton(AF_INET6, $2->string, &in6) < 0)
    {
      fprintf(stderr, "Failed converting IP address %s\n", $2->string);
      return -1;
    }

  while(ifcnt)
    {
      memcpy(&ifs->cnf->ipv6_multi_glbl.v6, &in6, sizeof(struct in6_addr));
      
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
  struct olsr_if *ifs = cnf->interfaces;

  if(PARSER_DEBUG) printf("\tHELLO interval: %0.2f\n", $2->floating);

  while(ifcnt)
    {
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
  struct olsr_if *ifs = cnf->interfaces;

  if(PARSER_DEBUG) printf("\tHELLO validity: %0.2f\n", $2->floating);

  while(ifcnt)
    {
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
  struct olsr_if *ifs = cnf->interfaces;

  if(PARSER_DEBUG) printf("\tTC interval: %0.2f\n", $2->floating);

  while(ifcnt)
    {
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
  struct olsr_if *ifs = cnf->interfaces;
  
  if(PARSER_DEBUG) printf("\tTC validity: %0.2f\n", $2->floating);
  while(ifcnt)
    {
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
  struct olsr_if *ifs = cnf->interfaces;


  if(PARSER_DEBUG) printf("\tMID interval: %0.2f\n", $2->floating);
  while(ifcnt)
    {
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
  struct olsr_if *ifs = cnf->interfaces;

  if(PARSER_DEBUG) printf("\tMID validity: %0.2f\n", $2->floating);
  while(ifcnt)
    {
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
  struct olsr_if *ifs = cnf->interfaces;
  
  if(PARSER_DEBUG) printf("\tHNA interval: %0.2f\n", $2->floating);
  while(ifcnt)
    {
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
  struct olsr_if *ifs = cnf->interfaces;

  if(PARSER_DEBUG) printf("\tHNA validity: %0.2f\n", $2->floating);
  while(ifcnt)
    {
      ifs->cnf->hna_params.validity_time = $2->floating;
      
      ifs = ifs->next;
      ifcnt--;
    }

  free($2);
}
;

isetlqmult: TOK_LQ_MULT TOK_DEFAULT TOK_FLOAT
{
  if (lq_mult_helper($2, $3) < 0)
    YYABORT;
}

          | TOK_LQ_MULT TOK_IP4_ADDR TOK_FLOAT
{
  if (lq_mult_helper($2, $3) < 0)
    YYABORT;
}

          | TOK_LQ_MULT TOK_IP6_ADDR TOK_FLOAT
{
  if (lq_mult_helper($2, $3) < 0)
    YYABORT;
}

          ;

idebug:       TOK_DEBUGLEVEL TOK_INTEGER
{

  cnf->debug_level = $2->integer;
  if(PARSER_DEBUG) printf("Debug level: %d\n", cnf->debug_level);
  free($2);
}
;


iipversion:    TOK_IPVERSION TOK_INTEGER
{
  if($2->integer == 4)
    cnf->ip_version = AF_INET;
  else if($2->integer == 6)
    cnf->ip_version = AF_INET6;
  else
    {
      fprintf(stderr, "IPversion must be 4 or 6!\n");
      YYABORT;
    }

  if(PARSER_DEBUG) printf("IpVersion: %d\n", $2->integer);
  free($2);
}
;


ihna4entry:     TOK_IP4_ADDR TOK_IP4_ADDR
{
  struct hna4_entry *h = malloc(sizeof(struct hna4_entry));
  struct in_addr in;

  if(PARSER_DEBUG) printf("HNA IPv4 entry: %s/%s\n", $1->string, $2->string);

  if(h == NULL)
    {
      fprintf(stderr, "Out of memory(HNA4)\n");
      YYABORT;
    }

  if(inet_aton($1->string, &in) == 0)
    {
      fprintf(stderr, "Failed converting IP address %s\n", $1->string);
      return -1;
    }
  h->net.v4 = in.s_addr;
  if(inet_aton($2->string, &in) == 0)
    {
      fprintf(stderr, "Failed converting IP address %s\n", $1->string);
      return -1;
    }
  h->netmask.v4 = in.s_addr;
  /* Queue */
  h->next = cnf->hna4_entries;
  cnf->hna4_entries = h;

  free($1->string);
  free($1);
  free($2->string);
  free($2);

}
;

ihna6entry:     TOK_IP6_ADDR TOK_INTEGER
{
  struct hna6_entry *h = malloc(sizeof(struct hna6_entry));
  struct in6_addr in6;

  if(PARSER_DEBUG) printf("HNA IPv6 entry: %s/%d\n", $1->string, $2->integer);

  if(h == NULL)
    {
      fprintf(stderr, "Out of memory(HNA6)\n");
      YYABORT;
    }

  if(inet_pton(AF_INET6, $1->string, &in6) < 0)
    {
      fprintf(stderr, "Failed converting IP address %s\n", $1->string);
      return -1;
    }
  memcpy(&h->net, &in6, sizeof(struct in6_addr));

  if(($2->integer < 0) || ($2->integer > 128))
    {
      fprintf(stderr, "Illegal IPv6 prefix length %d\n", $2->integer);
      return -1;
    }

  h->prefix_len = $2->integer;
  /* Queue */
  h->next = cnf->hna6_entries;
  cnf->hna6_entries = h;

  free($1->string);
  free($1);
  free($2);

}
;

ifstart: TOK_INTERFACE
{
  if(PARSER_DEBUG) printf("setting ifs_in_curr_cfg = 0\n");
  ifs_in_curr_cfg = 0;
}
;

ifnick: TOK_STRING
{
  struct olsr_if *in = malloc(sizeof(struct olsr_if));
  
  if(in == NULL)
    {
      fprintf(stderr, "Out of memory(ADD IF)\n");
      YYABORT;
    }

  in->cnf = get_default_if_config();

  if(in->cnf == NULL)
    {
      fprintf(stderr, "Out of memory(ADD IFRULE)\n");
      YYABORT;
    }

  in->name = $1->string;

  /* Queue */
  in->next = cnf->interfaces;
  cnf->interfaces = in;

  ifs_in_curr_cfg++;

  free($1);
}
;

bnoint: TOK_NOINT TOK_BOOLEAN
{
  if(PARSER_DEBUG) printf("Noint set to %d\n", $2->boolean);

  cnf->allow_no_interfaces = $2->boolean;

  free($2);
}
;

atos: TOK_TOS TOK_INTEGER
{
  if(PARSER_DEBUG) printf("TOS: %d\n", $2->integer);
  cnf->tos = $2->integer;

  free($2);

}
;

awillingness: TOK_WILLINGNESS TOK_INTEGER
{
  cnf->willingness_auto = OLSR_FALSE;

  if(PARSER_DEBUG) printf("Willingness: %d\n", $2->integer);
  cnf->willingness = $2->integer;

  free($2);

}
;



busehyst: TOK_USEHYST TOK_BOOLEAN
{
  cnf->use_hysteresis = $2->boolean;
  if(cnf->use_hysteresis)
    {
      if(PARSER_DEBUG) printf("Hysteresis enabled\n");
    }
  else
    {
      if(PARSER_DEBUG) printf("Hysteresis disabled\n");
    }
  free($2);

}
;


fhystscale: TOK_HYSTSCALE TOK_FLOAT
{
  cnf->hysteresis_param.scaling = $2->floating;
  if(PARSER_DEBUG) printf("Hysteresis Scaling: %0.2f\n", $2->floating);
  free($2);
}
;


fhystupper: TOK_HYSTUPPER TOK_FLOAT
{
  cnf->hysteresis_param.thr_high = $2->floating;
  if(PARSER_DEBUG) printf("Hysteresis UpperThr: %0.2f\n", $2->floating);
  free($2);
}
;


fhystlower: TOK_HYSTLOWER TOK_FLOAT
{
  cnf->hysteresis_param.thr_low = $2->floating;
  if(PARSER_DEBUG) printf("Hysteresis LowerThr: %0.2f\n", $2->floating);
  free($2);
}
;

fpollrate: TOK_POLLRATE TOK_FLOAT
{
  if(PARSER_DEBUG) printf("Pollrate %0.2f\n", $2->floating);
  cnf->pollrate = $2->floating;

  free($2);
}
;


atcredundancy: TOK_TCREDUNDANCY TOK_INTEGER
{
  if(PARSER_DEBUG) printf("TC redundancy %d\n", $2->integer);
  cnf->tc_redundancy = $2->integer;
  free($2);
}
;

amprcoverage: TOK_MPRCOVERAGE TOK_INTEGER
{
  if(PARSER_DEBUG) printf("MPR coverage %d\n", $2->integer);
  cnf->mpr_coverage = $2->integer;
  free($2);
}
;

alq_level: TOK_LQ_LEVEL TOK_INTEGER
{
  if(PARSER_DEBUG) printf("Link quality level %d\n", $2->integer);
  cnf->lq_level = $2->integer;
  free($2);
}
;

alq_fish: TOK_LQ_FISH TOK_INTEGER
{
  if(PARSER_DEBUG) printf("Link quality fish eye %d\n", $2->integer);
  cnf->lq_fish = $2->integer;
  free($2);
}
;

alq_dlimit: TOK_LQ_DLIMIT TOK_INTEGER TOK_FLOAT
{
  if(PARSER_DEBUG) printf("Link quality dijkstra limit %d, %0.2f\n", $2->integer, $3->floating);
  cnf->lq_dlimit = $2->integer;
  cnf->lq_dinter = $3->floating;
  free($2);
}
;

alq_wsize: TOK_LQ_WSIZE TOK_INTEGER
{
  if(PARSER_DEBUG) printf("Link quality window size %d\n", $2->integer);
  cnf->lq_wsize = $2->integer;
  free($2);
}
;

bclear_screen: TOK_CLEAR_SCREEN TOK_BOOLEAN
{
  cnf->clear_screen = $2->boolean;

  if (PARSER_DEBUG)
    printf("Clear screen %s\n", cnf->clear_screen ? "enabled" : "disabled");

  free($2);
}
;

plblock: TOK_PLUGIN TOK_STRING
{
  struct plugin_entry *pe = malloc(sizeof(struct plugin_entry));
  
  if(pe == NULL)
    {
      fprintf(stderr, "Out of memory(ADD PL)\n");
      YYABORT;
    }

  pe->name = $2->string;

  pe->params = NULL;
  
  if(PARSER_DEBUG) printf("Plugin: %s\n", $2->string);

  /* Queue */
  pe->next = cnf->plugins;
  cnf->plugins = pe;

  free($2);
}
;

plparam: TOK_PLPARAM TOK_STRING TOK_STRING
{
  struct plugin_param *pp = malloc(sizeof(struct plugin_param));
  
  if(pp == NULL)
    {
      fprintf(stderr, "Out of memory(ADD PP)\n");
      YYABORT;
    }
  
  if(PARSER_DEBUG) printf("Plugin param key:\"%s\" val: \"%s\"\n", $2->string, $3->string);
  
  pp->key = $2->string;
  pp->value = $3->string;

  /* Queue */
  pp->next = cnf->plugins->params;
  cnf->plugins->params = pp;

  free($2);
  free($3);
}
;

vcomment:       TOK_COMMENT
{
    //if(PARSER_DEBUG) printf("Comment\n");
}
;



%%

void yyerror (char *string)
{
  fprintf(stderr, "Config line %d: %s\n", current_line, string);
}
