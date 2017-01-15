/*
 *	BIRD -- RIP Configuration
 *
 *	(c) 1998--1999 Pavel Machek <pavel@ucw.cz>
 *	(c) 2004--2013 Ondrej Filip <feela@network.cz>
 *	(c) 2009--2015 Ondrej Zajicek <santiago@crfreenet.org>
 *	(c) 2009--2015 CZ.NIC z.s.p.o.
 *
 *	Can be freely distributed and used under the terms of the GNU GPL.
 */

CF_HDR

#include "proto/rip/rip.h"
#include "nest/iface.h"

CF_DEFINES

#define RIP_CFG ((struct rip_config *) this_proto)
#define RIP_IFACE ((struct rip_iface_config *) this_ipatt)

static inline int rip_cfg_is_v2(void) { return RIP_CFG->rip2; }
static inline int rip_cfg_is_ng(void) { return ! RIP_CFG->rip2; }

static inline void
rip_check_auth(void)
{
  if (rip_cfg_is_ng())
    cf_error("Authentication not supported in RIPng");
}


CF_DECLS

CF_KEYWORDS(RIP, ECMP, LIMIT, WEIGHT, INFINITY, METRIC, UPDATE, TIMEOUT,
	    GARBAGE, PORT, ADDRESS, MODE, BROADCAST, MULTICAST, PASSIVE,
	    VERSION, SPLIT, HORIZON, POISON, REVERSE, CHECK, ZERO, TIME, BFD,
	    AUTHENTICATION, NONE, PLAINTEXT, CRYPTOGRAPHIC, MD5, TTL, SECURITY,
	    RX, TX, BUFFER, LENGTH, PRIORITY, ONLY, LINK, RIP_METRIC, RIP_TAG)

%type <i> rip_auth

CF_GRAMMAR

CF_ADDTO(proto, rip_proto)

rip_proto_start: proto_start RIP
{
  this_proto = proto_config_new(&proto_rip, $1);
  init_list(&RIP_CFG->patt_list);

  RIP_CFG->rip2 = RIP_IS_V2;
  RIP_CFG->infinity = RIP_DEFAULT_INFINITY;

  RIP_CFG->min_timeout_time = 60;
  RIP_CFG->max_garbage_time = 60;
};

rip_proto_item:
   proto_item
 | ECMP bool		{ RIP_CFG->ecmp = $2 ? RIP_DEFAULT_ECMP_LIMIT : 0; }
 | ECMP bool LIMIT expr	{ RIP_CFG->ecmp = $2 ? $4 : 0; if ($4 < 0) cf_error("ECMP limit cannot be negative"); }
 | INFINITY expr	{ RIP_CFG->infinity = $2; }
 | INTERFACE rip_iface
 ;

rip_proto_opts:
   /* empty */
 | rip_proto_opts rip_proto_item ';'
 ;

rip_proto:
   rip_proto_start proto_name '{' rip_proto_opts '}';


rip_iface_start:
{
  this_ipatt = cfg_allocz(sizeof(struct rip_iface_config));
  add_tail(&RIP_CFG->patt_list, NODE this_ipatt);
  init_list(&this_ipatt->ipn_list);
  reset_passwords();

  RIP_IFACE->metric = 1;
  RIP_IFACE->port = rip_cfg_is_v2() ? RIP_PORT : RIP_NG_PORT;
  RIP_IFACE->version = rip_cfg_is_v2() ? RIP_V2 : RIP_V1;
  RIP_IFACE->split_horizon = 1;
  RIP_IFACE->poison_reverse = 1;
  RIP_IFACE->check_zero = 1;
  RIP_IFACE->ttl_security = rip_cfg_is_v2() ? 0 : 1;
  RIP_IFACE->rx_buffer = rip_cfg_is_v2() ? RIP_MAX_PKT_LENGTH : 0;
  RIP_IFACE->tx_length = rip_cfg_is_v2() ? RIP_MAX_PKT_LENGTH : 0;
  RIP_IFACE->tx_tos = IP_PREC_INTERNET_CONTROL;
  RIP_IFACE->tx_priority = sk_priority_control;
  RIP_IFACE->update_time = RIP_DEFAULT_UPDATE_TIME;
  RIP_IFACE->timeout_time = RIP_DEFAULT_TIMEOUT_TIME;
  RIP_IFACE->garbage_time = RIP_DEFAULT_GARBAGE_TIME;
};

rip_iface_finish:
{
  /* Default mode is broadcast for RIPv1, multicast for RIPv2 and RIPng */
  if (!RIP_IFACE->mode)
    RIP_IFACE->mode = (rip_cfg_is_v2() && (RIP_IFACE->version == RIP_V1)) ?
      RIP_IM_BROADCAST : RIP_IM_MULTICAST;

  RIP_IFACE->passwords = get_passwords();

  if (!RIP_IFACE->auth_type != !RIP_IFACE->passwords)
    log(L_WARN "Authentication and password options should be used together");

  if (RIP_IFACE->passwords)
  {
    struct password_item *pass;
    WALK_LIST(pass, *RIP_IFACE->passwords)
    {
      if (pass->alg && (RIP_IFACE->auth_type != RIP_AUTH_CRYPTO))
	cf_error("Password algorithm option requires cryptographic authentication");

      /* Set default crypto algorithm (MD5) */
      if (!pass->alg && (RIP_IFACE->auth_type == RIP_AUTH_CRYPTO))
	pass->alg = ALG_MD5;
    }
  }

  RIP_CFG->min_timeout_time = MIN_(RIP_CFG->min_timeout_time, RIP_IFACE->timeout_time);
  RIP_CFG->max_garbage_time = MAX_(RIP_CFG->max_garbage_time, RIP_IFACE->garbage_time);
};

