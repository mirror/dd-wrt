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
static list *this_nets;
static struct area_net_config *this_pref;
static struct ospf_stubnet_config *this_stubnet;

static inline int ospf_cfg_is_v2(void) { return OSPF_CFG->ospf2; }
static inline int ospf_cfg_is_v3(void) { return ! OSPF_CFG->ospf2; }

static void
ospf_iface_finish(void)
{
  struct ospf_iface_patt *ip = OSPF_PATT;

  if (ip->deadint == 0)
    ip->deadint = ip->deadc * ip->helloint;

  if (ip->waitint == 0)
    ip->waitint = ip->deadc * ip->helloint;

  ip->passwords = get_passwords();

  if ((ip->autype == OSPF_AUTH_CRYPT) && (ip->helloint < 5))
    log(L_WARN "Hello or poll interval less that 5 makes cryptographic authenication prone to replay attacks");

  if ((ip->autype == OSPF_AUTH_NONE) && (ip->passwords != NULL))
    log(L_WARN "Password option without authentication option does not make sense");

  if (ip->passwords)
  {
    struct password_item *pass;
    WALK_LIST(pass, *ip->passwords)
    {
      if (pass->alg && (ip->autype != OSPF_AUTH_CRYPT))
	cf_error("Password algorithm option requires cryptographic authentication");

      /* Set default OSPF crypto algorithms */
      if (!pass->alg && (ip->autype == OSPF_AUTH_CRYPT))
	pass->alg = ospf_cfg_is_v2() ? ALG_MD5 : ALG_HMAC_SHA256;
    }
  }
}

static void
ospf_area_finish(void)
{
  if ((this_area->areaid == 0) && (this_area->type != OPT_E))
    cf_error("Backbone area cannot be stub/NSSA");

  if (this_area->summary && (this_area->type == OPT_E))
    cf_error("Only stub/NSSA areas can use summary propagation");

  if (this_area->default_nssa && ((this_area->type != OPT_N) || ! this_area->summary))
    cf_error("Only NSSA areas with summary propagation can use NSSA default route");

  if ((this_area->default_cost & LSA_EXT3_EBIT) && ! this_area->default_nssa)
    cf_error("Only NSSA default route can use type 2 metric");
}

static void
ospf_proto_finish(void)
{
  struct ospf_config *cf = OSPF_CFG;

  if (EMPTY_LIST(cf->area_list))
    cf_error( "No configured areas in OSPF");

  int areano = 0;
  int backbone = 0;
  int nssa = 0;
  struct ospf_area_config *ac;
  WALK_LIST(ac, cf->area_list)
  {
    areano++;
    if (ac->areaid == 0)
      backbone = 1;
    if (ac->type == OPT_N)
      nssa = 1;
  }

  cf->abr = areano > 1;

  /* Route export or NSSA translation (RFC 3101 3.1) */
  cf->asbr = (this_proto->out_filter != FILTER_REJECT) || (nssa && cf->abr);

  if (cf->abr && !backbone)
  {
    struct ospf_area_config *ac = cfg_allocz(sizeof(struct ospf_area_config));
    ac->type = OPT_E; /* Backbone is non-stub */
    add_head(&cf->area_list, NODE ac);
    init_list(&ac->patt_list);
    init_list(&ac->net_list);
    init_list(&ac->enet_list);
    init_list(&ac->stubnet_list);
  }

  if (!cf->abr && !EMPTY_LIST(cf->vlink_list))
    cf_error("Vlinks cannot be used on single area router");

  if (cf->asbr && (areano == 1) && (this_area->type == 0))
    cf_error("ASBR must be in non-stub area");
}

static inline void
ospf_check_defcost(int cost)
{
  if ((cost <= 0) || (cost >= LSINFINITY))
   cf_error("Default cost must be in range 1-%d", LSINFINITY-1);
}

static inline void
ospf_check_auth(void)
{
  if (ospf_cfg_is_v3())
    cf_error("Authentication not supported in OSPFv3");
}


CF_DECLS

