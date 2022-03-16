/*
 *	BIRD -- Multi-Threaded Routing Toolkit (MRT) Protocol
 *
 *	(c) 2017--2018 Ondrej Zajicek <santiago@crfreenet.org>
 *	(c) 2017--2018 CZ.NIC z.s.p.o.
 *
 *	Can be freely distributed and used under the terms of the GNU GPL.
 */

CF_HDR

#include "proto/mrt/mrt.h"

CF_DEFINES

#define MRT_CFG ((struct mrt_config *) this_proto)

CF_DECLS

CF_KEYWORDS(MRT, TABLE, FILTER, FILENAME, PERIOD, ALWAYS, ADD, PATH, DUMP, TO)

%type <md> mrt_dump_args

CF_GRAMMAR

CF_ADDTO(proto, mrt_proto)

mrt_proto_start: proto_start MRT
{
  this_proto = proto_config_new(&proto_mrt, $1);
};

mrt_proto_item:
   proto_item
 | TABLE TEXT		{ MRT_CFG->table_expr = $2; }
 | FILTER filter	{ MRT_CFG->filter = $2; }
 | where_filter		{ MRT_CFG->filter = $1; }
 | FILENAME text	{ MRT_CFG->filename = $2; }
 | PERIOD expr		{ MRT_CFG->period = $2; }
 | ALWAYS ADD PATH bool	{ MRT_CFG->always_add_path = $4; }
 ;

mrt_proto_opts:
   /* empty */
 | mrt_proto_opts mrt_proto_item ';'
 ;

mrt_proto:
   mrt_proto_start proto_name '{' mrt_proto_opts '}' { mrt_check_config(this_proto); };

CF_CLI_HELP(MRT DUMP, [table <name>|\"<pattern>\"] [to \"<file>\"] [filter <filter>|where <where filter>] , [[Save MRT Table Dump into a file]])
CF_CLI(MRT DUMP, mrt_dump_args, [table <name>|\"<pattern>\"] [to \"<file>\"] [filter <filter>|where <where filter>], [[Save mrt table dump v2 of table name <t> right now]])
{ mrt_dump_cmd($3); } ;

mrt_dump_args:
   /* empty */ { $$ = cfg_allocz(sizeof(struct mrt_dump_data)); }
 | mrt_dump_args TABLE rtable	{ $$ = $1; $$->table_ptr = $3->table; }
 | mrt_dump_args TABLE TEXT	{ $$ = $1; $$->table_expr = $3; }
 | mrt_dump_args FILTER filter	{ $$ = $1; $$->filter = $3; }
 | mrt_dump_args where_filter	{ $$ = $1; $$->filter = $2; }
 | mrt_dump_args TO text 	{ $$ = $1; $$->filename = $3; }
 ;


CF_CODE

CF_END
