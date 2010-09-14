/*
 *	BIRD -- Core Configuration
 *
 *	(c) 1998--2000 Martin Mares <mj@ucw.cz>
 *      (c) 2004       Ondrej Filip <feela@network.cz>
 *
 *	Can be freely distributed and used under the terms of the GNU GPL.
 */

CF_HDR

#include "nest/rt-dev.h"
#include "nest/password.h"
#include "nest/cmds.h"
#include "lib/lists.h"

CF_DEFINES

static struct proto_config *this_proto;
static struct iface_patt *this_ipatt;
static struct iface_patt_node *this_ipn;
static list *this_p_list;
static struct password_item *this_p_item;
static int password_id;

static inline void
reset_passwords(void)
{
 this_p_list = NULL;
}

static inline list *
get_passwords(void)
{
  list *rv = this_p_list;
  this_p_list = NULL;
  return rv;
}


CF_DECLS

CF_KEYWORDS(ROUTER, ID, PROTOCOL, PREFERENCE, DISABLED, DEBUG, ALL, OFF, DIRECT)
CF_KEYWORDS(INTERFACE, IMPORT, EXPORT, FILTER, NONE, TABLE, STATES, ROUTES, FILTERS)
CF_KEYWORDS(PASSWORD, FROM, PASSIVE, TO, ID, EVENTS, PACKETS, PROTOCOLS, INTERFACES)
CF_KEYWORDS(PRIMARY, STATS, COUNT, FOR, COMMANDS, PREEXPORT, GENERATE)
CF_KEYWORDS(LISTEN, BGP, V6ONLY, ADDRESS, PORT, PASSWORDS, DESCRIPTION)
CF_KEYWORDS(RELOAD, IN, OUT, MRTDUMP, MESSAGES, RESTRICT, MEMORY)

CF_ENUM(T_ENUM_RTS, RTS_, DUMMY, STATIC, INHERIT, DEVICE, STATIC_DEVICE, REDIRECT,
	RIP, OSPF, OSPF_IA, OSPF_EXT1, OSPF_EXT2, BGP, PIPE)
CF_ENUM(T_ENUM_SCOPE, SCOPE_, HOST, LINK, SITE, ORGANIZATION, UNIVERSE)
CF_ENUM(T_ENUM_RTC, RTC_, UNICAST, BROADCAST, MULTICAST, ANYCAST)
CF_ENUM(T_ENUM_RTD, RTD_, ROUTER, DEVICE, BLACKHOLE, UNREACHABLE, PROHIBIT)

%type <i32> idval
%type <f> imexport
%type <r> rtable
%type <s> optsym
%type <ra> r_args
%type <i> echo_mask echo_size debug_mask debug_list debug_flag mrtdump_mask mrtdump_list mrtdump_flag export_or_preexport
%type <ps> proto_patt proto_patt2

CF_GRAMMAR

/* Setting of router ID */

CF_ADDTO(conf, rtrid)

rtrid: ROUTER ID idval ';' {
   new_config->router_id = $3;
   }
 ;

idval:
   NUM { $$ = $1; }
 | RTRID
 | IPA {
#ifndef IPV6
     $$ = ipa_to_u32($1);
#else
     cf_error("Router IDs must be entered as hexadecimal numbers or IPv4 addresses in IPv6 version");
#endif
   }
 ;


CF_ADDTO(conf, listen)

listen: LISTEN BGP listen_opts ';' ;

listen_opts:
   /* Nothing */
 | listen_opts listen_opt
 ;

listen_opt: 
   ADDRESS ipa { new_config->listen_bgp_addr = $2; }
 | PORT expr { new_config->listen_bgp_port = $2; }
 | V6ONLY { new_config->listen_bgp_flags |= SKF_V6ONLY; }
 ;


/* Creation of routing tables */

CF_ADDTO(conf, newtab)

newtab: TABLE SYM {
   rt_new_table($2);
   }
 ;

/* Definition of protocols */

CF_ADDTO(conf, proto)

proto_start: PROTOCOL
 ;

proto_name:
   /* EMPTY */ {
     struct symbol *s = cf_default_name(this_proto->protocol->template, &this_proto->protocol->name_counter);
     s->class = SYM_PROTO;
     s->def = this_proto;
     this_proto->name = s->name;
     }
 | SYM {
     cf_define_symbol($1, SYM_PROTO, this_proto);
     this_proto->name = $1->name;
   }
 ;

proto_item:
   /* EMPTY */
 | PREFERENCE expr {
     if ($2 < 0 || $2 > 0xFFFF) cf_error("Invalid preference");
     this_proto->preference = $2;
   }
 | DISABLED bool { this_proto->disabled = $2; }
 | DEBUG debug_mask { this_proto->debug = $2; }
 | MRTDUMP mrtdump_mask { this_proto->mrtdump = $2; }
 | IMPORT imexport { this_proto->in_filter = $2; }
 | EXPORT imexport { this_proto->out_filter = $2; }
 | TABLE rtable { this_proto->table = $2; }
 | ROUTER ID idval { this_proto->router_id = $3; }
 | DESCRIPTION TEXT { this_proto->dsc = $2; }
 ;

