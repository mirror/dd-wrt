/*
 *	BIRD -- OSPF Configuration
 *
 *	(c) 1999--2004 Ondrej Filip <feela@network.cz>
 *
 *	Can be freely distributed and used under the terms of the GNU GPL.
 */

CF_HDR

#include "proto/ospf/ospf.h"

CF_DEFINES

#define OSPF_CFG ((struct ospf_config *) this_proto)
#define OSPF_PATT ((struct ospf_iface_patt *) this_ipatt)

static struct ospf_area_config *this_area;
static struct nbma_node *this_nbma;
static struct area_net_config *this_pref;
static struct ospf_stubnet_config *this_stubnet; 

#ifdef OSPFv2
static void
finish_iface_config(struct ospf_iface_patt *ip)
{
  ip->passwords = get_passwords();

  if ((ip->autype == OSPF_AUTH_CRYPT) && (ip->helloint < 5))
    log(L_WARN "Hello or poll interval less that 5 makes cryptographic authenication prone to replay attacks");

  if ((ip->autype == OSPF_AUTH_NONE) && (ip->passwords != NULL))
    log(L_WARN "Password option without authentication option does not make sense");
}
#endif

#ifdef OSPFv3
static void
finish_iface_config(struct ospf_iface_patt *ip)
{
  if ((ip->autype != OSPF_AUTH_NONE) || (get_passwords() != NULL))
    cf_error("Authentication not supported in OSPFv3");
}
#endif

CF_DECLS

CF_KEYWORDS(OSPF, AREA, OSPF_METRIC1, OSPF_METRIC2, OSPF_TAG, OSPF_ROUTER_ID)
CF_KEYWORDS(BROADCAST, NEIGHBORS, RFC1583COMPAT, STUB, TICK, COST, RETRANSMIT)
CF_KEYWORDS(HELLO, TRANSMIT, PRIORITY, DEAD, NONBROADCAST, POINTOPOINT, TYPE)
CF_KEYWORDS(NONE, SIMPLE, AUTHENTICATION, STRICT, CRYPTOGRAPHIC)
CF_KEYWORDS(ELIGIBLE, POLL, NETWORKS, HIDDEN, VIRTUAL, LINK)
CF_KEYWORDS(RX, BUFFER, LARGE, NORMAL, STUBNET, HIDDEN, SUMMARY)
CF_KEYWORDS(WAIT, DELAY, LSADB)

%type <t> opttext

CF_GRAMMAR

CF_ADDTO(proto, ospf_proto '}')

ospf_proto_start: proto_start OSPF {
     this_proto = proto_config_new(&proto_ospf, sizeof(struct ospf_config));
     this_proto->preference = DEF_PREF_OSPF;
     init_list(&OSPF_CFG->area_list);
     OSPF_CFG->rfc1583 = DEFAULT_RFC1583;
     OSPF_CFG->tick = DEFAULT_OSPFTICK;
  }
 ;

ospf_proto:
   ospf_proto_start proto_name '{'
 | ospf_proto ospf_proto_item ';'
 ;

ospf_proto_item:
   proto_item
 | RFC1583COMPAT bool { OSPF_CFG->rfc1583 = $2; }
 | TICK expr { OSPF_CFG->tick = $2 ; if($2<=0) cf_error("Tick must be greater than zero"); }
 | ospf_area '}'
 ;

ospf_area_start: AREA idval '{' {
  this_area = cfg_allocz(sizeof(struct ospf_area_config));
  add_tail(&OSPF_CFG->area_list, NODE this_area);
  this_area->areaid = $2;
  this_area->stub = 0;
  init_list(&this_area->patt_list);
  init_list(&this_area->vlink_list);
  init_list(&this_area->net_list);
  init_list(&this_area->stubnet_list);
 }
 ;

ospf_area: ospf_area_start ospf_area_opts
 ;

ospf_area_opts:
   /* empty */
 | ospf_area_opts ospf_area_item ';'
 ;

