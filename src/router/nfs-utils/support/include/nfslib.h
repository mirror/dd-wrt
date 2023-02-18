/*
 * support/include/nfslib.h
 *
 * General support functions for NFS user-space programs.
 *
 * Copyright (C) 1995 Olaf Kirch <okir@monad.swb.de>
 */

#ifndef NFSLIB_H
#define NFSLIB_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <stdbool.h>
#include <paths.h>
#include <rpcsvc/nfs_prot.h>
#include <nfs/nfs.h>
#include "xlog.h"

#ifndef _PATH_EXPORTS
#define _PATH_EXPORTS		"/etc/exports"
#endif
#ifndef _PATH_EXPORTS_D
#define _PATH_EXPORTS_D         "/etc/exports.d"
#endif
#ifndef _EXT_EXPORT
#define _EXT_EXPORT             ".exports"
#endif
#ifndef _PATH_IDMAPDCONF
#define _PATH_IDMAPDCONF	"/etc/idmapd.conf"
#endif
#ifndef _PATH_PROC_EXPORTS
#define	_PATH_PROC_EXPORTS	"/proc/fs/nfs/exports"
#define	_PATH_PROC_EXPORTS_ALT	"/proc/fs/nfsd/exports"
#endif

#define ETAB		"etab"
#define ETABTMP		"etab.tmp"
#define ETABLCK 	".etab.lock"
#define RMTAB		"rmtab"
#define RMTABTMP	"rmtab.tmp"
#define RMTABLCK	".rmtab.lock"

struct state_paths {
	char *statefn;
	char *tmpfn;
	char *lockfn;
};

/* Maximum number of security flavors on an export: */
#define SECFLAVOR_COUNT 8

struct sec_entry {
	struct flav_info *flav;
	int flags;
};

/*
 * Data related to a single exports entry as returned by getexportent.
 * FIXME: export options should probably be parsed at a later time to
 * allow overrides when using exportfs.
 */
struct exportent {
	char *		e_hostname;
	char		e_path[NFS_MAXPATHLEN+1];
	int		e_flags;
	int		e_anonuid;
	int		e_anongid;
	int *		e_squids;
	int		e_nsquids;
	int *		e_sqgids;
	int		e_nsqgids;
	unsigned int	e_fsid;
	char *		e_mountpoint;
	int             e_fslocmethod;
	char *          e_fslocdata;
	char *		e_uuid;
	struct sec_entry e_secinfo[SECFLAVOR_COUNT+1];
	unsigned int	e_ttl;
	char *		e_realpath;
};

struct rmtabent {
	char		r_client[NFSCLNT_IDMAX+1];
	char		r_path[NFS_MAXPATHLEN+1];
	int		r_count;
};

/*
 * configuration file parsing
 */
void			setexportent(char *fname, char *type);
struct exportent *	getexportent(int,int);
void 			secinfo_show(FILE *fp, struct exportent *ep);
void			putexportent(struct exportent *xep);
void			endexportent(void);
struct exportent *	mkexportent(char *hname, char *path, char *opts);
void			dupexportent(struct exportent *dst,
					struct exportent *src);
int			updateexportent(struct exportent *eep, char *options);

extern struct state_paths rmtab;
int			setrmtabent(char *type);
struct rmtabent *	getrmtabent(int log, long *pos);
void			putrmtabent(struct rmtabent *xep, long *pos);
void			endrmtabent(void);
void			rewindrmtabent(void);
FILE *			fsetrmtabent(char *fname, char *type);
struct rmtabent *	fgetrmtabent(FILE *fp, int log, long *pos);
void			fputrmtabent(FILE *fp, struct rmtabent *xep, long *pos);
void			fendrmtabent(FILE *fp);
void			frewindrmtabent(FILE *fp);

_Bool state_setup_basedir(const char *, const char *);
int setup_state_path_names(const char *, const char *, const char *, const char *, struct state_paths *);
void free_state_path_names(struct state_paths *);

/* mydaemon */
void daemon_init(bool fg);
void daemon_ready(void);

/*
 * wildmat borrowed from INN
 */
int			wildmat(char *text, char *pattern);

int qword_get(char **bpp, char *dest, int bufsize);
int qword_get_int(char **bpp, int *anint);
void cache_flush(void);
void qword_add(char **bpp, int *lp, char *str);
void qword_addhex(char **bpp, int *lp, char *buf, int blen);
void qword_addint(char **bpp, int *lp, int n);
void qword_adduint(char **bpp, int *lp, unsigned int n);
void qword_addeol(char **bpp, int *lp);
int qword_get_uint(char **bpp, unsigned int *anint);

void closeall(int min);

int			svctcp_socket (u_long __number, int __reuse);
int			svcudp_socket (u_long __number);
int			svcsock_nonblock (int __sock);

/* Misc shared code prototypes */
size_t  strlcat(char *, const char *, size_t);
size_t  strlcpy(char *, const char *, size_t);
ssize_t atomicio(ssize_t (*f) (int, void*, size_t),
		 int, void *, size_t);

#ifdef HAVE_LIBTIRPC_SET_DEBUG
void  libtirpc_set_debug(char *name, int level, int use_stderr);
#endif

#define UNUSED(x) UNUSED_ ## x __attribute__((unused))

/*
 * Some versions of freeaddrinfo(3) do not tolerate being
 * passed a NULL pointer.
 */
static inline void nfs_freeaddrinfo(struct addrinfo *ai)
{
	if (ai) {
		freeaddrinfo(ai);
	}
}
#endif /* NFSLIB_H */
