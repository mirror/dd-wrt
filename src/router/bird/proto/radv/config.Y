/*
 *	BIRD -- Router Advertisement Configuration
 *
 *
 *	Can be freely distributed and used under the terms of the GNU GPL.
 */

CF_HDR

#include "proto/radv/radv.h"

CF_DEFINES

#define RADV_CFG ((struct radv_config *) this_proto)
#define RADV_IFACE ((struct radv_iface_config *) this_ipatt)
#define RADV_PREFIX this_radv_prefix

static struct radv_prefix_config *this_radv_prefix;

CF_DECLS

CF_KEYWORDS(RADV, PREFIX, INTERFACE, MIN, MAX, RA, DELAY, INTERVAL,
	MANAGED, OTHER, CONFIG, LINK, MTU, REACHABLE, TIME, RETRANS,
	TIMER, CURRENT, HOP, LIMIT, DEFAULT, VALID, PREFERRED,
	LIFETIME, SKIP, ONLINK, AUTONOMOUS)


CF_GRAMMAR

CF_ADDTO(proto, radv_proto '}')

radv_proto_start: proto_start RADV
{
  this_proto = proto_config_new(&proto_radv, sizeof(struct radv_config), $1);
  init_list(&RADV_CFG->patt_list);
  init_list(&RADV_CFG->pref_list);
};

radv_proto_item:
   proto_item
 | PREFIX radv_prefix { add_tail(&RADV_CFG->pref_list, NODE this_radv_prefix); }
 | INTERFACE radv_iface
 ;

radv_proto_opts:
   /* empty */
 | radv_proto_opts radv_proto_item ';'
 ;

radv_proto:
   radv_proto_start proto_name '{' radv_proto_opts;


radv_iface_start:
{
  this_ipatt = cfg_allocz(sizeof(struct radv_iface_config));
  add_tail(&RADV_CFG->patt_list, NODE this_ipatt);
  init_list(&this_ipatt->ipn_list);
  init_list(&RADV_IFACE->pref_list);

  RADV_IFACE->min_ra_int = -1; /* undefined */
  RADV_IFACE->max_ra_int = DEFAULT_MAX_RA_INT;
  RADV_IFACE->min_delay = DEFAULT_MIN_DELAY;
  RADV_IFACE->current_hop_limit = DEFAULT_CURRENT_HOP_LIMIT;
  RADV_IFACE->default_lifetime = -1;
};

radv_iface_item:
   MIN RA INTERVAL expr { RADV_IFACE->min_ra_int = $4; if ($4 < 3) cf_error("Min RA interval must be at least 3"); }
 | MAX RA INTERVAL expr { RADV_IFACE->max_ra_int = $4; if (($4 < 4) || ($4 > 1800)) cf_error("Max RA interval must be in range 4-1800"); }
 | MIN DELAY expr { RADV_IFACE->min_delay = $3; if ($3 <= 0) cf_error("Min delay must be positive"); }
 | MANAGED bool { RADV_IFACE->managed = $2; }
 | OTHER CONFIG bool { RADV_IFACE->other_config = $3; }
 | LINK MTU expr { RADV_IFACE->link_mtu = $3; if ($3 < 0) cf_error("Link MTU must be 0 or positive"); }
 | REACHABLE TIME expr { RADV_IFACE->reachable_time = $3; if (($3 < 0) || ($3 > 3600000)) cf_error("Reachable time must be in range 0-3600000"); }
 | RETRANS TIMER expr { RADV_IFACE->retrans_timer = $3; if ($3 < 0) cf_error("Retrans timer must be 0 or positive"); }
 | CURRENT HOP LIMIT expr { RADV_IFACE->current_hop_limit = $4; if (($4 < 0) || ($4 > 255))  cf_error("Current hop limit must be in range 0-255"); }
 | DEFAULT LIFETIME expr { RADV_IFACE->default_lifetime = $3; if (($3 < 0) || ($3 > 9000))  cf_error("Default lifetime must be in range 0-9000"); }
 | PREFIX radv_prefix { add_tail(&RADV_IFACE->pref_list, NODE this_radv_prefix); }
 ;

radv_iface_finish:
{
  struct radv_iface_config *ic = RADV_IFACE;

  if (ic->min_ra_int == (u32) -1)
    ic->min_ra_int = _MAX(ic->max_ra_int / 3, 3);

  if (ic->default_lifetime == (u32) -1)
    ic->default_lifetime = 3 * ic->max_ra_int;

  if ((ic->min_ra_int > 3) &&
      (ic->min_ra_int > (ic->max_ra_int * 3 / 4)))
    cf_error("Min RA interval must be at most 3/4 * Max RA interval %d %d", ic->min_ra_int, ic->max_ra_int);

  if ((ic->default_lifetime > 0) && (ic->default_lifetime < ic->max_ra_int))
    cf_error("Default lifetime must be either 0 or at least Max RA interval");
};


radv_iface_opts:
   /* empty */
 | radv_iface_opts radv_iface_item ';'
 ;

radv_iface_opt_list:
   /* empty */
 | '{' radv_iface_opts '}'
 ;

radv_iface:
  radv_iface_start iface_patt_list radv_iface_opt_list radv_iface_finish;


radv_prefix_start: prefix
{
  this_radv_prefix = cfg_allocz(sizeof(struct radv_prefix_config));
  RADV_PREFIX->prefix = $1.addr;
  RADV_PREFIX->pxlen = $1.len;

  RADV_PREFIX->onlink = 1;
  RADV_PREFIX->autonomous = 1;
  RADV_PREFIX->valid_lifetime = DEFAULT_VALID_LIFETIME;
  RADV_PREFIX->preferred_lifetime = DEFAULT_PREFERRED_LIFETIME;
};

radv_prefix_item:
   SKIP bool { RADV_PREFIX->skip = $2; }
 | ONLINK bool { RADV_PREFIX->onlink = $2; }
 | AUTONOMOUS bool { RADV_PREFIX->autonomous = $2; }
 | VALID LIFETIME expr { RADV_PREFIX->valid_lifetime = $3; if ($3 < 0) cf_error("Valid lifetime must be 0 or positive"); }
 | PREFERRED LIFETIME expr { RADV_PREFIX->preferred_lifetime = $3; if ($3 < 0) cf_error("Preferred lifetime must be 0 or positive"); }
 ;

radv_prefix_finish:
{
  if (RADV_PREFIX->preferred_lifetime > RADV_PREFIX->valid_lifetime)
    cf_error("Preferred lifetime must be at most Valid lifetime");
};

radv_prefix_opts:
   /* empty */
 | radv_prefix_opts radv_prefix_item ';'
 ;

radv_prefix_opt_list:
   /* empty */
 | '{' radv_prefix_opts '}'
 ;

radv_prefix:
  radv_prefix_start radv_prefix_opt_list radv_prefix_finish;


CF_CODE

CF_END
