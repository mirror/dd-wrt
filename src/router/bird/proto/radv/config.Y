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
#define RADV_RDNSS (&this_radv_rdnss)
#define RADV_DNSSL (&this_radv_dnssl)

static struct radv_prefix_config *this_radv_prefix;
static struct radv_rdnss_config this_radv_rdnss;
static struct radv_dnssl_config this_radv_dnssl;
static list radv_dns_list;	/* Used by radv_rdnss and radv_dnssl */
static u8 radv_mult_val;	/* Used by radv_mult for second return value */


CF_DECLS

CF_KEYWORDS(RADV, PREFIX, INTERFACE, MIN, MAX, RA, DELAY, INTERVAL,
	MANAGED, OTHER, CONFIG, LINK, MTU, REACHABLE, TIME, RETRANS,
	TIMER, CURRENT, HOP, LIMIT, DEFAULT, VALID, PREFERRED, MULT,
	LIFETIME, SKIP, ONLINK, AUTONOMOUS, RDNSS, DNSSL, NS, DOMAIN,
	LOCAL, TRIGGER, SENSITIVE, PREFERENCE, LOW, MEDIUM, HIGH)

%type<i> radv_mult radv_sensitive radv_preference

CF_GRAMMAR

CF_ADDTO(proto, radv_proto)

radv_proto_start: proto_start RADV
{
  this_proto = proto_config_new(&proto_radv, $1);
  init_list(&RADV_CFG->patt_list);
  init_list(&RADV_CFG->pref_list);
  init_list(&RADV_CFG->rdnss_list);
  init_list(&RADV_CFG->dnssl_list);
};

radv_proto_item:
   proto_item
 | INTERFACE radv_iface
 | PREFIX radv_prefix { add_tail(&RADV_CFG->pref_list, NODE this_radv_prefix); }
 | RDNSS { init_list(&radv_dns_list); } radv_rdnss { add_tail_list(&RADV_CFG->rdnss_list, &radv_dns_list); }
 | DNSSL { init_list(&radv_dns_list); } radv_dnssl { add_tail_list(&RADV_CFG->dnssl_list, &radv_dns_list); }
 | TRIGGER prefix {
     RADV_CFG->trigger_prefix = $2.addr;
     RADV_CFG->trigger_pxlen = $2.len;
     RADV_CFG->trigger_valid = 1;
   }
 ;

radv_proto_opts:
   /* empty */
 | radv_proto_opts radv_proto_item ';'
 ;

radv_proto:
   radv_proto_start proto_name '{' radv_proto_opts '}';


