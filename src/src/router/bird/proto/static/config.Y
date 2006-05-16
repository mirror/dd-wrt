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

static struct static_route *this_srt;

CF_DECLS

CF_KEYWORDS(STATIC, ROUTE, VIA, DROP, REJECT, PROHIBIT, PREFERENCE)

CF_GRAMMAR

CF_ADDTO(proto, static_proto '}')

static_proto_start: proto_start STATIC {
     this_proto = proto_config_new(&proto_static, sizeof(struct static_config));
     static_init_config((struct static_config *) this_proto);
  }
 ;

static_proto:
   static_proto_start proto_name '{'
 | static_proto proto_item ';'
 | static_proto stat_route ';'
 ;

stat_route0: ROUTE prefix {
     this_srt = cfg_allocz(sizeof(struct static_route));
     add_tail(&((struct static_config *) this_proto)->other_routes, &this_srt->n);
     this_srt->net = $2.addr;
     this_srt->masklen = $2.len;
  }
 ;

stat_route:
   stat_route0 VIA ipa {
      this_srt->dest = RTD_ROUTER;
      this_srt->via = $3;
   }
 | stat_route0 VIA TEXT {
      this_srt->dest = RTD_DEVICE;
      this_srt->if_name = $3;
      rem_node(&this_srt->n);
      add_tail(&((struct static_config *) this_proto)->iface_routes, &this_srt->n);
   }
 | stat_route0 DROP { this_srt->dest = RTD_BLACKHOLE; }
 | stat_route0 REJECT { this_srt->dest = RTD_UNREACHABLE; }
 | stat_route0 PROHIBIT { this_srt->dest = RTD_PROHIBIT; }
 ;

CF_CLI(SHOW STATIC, optsym, [<name>], [[Show details of static protocol]])
{ static_show(proto_get_named($3, &proto_static)); } ;

CF_CODE

CF_END
