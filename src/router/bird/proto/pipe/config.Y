/*
 *	BIRD -- Table-to-Table Protocol Configuration
 *
 *	(c) 1999 Martin Mares <mj@ucw.cz>
 *
 *	Can be freely distributed and used under the terms of the GNU GPL.
 */

CF_HDR

#include "proto/pipe/pipe.h"

CF_DEFINES

#define PIPE_CFG ((struct pipe_config *) this_proto)

CF_DECLS

CF_KEYWORDS(PIPE, PEER, TABLE, MODE, OPAQUE, TRANSPARENT)

CF_GRAMMAR

CF_ADDTO(proto, pipe_proto '}')

pipe_proto_start: proto_start PIPE {
     this_proto = proto_config_new(&proto_pipe, sizeof(struct pipe_config), $1);
     PIPE_CFG->mode = PIPE_TRANSPARENT;
  }
 ;

pipe_proto:
   pipe_proto_start proto_name '{'
 | pipe_proto proto_item ';'
 | pipe_proto PEER TABLE SYM ';' {
     if ($4->class != SYM_TABLE)
       cf_error("Routing table name expected");
     PIPE_CFG->peer = $4->def;
   }
 | pipe_proto MODE OPAQUE ';' { PIPE_CFG->mode = PIPE_OPAQUE; }
 | pipe_proto MODE TRANSPARENT ';' { PIPE_CFG->mode = PIPE_TRANSPARENT; }
 ;

CF_CODE

CF_END
