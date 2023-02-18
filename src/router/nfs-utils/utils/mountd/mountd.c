/*
 * utils/mountd/mountd.c
 *
 * Authenticate mount requests and retrieve file handle.
 *
 * Copyright (C) 1995, 1996 Olaf Kirch <okir@monad.swb.de>
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <signal.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/resource.h>
#include <sys/wait.h>

#include "conffile.h"
#include "xmalloc.h"
#include "misc.h"
#include "mountd.h"
#include "rpcmisc.h"
#include "pseudoflavors.h"
#include "nfsd_path.h"
#include "nfslib.h"
#include "export.h"

extern void my_svc_run(void);

static void		usage(const char *, int exitcode);
static exports		get_exportlist(void);
static struct nfs_fh_len *get_rootfh(struct svc_req *, dirpath *, nfs_export **, mountstat3 *, int v3);

int reverse_resolve = 0;
int manage_gids;
int use_ipaddr = -1;

/* PRC: a high-availability callout program can be specified with -H
 * When this is done, the program will receive callouts whenever clients
 * send mount or unmount requests -- the callout is not needed for 2.6 kernel */
char *ha_callout_prog = NULL;

/* Number of mountd threads to start.   Default is 1 and
 * that's probably enough unless you need hundreds of
 * clients to be able to mount at once.  */
static int num_threads = 1;
/* Arbitrary limit on number of threads */
#define MAX_THREADS 64

static struct option longopts[] =
{
	{ "foreground", 0, 0, 'F' },
	{ "descriptors", 1, 0, 'o' },
	{ "debug", 1, 0, 'd' },
	{ "help", 0, 0, 'h' },
	{ "nfs-version", 1, 0, 'V' },
	{ "no-nfs-version", 1, 0, 'N' },
	{ "version", 0, 0, 'v' },
	{ "port", 1, 0, 'p' },
	{ "no-tcp", 0, 0, 'n' },
	{ "ha-callout", 1, 0, 'H' },
	{ "state-directory-path", 1, 0, 's' },
	{ "num-threads", 1, 0, 't' },
	{ "reverse-lookup", 0, 0, 'r' },
	{ "manage-gids", 0, 0, 'g' },
	{ "no-udp", 0, 0, 'u' },
	{ "log-auth", 0, 0, 'l'},
	{ "cache-use-ipaddr", 0, 0, 'i'},
	{ "ttl", 1, 0, 'T'},
	{ NULL, 0, 0, 0 }
};
static char shortopts[] = "o:nFd:p:P:hH:N:V:vurs:t:gliT:";

#define NFSVERSBIT(vers)	(0x1 << (vers - 1))
#define NFSVERSBIT_ALL		(NFSVERSBIT(2) | NFSVERSBIT(3) | NFSVERSBIT(4))

static int nfs_version = NFSVERSBIT_ALL;

static int version2(void)
{
	return nfs_version & NFSVERSBIT(2);
}

static int version3(void)
{
	return nfs_version & NFSVERSBIT(3);
}

static int version23(void)
{
	return nfs_version & (NFSVERSBIT(2) | NFSVERSBIT(3));
}

static int version_any(void)
{
	return nfs_version & NFSVERSBIT_ALL;
}

static void
unregister_services (void)
{
	nfs_svc_unregister(MOUNTPROG, MOUNTVERS);
	nfs_svc_unregister(MOUNTPROG, MOUNTVERS_POSIX);
	nfs_svc_unregister(MOUNTPROG, MOUNTVERS_NFSV3);
}

static void
cleanup_lockfiles (void)
{
	unlink(etab.lockfn);
	unlink(rmtab.lockfn);
}

/* Wait for all worker child processes to exit and reap them */
static void
wait_for_workers (void)
{
	int status;
	pid_t pid;

	for (;;) {

		pid = waitpid(0, &status, 0);

		if (pid < 0) {
			if (errno == ECHILD)
				return; /* no more children */
			xlog(L_FATAL, "mountd: can't wait: %s\n",
					strerror(errno));
		}

		/* Note: because we SIG_IGN'd SIGCHLD earlier, this
		 * does not happen on 2.6 kernels, and waitpid() blocks
		 * until all the children are dead then returns with
		 * -ECHILD.  But, we don't need to do anything on the
		 * death of individual workers, so we don't care. */
		xlog(L_NOTICE, "mountd: reaped child %d, status %d\n",
				(int)pid, status);
	}
}