CF_KEYWORDS(OSPF, AREA, OSPF_METRIC1, OSPF_METRIC2, OSPF_TAG, OSPF_ROUTER_ID)
CF_KEYWORDS(NEIGHBORS, RFC1583COMPAT, STUB, TICK, COST, COST2, RETRANSMIT)
CF_KEYWORDS(HELLO, TRANSMIT, PRIORITY, DEAD, TYPE, BROADCAST, BCAST, DEFAULT)
CF_KEYWORDS(NONBROADCAST, NBMA, POINTOPOINT, PTP, POINTOMULTIPOINT, PTMP)
CF_KEYWORDS(NONE, SIMPLE, AUTHENTICATION, STRICT, CRYPTOGRAPHIC, TTL, SECURITY)
CF_KEYWORDS(ELIGIBLE, POLL, NETWORKS, HIDDEN, VIRTUAL, CHECK, LINK, ONLY, BFD)
CF_KEYWORDS(RX, BUFFER, LARGE, NORMAL, STUBNET, HIDDEN, SUMMARY, TAG, EXTERNAL)
CF_KEYWORDS(WAIT, DELAY, LSADB, ECMP, LIMIT, WEIGHT, NSSA, TRANSLATOR, STABILITY)
CF_KEYWORDS(GLOBAL, LSID, ROUTER, SELF, INSTANCE, REAL, NETMASK, TX, PRIORITY, LENGTH)
CF_KEYWORDS(SECONDARY, MERGE, LSA, SUPPRESSION)

%type <t> opttext
%type <ld> lsadb_args
%type <i> nbma_eligible

CF_GRAMMAR

CF_ADDTO(proto, ospf_proto '}' { ospf_proto_finish(); } )

ospf_proto_start: proto_start OSPF {
     this_proto = proto_config_new(&proto_ospf, $1);
     init_list(&OSPF_CFG->area_list);
     init_list(&OSPF_CFG->vlink_list);
     OSPF_CFG->tick = OSPF_DEFAULT_TICK;
     OSPF_CFG->ospf2 = OSPF_IS_V2;
  }
 ;

ospf_proto:
   ospf_proto_start proto_name '{'
 | ospf_proto ospf_proto_item ';'
 ;

ospf_proto_item:
   proto_item
 | RFC1583COMPAT bool { OSPF_CFG->rfc1583 = $2; }
 | STUB ROUTER bool { OSPF_CFG->stub_router = $3; }
 | ECMP bool { OSPF_CFG->ecmp = $2 ? OSPF_DEFAULT_ECMP_LIMIT : 0; }
 | ECMP bool LIMIT expr { OSPF_CFG->ecmp = $2 ? $4 : 0; if ($4 < 0) cf_error("ECMP limit cannot be negative"); }
 | MERGE EXTERNAL bool { OSPF_CFG->merge_external = $3; }
 | TICK expr { OSPF_CFG->tick = $2; if($2<=0) cf_error("Tick must be greater than zero"); }
 | INSTANCE ID expr { OSPF_CFG->instance_id = $3; if (($3<0) || ($3>255)) cf_error("Instance ID must be in range 0-255"); }
 | ospf_area
 ;

ospf_area_start: AREA idval {
  this_area = cfg_allocz(sizeof(struct ospf_area_config));
  add_tail(&OSPF_CFG->area_list, NODE this_area);
  this_area->areaid = $2;
  this_area->default_cost = OSPF_DEFAULT_STUB_COST;
  this_area->type = OPT_E;
  this_area->transint = OSPF_DEFAULT_TRANSINT;

  init_list(&this_area->patt_list);
  init_list(&this_area->net_list);
  init_list(&this_area->enet_list);
  init_list(&this_area->stubnet_list);
 }
 ;

ospf_area: ospf_area_start '{' ospf_area_opts '}' { ospf_area_finish(); }
 ;

ospf_area_opts:
   /* empty */
 | ospf_area_opts ospf_area_item ';'
 ;