ospf_area_item:
   STUB COST expr { this_area->stub = $3 ; if($3<=0) cf_error("Stub cost must be greater than zero"); }
 | STUB bool {if($2) { if(!this_area->stub) this_area->stub=DEFAULT_STUB_COST;}else{ this_area->stub=0;}}
 | NETWORKS '{' pref_list '}'
 | STUBNET ospf_stubnet
 | INTERFACE ospf_iface
 | ospf_vlink
 ;

ospf_stubnet:
   ospf_stubnet_start '{' ospf_stubnet_opts '}'
 | ospf_stubnet_start
 ;

ospf_stubnet_start:
   prefix {
     this_stubnet = cfg_allocz(sizeof(struct ospf_stubnet_config));
     add_tail(&this_area->stubnet_list, NODE this_stubnet);
     this_stubnet->px = $1;
     this_stubnet->cost = COST_D;
   } 
 ;

ospf_stubnet_opts:
   /* empty */
 | ospf_stubnet_opts ospf_stubnet_item ';'
 ;

ospf_stubnet_item:
   HIDDEN bool { this_stubnet->hidden = $2; }
 | SUMMARY bool { this_stubnet->summary = $2; }
 | COST expr { this_stubnet->cost = $2; }
 ;

ospf_vlink:
   ospf_vlink_start '{' ospf_vlink_opts '}' { finish_iface_config(OSPF_PATT); }
 | ospf_vlink_start
 ;

ospf_vlink_opts:
   /* empty */
 | ospf_vlink_opts ospf_vlink_item ';'
 ;

ospf_vlink_item:
 | HELLO expr { OSPF_PATT->helloint = $2 ; if (($2<=0) || ($2>65535)) cf_error("Hello interval must be in range 1-65535"); }
 | RETRANSMIT expr { OSPF_PATT->rxmtint = $2 ; if ($2<=0) cf_error("Retransmit int must be greater than zero"); }
 | TRANSMIT DELAY expr { OSPF_PATT->inftransdelay = $3 ; if (($3<=0) || ($3>65535)) cf_error("Transmit delay must be in range 1-65535"); }
 | WAIT expr { OSPF_PATT->waitint = $2 ; }
 | DEAD expr { OSPF_PATT->dead = $2 ; if ($2<=1) cf_error("Dead interval must be greater than one"); }
 | DEAD COUNT expr { OSPF_PATT->deadc = $3 ; if ($3<=1) cf_error("Dead count must be greater than one"); }
 | AUTHENTICATION NONE { OSPF_PATT->autype = OSPF_AUTH_NONE ; }
 | AUTHENTICATION SIMPLE { OSPF_PATT->autype = OSPF_AUTH_SIMPLE ; }
 | AUTHENTICATION CRYPTOGRAPHIC { OSPF_PATT->autype = OSPF_AUTH_CRYPT ; }
 | password_list 
 ;

ospf_vlink_start: VIRTUAL LINK idval
 {
  if (this_area->areaid == 0) cf_error("Virtual link cannot be in backbone");
  this_ipatt = cfg_allocz(sizeof(struct ospf_iface_patt));
  add_tail(&this_area->vlink_list, NODE this_ipatt);
  init_list(&this_ipatt->ipn_list);
  OSPF_PATT->vid = $3;
  OSPF_PATT->helloint = HELLOINT_D;
  OSPF_PATT->rxmtint = RXMTINT_D;
  OSPF_PATT->inftransdelay = INFTRANSDELAY_D;
  OSPF_PATT->waitint = WAIT_DMH*HELLOINT_D;
  OSPF_PATT->deadc = DEADC_D;
  OSPF_PATT->dead = 0;
  OSPF_PATT->type = OSPF_IT_VLINK;
  init_list(&OSPF_PATT->nbma_list);
  OSPF_PATT->autype = OSPF_AUTH_NONE;
  reset_passwords();
 }
;

