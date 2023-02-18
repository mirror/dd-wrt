/*
  gssd.c

  Copyright (c) 2000 The Regents of the University of Michigan.
  All rights reserved.

  Copyright (c) 2000 Dug Song <dugsong@UMICH.EDU>.
  Copyright (c) 2002 Andy Adamson <andros@UMICH.EDU>.
  Copyright (c) 2002 Marius Aamodt Eriksen <marius@UMICH.EDU>.
  Copyright (c) 2002 J. Bruce Fields <bfields@UMICH.EDU>.
  All rights reserved, all wrongs reversed.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions
  are met:

  1. Redistributions of source code must retain the above copyright
     notice, this list of conditions and the following disclaimer.
  2. Redistributions in binary form must reproduce the above copyright
     notice, this list of conditions and the following disclaimer in the
     documentation and/or other materials provided with the distribution.
  3. Neither the name of the University nor the names of its
     contributors may be used to endorse or promote products derived
     from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESS OR IMPLIED
  WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
  MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  DISCLAIMED. IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
  FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
  BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
  LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif	/* HAVE_CONFIG_H */

#include <sys/param.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <rpc/rpc.h>
#include <fcntl.h>
#include <errno.h>


#include <unistd.h>
#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <nfsidmap.h>
#include <event2/event.h>

#include "nfslib.h"
#include "svcgssd.h"
#include "gss_util.h"
#include "err_util.h"
#include "conffile.h"
#include "misc.h"
#include "svcgssd_krb5.h"

static bool signal_received = false;
static struct event_base *evbase = NULL;
static int nullrpc_fd = -1;
static struct event *nullrpc_event = NULL;
static struct event *wait_event = NULL;

#define NULLRPC_FILE "/proc/net/rpc/auth.rpcsec.init/channel"

static void
sig_die(int signal)
{
	if (signal_received) {
		/* destroy krb5 machine creds */
		printerr(1, "forced exiting on signal %d\n", signal);
		exit(0);
	}
	signal_received = true;
	printerr(1, "exiting on signal %d\n", signal);
	event_base_loopexit(evbase, NULL);
}

static void
sig_hup(int signal)
{
	/* don't exit on SIGHUP */
	printerr(1, "Received SIGHUP(%d)... Ignoring.\n", signal);
	return;
}

static void
usage(char *progname)
{
	fprintf(stderr, "usage: %s [-n] [-f] [-v] [-r] [-i] [-p principal]\n",
		progname);
	exit(1);
}

static void
svcgssd_nullrpc_cb(int fd, short UNUSED(which), void *UNUSED(data))
{
	char	lbuf[RPC_CHAN_BUF_SIZE];
	int	lbuflen = 0;

	printerr(1, "reading null request\n");

	lbuflen = read(fd, lbuf, sizeof(lbuf));
	if (lbuflen <= 0 || lbuf[lbuflen-1] != '\n') {
		printerr(0, "WARNING: handle_nullreq: failed reading request\n");
		return;
	}
	lbuf[lbuflen-1] = 0;

	handle_nullreq(lbuf);
}

static void
svcgssd_nullrpc_close(void)
{
	if (nullrpc_event) {
		printerr(2, "closing nullrpc channel %s\n", NULLRPC_FILE);
		event_free(nullrpc_event);
		nullrpc_event = NULL;
	}
	if (nullrpc_fd != -1) {
		close(nullrpc_fd);
		nullrpc_fd = -1;
	}
}

static void
svcgssd_nullrpc_open(void)
{
	nullrpc_fd = open(NULLRPC_FILE, O_RDWR);
	if (nullrpc_fd < 0) {
		printerr(0, "failed to open %s: %s\n",
			 NULLRPC_FILE, strerror(errno));
		return;
	}
	nullrpc_event = event_new(evbase, nullrpc_fd, EV_READ | EV_PERSIST,
				  svcgssd_nullrpc_cb, NULL);
	if (!nullrpc_event) {
		printerr(0, "failed to create event for %s: %s\n",
			 NULLRPC_FILE, strerror(errno));
		close(nullrpc_fd);
		nullrpc_fd = -1;
		return;
	}
	event_add(nullrpc_event, NULL);
	printerr(2, "opened nullrpc channel %s\n", NULLRPC_FILE);
}

static void
svcgssd_wait_cb(int UNUSED(fd), short UNUSED(which), void *UNUSED(data))
{
	static int times = 0;
	int rc;

	rc = access(NULLRPC_FILE, R_OK | W_OK);
	if (rc != 0) {
		struct timeval t = {times < 10 ? 1 : 10, 0};
		times++;
		if (times % 30 == 0)
			printerr(2, "still waiting for nullrpc channel: %s\n",
				NULLRPC_FILE);
		evtimer_add(wait_event, &t);
		return;
	}

	svcgssd_nullrpc_open();
	event_free(wait_event);
	wait_event = NULL;
}