ospf_area_item:
   STUB bool { this_area->type = $2 ? 0 : OPT_E; /* We should remove the option */ }
 | NSSA { this_area->type = OPT_N; }
 | SUMMARY bool { this_area->summary = $2; }
 | DEFAULT NSSA bool { this_area->default_nssa = $3; }
 | DEFAULT COST expr { this_area->default_cost = $3; ospf_check_defcost($3); }
 | DEFAULT COST2 expr { this_area->default_cost = $3 | LSA_EXT3_EBIT; ospf_check_defcost($3); }
 | STUB COST expr { this_area->default_cost = $3; ospf_check_defcost($3); }
 | TRANSLATOR bool { this_area->translator = $2; }
 | TRANSLATOR STABILITY expr { this_area->transint = $3; }
 | NETWORKS { this_nets = &this_area->net_list; } '{' pref_list '}'
 | EXTERNAL { this_nets = &this_area->enet_list; } '{' pref_list '}'
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
   ospf_vlink_start ospf_instance_id '{' ospf_vlink_opts '}' { ospf_iface_finish(); }
 | ospf_vlink_start ospf_instance_id { ospf_iface_finish(); }
 ;

ospf_vlink_opts:
   /* empty */
 | ospf_vlink_opts ospf_vlink_item ';'
 ;

ospf_vlink_item:
 | HELLO expr { OSPF_PATT->helloint = $2 ; if (($2<=0) || ($2>65535)) cf_error("Hello interval must be in range 1-65535"); }
 | RETRANSMIT expr { OSPF_PATT->rxmtint = $2 ; if ($2<=1) cf_error("Retransmit int must be greater than one"); }
 | TRANSMIT DELAY expr { OSPF_PATT->inftransdelay = $3 ; if (($3<=0) || ($3>65535)) cf_error("Transmit delay must be in range 1-65535"); }
 | WAIT expr { OSPF_PATT->waitint = $2 ; if ($2<=1) cf_error("Wait interval must be greater than one"); }
 | DEAD expr { OSPF_PATT->deadint = $2 ; if ($2<=1) cf_error("Dead interval must be greater than one"); }
 | DEAD COUNT expr { OSPF_PATT->deadc = $3 ; if ($3<=1) cf_error("Dead count must be greater than one"); }
 | AUTHENTICATION NONE { OSPF_PATT->autype = OSPF_AUTH_NONE;  }
 | AUTHENTICATION SIMPLE { OSPF_PATT->autype = OSPF_AUTH_SIMPLE; ospf_check_auth(); }
 | AUTHENTICATION CRYPTOGRAPHIC { OSPF_PATT->autype = OSPF_AUTH_CRYPT; ospf_check_auth(); }
 | password_list { ospf_check_auth(); }
 ;

ospf_vlink_start: VIRTUAL LINK idval
 {
  if (this_area->areaid == 0) cf_error("Virtual link cannot be in backbone");
  this_ipatt = cfg_allocz(sizeof(struct ospf_iface_patt));
  add_tail(&OSPF_CFG->vlink_list, NODE this_ipatt);
  init_list(&this_ipatt->ipn_list);
  OSPF_PATT->voa = this_area->areaid;
  OSPF_PATT->vid = $3;
  OSPF_PATT->helloint = HELLOINT_D;
  OSPF_PATT->rxmtint = RXMTINT_D;
  OSPF_PATT->inftransdelay = INFTRANSDELAY_D;
  OSPF_PATT->deadc = DEADC_D;
  OSPF_PATT->type = OSPF_IT_VLINK;
  OSPF_PATT->instance_id = OSPF_CFG->instance_id;
  init_list(&OSPF_PATT->nbma_list);
  reset_passwords();
 }
;