imexport:
   FILTER filter { $$ = $2; }
 | where_filter
 | ALL { $$ = FILTER_ACCEPT; }
 | NONE { $$ = FILTER_REJECT; }
 ;

rtable:
   SYM {
     if ($1->class != SYM_TABLE) cf_error("Table name expected");
     $$ = $1->def;
   }
 ;

CF_ADDTO(conf, debug_default)

debug_default:
   DEBUG PROTOCOLS debug_mask { new_config->proto_default_debug = $3; }
 | DEBUG COMMANDS expr { new_config->cli_debug = $3; }
 ;

/* MRTDUMP PROTOCOLS is in systep/unix/config.Y */

/* Interface patterns */

iface_patt_node_init:
   /* EMPTY */ {
     struct iface_patt_node *ipn = cfg_allocz(sizeof(struct iface_patt_node));
     add_tail(&this_ipatt->ipn_list, NODE ipn);
     this_ipn = ipn;
   }
 ;

iface_patt_node_body:
   TEXT { this_ipn->pattern = $1; this_ipn->prefix = IPA_NONE; this_ipn->pxlen = 0; }
 | prefix_or_ipa { this_ipn->pattern = NULL; this_ipn->prefix = $1.addr; this_ipn->pxlen = $1.len; }
 | TEXT prefix_or_ipa { this_ipn->pattern = $1; this_ipn->prefix = $2.addr; this_ipn->pxlen = $2.len; }
 ;

iface_negate:
       { this_ipn->positive = 1; }
 | '-' { this_ipn->positive = 0; }
 ;

iface_patt_node:
   iface_patt_node_init iface_negate iface_patt_node_body 
 ;


iface_patt_list:
   iface_patt_node
 | iface_patt_list ',' iface_patt_node
 ;


/* Direct device route protocol */

CF_ADDTO(proto, dev_proto '}')

dev_proto_start: proto_start DIRECT {
     struct rt_dev_config *p = proto_config_new(&proto_device, sizeof(struct rt_dev_config));
     this_proto = &p->c;
     p->c.preference = DEF_PREF_DIRECT;
     init_list(&p->iface_list);
   }
 ;

dev_proto:
   dev_proto_start proto_name '{'
 | dev_proto proto_item ';'
 | dev_proto dev_iface_patt ';'
 ;

dev_iface_init:
   /* EMPTY */ {
     struct rt_dev_config *p = (void *) this_proto;
     this_ipatt = cfg_allocz(sizeof(struct iface_patt));
     add_tail(&p->iface_list, NODE this_ipatt);
     init_list(&this_ipatt->ipn_list);
   }
 ;

dev_iface_patt:
   INTERFACE dev_iface_init iface_patt_list
 ;

/* Debug flags */

debug_mask:
   ALL { $$ = ~0; }
 | OFF { $$ = 0; }
 | '{' debug_list '}' { $$ = $2; }
 ;

debug_list:
   debug_flag
 | debug_list ',' debug_flag { $$ = $1 | $3; }
 ;

debug_flag:
   STATES	{ $$ = D_STATES; }
 | ROUTES	{ $$ = D_ROUTES; }
 | FILTERS	{ $$ = D_FILTERS; }
 | INTERFACES	{ $$ = D_IFACES; }
 | EVENTS	{ $$ = D_EVENTS; }
 | PACKETS	{ $$ = D_PACKETS; }
 ;

/* MRTDump flags */

mrtdump_mask:
   ALL { $$ = ~0; }
 | OFF { $$ = 0; }
 | '{' mrtdump_list '}' { $$ = $2; }
 ;

mrtdump_list:
   mrtdump_flag
 | mrtdump_list ',' mrtdump_flag { $$ = $1 | $3; }
 ;

mrtdump_flag:
   STATES	{ $$ = MD_STATES; }
 | MESSAGES	{ $$ = MD_MESSAGES; }
 ;

/* Password lists */

password_list:
   PASSWORDS '{' password_items '}'
 | password_item
;

password_items: 
    /* empty */
  | password_item ';' password_items
;

password_item:
    password_item_begin '{' password_item_params '}'
  | password_item_begin
;