/* Fork num_threads worker children and wait for them */
static void
fork_workers(void)
{
	int i;
	pid_t pid;

	xlog(L_NOTICE, "mountd: starting %d threads\n", num_threads);

	for (i = 0 ; i < num_threads ; i++) {
		pid = fork();
		if (pid < 0) {
			xlog(L_FATAL, "mountd: cannot fork: %s\n",
					strerror(errno));
		}
		if (pid == 0) {
			/* worker child */

			/* Re-enable the default action on SIGTERM et al
			 * so that workers die naturally when sent them.
			 * Only the parent unregisters with pmap and
			 * hence needs to do special SIGTERM handling. */
			struct sigaction sa;
			sa.sa_handler = SIG_DFL;
			sa.sa_flags = 0;
			sigemptyset(&sa.sa_mask);
			sigaction(SIGHUP, &sa, NULL);
			sigaction(SIGINT, &sa, NULL);
			sigaction(SIGTERM, &sa, NULL);

			/* fall into my_svc_run in caller */
			return;
		}
	}

	/* in parent */
	wait_for_workers();
	unregister_services();
	cleanup_lockfiles();
	free_state_path_names(&etab);
	free_state_path_names(&rmtab);
	xlog(L_NOTICE, "mountd: no more workers, exiting\n");
	exit(0);
}

/*
 * Signal handler.
 */
static void 
killer (int sig)
{
	unregister_services();
	if (num_threads > 1) {
		/* play Kronos and eat our children */
		kill(0, SIGTERM);
		wait_for_workers();
	}
	cleanup_lockfiles();
	free_state_path_names(&etab);
	free_state_path_names(&rmtab);
	xlog (L_NOTICE, "Caught signal %d, un-registering and exiting.", sig);
	exit(0);
}

static void
sig_hup (int UNUSED(sig))
{
	/* don't exit on SIGHUP */
	xlog (L_NOTICE, "Received SIGHUP... Ignoring.\n");
	return;
}

bool_t
mount_null_1_svc(struct svc_req *rqstp, void *UNUSED(argp), 
	void *UNUSED(resp))
{
	struct sockaddr *sap = nfs_getrpccaller(rqstp->rq_xprt);
	char buf[INET6_ADDRSTRLEN];

	xlog(D_CALL, "Received NULL request from %s",
		host_ntop(sap, buf, sizeof(buf)));

	return 1;
}

bool_t
mount_mnt_1_svc(struct svc_req *rqstp, dirpath *path, fhstatus *res)
{
	struct sockaddr *sap = nfs_getrpccaller(rqstp->rq_xprt);
	char buf[INET6_ADDRSTRLEN];
	struct nfs_fh_len *fh;

	xlog(D_CALL, "Received MNT1(%s) request from %s", *path,
		host_ntop(sap, buf, sizeof(buf)));

	fh = get_rootfh(rqstp, path, NULL, &res->fhs_status, 0);
	if (fh)
		memcpy(&res->fhstatus_u.fhs_fhandle, fh->fh_handle, 32);
	return 1;
}

bool_t
mount_dump_1_svc(struct svc_req *rqstp, void *UNUSED(argp), mountlist *res)
{
	struct sockaddr *sap = nfs_getrpccaller(rqstp->rq_xprt);
	char buf[INET6_ADDRSTRLEN];

	xlog(D_CALL, "Received DUMP request from %s",
		host_ntop(sap, buf, sizeof(buf)));

	*res = mountlist_list();

	return 1;
}

bool_t
mount_umnt_1_svc(struct svc_req *rqstp, dirpath *argp, void *UNUSED(resp))
{
	struct sockaddr *sap = nfs_getrpccaller(rqstp->rq_xprt);
	nfs_export	*exp;
	char		*p = *argp;
	char		rpath[MAXPATHLEN+1];
	char		buf[INET6_ADDRSTRLEN];

	if (*p == '\0')
		p = "/";

	if (nfsd_realpath(p, rpath) != NULL) {
		rpath[sizeof (rpath) - 1] = '\0';
		p = rpath;
	}

	xlog(D_CALL, "Received UMNT(%s) request from %s", p,
		host_ntop(sap, buf, sizeof(buf)));

	exp = auth_authenticate("unmount", sap, p);
	if (exp == NULL)
		return 1;

	mountlist_del(host_ntop(sap, buf, sizeof(buf)), p);
	return 1;
}

