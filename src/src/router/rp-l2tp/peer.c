/***********************************************************************
*
* peer.c
*
* Manage lists of peers for L2TP
*
* Copyright (C) 2002 by Roaring Penguin Software Inc.
*
* This software may be distributed under the terms of the GNU General
* Public License, Version 2, or (at your option) any later version.
*
* LIC: GPL
*
***********************************************************************/

static char const RCSID[] =
"$Id: peer.c,v 1.1 2004/02/04 04:01:51 kanki Exp $";

#include "l2tp.h"
#include <stddef.h>
#include <string.h>

static hash_table all_peers;
static int peer_process_option(EventSelector *es,
			       char const *name,
			       char const *value);

static l2tp_peer prototype;

static option_handler peer_option_handler = {
    NULL, "peer", peer_process_option
};

static int port;

static int handle_secret_option(EventSelector *es, l2tp_opt_descriptor *desc, char const *value);
static int set_lac_handler(EventSelector *es, l2tp_opt_descriptor *desc, char const *value);
static int set_lns_handler(EventSelector *es, l2tp_opt_descriptor *desc, char const *value);

/* Peer options */
static l2tp_opt_descriptor peer_opts[] = {
    /*  name               type                 addr */
    { "peer",              OPT_TYPE_IPADDR,   &prototype.addr.sin_addr.s_addr},
    { "secret",            OPT_TYPE_CALLFUNC, (void *) handle_secret_option},
    { "port",              OPT_TYPE_PORT,     &port },
    { "lac-handler",       OPT_TYPE_CALLFUNC, (void *) set_lac_handler},
    { "lns-handler",       OPT_TYPE_CALLFUNC, (void *) set_lns_handler},
    { "hide-avps",         OPT_TYPE_BOOL,     &prototype.hide_avps},
    { "retain-tunnel",     OPT_TYPE_BOOL,     &prototype.retain_tunnel},
    { "strict-ip-check",   OPT_TYPE_BOOL,     &prototype.validate_peer_ip},
    { NULL,                OPT_TYPE_BOOL,     NULL }
};

static int
set_lac_handler(EventSelector *es,
		l2tp_opt_descriptor *desc,
		char const *value)
{
    l2tp_lac_handler *handler = l2tp_session_find_lac_handler(value);
    if (!handler) {
	l2tp_set_errmsg("No LAC handler named '%s'", value);
	return -1;
    }
    prototype.lac_ops = handler->call_ops;
    return 0;
}

static int
set_lns_handler(EventSelector *es,
		l2tp_opt_descriptor *desc,
		char const *value)
{
    l2tp_lns_handler *handler = l2tp_session_find_lns_handler(value);
    if (!handler) {
	l2tp_set_errmsg("No LNS handler named '%s'", value);
	return -1;
    }
    prototype.lns_ops = handler->call_ops;
    return 0;
}

/**********************************************************************
* %FUNCTION: handle_secret_option
* %ARGUMENTS:
*  es -- event selector
*  desc -- descriptor
*  value -- the secret
* %RETURNS:
*  0
* %DESCRIPTION:
*  Copies secret to prototype
***********************************************************************/
static int
handle_secret_option(EventSelector *es,
		     l2tp_opt_descriptor *desc,
		     char const *value)
{
    strncpy(prototype.secret, value, MAX_SECRET_LEN);
    prototype.secret[MAX_SECRET_LEN-1] = 0;
    prototype.secret_len = strlen(prototype.secret);
    return 0;
}