password_item_begin:
   PASSWORD TEXT {
     if (!this_p_list) {
     	this_p_list = cfg_alloc(sizeof(list));
     	init_list(this_p_list);
        password_id = 1;
     }
     this_p_item = cfg_alloc(sizeof (struct password_item));
     this_p_item->password = $2;
     this_p_item->genfrom = 0;
     this_p_item->gento = TIME_INFINITY;
     this_p_item->accfrom = 0;
     this_p_item->accto = TIME_INFINITY;
     this_p_item->id = password_id++;
     add_tail(this_p_list, &this_p_item->n);
   }
;

password_item_params:
   /* empty */ { } 
 | GENERATE FROM datetime ';' password_item_params { this_p_item->genfrom = $3; }
 | GENERATE TO datetime ';' password_item_params { this_p_item->gento = $3; }
 | ACCEPT FROM datetime ';' password_item_params { this_p_item->accfrom = $3; }
 | ACCEPT TO datetime ';' password_item_params { this_p_item->accto = $3; }
 | ID expr ';' password_item_params { this_p_item->id = $2; if ($2 <= 0) cf_error("Password ID has to be greated than zero."); }
 ;



/* Core commands */
CF_CLI_HELP(SHOW, ..., [[Show status information]])

CF_CLI(SHOW STATUS,,, [[Show router status]])
{ cmd_show_status(); } ;

CF_CLI(SHOW MEMORY,,, [[Show memory usage]])
{ cmd_show_memory(); } ;