bool_t
mount_umntall_1_svc(struct svc_req *rqstp, void *UNUSED(argp), 
	void *UNUSED(resp))
{
	struct sockaddr *sap = nfs_getrpccaller(rqstp->rq_xprt);
	char		buf[INET6_ADDRSTRLEN];

	xlog(D_CALL, "Received UMNTALL request from %s",
		host_ntop(sap, buf, sizeof(buf)));

	/* Reload /etc/exports if necessary */
	auth_reload();

	mountlist_del_all(nfs_getrpccaller(rqstp->rq_xprt));
	return 1;
}

bool_t
mount_export_1_svc(struct svc_req *rqstp, void *UNUSED(argp), exports *resp)
{
	struct sockaddr *sap = nfs_getrpccaller(rqstp->rq_xprt);
	char buf[INET6_ADDRSTRLEN];

	xlog(D_CALL, "Received EXPORT request from %s.",
		host_ntop(sap, buf, sizeof(buf)));

	*resp = get_exportlist();
		
	return 1;
}

bool_t
mount_exportall_1_svc(struct svc_req *rqstp, void *UNUSED(argp), exports *resp)
{
	struct sockaddr *sap = nfs_getrpccaller(rqstp->rq_xprt);
	char buf[INET6_ADDRSTRLEN];

	xlog(D_CALL, "Received EXPORTALL request from %s.",
		host_ntop(sap, buf, sizeof(buf)));

	*resp = get_exportlist();

	return 1;
}

/*
 * MNTv2 pathconf procedure
 *
 * The protocol doesn't include a status field, so Sun apparently considers
 * it good practice to let anyone snoop on your system, even if it's
 * pretty harmless data such as pathconf. We don't.
 *
 * Besides, many of the pathconf values don't make much sense on NFS volumes.
 * FIFOs and tty device files represent devices on the *client*, so there's
 * no point in getting the server's buffer sizes etc.
 */
bool_t
mount_pathconf_2_svc(struct svc_req *rqstp, dirpath *path, ppathcnf *res)
{
	struct sockaddr *sap = nfs_getrpccaller(rqstp->rq_xprt);
	struct stat	stb;
	nfs_export	*exp;
	char		rpath[MAXPATHLEN+1];
	char		*p = *path;
	char buf[INET6_ADDRSTRLEN];

	memset(res, 0, sizeof(*res));

	if (*p == '\0')
		p = "/";

	/* Reload /etc/exports if necessary */
	auth_reload();

	/* Resolve symlinks */
	if (nfsd_realpath(p, rpath) != NULL) {
		rpath[sizeof (rpath) - 1] = '\0';
		p = rpath;
	}

	xlog(D_CALL, "Received PATHCONF(%s) request from %s", p,
		host_ntop(sap, buf, sizeof(buf)));

	/* Now authenticate the intruder... */
	exp = auth_authenticate("pathconf", sap, p);
	if (exp == NULL)
		return 1;
	else if (nfsd_path_stat(p, &stb) < 0) {
		xlog(L_WARNING, "can't stat exported dir %s: %s",
				p, strerror(errno));
		return 1;
	}

	res->pc_link_max  = pathconf(p, _PC_LINK_MAX);
	res->pc_max_canon = pathconf(p, _PC_MAX_CANON);
	res->pc_max_input = pathconf(p, _PC_MAX_INPUT);
	res->pc_name_max  = pathconf(p, _PC_NAME_MAX);
	res->pc_path_max  = pathconf(p, _PC_PATH_MAX);
	res->pc_pipe_buf  = pathconf(p, _PC_PIPE_BUF);
	res->pc_vdisable  = pathconf(p, _PC_VDISABLE);

	/* Can't figure out what to do with pc_mask */
	res->pc_mask[0]   = 0;
	res->pc_mask[1]   = 0;

	return 1;
}

/*
 * We should advertise the preferred flavours first. (See RFC 2623
 * section 2.7.)  We leave that to the administrator, by advertising
 * flavours in the order they were listed in /etc/exports.  AUTH_NULL is
 * dropped from the list to avoid backward compatibility issue with
 * older Linux clients, who inspect the list in reversed order.
 *
 * XXX: It might be more helpful to rearrange these so that flavors
 * giving more access (as determined from readonly and id-squashing
 * options) come first.  (If we decide to do that we should probably do
 * that when reading the exports rather than here.)
 */