rip_iface_item:
   METRIC expr		{ RIP_IFACE->metric = $2; if (($2<1) || ($2>255)) cf_error("Metric must be in range 1-255"); }
 | MODE MULTICAST	{ RIP_IFACE->mode = RIP_IM_MULTICAST; }
 | MODE BROADCAST	{ RIP_IFACE->mode = RIP_IM_BROADCAST; if (rip_cfg_is_ng()) cf_error("Broadcast not supported in RIPng"); }
 | PASSIVE bool		{ RIP_IFACE->passive = $2; }
 | ADDRESS ipa		{ RIP_IFACE->address = $2; }
 | PORT expr		{ RIP_IFACE->port = $2; if (($2<1) || ($2>65535)) cf_error("Invalid port number"); }
 | VERSION expr		{ RIP_IFACE->version = $2;
			  if (rip_cfg_is_ng()) cf_error("Version not supported in RIPng");
			  if (($2 != RIP_V1) && ($2 != RIP_V2)) cf_error("Unsupported version");
			}
 | VERSION ONLY bool	{ RIP_IFACE->version_only = $3; }
 | SPLIT HORIZON bool	{ RIP_IFACE->split_horizon = $3; }
 | POISON REVERSE bool	{ RIP_IFACE->poison_reverse = $3; }
 | CHECK ZERO bool	{ RIP_IFACE->check_zero = $3; }
 | UPDATE TIME expr	{ RIP_IFACE->update_time = $3; if ($3<=0) cf_error("Update time must be positive"); }
 | TIMEOUT TIME expr	{ RIP_IFACE->timeout_time = $3; if ($3<=0) cf_error("Timeout time must be positive"); }
 | GARBAGE TIME expr	{ RIP_IFACE->garbage_time = $3; if ($3<=0) cf_error("Garbage time must be positive"); }
 | ECMP WEIGHT expr	{ RIP_IFACE->ecmp_weight = $3 - 1; if (($3<1) || ($3>256)) cf_error("ECMP weight must be in range 1-256"); }
 | RX BUFFER expr	{ RIP_IFACE->rx_buffer = $3; if (($3<256) || ($3>65535)) cf_error("RX length must be in range 256-65535"); }
 | TX LENGTH expr	{ RIP_IFACE->tx_length = $3; if (($3<256) || ($3>65535)) cf_error("TX length must be in range 256-65535"); }
 | TX tos		{ RIP_IFACE->tx_tos = $2; }
 | TX PRIORITY expr	{ RIP_IFACE->tx_priority = $3; }
 | TTL SECURITY bool	{ RIP_IFACE->ttl_security = $3; }
 | TTL SECURITY TX ONLY	{ RIP_IFACE->ttl_security = 2; }
 | CHECK LINK bool	{ RIP_IFACE->check_link = $3; }
 | BFD bool		{ RIP_IFACE->bfd = $2; cf_check_bfd($2); }
 | AUTHENTICATION rip_auth { RIP_IFACE->auth_type = $2; if ($2) rip_check_auth(); }
 | password_list	{ rip_check_auth(); }
;

rip_auth:
   NONE			{ $$ = RIP_AUTH_NONE; }
 | PLAINTEXT		{ $$ = RIP_AUTH_PLAIN; }
 | CRYPTOGRAPHIC	{ $$ = RIP_AUTH_CRYPTO; }
 | MD5			{ $$ = RIP_AUTH_CRYPTO; }	/* For backward compatibility */
 ;

rip_iface_opts:
   /* empty */
 | rip_iface_opts rip_iface_item ';'
 ;

rip_iface_opt_list:
   /* empty */
 | '{' rip_iface_opts '}'
 ;

rip_iface:
  rip_iface_start iface_patt_list_nopx rip_iface_opt_list rip_iface_finish;


CF_ADDTO(dynamic_attr, RIP_METRIC { $$ = f_new_dynamic_attr(EAF_TYPE_INT | EAF_TEMP, T_INT, EA_RIP_METRIC); })
CF_ADDTO(dynamic_attr, RIP_TAG { $$ = f_new_dynamic_attr(EAF_TYPE_INT | EAF_TEMP, T_INT, EA_RIP_TAG); })

CF_CLI_HELP(SHOW RIP, ..., [[Show information about RIP protocol]]);

CF_CLI(SHOW RIP INTERFACES, optsym opttext, [<name>] [\"<interface>\"], [[Show information about RIP interfaces]])
{ rip_show_interfaces(proto_get_named($4, &proto_rip), $5); };

CF_CLI(SHOW RIP NEIGHBORS, optsym opttext, [<name>] [\"<interface>\"], [[Show information about RIP neighbors]])
{ rip_show_neighbors(proto_get_named($4, &proto_rip), $5); };


CF_CODE

CF_END