/**********************************************************************
* %FUNCTION: peer_process_option
* %ARGUMENTS:
*  es -- event selector
*  name, value -- name and value of option
* %RETURNS:
*  0 on success, -1 on failure
* %DESCRIPTION:
*  Processes an option in the "peer" section
***********************************************************************/
static int
peer_process_option(EventSelector *es,
		    char const *name,
		    char const *value)
{
    l2tp_peer *peer;

    /* Special cases: begin and end */
    if (!strcmp(name, "*begin*")) {
	/* Switching in to peer context */
	memset(&prototype, 0, sizeof(prototype));
	prototype.validate_peer_ip = 1;
	port = 1701;
	return 0;
    }

    if (!strcmp(name, "*end*")) {
	/* Validate settings */
	uint16_t u16 = (uint16_t) port;
	prototype.addr.sin_port = htons(u16);
	prototype.addr.sin_family = AF_INET;

	/* Allow non-authenticated tunnels
	if (!prototype.secret_len) {
	    l2tp_set_errmsg("No secret supplied for peer");
	    return -1;
	}
	*/
	if (!prototype.lns_ops && !prototype.lac_ops) {
	    l2tp_set_errmsg("You must enable at least one of lns-handler or lac-handler");
	    return -1;
	}

	/* Add the peer */
	peer = l2tp_peer_insert(&prototype.addr);
	if (!peer) return -1;

	memcpy(&peer->secret, &prototype.secret, MAX_SECRET_LEN);
	peer->secret_len = prototype.secret_len;
	peer->lns_ops = prototype.lns_ops;
	peer->lac_ops = prototype.lac_ops;
	peer->hide_avps = prototype.hide_avps;
	peer->retain_tunnel = prototype.retain_tunnel;
	peer->validate_peer_ip = prototype.validate_peer_ip;
	return 0;
    }

    /* Process option */
    return l2tp_option_set(es, name, value, peer_opts);
}

/**********************************************************************
* %FUNCTION: peer_compute_hash
* %ARGUMENTS:
*  data -- a void pointer which is really a peer
* %RETURNS:
*  Inet address
***********************************************************************/
static unsigned int
peer_compute_hash(void *data)
{
    unsigned int hash = (unsigned int) (((l2tp_peer *) data)->addr.sin_addr.s_addr);
    return hash;
}

/**********************************************************************
* %FUNCTION: peer_compare
* %ARGUMENTS:
*  item1 -- first peer
*  item2 -- second peer
* %RETURNS:
*  0 if both peers have same ID, non-zero otherwise
***********************************************************************/
static int
peer_compare(void *item1, void *item2)
{
    return ((l2tp_peer *) item1)->addr.sin_addr.s_addr !=
	((l2tp_peer *) item2)->addr.sin_addr.s_addr;
}

/**********************************************************************
* %FUNCTION: peer_init
* %ARGUMENTS:
*  None
* %RETURNS:
*  Nothing
* %DESCRIPTION:
*  Initializes peer hash table
***********************************************************************/
void
l2tp_peer_init(void)
{
    hash_init(&all_peers,
	      offsetof(l2tp_peer, hash),
	      peer_compute_hash,
	      peer_compare);
    l2tp_option_register_section(&peer_option_handler);
}

/**********************************************************************
* %FUNCTION: peer_find
* %ARGUMENTS:
*  addr -- IP address of peer
* %RETURNS:
*  A pointer to the peer with given IP address, or NULL if not found.
* %DESCRIPTION:
*  Searches peer hash table for specified peer.
***********************************************************************/
l2tp_peer *
l2tp_peer_find(struct sockaddr_in *addr)
{
    l2tp_peer candidate;

    candidate.addr = *addr;

    return hash_find(&all_peers, &candidate);
}

/**********************************************************************
* %FUNCTION: peer_insert
* %ARGUMENTS:
*  addr -- IP address of peer
* %RETURNS:
*  NULL if insert failed, pointer to new peer structure otherwise
* %DESCRIPTION:
*  Inserts a new peer in the all_peers table
***********************************************************************/
l2tp_peer *
l2tp_peer_insert(struct sockaddr_in *addr)
{
    l2tp_peer *peer = malloc(sizeof(l2tp_peer));
    if (!peer) {
	l2tp_set_errmsg("peer_insert: Out of memory");
	return NULL;
    }
    memset(peer, 0, sizeof(*peer));

    peer->addr = *addr;
    hash_insert(&all_peers, peer);
    return peer;
}
