/*
 *	BIRD -- Static Protocol Configuration
 *
 *	(c) 1998--1999 Martin Mares <mj@ucw.cz>
 *
 *	Can be freely distributed and used under the terms of the GNU GPL.
 */

CF_HDR

#include "proto/static/static.h"

CF_DEFINES

#define STATIC_CFG ((struct static_config *) this_proto)
static struct static_route *this_srt, *this_srt_nh, *last_srt_nh;

CF_DECLS

CF_KEYWORDS(STATIC, ROUTE, VIA, DROP, REJECT, PROHIBIT, PREFERENCE, CHECK, LINK)
CF_KEYWORDS(MULTIPATH, WEIGHT, RECURSIVE, IGP, TABLE, BLACKHOLE, UNREACHABLE)


CF_GRAMMAR

CF_ADDTO(proto, static_proto '}')

static_proto_start: proto_start STATIC {
     this_proto = proto_config_new(&proto_static, sizeof(struct static_config), $1);
     static_init_config((struct static_config *) this_proto);
  }
 ;

static_proto:
   static_proto_start proto_name '{'
 | static_proto proto_item ';'
 | static_proto CHECK LINK bool ';' { STATIC_CFG->check_link = $4; }
 | static_proto IGP TABLE rtable ';' { STATIC_CFG->igp_table = $4; }
 | static_proto stat_route ';'
 ;

stat_route0: ROUTE prefix {
     this_srt = cfg_allocz(sizeof(struct static_route));
     add_tail(&STATIC_CFG->other_routes, &this_srt->n);
     this_srt->net = $2.addr;
     this_srt->masklen = $2.len;
  }
 ;

stat_multipath1:
   VIA ipa ipa_scope {
     last_srt_nh = this_srt_nh;
     this_srt_nh = cfg_allocz(sizeof(struct static_route));
     this_srt_nh->dest = RTD_NONE;
     this_srt_nh->via = $2;
     this_srt_nh->via_if = $3;
     this_srt_nh->if_name = (void *) this_srt; /* really */
   }
 | stat_multipath1 WEIGHT expr {
     this_srt_nh->masklen = $3 - 1; /* really */
     if (($3<1) || ($3>256)) cf_error("Weight must be in range 1-256"); 
   }
 ;

stat_multipath:
   stat_multipath1 { this_srt->mp_next = this_srt_nh; }
 | stat_multipath stat_multipath1 { last_srt_nh->mp_next = this_srt_nh; }
 ;

stat_route:
   stat_route0 VIA ipa ipa_scope {
      this_srt->dest = RTD_ROUTER;
      this_srt->via = $3;
      this_srt->via_if = $4;
   }
 | stat_route0 VIA TEXT {
      this_srt->dest = RTD_DEVICE;
      this_srt->if_name = $3;
      rem_node(&this_srt->n);
      add_tail(&STATIC_CFG->iface_routes, &this_srt->n);
   }
 | stat_route0 MULTIPATH stat_multipath {
      this_srt->dest = RTD_MULTIPATH;
   }
 | stat_route0 RECURSIVE ipa {
      this_srt->dest = RTDX_RECURSIVE;
      this_srt->via = $3;
   }

 | stat_route0 DROP		{ this_srt->dest = RTD_BLACKHOLE; }
 | stat_route0 REJECT		{ this_srt->dest = RTD_UNREACHABLE; }
 | stat_route0 BLACKHOLE	{ this_srt->dest = RTD_BLACKHOLE; }
 | stat_route0 UNREACHABLE	{ this_srt->dest = RTD_UNREACHABLE; }
 | stat_route0 PROHIBIT		{ this_srt->dest = RTD_PROHIBIT; }
 ;

CF_CLI(SHOW STATIC, optsym, [<name>], [[Show details of static protocol]])
{ static_show(proto_get_named($3, &proto_static)); } ;

CF_CODE

CF_END
