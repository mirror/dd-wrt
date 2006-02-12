/***********************************************************************
*
* sync-pppd.c
*
* An LNS handler which starts pppd attached to a PTY in
* synchronous mode.
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
"$Id: sync-pppd.c,v 1.1 2004/02/04 04:01:51 kanki Exp $";

#include "l2tp.h"
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>

#define HANDLER_NAME "sync-pppd"

#define DEFAULT_PPPD_PATH "/usr/sbin/pppd"

#define MAX_FDS 256

extern int pty_get(int *mfp, int *sfp);
static int establish_session(l2tp_session *ses);
static void close_session(l2tp_session *ses, char const *reason);
static void handle_frame(l2tp_session *ses, unsigned char *buf, size_t len);

/* Options for invoking pppd */
#define MAX_OPTS 64
static char *pppd_lns_options[MAX_OPTS+1];
static char *pppd_lac_options[MAX_OPTS+1];
static int num_pppd_lns_options = 0;
static int num_pppd_lac_options = 0;
static int use_unit_option = 0;
static char *pppd_path = NULL;

#define PUSH_LNS_OPT(x) pppd_lns_options[num_pppd_lns_options++] = (x)
#define PUSH_LAC_OPT(x) pppd_lac_options[num_pppd_lac_options++] = (x)

/* Our call ops */
static l2tp_call_ops my_ops = {
    establish_session,
    close_session,
    handle_frame
};

/* The slave process */
struct slave {
    EventSelector *es;		/* Event selector */
    l2tp_session *ses;		/* L2TP session we're hooked to */
    pid_t pid;			/* PID of child PPPD process */
    int fd;			/* File descriptor for event-handler loop */
    EventHandler *event;	/* Event handler */
};

static int handle_lac_opts(EventSelector *es, l2tp_opt_descriptor *desc, char const *value);
static int handle_lns_opts(EventSelector *es, l2tp_opt_descriptor *desc, char const *value);

/* Options */
static l2tp_opt_descriptor my_opts[] = {
    /*  name               type                 addr */
    { "lac-pppd-opts",     OPT_TYPE_CALLFUNC,   (void *) handle_lac_opts},
    { "lns-pppd-opts",     OPT_TYPE_CALLFUNC,   (void *) handle_lns_opts},
    { "set-ppp-if-name",   OPT_TYPE_BOOL,       &use_unit_option},
    { "pppd-path",         OPT_TYPE_STRING,     &pppd_path},
    { NULL,                OPT_TYPE_BOOL,       NULL }
};

static int
process_option(EventSelector *es, char const *name, char const *value)
{
    if (!strcmp(name, "*begin*")) return 0;
    if (!strcmp(name, "*end*")) return 0;
    return l2tp_option_set(es, name, value, my_opts);
}

static option_handler my_option_handler = {
    NULL, HANDLER_NAME, process_option
};

static int
handle_lac_opts(EventSelector *es,
		l2tp_opt_descriptor *desc, char const *value)
{
    char word[512];
    while (value && *value) {
	value = l2tp_chomp_word(value, word);
	if (!word[0]) break;
	if (num_pppd_lac_options < MAX_OPTS) {
	    char *x = strdup(word);
	    if (x) PUSH_LAC_OPT(x);
	    pppd_lac_options[num_pppd_lac_options] = NULL;
	} else {
	    break;
	}
    }
    return 0;
}

static int
handle_lns_opts(EventSelector *es,
		l2tp_opt_descriptor *desc, char const *value)
{
    char word[512];
    while (value && *value) {
	value = l2tp_chomp_word(value, word);
	if (!word[0]) break;
	if (num_pppd_lns_options < MAX_OPTS) {
	    char *x = strdup(word);
	    if (x) PUSH_LNS_OPT(x);
	    pppd_lns_options[num_pppd_lns_options] = NULL;
	} else {
	    break;
	}
    }
    return 0;
}