radv_iface_start:
{
  this_ipatt = cfg_allocz(sizeof(struct radv_iface_config));
  add_tail(&RADV_CFG->patt_list, NODE this_ipatt);
  init_list(&this_ipatt->ipn_list);
  init_list(&RADV_IFACE->pref_list);
  init_list(&RADV_IFACE->rdnss_list);
  init_list(&RADV_IFACE->dnssl_list);

  RADV_IFACE->min_ra_int = -1; /* undefined */
  RADV_IFACE->max_ra_int = DEFAULT_MAX_RA_INT;
  RADV_IFACE->min_delay = DEFAULT_MIN_DELAY;
  RADV_IFACE->current_hop_limit = DEFAULT_CURRENT_HOP_LIMIT;
  RADV_IFACE->default_lifetime = -1;
  RADV_IFACE->default_lifetime_sensitive = 1;
  RADV_IFACE->default_preference = RA_PREF_MEDIUM;
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
 | DEFAULT LIFETIME expr radv_sensitive {
     RADV_IFACE->default_lifetime = $3;
     if (($3 < 0) || ($3 > 9000))  cf_error("Default lifetime must be in range 0-9000");
     if ($4 != -1) RADV_IFACE->default_lifetime_sensitive = $4;
   }
 | DEFAULT PREFERENCE radv_preference { RADV_IFACE->default_preference = $3; }
 | PREFIX radv_prefix { add_tail(&RADV_IFACE->pref_list, NODE this_radv_prefix); }
 | RDNSS { init_list(&radv_dns_list); } radv_rdnss { add_tail_list(&RADV_IFACE->rdnss_list, &radv_dns_list); }
 | DNSSL { init_list(&radv_dns_list); } radv_dnssl { add_tail_list(&RADV_IFACE->dnssl_list, &radv_dns_list); }
 | RDNSS LOCAL bool { RADV_IFACE->rdnss_local = $3; }
 | DNSSL LOCAL bool { RADV_IFACE->dnssl_local = $3; }
 ;

radv_preference:
   LOW { $$ = RA_PREF_LOW; }
 | MEDIUM { $$ = RA_PREF_MEDIUM; }
 | HIGH { $$ = RA_PREF_HIGH; }

radv_iface_finish:
{
  struct radv_iface_config *ic = RADV_IFACE;

  if (ic->min_ra_int == (u32) -1)
    ic->min_ra_int = MAX_(ic->max_ra_int / 3, 3);

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
  radv_iface_start iface_patt_list_nopx radv_iface_opt_list radv_iface_finish;


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
 | VALID LIFETIME expr radv_sensitive {
     RADV_PREFIX->valid_lifetime = $3;
     if ($3 < 0) cf_error("Valid lifetime must be 0 or positive");
     if ($4 != -1) RADV_PREFIX->valid_lifetime_sensitive = $4;
   }
 | PREFERRED LIFETIME expr radv_sensitive {
     RADV_PREFIX->preferred_lifetime = $3;
     if ($3 < 0) cf_error("Preferred lifetime must be 0 or positive");
     if ($4 != -1) RADV_PREFIX->preferred_lifetime_sensitive = $4;
   }
 ;

radv_prefix_finish:
{
  if (RADV_PREFIX->preferred_lifetime > RADV_PREFIX->valid_lifetime)
    cf_error("Preferred lifetime must be at most Valid lifetime");

  if (RADV_PREFIX->valid_lifetime_sensitive > RADV_PREFIX->preferred_lifetime_sensitive)
    cf_error("Valid lifetime sensitive requires that Preferred lifetime is sensitive too");
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



radv_rdnss_node: ipa
{
  struct radv_rdnss_config *cf = cfg_allocz(sizeof(struct radv_rdnss_config));
  add_tail(&radv_dns_list, NODE cf);

  cf->server = $1;
  cf->lifetime_mult = DEFAULT_DNS_LIFETIME_MULT;
};

radv_rdnss_start:
{
  RADV_RDNSS->lifetime = 0;
  RADV_RDNSS->lifetime_mult = DEFAULT_DNS_LIFETIME_MULT;
};

radv_rdnss_item:
 | NS radv_rdnss_node
 | LIFETIME radv_mult { RADV_RDNSS->lifetime = $2; RADV_RDNSS->lifetime_mult = radv_mult_val; }
 ;

radv_rdnss_finish:
{
  if (EMPTY_LIST(radv_dns_list))
    cf_error("No nameserver in RDNSS section");

  struct radv_rdnss_config *cf;
  WALK_LIST(cf, radv_dns_list)
  {
    cf->lifetime = RADV_RDNSS->lifetime;
    cf->lifetime_mult = RADV_RDNSS->lifetime_mult;
  }
};

radv_rdnss_opts:
   /* empty */
 | radv_rdnss_opts radv_rdnss_item ';'
 ;

radv_rdnss:
   radv_rdnss_node
 | '{' radv_rdnss_start radv_rdnss_opts '}' radv_rdnss_finish
 ;


radv_dnssl_node: TEXT
{
  struct radv_dnssl_config *cf = cfg_allocz(sizeof(struct radv_dnssl_config));
  add_tail(&radv_dns_list, NODE cf);

  cf->domain = $1;
  cf->lifetime_mult = DEFAULT_DNS_LIFETIME_MULT;

  if (radv_process_domain(cf) < 0)
    cf_error("Invalid domain dame");
};

radv_dnssl_start:
{
  RADV_DNSSL->lifetime = 0;
  RADV_DNSSL->lifetime_mult = DEFAULT_DNS_LIFETIME_MULT;
};

radv_dnssl_item:
 | DOMAIN radv_dnssl_node
 | LIFETIME radv_mult { RADV_DNSSL->lifetime = $2; RADV_DNSSL->lifetime_mult = radv_mult_val; }
 ;

radv_dnssl_finish:
{
  if (EMPTY_LIST(radv_dns_list))
    cf_error("No domain in DNSSL section");

  struct radv_dnssl_config *cf;
  WALK_LIST(cf, radv_dns_list)
  {
    cf->lifetime = RADV_DNSSL->lifetime;
    cf->lifetime_mult = RADV_DNSSL->lifetime_mult;
  }
};

radv_dnssl_opts:
   /* empty */
 | radv_dnssl_opts radv_dnssl_item ';'
 ;

radv_dnssl:
   radv_dnssl_node
 | '{' radv_dnssl_start radv_dnssl_opts '}' radv_dnssl_finish
 ;


radv_mult:
   expr { $$ = $1; radv_mult_val = 0; }
 | MULT expr { $$ = 0; radv_mult_val = $2; if (($2 < 1) || ($2 > 254)) cf_error("Multiplier must be in range 1-254"); }
 ;

radv_sensitive:
   /* empty */ { $$ = -1; }
 | SENSITIVE bool { $$ = $2; }
 ;

CF_CODE

CF_END
