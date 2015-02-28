/*
 *	BIRD -- RIP Configuration
 *
 *	Can be freely distributed and used under the terms of the GNU GPL.
 */

/*
To add:

version1 switch

*/


CF_HDR

#include "proto/rip/rip.h"
#include "nest/iface.h"

CF_DEFINES

#define RIP_CFG ((struct rip_proto_config *) this_proto)
#define RIP_IPATT ((struct rip_patt *) this_ipatt)

#ifdef IPV6
#define RIP_DEFAULT_TTL_SECURITY 2
#else
#define RIP_DEFAULT_TTL_SECURITY 0
#endif

CF_DECLS

CF_KEYWORDS(RIP, INFINITY, METRIC, PORT, PERIOD, GARBAGE, TIMEOUT,
	    MODE, BROADCAST, MULTICAST, QUIET, NOLISTEN, VERSION1, 
	    AUTHENTICATION, NONE, PLAINTEXT, MD5, TTL, SECURITY,
	    HONOR, NEVER, NEIGHBOR, ALWAYS, TX, PRIORITY, ONLY,
	    RIP_METRIC, RIP_TAG)

%type <i> rip_mode rip_auth

CF_GRAMMAR

CF_ADDTO(proto, rip_cfg '}' { RIP_CFG->passwords = get_passwords(); } )

rip_cfg_start: proto_start RIP {
     this_proto = proto_config_new(&proto_rip, sizeof(struct rip_proto_config), $1);
     rip_init_config(RIP_CFG);
   }
 ;

rip_cfg:
   rip_cfg_start proto_name '{'
 | rip_cfg proto_item ';'
 | rip_cfg INFINITY expr ';'	{ RIP_CFG->infinity = $3; }
 | rip_cfg PORT expr ';'	{ RIP_CFG->port = $3; }
 | rip_cfg PERIOD expr ';'	{ RIP_CFG->period = $3; }
 | rip_cfg GARBAGE TIME expr ';' { RIP_CFG->garbage_time = $4; }
 | rip_cfg TIMEOUT TIME expr ';' { RIP_CFG->timeout_time = $4; }
 | rip_cfg AUTHENTICATION rip_auth ';' {RIP_CFG->authtype = $3; }
 | rip_cfg password_list ';'
 | rip_cfg HONOR ALWAYS ';'    { RIP_CFG->honor = HO_ALWAYS; }
 | rip_cfg HONOR NEIGHBOR ';'    { RIP_CFG->honor = HO_NEIGHBOR; }
 | rip_cfg HONOR NEVER ';'    { RIP_CFG->honor = HO_NEVER; }
 | rip_cfg INTERFACE rip_iface ';'
 ;

rip_auth:
   PLAINTEXT { $$=AT_PLAINTEXT; }
 | MD5 { $$=AT_MD5; }
 | NONE { $$=AT_NONE; }
 ;


rip_mode: 
    BROADCAST { $$=IM_BROADCAST; }
  | MULTICAST { $$=0; }
  | QUIET     { $$=IM_QUIET; }
  | NOLISTEN  { $$=IM_NOLISTEN; }
  | VERSION1  { $$=IM_VERSION1 | IM_BROADCAST; }
 ;

rip_iface_item:
 | METRIC expr { RIP_IPATT->metric = $2; }
 | MODE rip_mode { RIP_IPATT->mode |= $2; }
 | TX tos { RIP_IPATT->tx_tos = $2; }
 | TX PRIORITY expr { RIP_IPATT->tx_priority = $3; }
 | TTL SECURITY bool { RIP_IPATT->ttl_security = $3; }
 | TTL SECURITY TX ONLY { RIP_IPATT->ttl_security = 2; }
 ;

rip_iface_opts: 
   /* empty */
 | rip_iface_opts rip_iface_item ';'
 ;

rip_iface_opt_list:
   /* empty */
 | '{' rip_iface_opts '}'
 ;

rip_iface_init:
   /* EMPTY */ {
     this_ipatt = cfg_allocz(sizeof(struct rip_patt));
     add_tail(&RIP_CFG->iface_list, NODE this_ipatt);
     init_list(&this_ipatt->ipn_list);
     RIP_IPATT->metric = 1;
     RIP_IPATT->tx_tos = IP_PREC_INTERNET_CONTROL;
     RIP_IPATT->tx_priority = sk_priority_control;
     RIP_IPATT->ttl_security = RIP_DEFAULT_TTL_SECURITY;
   }
 ;

rip_iface:	/* TODO: switch to iface_patt_list_nopx */
   rip_iface_init iface_patt_list rip_iface_opt_list
 ;

CF_ADDTO(dynamic_attr, RIP_METRIC { $$ = f_new_dynamic_attr(EAF_TYPE_INT | EAF_TEMP, T_INT, EA_RIP_METRIC); })
CF_ADDTO(dynamic_attr, RIP_TAG { $$ = f_new_dynamic_attr(EAF_TYPE_INT | EAF_TEMP, T_INT, EA_RIP_TAG); })

CF_CODE

CF_END