ospf_iface_item:
   COST expr { OSPF_PATT->cost = $2 ; if (($2<=0) || ($2>65535)) cf_error("Cost must be in range 1-65535"); }
 | HELLO expr { OSPF_PATT->helloint = $2 ; if (($2<=0) || ($2>65535)) cf_error("Hello interval must be in range 1-65535"); }
 | POLL expr { OSPF_PATT->pollint = $2 ; if ($2<=0) cf_error("Poll int must be greater than zero"); }
 | RETRANSMIT expr { OSPF_PATT->rxmtint = $2 ; if ($2<=0) cf_error("Retransmit int must be greater than zero"); }
 | TRANSMIT DELAY expr { OSPF_PATT->inftransdelay = $3 ; if (($3<=0) || ($3>65535)) cf_error("Transmit delay must be in range 1-65535"); }
 | PRIORITY expr { OSPF_PATT->priority = $2 ; if (($2<0) || ($2>255)) cf_error("Priority must be in range 0-255"); }
 | WAIT expr { OSPF_PATT->waitint = $2 ; }
 | DEAD expr { OSPF_PATT->dead = $2 ; if ($2<=1) cf_error("Dead interval must be greater than one"); }
 | DEAD COUNT expr { OSPF_PATT->deadc = $3 ; if ($3<=1) cf_error("Dead count must be greater than one"); }
 | TYPE BROADCAST { OSPF_PATT->type = OSPF_IT_BCAST ; }
 | TYPE NONBROADCAST { OSPF_PATT->type = OSPF_IT_NBMA ; }
 | TYPE POINTOPOINT { OSPF_PATT->type = OSPF_IT_PTP ; }
 | STRICT NONBROADCAST bool { OSPF_PATT->strictnbma = $3 ; }
 | STUB bool { OSPF_PATT->stub = $2 ; }
 | NEIGHBORS '{' ipa_list '}'
 | AUTHENTICATION NONE { OSPF_PATT->autype = OSPF_AUTH_NONE ; }
 | AUTHENTICATION SIMPLE { OSPF_PATT->autype = OSPF_AUTH_SIMPLE ; }
 | AUTHENTICATION CRYPTOGRAPHIC { OSPF_PATT->autype = OSPF_AUTH_CRYPT ; }
 | RX BUFFER LARGE { OSPF_PATT->rxbuf = OSPF_RXBUF_LARGE ; } 
 | RX BUFFER NORMAL { OSPF_PATT->rxbuf = OSPF_RXBUF_NORMAL ; } 
 | RX BUFFER expr { OSPF_PATT->rxbuf = $3 ; if ($3 < OSPF_RXBUF_MINSIZE) cf_error("Buffer size is too small") ; } 
 | password_list
 ;

pref_list:
 /* empty */
 | pref_list pref_item
 ;

pref_item:
   pref_el
 | pref_hid;

pref_el: prefix ';'
 {
   this_pref = cfg_allocz(sizeof(struct area_net_config));
   add_tail(&this_area->net_list, NODE this_pref);
   this_pref->px.addr = $1.addr;
   this_pref->px.len = $1.len;
 }
;

pref_hid: prefix HIDDEN ';'
 {
   this_pref = cfg_allocz(sizeof(struct area_net_config));
   add_tail(&this_area->net_list, NODE this_pref);
   this_pref->px.addr = $1.addr;
   this_pref->px.len = $1.len;
   this_pref->hidden = 1;
 }
;

ipa_list:
 /* empty */
 | ipa_list ipa_item
 ;

ipa_item:
    ipa_el
  | ipa_ne;
 
ipa_el: IPA ';'
 {
   this_nbma = cfg_allocz(sizeof(struct nbma_node));
   add_tail(&OSPF_PATT->nbma_list, NODE this_nbma);
   this_nbma->ip=$1;
   this_nbma->eligible=0;
 }
;

ipa_ne: IPA ELIGIBLE ';'
 {
   this_nbma = cfg_allocz(sizeof(struct nbma_node));
   add_tail(&OSPF_PATT->nbma_list, NODE this_nbma);
   this_nbma->ip=$1;
   this_nbma->eligible=1;
 }
