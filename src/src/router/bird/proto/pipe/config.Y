/*
 *	BIRD -- Table-to-Table Protocol Configuration
 *
 *	(c) 1999 Martin Mares <mj@ucw.cz>
 *
 *	Can be freely distributed and used under the terms of the GNU GPL.
 */

CF_HDR

#include "proto/pipe/pipe.h"

CF_DECLS

CF_KEYWORDS(PIPE, PEER, TABLE)

CF_GRAMMAR

CF_ADDTO(proto, pipe_proto '}')

pipe_proto_start: proto_start PIPE {
     this_proto = proto_config_new(&proto_pipe, sizeof(struct pipe_config));
     this_proto->preference = DEF_PREF_PIPE;
  }
 ;

pipe_proto:
   pipe_proto_start proto_name '{'
 | pipe_proto proto_item ';'
 | pipe_proto PEER TABLE SYM ';' {
     if ($4->class != SYM_TABLE)
       cf_error("Routing table name expected");
     ((struct pipe_config *) this_proto)->peer = $4->def;
   }
 ;

CF_CODE

CF_END