CF_CLI(SHOW PROTOCOLS, proto_patt2, [<protocol> | \"<pattern>\"], [[Show routing protocols]])
{ proto_apply_cmd($3, proto_cmd_show, 0, 0); } ;

CF_CLI(SHOW PROTOCOLS ALL, proto_patt2, [<protocol> | \"<pattern>\"], [[Show routing protocol details]])
{ proto_apply_cmd($4, proto_cmd_show, 0, 1); } ;

optsym:
   SYM
 | /* empty */ { $$ = NULL; }
 ;

CF_CLI(SHOW INTERFACES,,, [[Show network interfaces]])
{ if_show(); } ;

CF_CLI(SHOW INTERFACES SUMMARY,,, [[Show summary of network interfaces]])
{ if_show_summary(); } ;

CF_CLI(SHOW ROUTE, r_args, [[[<prefix>|for <prefix>|for <ip>] [table <t>] [filter <f>|where <cond>] [all] [primary] [(export|preexport) <p>] [protocol <p>] [stats|count]]], [[Show routing table]])
{ rt_show($3); } ;

r_args:
   /* empty */ {
     $$ = cfg_allocz(sizeof(struct rt_show_data));
     $$->pxlen = 256;
     $$->filter = FILTER_ACCEPT;
     $$->table = config->master_rtc->table;
   }
 | r_args prefix {
     $$ = $1;
     if ($$->pxlen != 256) cf_error("Only one prefix expected");
     $$->prefix = $2.addr;
     $$->pxlen = $2.len;
   }
 | r_args FOR prefix_or_ipa {
     $$ = $1;
     if ($$->pxlen != 256) cf_error("Only one prefix expected");
     $$->prefix = $3.addr;
     $$->pxlen = $3.len;
     $$->show_for = 1;
   }
 | r_args TABLE SYM {
     $$ = $1;
     if ($3->class != SYM_TABLE) cf_error("%s is not a table", $3->name);
     $$->table = ((struct rtable_config *)$3->def)->table;
   }
 | r_args FILTER filter {
     $$ = $1;
     if ($$->filter != FILTER_ACCEPT) cf_error("Filter specified twice");
     $$->filter = $3;
   }
 | r_args where_filter {
     $$ = $1;
     if ($$->filter != FILTER_ACCEPT) cf_error("Filter specified twice");
     $$->filter = $2;
   }
 | r_args ALL {
     $$ = $1;
     $$->verbose = 1;
   }
 | r_args PRIMARY {
     $$ = $1;
     $$->primary_only = 1;
   }
 | r_args export_or_preexport SYM {
     struct proto_config *c = (struct proto_config *) $3->def;
     $$ = $1;
     if ($$->export_mode) cf_error("Protocol specified twice");
     if ($3->class != SYM_PROTO || !c->proto) cf_error("%s is not a protocol", $3->name);
     $$->export_mode = $2;
     $$->primary_only = 1;
     $$->export_protocol = c->proto;
     $$->running_on_config = c->proto->cf->global;
   }
 | r_args PROTOCOL SYM {
     struct proto_config *c = (struct proto_config *) $3->def;
     $$ = $1;
     if ($$->show_protocol) cf_error("Protocol specified twice");
     if ($3->class != SYM_PROTO || !c->proto) cf_error("%s is not a protocol", $3->name);
     $$->show_protocol = c->proto;
     $$->running_on_config = c->proto->cf->global;
   }
 | r_args STATS {
     $$ = $1;
     $$->stats = 1;
   }
 | r_args COUNT {
     $$ = $1;
     $$->stats = 2;
   }
 ;

export_or_preexport:
   PREEXPORT { $$ = 1; }
 | EXPORT { $$ = 2; }
 ;

CF_CLI(SHOW SYMBOLS, optsym, [<symbol>], [[Show all known symbolic names]])
{ cmd_show_symbols($3); } ;

CF_CLI_HELP(DUMP, ..., [[Dump debugging information]])
CF_CLI(DUMP RESOURCES,,, [[Dump all allocated resource]])
{ rdump(&root_pool); cli_msg(0, ""); } ;
CF_CLI(DUMP SOCKETS,,, [[Dump open sockets]])
{ sk_dump_all(); cli_msg(0, ""); } ;
CF_CLI(DUMP INTERFACES,,, [[Dump interface information]])
{ if_dump_all(); cli_msg(0, ""); } ;
CF_CLI(DUMP NEIGHBORS,,, [[Dump neighbor cache]])
{ neigh_dump_all(); cli_msg(0, ""); } ;
CF_CLI(DUMP ATTRIBUTES,,, [[Dump attribute cache]])
{ rta_dump_all(); cli_msg(0, ""); } ;
CF_CLI(DUMP ROUTES,,, [[Dump routing table]])
{ rt_dump_all(); cli_msg(0, ""); } ;
CF_CLI(DUMP PROTOCOLS,,, [[Dump protocol information]])
{ protos_dump_all(); cli_msg(0, ""); } ;

CF_CLI(ECHO, echo_mask echo_size, [all | off | <mask>] [<buffer-size>], [[Configure echoing of log messages]]) {
  cli_set_log_echo(this_cli, $2, $3);
  cli_msg(0, "");
} ;

echo_mask:
   ALL { $$ = ~0; }
 | OFF { $$ = 0; }
 | NUM
 ;

echo_size:
   /* empty */ { $$ = 4096; }
 | NUM {
     if ($1 < 256 || $1 > 65536) cf_error("Invalid log buffer size");
     $$ = $1;
   }
 ;

CF_CLI(DISABLE, proto_patt, <protocol> | \"<pattern>\" | all, [[Disable protocol]])
{ proto_apply_cmd($2, proto_cmd_disable, 1, 0); } ;
CF_CLI(ENABLE, proto_patt, <protocol> | \"<pattern>\" | all, [[Enable protocol]])
{ proto_apply_cmd($2, proto_cmd_enable, 1, 0); } ;
CF_CLI(RESTART, proto_patt, <protocol> | \"<pattern>\" | all, [[Restart protocol]])
{ proto_apply_cmd($2, proto_cmd_restart, 1, 0); } ;
CF_CLI(RELOAD, proto_patt, <protocol> | \"<pattern>\" | all, [[Reload protocol]])
{ proto_apply_cmd($2, proto_cmd_reload, 1, CMD_RELOAD); } ;
CF_CLI(RELOAD IN, proto_patt, <protocol> | \"<pattern>\" | all, [[Reload protocol (just imported routes)]])
{ proto_apply_cmd($3, proto_cmd_reload, 1, CMD_RELOAD_IN); } ;
CF_CLI(RELOAD OUT, proto_patt, <protocol> | \"<pattern>\" | all, [[Reload protocol (just exported routes)]])
{ proto_apply_cmd($3, proto_cmd_reload, 1, CMD_RELOAD_OUT); } ;

CF_CLI_HELP(DEBUG, ..., [[Control protocol debugging via BIRD logs]])
CF_CLI(DEBUG, proto_patt debug_mask, (<protocol> | <pattern> | all) (all | off | { states | routes | filters | interfaces | events | packets }), [[Control protocol debugging via BIRD logs]])
{ proto_apply_cmd($2, proto_cmd_debug, 1, $3); } ;

CF_CLI_HELP(MRTDUMP, ..., [[Control protocol debugging via MRTdump files]])
CF_CLI(MRTDUMP, proto_patt mrtdump_mask, (<protocol> | <pattern> | all) (all | off | { states | messages }), [[Control protocol debugging via MRTdump format]])
{ proto_apply_cmd($2, proto_cmd_mrtdump, 1, $3); } ;

CF_CLI(RESTRICT,,,[[Restrict current CLI session to safe commands]])
{ this_cli->restricted = 1; cli_msg(16, "Access restricted"); } ;

proto_patt:
   SYM  { $$.ptr = $1; $$.patt = 0; }
 | ALL  { $$.ptr = NULL; $$.patt = 1; }
 | TEXT { $$.ptr = $1; $$.patt = 1; }
 ;

proto_patt2:
   SYM  { $$.ptr = $1; $$.patt = 0; }
 |      { $$.ptr = NULL; $$.patt = 1; }
 | TEXT { $$.ptr = $1; $$.patt = 1; }
 ;


CF_CODE

CF_END