static void set_authflavors(struct mountres3_ok *ok, nfs_export *exp)
{
	struct sec_entry *s;
	static int flavors[SECFLAVOR_COUNT];
	int i = 0;

	for (s = exp->m_export.e_secinfo; s->flav; s++) {
		if (s->flav->fnum == AUTH_NULL)
			continue;
		flavors[i] = s->flav->fnum;
		i++;
	}
	if (i == 0) {
		/* default when there is no sec= option: */
		i = 1;
		flavors[0] = AUTH_UNIX;
	}
	ok->auth_flavors.auth_flavors_val = flavors;
	ok->auth_flavors.auth_flavors_len = i;
}

/*
 * NFSv3 MOUNT procedure
 */
bool_t
mount_mnt_3_svc(struct svc_req *rqstp, dirpath *path, mountres3 *res)
{
	struct sockaddr *sap = nfs_getrpccaller(rqstp->rq_xprt);
	struct mountres3_ok *ok = &res->mountres3_u.mountinfo;
	char buf[INET6_ADDRSTRLEN];
	nfs_export *exp;
	struct nfs_fh_len *fh;

	xlog(D_CALL, "Received MNT3(%s) request from %s", *path,
		host_ntop(sap, buf, sizeof(buf)));

	fh = get_rootfh(rqstp, path, &exp, &res->fhs_status, 1);
	if (!fh)
		return 1;

	ok->fhandle.fhandle3_len = fh->fh_size;
	ok->fhandle.fhandle3_val = (char *)fh->fh_handle;
	set_authflavors(ok, exp);
	return 1;
}

static struct nfs_fh_len *
get_rootfh(struct svc_req *rqstp, dirpath *path, nfs_export **expret,
		mountstat3 *error, int v3)
{
	struct sockaddr *sap = nfs_getrpccaller(rqstp->rq_xprt);
	struct stat	stb, estb;
	nfs_export	*exp;
	struct nfs_fh_len *fh;
	char		rpath[MAXPATHLEN+1];
	char		*p = *path;
	char		buf[INET6_ADDRSTRLEN];

	if (*p == '\0')
		p = "/";

	/* Reload /var/lib/nfs/etab if necessary */
	auth_reload();

	/* Resolve symlinks */
	if (nfsd_realpath(p, rpath) != NULL) {
		rpath[sizeof (rpath) - 1] = '\0';
		p = rpath;
	}

	/* Now authenticate the intruder... */
	exp = auth_authenticate("mount", sap, p);
	if (exp == NULL) {
		*error = MNT3ERR_ACCES;
		return NULL;
	}
	if (nfsd_path_stat(p, &stb) < 0) {
		xlog(L_WARNING, "can't stat exported dir %s: %s",
				p, strerror(errno));
		if (errno == ENOENT)
			*error = MNT3ERR_NOENT;
		else
			*error = MNT3ERR_ACCES;
		return NULL;
	}
	if (!S_ISDIR(stb.st_mode) && !S_ISREG(stb.st_mode)) {
		xlog(L_WARNING, "%s is not a directory or regular file", p);
		*error = MNT3ERR_NOTDIR;
		return NULL;
	}
	if (nfsd_path_stat(exp->m_export.e_path, &estb) < 0) {
		xlog(L_WARNING, "can't stat export point %s: %s",
		     p, strerror(errno));
		*error = MNT3ERR_NOENT;
		return NULL;
	}
	if (estb.st_dev != stb.st_dev
	    && !(exp->m_export.e_flags & NFSEXP_CROSSMOUNT)) {
		xlog(L_WARNING, "request to export directory %s below nearest filesystem %s",
		     p, exp->m_export.e_path);
		*error = MNT3ERR_ACCES;
		return NULL;
	}
	if (exp->m_export.e_mountpoint &&
		   !check_is_mountpoint(exp->m_export.e_mountpoint[0]?
				  exp->m_export.e_mountpoint:
				  exp->m_export.e_path,
				  nfsd_path_lstat)) {
		xlog(L_WARNING, "request to export an unmounted filesystem: %s",
		     p);
		*error = MNT3ERR_NOENT;
		return NULL;
	}

	/* This will be a static private nfs_export with just one
	 * address.  We feed it to kernel then extract the filehandle,
	 */

	if (cache_export(exp, p)) {
		*error = MNT3ERR_ACCES;
		return NULL;
	}
	fh = cache_get_filehandle(exp, v3?64:32, p);
	if (fh == NULL) {
		*error = MNT3ERR_ACCES;
		return NULL;
	}
	*error = MNT_OK;
	mountlist_add(host_ntop(sap, buf, sizeof(buf)), p);
	if (expret)
		*expret = exp;
	return fh;
}

