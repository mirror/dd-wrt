/*
 *	BIRD -- Router Advertisement Configuration
 *
 *
 *	Can be freely distributed and used under the terms of the GNU GPL.
 */

CF_HDR

#include "proto/bfd/bfd.h"

CF_DEFINES

#define BFD_CFG ((struct bfd_config *) this_proto)
#define BFD_IFACE ((struct bfd_iface_config *) this_ipatt)
#define BFD_NEIGHBOR this_bfd_neighbor

static struct bfd_neighbor *this_bfd_neighbor;

extern struct bfd_config *bfd_cf;

CF_DECLS

CF_KEYWORDS(BFD, MIN, IDLE, RX, TX, INTERVAL, MULTIPLIER, PASSIVE,
	INTERFACE, MULTIHOP, NEIGHBOR, DEV, LOCAL, AUTHENTICATION,
	NONE, SIMPLE, METICULOUS, KEYED, MD5, SHA1)

%type <iface> bfd_neigh_iface
%type <a> bfd_neigh_local
%type <i> bfd_neigh_multihop bfd_auth_type

CF_GRAMMAR

CF_ADDTO(proto, bfd_proto)

bfd_proto_start: proto_start BFD
{
  this_proto = proto_config_new(&proto_bfd, $1);
  init_list(&BFD_CFG->patt_list);
  init_list(&BFD_CFG->neigh_list);

  if (bfd_cf)
    cf_error("Only one BFD instance allowed");
  bfd_cf = BFD_CFG;
};

bfd_proto_item:
   proto_item
 | INTERFACE bfd_iface
 | MULTIHOP bfd_multihop
 | NEIGHBOR bfd_neighbor
 ;

bfd_proto_opts:
   /* empty */
 | bfd_proto_opts bfd_proto_item ';'
 ;

bfd_proto:
   bfd_proto_start proto_name '{' bfd_proto_opts '}';


bfd_iface_start:
{
  this_ipatt = cfg_allocz(sizeof(struct bfd_iface_config));
  add_tail(&BFD_CFG->patt_list, NODE this_ipatt);
  init_list(&this_ipatt->ipn_list);

  BFD_IFACE->min_rx_int = BFD_DEFAULT_MIN_RX_INT;
  BFD_IFACE->min_tx_int = BFD_DEFAULT_MIN_TX_INT;
  BFD_IFACE->idle_tx_int = BFD_DEFAULT_IDLE_TX_INT;
  BFD_IFACE->multiplier = BFD_DEFAULT_MULTIPLIER;

  reset_passwords();
};

bfd_iface_finish:
{
  BFD_IFACE->passwords = get_passwords();

  if (!BFD_IFACE->auth_type != !BFD_IFACE->passwords)
    log(L_WARN "Authentication and password options should be used together");

  if (BFD_IFACE->passwords)
  {
    struct password_item *pass;
    WALK_LIST(pass, *BFD_IFACE->passwords)
    {
      if (pass->alg)
        cf_error("Password algorithm option not available in BFD protocol");

      pass->alg = bfd_auth_type_to_hash_alg[BFD_IFACE->auth_type];
    }
  }
};

bfd_iface_item:
   INTERVAL expr_us { BFD_IFACE->min_rx_int = BFD_IFACE->min_tx_int = $2; }
 | MIN RX INTERVAL expr_us { BFD_IFACE->min_rx_int = $4; }
 | MIN TX INTERVAL expr_us { BFD_IFACE->min_tx_int = $4; }
 | IDLE TX INTERVAL expr_us { BFD_IFACE->idle_tx_int = $4; }
 | MULTIPLIER expr { BFD_IFACE->multiplier = $2; }
 | PASSIVE bool { BFD_IFACE->passive = $2; }
 | AUTHENTICATION bfd_auth_type { BFD_IFACE->auth_type = $2; }
 | password_list {}
 ;

bfd_auth_type:
   NONE			 { $$ = BFD_AUTH_NONE; }
 | SIMPLE 		 { $$ = BFD_AUTH_SIMPLE; }
 | KEYED MD5		 { $$ = BFD_AUTH_KEYED_MD5; }
 | KEYED SHA1   	 { $$ = BFD_AUTH_KEYED_SHA1; }
 | METICULOUS KEYED MD5	 { $$ = BFD_AUTH_METICULOUS_KEYED_MD5; }
 | METICULOUS KEYED SHA1 { $$ = BFD_AUTH_METICULOUS_KEYED_SHA1; }
 ;

bfd_iface_opts:
   /* empty */
 | bfd_iface_opts bfd_iface_item ';'
 ;

bfd_iface_opt_list:
   /* empty */
 | '{' bfd_iface_opts '}'
 ;

bfd_iface:
  bfd_iface_start iface_patt_list_nopx bfd_iface_opt_list bfd_iface_finish;

bfd_multihop:
  bfd_iface_start bfd_iface_opt_list bfd_iface_finish
{ BFD_CFG->multihop = BFD_IFACE; };


bfd_neigh_iface:
   /* empty */ { $$ = NULL; }
 | '%' SYM { $$ = if_get_by_name($2->name); }
 | DEV text { $$ = if_get_by_name($2); }
 ;

bfd_neigh_local:
   /* empty */ { $$ = IPA_NONE; }
 | LOCAL ipa { $$ = $2; }
 ;

bfd_neigh_multihop:
   /* empty */ { $$ = 0; }
 | MULTIHOP bool { $$ = $2; }
 ;

bfd_neighbor: ipa bfd_neigh_iface bfd_neigh_local bfd_neigh_multihop
{
  this_bfd_neighbor = cfg_allocz(sizeof(struct bfd_neighbor));
  add_tail(&BFD_CFG->neigh_list, NODE this_bfd_neighbor);

  BFD_NEIGHBOR->addr = $1;
  BFD_NEIGHBOR->local = $3;
  BFD_NEIGHBOR->iface = $2;
  BFD_NEIGHBOR->multihop = $4;

  if ($4 && $2)
    cf_error("Neighbor cannot set both interface and multihop");

  if ($4 && ipa_zero($3))
    cf_error("Multihop neighbor requires specified local address");
};


CF_CLI_HELP(SHOW BFD, ..., [[Show information about BFD protocol]]);
CF_CLI(SHOW BFD SESSIONS, optsym, [<name>], [[Show information about BFD sessions]])
{ bfd_show_sessions(proto_get_named($4, &proto_bfd)); };

CF_CODE

CF_END