ospf_iface_item:
   COST expr { OSPF_PATT->cost = $2 ; if (($2<=0) || ($2>65535)) cf_error("Cost must be in range 1-65535"); }
 | HELLO expr { OSPF_PATT->helloint = $2 ; if (($2<=0) || ($2>65535)) cf_error("Hello interval must be in range 1-65535"); }
 | POLL expr { OSPF_PATT->pollint = $2 ; if ($2<=0) cf_error("Poll int must be greater than zero"); }
 | RETRANSMIT expr { OSPF_PATT->rxmtint = $2 ; if ($2<=1) cf_error("Retransmit int must be greater than one"); }
 | WAIT expr { OSPF_PATT->waitint = $2 ; if ($2<=1) cf_error("Wait interval must be greater than one"); }
 | DEAD expr { OSPF_PATT->deadint = $2 ; if ($2<=1) cf_error("Dead interval must be greater than one"); }
 | DEAD COUNT expr { OSPF_PATT->deadc = $3 ; if ($3<=1) cf_error("Dead count must be greater than one"); }
 | TYPE BROADCAST { OSPF_PATT->type = OSPF_IT_BCAST ; }
 | TYPE BCAST { OSPF_PATT->type = OSPF_IT_BCAST ; }
 | TYPE NONBROADCAST { OSPF_PATT->type = OSPF_IT_NBMA ; }
 | TYPE NBMA { OSPF_PATT->type = OSPF_IT_NBMA ; }
 | TYPE POINTOPOINT { OSPF_PATT->type = OSPF_IT_PTP ; }
 | TYPE PTP { OSPF_PATT->type = OSPF_IT_PTP ; }
 | TYPE POINTOMULTIPOINT { OSPF_PATT->type = OSPF_IT_PTMP ; }
 | TYPE PTMP { OSPF_PATT->type = OSPF_IT_PTMP ; }
 | REAL BROADCAST bool { OSPF_PATT->real_bcast = $3; if (!ospf_cfg_is_v2()) cf_error("Real broadcast option requires OSPFv2"); }
 | PTP NETMASK bool { OSPF_PATT->ptp_netmask = $3; if (!ospf_cfg_is_v2()) cf_error("PtP netmask option requires OSPFv2"); }
 | TRANSMIT DELAY expr { OSPF_PATT->inftransdelay = $3 ; if (($3<=0) || ($3>65535)) cf_error("Transmit delay must be in range 1-65535"); }
 | PRIORITY expr { OSPF_PATT->priority = $2 ; if (($2<0) || ($2>255)) cf_error("Priority must be in range 0-255"); }
 | STRICT NONBROADCAST bool { OSPF_PATT->strictnbma = $3 ; }
 | STUB bool { OSPF_PATT->stub = $2 ; }
 | CHECK LINK bool { OSPF_PATT->check_link = $3; }
 | ECMP WEIGHT expr { OSPF_PATT->ecmp_weight = $3 - 1; if (($3<1) || ($3>256)) cf_error("ECMP weight must be in range 1-256"); }
 | LINK LSA SUPPRESSION bool { OSPF_PATT->link_lsa_suppression = $4; if (!ospf_cfg_is_v3()) cf_error("Link LSA suppression option requires OSPFv3"); }
 | NEIGHBORS '{' nbma_list '}'
 | AUTHENTICATION NONE { OSPF_PATT->autype = OSPF_AUTH_NONE; }
 | AUTHENTICATION SIMPLE { OSPF_PATT->autype = OSPF_AUTH_SIMPLE; ospf_check_auth(); }
 | AUTHENTICATION CRYPTOGRAPHIC { OSPF_PATT->autype = OSPF_AUTH_CRYPT; ospf_check_auth(); }
 | RX BUFFER NORMAL { OSPF_PATT->rx_buffer = 0; }
 | RX BUFFER LARGE { OSPF_PATT->rx_buffer = OSPF_MAX_PKT_SIZE; }
 | RX BUFFER expr { OSPF_PATT->rx_buffer = $3; if (($3 < OSPF_MIN_PKT_SIZE) || ($3 > OSPF_MAX_PKT_SIZE)) cf_error("Buffer size must be in range 256-65535"); }
 | TX tos { OSPF_PATT->tx_tos = $2; }
 | TX PRIORITY expr { OSPF_PATT->tx_priority = $3; }
 | TX LENGTH expr { OSPF_PATT->tx_length = $3; if (($3 < OSPF_MIN_PKT_SIZE) || ($3 > OSPF_MAX_PKT_SIZE)) cf_error("TX length must be in range 256-65535"); }
 | TTL SECURITY bool { OSPF_PATT->ttl_security = $3; }
 | TTL SECURITY TX ONLY { OSPF_PATT->ttl_security = 2; }
 | BFD bool { OSPF_PATT->bfd = $2; cf_check_bfd($2); }
 | SECONDARY bool { OSPF_PATT->bsd_secondary = $2; }
 | password_list { ospf_check_auth(); }
 ;

pref_list:
 /* empty */
 | pref_list pref_item
 ;

pref_item: pref_base pref_opt ';' ;

pref_base: prefix
 {
   this_pref = cfg_allocz(sizeof(struct area_net_config));
   add_tail(this_nets, NODE this_pref);
   this_pref->px.addr = $1.addr;
   this_pref->px.len = $1.len;
 }