int
main(int argc, char *argv[])
{
	int get_creds = 1;
	int fg = 0;
	int verbosity = 0;
	int rpc_verbosity = 0;
	int idmap_verbosity = 0;
	int opt, status;
	extern char *optarg;
	char *progname;
	char *principal = NULL;
	char *s;
	int rc;

	conf_init_file(NFS_CONFFILE);

	s = conf_get_str("svcgssd", "principal");
	if (!s)
		;
	else if (strcmp(s, "system")== 0)
		get_creds = 0;
	else
		principal = s;

	verbosity = conf_get_num("svcgssd", "Verbosity", verbosity);
	rpc_verbosity = conf_get_num("svcgssd", "RPC-Verbosity", rpc_verbosity);
	idmap_verbosity = conf_get_num("svcgssd", "IDMAP-Verbosity", idmap_verbosity);

	while ((opt = getopt(argc, argv, "fivrnp:")) != -1) {
		switch (opt) {
			case 'f':
				fg = 1;
				break;
			case 'i':
				idmap_verbosity++;
				break;
			case 'n':
				get_creds = 0;
				break;
			case 'v':
				verbosity++;
				break;
			case 'r':
				rpc_verbosity++;
				break;
			case 'p':
				principal = optarg;
				break;
			default:
				usage(argv[0]);
				break;
		}
	}

	if ((progname = strrchr(argv[0], '/')))
		progname++;
	else
		progname = argv[0];

	initerr(progname, verbosity, fg);
#ifdef HAVE_AUTHGSS_SET_DEBUG_LEVEL
	if (verbosity && rpc_verbosity == 0)
		rpc_verbosity = verbosity;
	authgss_set_debug_level(rpc_verbosity);
#elif HAVE_LIBTIRPC_SET_DEBUG
        /*
	 * Only set the libtirpc debug level if explicitly requested via -r...
	 * svcgssd is chatty enough as it is.
	 */
        if (rpc_verbosity > 0)
                libtirpc_set_debug(progname, rpc_verbosity, fg);
#else
	if (rpc_verbosity > 0)
		printerr(0, "Warning: rpcsec_gss library does not "
			    "support setting debug level\n");
#endif
#ifdef HAVE_NFS4_SET_DEBUG
		if (verbosity && idmap_verbosity == 0)
			idmap_verbosity = verbosity;
        nfs4_set_debug(idmap_verbosity, NULL);
#else
	if (idmap_verbosity > 0)
		printerr(0, "Warning: your nfsidmap library does not "
			    "support setting debug level\n");
#endif

	if (gssd_check_mechs() != 0) {
		printerr(0, "ERROR: Problem with gssapi library\n");
		exit(1);
	}

	daemon_init(fg);

	evbase = event_base_new();
	if (!evbase) {
		printerr(0, "ERROR: failed to create event base: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}

	signal(SIGINT, sig_die);
	signal(SIGTERM, sig_die);
	signal(SIGHUP, sig_hup);

	if (get_creds) {
		if (principal)
			status = gssd_acquire_cred(principal, 
				((const gss_OID)GSS_C_NT_USER_NAME));
		else
			status = gssd_acquire_cred(GSSD_SERVICE_NAME, 
				(const gss_OID)GSS_C_NT_HOSTBASED_SERVICE);
		if (status == FALSE) {
			printerr(0, "unable to obtain root (machine) credentials\n");
			printerr(0, "do you have a keytab entry for %s in"
				"/etc/krb5.keytab?\n",
				principal ? principal : "nfs/<your.host>@<YOUR.REALM>");
			exit(1);
		}
	} else {
		status = gssd_acquire_cred(NULL,
			(const gss_OID)GSS_C_NT_HOSTBASED_SERVICE);
		if (status == FALSE) {
			printerr(0, "unable to obtain nameless credentials\n");
			exit(1);
		}
	}

	svcgssd_nullrpc_open();
	if (!nullrpc_event) {
		struct timeval t = {1, 0};

		printerr(2, "waiting for nullrpc channel to appear\n");
		wait_event = evtimer_new(evbase, svcgssd_wait_cb, NULL);
		if (!wait_event) {
			printerr(0, "ERROR: failed to create wait event: %s\n",
				 strerror(errno));
			exit(EXIT_FAILURE);
		}
		evtimer_add(wait_event, &t);
	}

	daemon_ready();

	/* We don't need the config anymore */
	conf_cleanup();

	nfs4_init_name_mapping(NULL); /* XXX: should only do this once */

	rc = event_base_dispatch(evbase);
	if (rc < 0)
		printerr(0, "event_base_dispatch() returned %i!\n", rc);

	svcgssd_nullrpc_close();
	if (wait_event)
		event_free(wait_event);

	event_base_free(evbase);

	nfs4_term_name_mapping();
	svcgssd_free_enctypes();
	gssd_cleanup();

	return EXIT_SUCCESS;
}
