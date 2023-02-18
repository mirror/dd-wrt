/*
 * nfsd
 *
 * This is the user level part of nfsd. This is very primitive, because
 * all the work is now done in the kernel module.
 *
 * Copyright (C) 1995, 1996 Olaf Kirch <okir@monad.swb.de>
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <getopt.h>
#include <netdb.h>
#include <libgen.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "conffile.h"
#include "nfslib.h"
#include "nfssvc.h"
#include "xlog.h"
#include "xcommon.h"

#ifndef NFSD_NPROC
#define NFSD_NPROC 8
#endif

static void	usage(const char *);

static struct option longopts[] =
{
	{ "host", 1, 0, 'H' },
	{ "help", 0, 0, 'h' },
	{ "no-nfs-version", 1, 0, 'N' },
	{ "nfs-version", 1, 0, 'V' },
	{ "tcp", 0, 0, 't' },
	{ "no-tcp", 0, 0, 'T' },
	{ "udp", 0, 0, 'u' },
	{ "no-udp", 0, 0, 'U' },
	{ "port", 1, 0, 'P' },
	{ "port", 1, 0, 'p' },
	{ "debug", 0, 0, 'd' },
	{ "syslog", 0, 0, 's' },
	{ "rdma", 2, 0, 'R' },
	{ "grace-time", 1, 0, 'G'},
	{ "lease-time", 1, 0, 'L'},
	{ NULL, 0, 0, 0 }
};

inline static void 
read_nfsd_conf(void)
{
	conf_init_file(NFS_CONFFILE); 
	xlog_set_debug("nfsd");
}

int
main(int argc, char **argv)
{
	int	count = NFSD_NPROC, c, i, error = 0, portnum, fd, found_one;
	char *p, *progname, *port, *rdma_port = NULL;
	char **haddr = NULL;
	int hcounter = 0;
	struct conf_list *hosts;
	int	socket_up = 0;
	unsigned int minorvers = NFSCTL_MINDEFAULT;
	unsigned int minorversset = NFSCTL_MINDEFAULT;
	unsigned int minormask = 0;
	unsigned int versbits = NFSCTL_VERDEFAULT;
	unsigned int protobits = NFSCTL_PROTODEFAULT;
	int grace = -1;
	int lease = -1;
	int force4dot0 = 0;

	progname = basename(argv[0]);
	haddr = xmalloc(sizeof(char *));
	haddr[0] = NULL;

	xlog_syslog(0);
	xlog_stderr(1);

	/* Read in config setting */
	read_nfsd_conf();

	nfssvc_get_minormask(&minormask);

	count = conf_get_num("nfsd", "threads", count);
	grace = conf_get_num("nfsd", "grace-time", grace);
	lease = conf_get_num("nfsd", "lease-time", lease);
	port = conf_get_str("nfsd", "port");
	if (!port)
		port = "nfs";
	if (conf_get_bool("nfsd", "rdma", false)) {
		rdma_port = conf_get_str("nfsd", "rdma-port");
		if (!rdma_port)
			rdma_port = "nfsrdma";
	}
	/* backward compatibility - nfs.conf used to set rdma port directly */
	if (!rdma_port)
		rdma_port = conf_get_str("nfsd", "rdma");
	if (conf_get_bool("nfsd", "udp", NFSCTL_UDPISSET(protobits)))
		NFSCTL_UDPSET(protobits);
	else
		NFSCTL_UDPUNSET(protobits);
	if (conf_get_bool("nfsd", "tcp", NFSCTL_TCPISSET(protobits)))
		NFSCTL_TCPSET(protobits);
	else
		NFSCTL_TCPUNSET(protobits);
	for (i = 2; i <= 4; i++) {
		char tag[20];
		sprintf(tag, "vers%d", i);
		if (conf_get_bool("nfsd", tag, NFSCTL_VERISSET(versbits, i))) {
			NFSCTL_VERSET(versbits, i);
			if (i == 4)
				minorvers = minorversset = minormask;
		} else {
			NFSCTL_VERUNSET(versbits, i);
			if (i == 4) {
				minorvers = 0;
				minorversset = minormask;
			}
		}
	}

	/* We assume the kernel will default all minor versions to 'on',
	 * and allow the config file to disable some.
	 */
	for (i = NFS4_MINMINOR; i <= NFS4_MAXMINOR; i++) {
		char tag[20];
		sprintf(tag, "vers4.%d", i);
		/* The default for minor version support is to let the
		 * kernel decide.  We could ask the kernel what that choice
		 * will be, but that is needlessly complex.
		 * Instead, perform a config-file lookup using each of the
		 * two possible default.  If the result is different from the
		 * default, then impose that value, else don't make a change
		 * (i.e. don't set the bit in minorversset).
		 */
		if (!conf_get_bool("nfsd", tag, 1)) {
			NFSCTL_MINORSET(minorversset, i);
			NFSCTL_MINORUNSET(minorvers, i);
			if (i == 0)
				force4dot0 = 1;
		}
		if (conf_get_bool("nfsd", tag, 0)) {
			NFSCTL_MINORSET(minorversset, i);
			NFSCTL_MINORSET(minorvers, i);
			if (i == 0)
				force4dot0 = 1;
		}
	}

	hosts = conf_get_list("nfsd", "host");
	if (hosts && hosts->cnt) {
		struct conf_list_node *n;
		haddr = realloc(haddr, sizeof(char*) * hosts->cnt);
		TAILQ_FOREACH(n, &(hosts->fields), link) {
			haddr[hcounter] = n->field;
			hcounter++;
		}
	}

	while ((c = getopt_long(argc, argv, "dH:hN:V:p:P:stTuUrG:L:", longopts, NULL)) != EOF) {
		switch(c) {
		case 'd':
			xlog_config(D_ALL, 1);
			break;
		case 'H':
			if (hosts) {
				hosts = NULL;
				hcounter = 0;
			}
			if (hcounter) {
				haddr = realloc(haddr, sizeof(char*) * hcounter+1);
				if(!haddr) {
					fprintf(stderr, "%s: unable to allocate "
							"memory.\n", progname);
					exit(1);
				}
			}
			haddr[hcounter] = optarg;
			hcounter++;
			break;
		case 'P':	/* XXX for nfs-server compatibility */
		case 'p':
			/* only the last -p option has any effect */
			port = optarg;
			break;
		case 'r':
			rdma_port = "nfsrdma";
			break;
		case 'R': /* --rdma */
			if (optarg)
				rdma_port = optarg;
			else
				rdma_port = "nfsrdma";
			break;

		case 'N':
			switch((c = strtol(optarg, &p, 0))) {
			case 4:
				if (*p == '.') {
					int i = atoi(p+1);
					if (i < 0 || i > NFS4_MAXMINOR) {
						fprintf(stderr, "%s: unsupported minor version\n", optarg);
						exit(1);
					}
					NFSCTL_MINORSET(minorversset, i);
					NFSCTL_MINORUNSET(minorvers, i);
					if (i == 0)
						force4dot0 = 1;
					if (minorvers != 0)
						break;
				} else {
					minorvers = 0;
					minorversset = minormask;
				}
				/* FALLTHRU */
			case 3:
				NFSCTL_VERUNSET(versbits, c);
				break;
			default:
				fprintf(stderr, "%s: Unsupported version\n", optarg);
				exit(1);
			}
			break;
		case 'V':
			switch((c = strtol(optarg, &p, 0))) {
			case 4:
				if (*p == '.') {
					int i = atoi(p+1);
					if (i < 0 || i > NFS4_MAXMINOR) {
						fprintf(stderr, "%s: unsupported minor version\n", optarg);
						exit(1);
					}
					NFSCTL_MINORSET(minorversset, i);
					NFSCTL_MINORSET(minorvers, i);
					if (i == 0)
						force4dot0 = 1;
				} else
					minorvers = minorversset = minormask;
				/* FALLTHRU */
			case 3:
				NFSCTL_VERSET(versbits, c);
				break;
			default:
				fprintf(stderr, "%s: Unsupported version\n", optarg);
				exit(1);
			}
			break;
		case 's':
			xlog_syslog(1);
			xlog_stderr(0);
			break;
		case 't':
			NFSCTL_TCPSET(protobits);
			break;
		case 'T':
			NFSCTL_TCPUNSET(protobits);
			break;
		case 'u':
			NFSCTL_UDPSET(protobits);
			break;
		case 'U':
			NFSCTL_UDPUNSET(protobits);
			break;
		case 'G':
			grace = strtol(optarg, &p, 0);
			if (*p || grace <= 0) {
				fprintf(stderr, "%s: Unrecognized grace time.\n", optarg);
				exit(1);
			}
			break;
		case 'L':
			lease = strtol(optarg, &p, 0);
			if (*p || lease <= 0) {
				fprintf(stderr, "%s: Unrecognized lease time.\n", optarg);
				exit(1);
			}
			break;
		default:
			fprintf(stderr, "Invalid argument: '%c'\n", c);
			/* FALLTHRU */
		case 'h':
			usage(progname);
		}
	}

	if (optind < argc) {
		if ((count = atoi(argv[optind])) < 0) {
			/* insane # of servers */
			fprintf(stderr,
				"%s: invalid server count (%d), using 1\n",
				argv[0], count);
			count = 1;
		} else if (count == 0) {
			/*
			 * don't bother setting anything else if the threads
			 * are coming down anyway.
			 */
			socket_up = 1;
			goto set_threads;
		}
	}

	xlog_open(progname);

	portnum = strtol(port, &p, 0);
	if (!*p && (portnum <= 0 || portnum > 65535)) {
		/* getaddrinfo will catch other errors, but not
		 * out-of-range numbers.
		 */
		xlog(L_ERROR, "invalid port number: %s", port);
		exit(1);
	}

	/* make sure that at least one version is enabled */
	found_one = 0;
	for (c = NFSD_MINVERS; c <= NFSD_MAXVERS; c++) {
		if (NFSCTL_VERISSET(versbits, c))
			found_one = 1;
	}
	if (!found_one) {
		xlog(L_ERROR, "no version specified");
		exit(1);
	}

	if (NFSCTL_VERISSET(versbits, 4) &&
	    !NFSCTL_TCPISSET(protobits)) {
		xlog(L_ERROR, "version 4 requires the TCP protocol");
		exit(1);
	}

	if (chdir(NFS_STATEDIR)) {
		xlog(L_ERROR, "chdir(%s) failed: %m", NFS_STATEDIR);
		exit(1);
	}

	/* make sure nfsdfs is mounted if it's available */
	nfssvc_mount_nfsdfs(progname);

	/* can only change number of threads if nfsd is already up */
	if (nfssvc_inuse()) {
		socket_up = 1;
		goto set_threads;
	}

	/*
	 * Must set versions before the fd's so that the right versions get
	 * registered with rpcbind. Note that on older kernels w/o the right
	 * interfaces, these are a no-op.
	 * Timeouts must also be set before ports are created else we get
	 * EBUSY.
	 */
	nfssvc_setvers(versbits, minorvers, minorversset, force4dot0);
	if (grace > 0)
		nfssvc_set_time("grace", grace);
	if (lease  > 0)
		nfssvc_set_time("lease", lease);

	i = 0;
	do {
		error = nfssvc_set_sockets(protobits, haddr[i], port);
		if (!error)
			socket_up = 1;
	} while (++i < hcounter);

	if (rdma_port) {
		error = nfssvc_set_rdmaport(rdma_port);
		if (!error)
			socket_up = 1;
	}