/**********************************************************************
* %FUNCTION: handle_frame
* %ARGUMENTS:
*  ses -- l2tp session
*  buf -- received PPP frame
*  len -- length of frame
* %RETURNS:
*  Nothing
* %DESCRIPTION:
*  Shoots the frame to PPP's pty
***********************************************************************/
static void
handle_frame(l2tp_session *ses,
	     unsigned char *buf,
	     size_t len)
{
    struct slave *sl = ses->private;
    int n;

    if (!sl) return;

    /* Add framing bytes */
    *--buf = 0x03;
    *--buf = 0xFF;
    len += 2;

    /* TODO: Add error checking */
    n = write(sl->fd, buf, len);
}

/**********************************************************************
* %FUNCTION: close_session
* %ARGUMENTS:
*  ses -- L2TP session
*  reason -- reason why session is closing
* %RETURNS:
*  Nothing
* %DESCRIPTION:
*  Kills pppd.
***********************************************************************/
static void
close_session(l2tp_session *ses, char const *reason)
{
    struct slave *sl = ses->private;
    if (!sl) return;

    /* Detach slave */
    ses->private = NULL;
    sl->ses = NULL;

    kill(SIGTERM, sl->pid);
    close(sl->fd);
    sl->fd = -1;
    Event_DelHandler(sl->es, sl->event);
    sl->event = NULL;
}

/**********************************************************************
* %FUNCTION: slave_exited
* %ARGUMENTS:
*  pid -- PID of exiting slave
*  status -- exit status of slave
*  data -- the slave structure
* %RETURNS:
*  Nothing
* %DESCRIPTION:
*  Handles an exiting slave
***********************************************************************/
static void
slave_exited(pid_t pid, int status, void *data)
{
    l2tp_session *ses;
    struct slave *sl = (struct slave *) data;
    if (!sl) return;

    ses = sl->ses;

    if (sl->fd >= 0) close(sl->fd);
    if (sl->event) Event_DelHandler(sl->es, sl->event);

    if (ses) {
	ses->private = NULL;
	l2tp_session_send_CDN(ses, RESULT_GENERAL_REQUEST, 0,
			      "pppd process exited");
    }
    free(sl);
}

/**********************************************************************
* %FUNCTION: readable
* %ARGUMENTS:
*  es -- event selector
*  fd -- file descriptor
*  flags -- we ignore
*  data -- the L2TP session
* %RETURNS:
*  Nothing
* %DESCRIPTION:
*  Handles readability on PTY; shoots PPP frame over tunnel
***********************************************************************/
static void
readable(EventSelector *es, int fd, unsigned int flags, void *data)
{
    unsigned char buf[4096+EXTRA_HEADER_ROOM];
    int n;
    l2tp_session *ses = (l2tp_session *) data;
    int iters = 5;

    /* It seems to be better to read in a loop than to go
       back to select loop.  However, don't loop forever, or
       we could have a DoS potential */
    while(iters--) {
	/* EXTRA_HEADER_ROOM bytes extra space for l2tp header */
	n = read(fd, buf+EXTRA_HEADER_ROOM, sizeof(buf)-EXTRA_HEADER_ROOM);

	/* TODO: Check this.... */
	if (n <= 2) return;

	if (!ses) continue;

	/* Chop off framing bytes */
	l2tp_dgram_send_ppp_frame(ses, buf+EXTRA_HEADER_ROOM+2, n-2);
    }
}