static void remove_all_clients(exportnode *e)
{
	struct groupnode *g, *ng;

	for (g = e->ex_groups; g; g = ng) {
		ng = g->gr_next;
		xfree(g->gr_name);
		xfree(g);
	}
	e->ex_groups = NULL;
}

static void free_exportlist(exports *elist)
{
	struct exportnode *e, *ne;

	for (e = *elist; e != NULL; e = ne) {
		ne = e->ex_next;
		remove_all_clients(e);
		xfree(e->ex_dir);
		xfree(e);
	}
	*elist = NULL;
}

static void prune_clients(nfs_export *exp, struct exportnode *e)
{
	struct addrinfo *ai = NULL;
	struct groupnode *c, **cp;

	cp = &e->ex_groups;
	while ((c = *cp) != NULL) {
		if (client_gettype(c->gr_name) == MCL_FQDN
		    && (ai = host_addrinfo(c->gr_name))) {
			if (client_check(exp->m_client, ai)) {
				*cp = c->gr_next;
				xfree(c->gr_name);
				xfree(c);
				nfs_freeaddrinfo(ai);
				continue;
			}
			nfs_freeaddrinfo(ai);
		}
		cp = &(c->gr_next);
	}
}

static exportnode *lookup_or_create_elist_entry(exports *elist, nfs_export *exp)
{
	exportnode *e;

	for (e = *elist; e != NULL; e = e->ex_next) {
		if (!strcmp(exp->m_export.e_path, e->ex_dir))
			return e;
	}
	e = xmalloc(sizeof(*e));
	e->ex_next = *elist;
	e->ex_groups = NULL;
	e->ex_dir = xstrdup(exp->m_export.e_path);
	*elist = e;
	return e;
}

static void insert_group(struct exportnode *e, char *newname)
{
	struct groupnode *g;

	for (g = e->ex_groups; g; g = g->gr_next)
		if (!strcmp(g->gr_name, newname))
			return;

	g = xmalloc(sizeof(*g));
	g->gr_name = xstrdup(newname);
	g->gr_next = e->ex_groups;
	e->ex_groups = g;
}

static exports
get_exportlist(void)
{
	static exports		elist = NULL;
	struct exportnode	*e;
	nfs_export		*exp;
	int			i;
	static unsigned int	ecounter;
	unsigned int		acounter;

	acounter = auth_reload();
	if (elist && acounter == ecounter)
		return elist;

	ecounter = acounter;

	free_exportlist(&elist);

	for (i = 0; i < MCL_MAXTYPES; i++) {
		for (exp = exportlist[i].p_head; exp; exp = exp->m_next) {
			 /* Don't show pseudo exports */
			if (exp->m_export.e_flags & NFSEXP_V4ROOT)
				continue;
			e = lookup_or_create_elist_entry(&elist, exp);

			/* exports to "*" absorb any others */
			if (i == MCL_ANONYMOUS && e->ex_groups) {
				remove_all_clients(e);
				continue;
			}
			/* non-FQDN's absorb FQDN's they contain: */
			if (i != MCL_FQDN && e->ex_groups)
				prune_clients(exp, e);

			if (exp->m_export.e_hostname[0] != '\0')
				insert_group(e, exp->m_export.e_hostname);
		}
	}

	return elist;
}

int	vers;
int	port = 0;
int	descriptors = 0;