set_threads:
	/* don't start any threads if unable to hand off any sockets */
	if (!socket_up) {
		xlog(L_ERROR, "unable to set any sockets for nfsd");
		goto out;
	}
	error = 0;

	/*
	 * KLUDGE ALERT:
	 * Some kernels let nfsd kernel threads inherit open files
	 * from the program that spawns them (i.e. us).  So close
	 * everything before spawning kernel threads.  --Chip
	 */
	fd = open("/dev/null", O_RDWR);
	if (fd == -1)
		xlog(L_ERROR, "Unable to open /dev/null: %m");
	else {
		/* switch xlog output to syslog since stderr is being closed */
		xlog_syslog(1);
		xlog_stderr(0);
		(void) dup2(fd, 0);
		(void) dup2(fd, 1);
		(void) dup2(fd, 2);
	}
	closeall(3);

	if ((error = nfssvc_threads(count)) < 0)
		xlog(L_ERROR, "error starting threads: errno %d (%m)", errno);
out:
	free(haddr);
	return (error != 0);
}

static void
usage(const char *prog)
{
	fprintf(stderr, "Usage:\n"
		"%s [-d|--debug] [-H hostname] [-p|-P|--port port]\n"
		"   [-N|--no-nfs-version version] [-V|--nfs-version version]\n"
		"   [-s|--syslog] [-t|--tcp] [-T|--no-tcp] [-u|--udp] [-U|--no-udp]\n"
		"   [-r|--rdma=] [-G|--grace-time secs] [-L|--leasetime secs] nrservs\n",
		prog);
	exit(2);
}
