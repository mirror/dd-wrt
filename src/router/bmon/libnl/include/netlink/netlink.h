/*
 * netlink/netlink.h		Netlink Interface
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU Lesser General Public
 *	License as published by the Free Software Foundation version 2.1
 *	of the License.
 *
 * Copyright (c) 2003-2006 Thomas Graf <tgraf@suug.ch>
 */

#ifndef NETLINK_H_
#define NETLINK_H_

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netlink/types.h>
#include <netlink/netlink-compat.h>
#include <netlink/netlink-kernel.h>
#include <netlink/rtnetlink-kernel.h>
#include <netlink/handlers.h>

extern int nl_debug;

struct nl_handle;

/* General */
extern struct nl_handle *	nl_handle_alloc(void);
extern struct nl_handle *	nl_handle_alloc_nondefault(enum nl_cb_kind);
extern void			nl_handle_destroy(struct nl_handle *);
extern void			nl_join_groups(struct nl_handle *, int);
extern void			nl_disable_sequence_check(struct nl_handle *);

extern int			nl_join_group(struct nl_handle *, int);
extern int			nl_set_passcred(struct nl_handle *, int);

/* Access Functions */
extern pid_t			nl_handle_get_pid(struct nl_handle *);
extern void			nl_handle_set_pid(struct nl_handle *, pid_t);
extern pid_t			nl_handle_get_peer_pid(struct nl_handle *);
extern void			nl_handle_set_peer_pid(struct nl_handle *, pid_t);
extern int			nl_handle_get_fd(struct nl_handle *);
extern struct sockaddr_nl *	nl_handle_get_local_addr(struct nl_handle *);
extern struct sockaddr_nl *	nl_handle_get_peer_addr(struct nl_handle *);
extern struct nl_cb *		nl_handle_get_cb(struct nl_handle *);

/* Connection Management */
extern int			nl_connect(struct nl_handle *, int);
extern void			nl_close(struct nl_handle *);

/* Send */
extern int			nl_sendto(struct nl_handle *, void *, size_t);
extern int			nl_sendmsg(struct nl_handle *, struct nl_msg *,
					   struct msghdr *);
extern int			nl_send(struct nl_handle *, struct nl_msg *);
extern int			nl_send_auto_complete(struct nl_handle *,
						      struct nl_msg *);
extern int			nl_send_simple(struct nl_handle *, int, int,
					       void *, size_t);

/* Receive */
extern int			nl_recv(struct nl_handle *,
					struct sockaddr_nl *, unsigned char **,
					struct ucred **);

extern int			nl_recvmsgs(struct nl_handle *, struct nl_cb *);
extern int			nl_recvmsgs_def(struct nl_handle *);

extern int			nl_wait_for_ack(struct nl_handle *);

/* Netlink Family Translations */
extern char *			nl_nlfamily2str(int, char *, size_t);
extern int			nl_str2nlfamily(const char *);

#endif