;

pref_opt:
 /* empty */
 | HIDDEN { this_pref->hidden = 1; }
 | TAG expr { this_pref->tag = $2; }
 ;

nbma_list:
 /* empty */
 | nbma_list nbma_item
 ;

nbma_eligible:
 /* empty */ { $$ = 0; }
 | ELIGIBLE { $$ = 1; }
 ;

nbma_item: ipa nbma_eligible ';'
 {
   this_nbma = cfg_allocz(sizeof(struct nbma_node));
   add_tail(&OSPF_PATT->nbma_list, NODE this_nbma);
   this_nbma->ip=$1;
   this_nbma->eligible=$2;
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
  OSPF_PATT->deadc = DEADC_D;
  OSPF_PATT->type = OSPF_IT_UNDEF;
  OSPF_PATT->instance_id = OSPF_CFG->instance_id;
  init_list(&OSPF_PATT->nbma_list);
  OSPF_PATT->ptp_netmask = 2; /* not specified */
  OSPF_PATT->tx_tos = IP_PREC_INTERNET_CONTROL;
  OSPF_PATT->tx_priority = sk_priority_control;
  reset_passwords();
 }
;

ospf_instance_id:
   /* empty */
 | INSTANCE expr { OSPF_PATT->instance_id = $2; if (($2<0) || ($2>255)) cf_error("Instance ID must be in range 0-255"); }
 ;

ospf_iface_patt_list:
   iface_patt_list { if (ospf_cfg_is_v3()) iface_patt_check(); } ospf_instance_id
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
  ospf_iface_start ospf_iface_patt_list ospf_iface_opt_list { ospf_iface_finish(); }
 ;

opttext:
    TEXT
 | /* empty */ { $$ = NULL; }
 ;

CF_ADDTO(dynamic_attr, OSPF_METRIC1 { $$ = f_new_dynamic_attr(EAF_TYPE_INT | EAF_TEMP, T_INT, EA_OSPF_METRIC1); })
CF_ADDTO(dynamic_attr, OSPF_METRIC2 { $$ = f_new_dynamic_attr(EAF_TYPE_INT | EAF_TEMP, T_INT, EA_OSPF_METRIC2); })
CF_ADDTO(dynamic_attr, OSPF_TAG { $$ = f_new_dynamic_attr(EAF_TYPE_INT | EAF_TEMP, T_INT, EA_OSPF_TAG); })
CF_ADDTO(dynamic_attr, OSPF_ROUTER_ID { $$ = f_new_dynamic_attr(EAF_TYPE_ROUTER_ID | EAF_TEMP, T_QUAD, EA_OSPF_ROUTER_ID); })

CF_CLI_HELP(SHOW OSPF, ..., [[Show information about OSPF protocol]]);
CF_CLI(SHOW OSPF, optsym, [<name>], [[Show information about OSPF protocol XXX]])
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

CF_CLI_HELP(SHOW OSPF LSADB, ..., [[Show content of OSPF LSA database]]);
CF_CLI(SHOW OSPF LSADB, lsadb_args, [global | area <id> | link] [type <num>] [lsid <id>] [self | router <id>] [<proto>], [[Show content of OSPF LSA database]])
{ ospf_sh_lsadb($4); };

lsadb_args:
   /* empty */ {
     $$ = cfg_allocz(sizeof(struct lsadb_show_data));
   }
 | lsadb_args GLOBAL { $$ = $1; $$->scope = LSA_SCOPE_AS; }
 | lsadb_args AREA idval { $$ = $1; $$->scope = LSA_SCOPE_AREA; $$->area = $3; }
 | lsadb_args LINK { $$ = $1; $$->scope = 1; /* hack, 0 is no filter */ }
 | lsadb_args TYPE NUM { $$ = $1; $$->type = $3; }
 | lsadb_args LSID idval { $$ = $1; $$->lsid = $3; }
 | lsadb_args SELF { $$ = $1; $$->router = SH_ROUTER_SELF; }
 | lsadb_args ROUTER idval { $$ = $1; $$->router = $3; }
 | lsadb_args SYM { $$ = $1; $$->name = $2; }
 ;

CF_CODE

CF_END