inline static void 
read_mountd_conf(char **argv)
{
	char	*s;
	int	ttl;

	conf_init_file(NFS_CONFFILE);

	xlog_set_debug("mountd");
	manage_gids = conf_get_bool("mountd", "manage-gids", manage_gids);
	descriptors = conf_get_num("mountd", "descriptors", descriptors);
	port = conf_get_num("mountd", "port", port);
	num_threads = conf_get_num("mountd", "threads", num_threads);
	reverse_resolve = conf_get_bool("mountd", "reverse-lookup", reverse_resolve);
	ha_callout_prog = conf_get_str("mountd", "ha-callout");
	if (conf_get_bool("mountd", "cache-use-ipaddr", 0))
		use_ipaddr = 2;

	s = conf_get_str("mountd", "state-directory-path");
	if (s && !state_setup_basedir(argv[0], s))
		exit(1);

	/* NOTE: following uses "nfsd" section of nfs.conf !!!! */
	if (conf_get_bool("nfsd", "udp", NFSCTL_UDPISSET(_rpcprotobits)))
		NFSCTL_UDPSET(_rpcprotobits);
	else
		NFSCTL_UDPUNSET(_rpcprotobits);
	if (conf_get_bool("nfsd", "tcp", NFSCTL_TCPISSET(_rpcprotobits)))
		NFSCTL_TCPSET(_rpcprotobits);
	else
		NFSCTL_TCPUNSET(_rpcprotobits);
	for (vers = 2; vers <= 4; vers++) {
		char tag[20];
		sprintf(tag, "vers%d", vers);
		if (conf_get_bool("nfsd", tag, NFSCTL_VERISSET(nfs_version, vers)))
			NFSCTL_VERSET(nfs_version, vers);
		else
			NFSCTL_VERUNSET(nfs_version, vers);
	}

	ttl = conf_get_num("mountd", "ttl", default_ttl);
	if (ttl > 0)
		default_ttl = ttl;
}

