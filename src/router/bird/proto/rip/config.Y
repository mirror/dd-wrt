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

CF_DECLS

CF_KEYWORDS(RIP, INFINITY, METRIC, PORT, PERIOD, GARBAGE, TIMEOUT, PASSWORDS,
	    MODE, BROADCAST, MULTICAST, QUIET, NOLISTEN, VERSION1, 
	    AUTHENTICATION, NONE, PLAINTEXT, MD5,
	    HONOR, NEVER, NEIGHBOR, ALWAYS,
	    RIP_METRIC, RIP_TAG)

%type <i> rip_mode rip_auth

CF_GRAMMAR

CF_ADDTO(proto, rip_cfg '}')

rip_cfg_start: proto_start RIP {
     this_proto = proto_config_new(&proto_rip, sizeof(struct rip_proto_config));
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
 | rip_cfg password_list ';' {RIP_CFG->passwords = (list *)$2; }
 | rip_cfg HONOR ALWAYS ';'    { RIP_CFG->honor = HO_ALWAYS; }
 | rip_cfg HONOR NEIGHBOR ';'    { RIP_CFG->honor = HO_NEIGHBOR; }
 | rip_cfg HONOR NEVER ';'    { RIP_CFG->honor = HO_NEVER; }
 | rip_cfg rip_iface_list ';'
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
 ;

rip_iface_opts: 
   '{'
 | rip_iface_opts rip_iface_item ';'
 ;

rip_iface_opt_list: /* EMPTY */ | rip_iface_opts '}' ;

rip_iface_init:
   /* EMPTY */ {
     struct rip_patt *k = cfg_allocz(sizeof(struct rip_patt));
     k->metric = 1;
     add_tail(&RIP_CFG->iface_list, &k->i.n);
     this_ipatt = &k->i;
   }
 ;

rip_iface:
   rip_iface_init iface_patt rip_iface_opt_list
 ;

rip_iface_list:
   INTERFACE rip_iface
 | rip_iface_list ',' rip_iface
 ;

CF_ADDTO(dynamic_attr, RIP_METRIC { $$ = f_new_dynamic_attr(EAF_TYPE_INT | EAF_TEMP, T_INT, EA_RIP_METRIC); })
CF_ADDTO(dynamic_attr, RIP_TAG { $$ = f_new_dynamic_attr(EAF_TYPE_INT | EAF_TEMP, T_INT, EA_RIP_TAG); })

CF_CODE

CF_END