;

 
ospf_iface_start:
 {
  this_ipatt = cfg_allocz(sizeof(struct ospf_iface_patt));
  add_tail(&this_area->patt_list, NODE this_ipatt);
  init_list(&this_ipatt->ipn_list);
  OSPF_PATT->cost = COST_D;
  OSPF_PATT->helloint = HELLOINT_D;
  OSPF_PATT->pollint = POLLINT_D;
  OSPF_PATT->rxmtint = RXMTINT_D;
  OSPF_PATT->inftransdelay = INFTRANSDELAY_D;
  OSPF_PATT->priority = PRIORITY_D;
  OSPF_PATT->waitint = WAIT_DMH*HELLOINT_D;
  OSPF_PATT->deadc = DEADC_D;
  OSPF_PATT->dead = 0;
  OSPF_PATT->type = OSPF_IT_UNDEF;
  OSPF_PATT->strictnbma = 0;
  OSPF_PATT->stub = 0;
  init_list(&OSPF_PATT->nbma_list);
  OSPF_PATT->autype = OSPF_AUTH_NONE;
  reset_passwords();
 }
;

ospf_iface_opts:
   /* empty */
 | ospf_iface_opts ospf_iface_item ';'
 ;

ospf_iface_opt_list:
   /* empty */
 | '{' ospf_iface_opts '}'
 ;

ospf_iface:
  ospf_iface_start iface_patt_list ospf_iface_opt_list { finish_iface_config(OSPF_PATT); }
 ;

opttext:
    TEXT
 | /* empty */ { $$ = NULL; }
 ;

CF_ADDTO(dynamic_attr, OSPF_METRIC1 { $$ = f_new_dynamic_attr(EAF_TYPE_INT | EAF_TEMP, T_INT, EA_OSPF_METRIC1); })
CF_ADDTO(dynamic_attr, OSPF_METRIC2 { $$ = f_new_dynamic_attr(EAF_TYPE_INT | EAF_TEMP, T_INT, EA_OSPF_METRIC2); })
CF_ADDTO(dynamic_attr, OSPF_TAG { $$ = f_new_dynamic_attr(EAF_TYPE_INT | EAF_TEMP, T_INT, EA_OSPF_TAG); })
CF_ADDTO(dynamic_attr, OSPF_ROUTER_ID { $$ = f_new_dynamic_attr(EAF_TYPE_ROUTER_ID | EAF_TEMP, T_QUAD, EA_OSPF_ROUTER_ID); })

CF_CLI(SHOW OSPF, optsym, [<name>], [[Show information about OSPF protocol]])
{ ospf_sh(proto_get_named($3, &proto_ospf)); };

CF_CLI(SHOW OSPF NEIGHBORS, optsym opttext, [<name>] [\"<interface>\"], [[Show information about OSPF neighbors]])
{ ospf_sh_neigh(proto_get_named($4, &proto_ospf), $5); };

CF_CLI(SHOW OSPF INTERFACE, optsym opttext, [<name>] [\"<interface>\"], [[Show information about interface]])
{ ospf_sh_iface(proto_get_named($4, &proto_ospf), $5); };

CF_CLI_HELP(SHOW OSPF TOPOLOGY, [all] [<name>], [[Show information about OSPF network topology]])

CF_CLI(SHOW OSPF TOPOLOGY, optsym opttext, [<name>], [[Show information about reachable OSPF network topology]])
{ ospf_sh_state(proto_get_named($4, &proto_ospf), 0, 1); };

CF_CLI(SHOW OSPF TOPOLOGY ALL, optsym opttext, [<name>], [[Show information about all OSPF network topology]])
{ ospf_sh_state(proto_get_named($5, &proto_ospf), 0, 0); };

CF_CLI_HELP(SHOW OSPF STATE, [all] [<name>], [[Show information about OSPF network state]])

CF_CLI(SHOW OSPF STATE, optsym opttext, [<name>], [[Show information about reachable OSPF network state]])
{ ospf_sh_state(proto_get_named($4, &proto_ospf), 1, 1); };

CF_CLI(SHOW OSPF STATE ALL, optsym opttext, [<name>], [[Show information about all OSPF network state]])
{ ospf_sh_state(proto_get_named($5, &proto_ospf), 1, 0); };

CF_CLI(SHOW OSPF LSADB, optsym opttext, [<name>], [[Show content of OSPF LSA database]])
{ ospf_sh_lsadb(proto_get_named($4, &proto_ospf)); };

CF_CODE

CF_END