/**********************************************************************
* %FUNCTION: establish_session
* %ARGUMENTS:
*  ses -- the L2TP session
* %RETURNS:
*  0 if session could be established, -1 otherwise.
* %DESCRIPTION:
*  Forks a pppd process and connects fd to pty
***********************************************************************/
static int
establish_session(l2tp_session *ses)
{
    int m_pty, s_pty;
    pid_t pid;
    EventSelector *es = ses->tunnel->es;
    struct slave *sl = malloc(sizeof(struct slave));
    int i;
    char unit[32];

    ses->private = NULL;
    if (!sl) return -1;
    sl->ses = ses;
    sl->es = es;

    /* Get pty */
    if (pty_get(&m_pty, &s_pty) < 0) {
	free(sl);
	return -1;
    }

    /* Fork */
    pid = fork();
    if (pid == (pid_t) -1) {
	free(sl);
	return -1;
    }

    if (pid) {
	int flags;

	/* In the parent */
	sl->pid = pid;

	/* Set up handler for when pppd exits */
	Event_HandleChildExit(es, pid, slave_exited, sl);

	/* Close the slave tty */
	close(s_pty);

	sl->fd = m_pty;

	/* Set slave FD non-blocking */
	flags = fcntl(sl->fd, F_GETFL);
	if (flags >= 0) fcntl(sl->fd, F_SETFL, (long) flags | O_NONBLOCK);

	/* Handle readability on slave end */
	sl->event = Event_AddHandler(es, m_pty, EVENT_FLAG_READABLE,
			 readable, ses);

	ses->private = sl;
	return 0;
    }

    /* In the child.  Exec pppd */
    /* Close all file descriptors except s_pty */
    for (i=0; i<MAX_FDS; i++) {
	if (i != s_pty) close(i);
    }

    /* Dup s_pty onto stdin and stdout */
    dup2(s_pty, 0);
    dup2(s_pty, 1);
    if (s_pty > 1) close(s_pty);

    /* Create unit */
    sprintf(unit, "%d", (int) getpid());

    if (ses->we_are_lac) {
	/* Push a unit option */
	if (use_unit_option && num_pppd_lac_options <= MAX_OPTS-2) {
	    PUSH_LAC_OPT("unit");
	    PUSH_LAC_OPT(unit);
	}
	if (pppd_path) {
	    execv(pppd_path, pppd_lac_options);
	} else {
	    execv(DEFAULT_PPPD_PATH, pppd_lac_options);
	}
    } else {
	/* Push a unit option */
	if (use_unit_option && num_pppd_lns_options <= MAX_OPTS-2) {
	    PUSH_LNS_OPT("unit");
	    PUSH_LNS_OPT(unit);
	}
	if (pppd_path) {
	    execv(pppd_path, pppd_lns_options);
	} else {
	    execv(DEFAULT_PPPD_PATH, pppd_lns_options);
	}
    }

    /* Doh.. execl failed */
    _exit(1);
}

static l2tp_lns_handler my_lns_handler = {
    NULL,
    HANDLER_NAME,
    &my_ops
};

static l2tp_lac_handler my_lac_handler = {
    NULL,
    HANDLER_NAME,
    &my_ops
};

void
handler_init(EventSelector *es)
{
    l2tp_session_register_lns_handler(&my_lns_handler);
    l2tp_session_register_lac_handler(&my_lac_handler);
    l2tp_option_register_section(&my_option_handler);

    PUSH_LNS_OPT("pppd");
    PUSH_LNS_OPT("sync");
    PUSH_LNS_OPT("nodetach");
    PUSH_LNS_OPT("noaccomp");
    PUSH_LNS_OPT("nobsdcomp");
    PUSH_LNS_OPT("nodeflate");
    PUSH_LNS_OPT("nopcomp");
    PUSH_LNS_OPT("novj");
    PUSH_LNS_OPT("novjccomp");
    PUSH_LNS_OPT("logfile");
    PUSH_LNS_OPT("/dev/null");
    PUSH_LNS_OPT("nolog");
    pppd_lns_options[num_pppd_lns_options] = NULL;

    PUSH_LAC_OPT("pppd");
    PUSH_LAC_OPT("sync");
    PUSH_LAC_OPT("nodetach");
    PUSH_LAC_OPT("noaccomp");
    PUSH_LAC_OPT("nobsdcomp");
    PUSH_LAC_OPT("nodeflate");
    PUSH_LAC_OPT("nopcomp");
    PUSH_LAC_OPT("novj");
    PUSH_LAC_OPT("novjccomp");
    PUSH_LAC_OPT("logfile");
    PUSH_LAC_OPT("/dev/null");
    PUSH_LAC_OPT("nolog");
    pppd_lac_options[num_pppd_lac_options] = NULL;
}

