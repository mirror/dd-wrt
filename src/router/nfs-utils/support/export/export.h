/*
 * Copyright (C) 2021 Red Hat <nfs@redhat.com>
 *
 * support/export/export.h
 *
 * Declarations for export support
 */

#ifndef EXPORT_H
#define EXPORT_H

#include "nfslib.h"
#include "exportfs.h"

unsigned int	auth_reload(void);
nfs_export *	auth_authenticate(const char *what,
					const struct sockaddr *caller,
					const char *path);

void		cache_open(void);
void		cache_set_fds(fd_set *fdset);
int		cache_process_req(fd_set *readfds);
void		cache_process_loop(void);

void		v4clients_init(void);
void		v4clients_set_fds(fd_set *fdset);
int		v4clients_process(fd_set *fdset);

struct nfs_fh_len *
		cache_get_filehandle(nfs_export *exp, int len, char *p);
int		cache_export(nfs_export *exp, char *path);

bool ipaddr_client_matches(nfs_export *exp, struct addrinfo *ai);
bool namelist_client_matches(nfs_export *exp, char *dom);
bool client_matches(nfs_export *exp, char *dom, struct addrinfo *ai);

static inline bool is_ipaddr_client(char *dom)
{
	return dom[0] == '$';
}
#endif /* EXPORT__H */