int
main(int argc, char **argv)
{
	char	*progname;
	unsigned int listeners = 0;
	int	foreground = 0;
	int	c;
	int	ttl;
	struct sigaction sa;
	struct rlimit rlim;

	/* Set the basename */
	if ((progname = strrchr(argv[0], '/')) != NULL)
		progname++;
	else
		progname = argv[0];

	/* Initialize logging. */
	xlog_open(progname);

	/* Read in config setting */
	read_mountd_conf(argv);

	/* Parse the command line options and arguments. */
	opterr = 0;
	while ((c = getopt_long(argc, argv, shortopts, longopts, NULL)) != EOF)
		switch (c) {
		case 'g':
			manage_gids = 1;
			break;
		case 'o':
			descriptors = atoi(optarg);
			if (descriptors <= 0) {
				fprintf(stderr, "%s: bad descriptors: %s\n",
					progname, optarg);
				usage(progname, 1);
			}
			break;
		case 'F':
			foreground = 1;
			break;
		case 'd':
			xlog_sconfig(optarg, 1);
			break;
		case 'H': /* PRC: specify a high-availability callout program */
			ha_callout_prog = optarg;
			break;
		case 'h':
			usage(progname, 0);
			break;
		case 'P':	/* XXX for nfs-server compatibility */
		case 'p':
			port = atoi(optarg);
			if (port <= 0 || port > 65535) {
				fprintf(stderr, "%s: bad port number: %s\n",
					progname, optarg);
				usage(progname, 1);
			}
			break;
		case 'N':
			vers = atoi(optarg);
			if (vers < 2 || vers > 4) {
				fprintf(stderr, "%s: bad version number: %s\n",
					argv[0], optarg);
				usage(argv[0], 1);
			}
			nfs_version &= ~NFSVERSBIT(vers);
			break;
		case 'n':
			NFSCTL_TCPUNSET(_rpcprotobits);
			break;
		case 'r':
			reverse_resolve = 1;
			break;
		case 's':
			if (!state_setup_basedir(argv[0], optarg))
				exit(1);
			break;
		case 't':
			num_threads = atoi (optarg);
			break;
		case 'V':
			vers = atoi(optarg);
			if (vers < 2 || vers > 4) {
				fprintf(stderr, "%s: bad version number: %s\n",
					argv[0], optarg);
				usage(argv[0], 1);
			}
			nfs_version |= NFSVERSBIT(vers);
			break;
		case 'v':
			printf("%s version " VERSION "\n", progname);
			exit(0);
		case 'u':
			NFSCTL_UDPUNSET(_rpcprotobits);
			break;
		case 'l':
			xlog_sconfig("auth", 1);
			break;
		case 'i':
			use_ipaddr = 2;
			break;
		case 'T':
			ttl = atoi(optarg);
			if (ttl <= 0) {
				fprintf(stderr, "%s: bad ttl number of seconds: %s\n",
					argv[0], optarg);
				usage(argv[0], 1);
			}
			default_ttl = ttl;
			break;
		case 0:
			break;
		case '?':
		default:
			usage(progname, 1);
		}

	/* No more arguments allowed. */
	if (optind != argc || !version_any()) {
		fprintf(stderr, "%s: No protocol versions specified!\n", progname); 
		usage(progname, 1);
	}
	if (!setup_state_path_names(progname, ETAB, ETABTMP, ETABLCK, &etab))
		return 1;
	if (!setup_state_path_names(progname, RMTAB, RMTABTMP, RMTABLCK, &rmtab))
		return 1;

	if (getrlimit (RLIMIT_NOFILE, &rlim) != 0)
		fprintf(stderr, "%s: getrlimit (RLIMIT_NOFILE) failed: %s\n",
				progname, strerror(errno));
	else {
		/* glibc sunrpc code dies if getdtablesize > FD_SETSIZE */
		if ((descriptors == 0 && rlim.rlim_cur > FD_SETSIZE) ||
		    descriptors > FD_SETSIZE)
			descriptors = FD_SETSIZE;
		if (descriptors) {
			rlim.rlim_cur = descriptors;
			if (setrlimit (RLIMIT_NOFILE, &rlim) != 0) {
				fprintf(stderr, "%s: setrlimit (RLIMIT_NOFILE) failed: %s\n",
					progname, strerror(errno));
				exit(1);
			}
		}
	}
	if (!foreground) xlog_stderr(0);

	sa.sa_handler = SIG_IGN;
	sa.sa_flags = 0;
	sigemptyset(&sa.sa_mask);
	sigaction(SIGHUP, &sa, NULL);
	sigaction(SIGINT, &sa, NULL);
	sigaction(SIGTERM, &sa, NULL);
	sigaction(SIGPIPE, &sa, NULL);
	/* WARNING: the following works on Linux and SysV, but not BSD! */
	sigaction(SIGCHLD, &sa, NULL);

	unregister_services();
	if (version2()) {
		listeners += nfs_svc_create("mountd", MOUNTPROG,
					MOUNTVERS, mount_dispatch, port);
		listeners += nfs_svc_create("mountd", MOUNTPROG,
					MOUNTVERS_POSIX, mount_dispatch, port);
	}
	if (version3())
		listeners += nfs_svc_create("mountd", MOUNTPROG,
					MOUNTVERS_NFSV3, mount_dispatch, port);
	if (version23() && listeners == 0)
		xlog(L_WARNING, "mountd: No V2 or V3 listeners created!");

	sa.sa_handler = killer;
	sigaction(SIGINT, &sa, NULL);
	sigaction(SIGTERM, &sa, NULL);
	sa.sa_handler = sig_hup;
	sigaction(SIGHUP, &sa, NULL);

	if (!foreground) {
		/* We first fork off a child. */
		if ((c = fork()) > 0)
			exit(0);
		if (c < 0) {
			xlog(L_FATAL, "mountd: cannot fork: %s\n",
						strerror(errno));
		}
		/* Now we remove ourselves from the foreground.
		   Redirect stdin/stdout/stderr first. */
		{
			int fd = open("/dev/null", O_RDWR);
			(void) dup2(fd, 0);
			(void) dup2(fd, 1);
			(void) dup2(fd, 2);
			if (fd > 2) (void) close(fd);
		}
		setsid();
	}

	/* silently bounds check num_threads */
	if (foreground)
		num_threads = 1;
	else if (num_threads < 1)
		num_threads = 1;
	else if (num_threads > MAX_THREADS)
		num_threads = MAX_THREADS;

	if (num_threads > 1)
		fork_workers();

	nfsd_path_init();
	/* Open files now to avoid sharing descriptors among forked processes */
	cache_open();
	v4clients_init();

	xlog(L_NOTICE, "Version " VERSION " starting");
	my_svc_run();

	xlog(L_ERROR, "RPC service loop terminated unexpectedly. Exiting...\n");
	unregister_services();
	free_state_path_names(&etab);
	free_state_path_names(&rmtab);
	exit(1);
}

static void
usage(const char *prog, int n)
{
	fprintf(stderr,
"Usage: %s [-F|--foreground] [-h|--help] [-v|--version] [-d kind|--debug kind]\n"
"	[-l|--log-auth] [-i|--cache-use-ipaddr] [-T|--ttl ttl]\n"
"	[-o num|--descriptors num]\n"
"	[-p|--port port] [-V version|--nfs-version version]\n"
"	[-N version|--no-nfs-version version] [-n|--no-tcp]\n"
"	[-H prog |--ha-callout prog] [-r |--reverse-lookup]\n"
"	[-s|--state-directory-path path] [-g|--manage-gids]\n"
"	[-t num|--num-threads=num] [-u|--no-udp]\n", prog);
	exit(n);
}
