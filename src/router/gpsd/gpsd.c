/* $Id: gpsd.c 4136 2006-12-14 21:20:57Z ckuethe $ */
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdlib.h>
#include <syslog.h>
#include <signal.h>
#include <errno.h>
#include <ctype.h>
#include <fcntl.h>
#include <string.h>
#include <netdb.h>
#include <stdarg.h>
#include <setjmp.h>
#include <stdio.h>
#include <assert.h>
#include <pwd.h>
#include <stdbool.h>
#include <math.h>

#include "gpsd_config.h"
#if defined (HAVE_PATH_H)
#include <paths.h>
#else
#if !defined (_PATH_DEVNULL)
#define _PATH_DEVNULL    "/dev/null"
#endif
#endif
#if defined (HAVE_SYS_SELECT_H)
#include <sys/select.h>
#endif
#if defined (HAVE_SYS_STAT_H)
#include <sys/stat.h>
#endif
#if defined(HAVE_SYS_TIME_H)
#include <sys/time.h>
#endif

#ifdef DBUS_ENABLE
#include <gpsd_dbus.h>
#endif

#include "gpsd.h"
#include "timebase.h"

/*
 * Timeout policy.  We can't rely on clients closing connections 
 * correctly, so we need timeouts to tell us when it's OK to 
 * reclaim client fds.  The assignment timeout fends off programs
 * that open connections and just sit there, not issuing a W or
 * doing anything else that triggers a device assignment.  Clients
 * in watcher or raw mode that don't read their data will get dropped 
 * when throttled_write() fills up the outbound buffers and the 
 * NOREAD_TIMEOUT expires.  Clients in the original polling mode have 
 * to be timed out as well.
 */
#define ASSIGNMENT_TIMEOUT	60
#define POLLER_TIMEOUT  	60*15
#define NOREAD_TIMEOUT		60*3

#define QLEN			5

/*
 * The name of a tty device from which to pick up whatever the local
 * owning group for tty devices is.  Used when we drop privileges.
 */
#define PROTO_TTY "/dev/ttyS0"

#define sub_index(s) (s - subscribers)

static fd_set all_fds;
static int maxfd;
static int debuglevel;
static bool in_background = false;
static bool nowait = false;
static jmp_buf restartbuf;
/*@ -initallelements -nullassign -nullderef @*/
static struct gps_context_t context = {
    .valid              = 0, 
    .sentdgps           = false, 
    .dgnss_service      = dgnss_none,
    .fixcnt             = 0, 
    .dsock              = -1, 
    .dgnss_privdata     = NULL,
    .rtcmbytes          = 0, 
    .rtcmbuf            = {'\0'}, 
    .rtcmtime           = 0,
    .leap_seconds       = LEAP_SECONDS, 
    .century            = CENTURY_BASE, 
#ifdef NTPSHM_ENABLE
    .enable_ntpshm      = false,
    .shmTime            = {0},
    .shmTimeInuse       = {0},
# ifdef PPS_ENABLE
    .shmTimePPS         = false,
# endif /* PPS_ENABLE */
#endif /* NTPSHM_ENABLE */
};
/*@ +initallelements +nullassign +nullderef @*/

static void onsig(int sig)
{
    /* go back to the default signal action until we can reset properly */
    (void)signal(sig, SIG_DFL);
    longjmp(restartbuf, sig+1);
}

static int daemonize(void)
{
    int fd;
    pid_t pid;

    switch (pid = fork()) {
    case -1:
	return -1;
    case 0:	/* child side */
	break;
    default:	/* parent side */
	exit(0);
    }

    if (setsid() == -1)
	return -1;
    (void)chdir("/");
    /*@ -nullpass @*/
    if ((fd = open(_PATH_DEVNULL, O_RDWR, 0)) != -1) {
	(void)dup2(fd, STDIN_FILENO);
	(void)dup2(fd, STDOUT_FILENO);
	(void)dup2(fd, STDERR_FILENO);
	if (fd > 2)
	    (void)close(fd);
    }
    /*@ +nullpass @*/
    in_background = true;
    return 0;
}

#if defined(PPS_ENABLE)
static pthread_mutex_t report_mutex;
#endif /* PPS_ENABLE */

void gpsd_report(int errlevel, const char *fmt, ... )
/* assemble command in printf(3) style, use stderr or syslog */
{
#ifndef SQUELCH_ENABLE
    if (errlevel <= debuglevel) {
	char buf[BUFSIZ], buf2[BUFSIZ], *sp;
	va_list ap;

#if defined(PPS_ENABLE)
	/*@ -unrecog  (splint has no pthread declarations as yet) @*/
	(void)pthread_mutex_lock(&report_mutex);
	/* +unrecog */
#endif /* PPS_ENABLE */
	(void)strlcpy(buf, "gpsd: ", BUFSIZ);
	va_start(ap, fmt) ;
	(void)vsnprintf(buf + strlen(buf), sizeof(buf)-strlen(buf), fmt, ap);
	va_end(ap);

	buf2[0] = '\0';
	for (sp = buf; *sp != '\0'; sp++)
	    if (isprint(*sp) || (isspace(*sp) && (sp[1]=='\0' || sp[2]=='\0')))
		(void)snprintf(buf2+strlen(buf2), 2, "%c", *sp);
	    else
		(void)snprintf(buf2+strlen(buf2), 6, "\\x%02x", (unsigned)*sp);

	if (in_background)
	    syslog((errlevel == 0) ? LOG_ERR : LOG_NOTICE, "%s", buf2);
	else
	    (void)fputs(buf2, stderr);
#if defined(PPS_ENABLE)
	/*@ -unrecog (splint has no pthread declarations as yet) @*/
	(void)pthread_mutex_unlock(&report_mutex);
	/* +unrecog */
#endif /* PPS_ENABLE */
    }
#endif /* !SQUELCH_ENABLE */
}

static void usage(void)
{
    (void)printf("usage: gpsd [-n] [-N] [-D n] [-F sockfile] [-P pidfile] [-S port] [-h] device...\n\
  Options include: \n\
  -n                            = don't wait for client connects to poll GPS\n\
  -N                            = don't go into background\n\
  -F sockfile                   = specify control socket location\n\
  -P pidfile              	= set file to record process ID \n\
  -D integer (default 0)  	= set debug level \n\
  -S integer (default %s)	= set port for daemon \n\
  -h                     	= help message \n\
  -V                            = emit version and exit.\n\
A device may be a local serial device for GPS input, or a URL of the form:\n\
     [{dgpsip|ntrip}://][user:passwd@]host[:port][/stream]\n\
in which case it specifies an input source for DGPS or ntrip data.\n",

	   DEFAULT_GPSD_PORT);
}

static int passivesock(char *service, char *protocol, int qlen)
{
    struct servent *pse;
    struct protoent *ppe ;
    struct sockaddr_in sin;
    int s, type, one = 1;

    /*@ -mustfreefresh @*/
    memset((char *) &sin, 0, sizeof(sin));
    /*@i1@*/sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = INADDR_ANY;

    if ((pse = getservbyname(service, protocol)))
	sin.sin_port = htons(ntohs((in_port_t)pse->s_port));
    else if ((sin.sin_port = htons((in_port_t)atoi(service))) == 0) {
	gpsd_report(LOG_ERROR, "Can't get \"%s\" service entry.\n", service);
	return -1;
    }
    if ((ppe = getprotobyname(protocol)) == NULL) {
	gpsd_report(LOG_ERROR, "Can't get \"%s\" protocol entry.\n", protocol);
	return -1;
    }
    if (strcmp(protocol, "udp") == 0)
	type = SOCK_DGRAM;
    else
	type = SOCK_STREAM;
    if ((s = socket(PF_INET, type, /*@i1@*/ppe->p_proto)) < 0) {
	gpsd_report(LOG_ERROR, "Can't create socket\n");
	return -1;
    }
    if (setsockopt(s,SOL_SOCKET,SO_REUSEADDR,(char *)&one,(int)sizeof(one)) == -1) {
	gpsd_report(LOG_ERROR, "Error: SETSOCKOPT SO_REUSEADDR\n");
	return -1;
    }
    if (bind(s, (struct sockaddr *) &sin, (int)sizeof(sin)) < 0) {
	gpsd_report(LOG_ERROR, "Can't bind to port %s\n", service);
        if (errno == EADDRINUSE) {
                gpsd_report(LOG_ERROR, "Maybe gpsd is already running!\n");
        }
	return -1;
    }
    if (type == SOCK_STREAM && listen(s, qlen) < 0) {
	gpsd_report(LOG_ERROR, "Can't listen on %s port%s\n", service);
	return -1;
    }
    return s;
    /*@ +mustfreefresh @*/
}

static int filesock(char *filename)
{
    struct sockaddr_un addr;
    int sock;

    /*@ -mayaliasunique @*/
    if ((sock = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
	gpsd_report(LOG_ERROR, "Can't create device-control socket\n");
	return -1;
    }
    (void)strlcpy(addr.sun_path, filename, 104); /* from sys/un.h */
    /*@i1@*/addr.sun_family = AF_UNIX;
    (void)bind(sock, (struct sockaddr *) &addr,  (int)sizeof(addr));
    if (listen(sock, QLEN) < 0) {
	gpsd_report(LOG_ERROR, "can't listen on local socket %s\n", filename);
	return -1;
    }
    /*@ +mayaliasunique @*/
    return sock;
}

/*
 * This hackery is intended to support SBCs that are resource-limited
 * and only need to support one or a few devices each.  It avoids the
 * space overhead of allocating thousands of unused device structures.
 * This array fills from the bottom, so as an extreme case you could
 * reduce LIMITED_MAX_DEVICES to 1.
 */
#ifdef LIMITED_MAX_DEVICES
#define MAXDEVICES	LIMITED_MAX_DEVICES
#else
/* we used to make this FD_SETSIZE, but that cost 14MB of wasted core! */
#define MAXDEVICES	4
#endif

/* 
 * Note: stdin/stdout, logging, and the control socket may eat several
 * file descriptors, so don't set this too low. 16 should probably be
 * the minimum.
 */
#ifdef LIMITED_MAX_CLIENTS
#define MAXSUBSCRIBERS LIMITED_MAX_CLIENTS
#else
/* subscriber structure is small enough that there's no need to limit this */ 
#define MAXSUBSCRIBERS	FD_SETSIZE
#endif

/*
 * Multi-session support requires us to have two arrays, one of GPS 
 * devices currently available and one of client sessions.  The number
 * of slots in each array is limited by the maximum number of client
 * sessions we can have open.
 */

static struct gps_device_t channels[MAXDEVICES];
#define allocated_channel(chp)	((chp)->gpsdata.gps_device[0] != '\0')
#define free_channel(chp)	(chp)->gpsdata.gps_device[0] = '\0'
#define syncing(chp)	(chp->gpsdata.gps_fd>-1&& chp->packet_type==BAD_PACKET)

static struct subscriber_t {
    int fd;				/* client file descriptor. -1 if unused */
    double active;			/* when subscriber last polled for data */
    bool tied;				/* client set device with F */
    bool watcher;			/* is client in watcher mode? */
    int raw;				/* is client in raw mode? */
    enum {GPS,RTCM104,ANY} requires;	/* type of device requested */
    struct gps_fix_t fixbuffer;		/* info to report to the client */
    struct gps_fix_t oldfix;		/* previous fix for error modeling */
    enum {casoc=0, nocasoc=1} buffer_policy;	/* buffering policy */
    /*@relnull@*/struct gps_device_t *device;	/* device subscriber listens to */
} subscribers[MAXSUBSCRIBERS];		/* indexed by client file descriptor */

static void adjust_max_fd(int fd, bool on)
/* track the largest fd currently in use */
{
    if (on) {
	if (fd > maxfd)
	    maxfd = fd;
    } 
#if !defined(LIMITED_MAX_DEVICES) && !defined(LIMITED_MAX_CLIENT_FD)
    /*
     * I suspect there could be some weird interactions here if
     * either of these were set lower than FD_SETSIZE.  We'll avoid 
     * potential bugs by not scavenging in this case at all -- should
     * be OK, as the use case for limiting is SBCs where the limits
     * will be very low (typically 1) and the maximum size of fd
     * set to scan through correspondingly small.
     */
    else {
	if (fd == maxfd) {
	    int tfd;

	    for (maxfd = tfd = 0; tfd < FD_SETSIZE; tfd++)
		if (FD_ISSET(tfd, &all_fds))
		    maxfd = tfd;
	}
    }
#endif /* !defined(LIMITED_MAX_DEVICES) && !defined(LIMITED_MAX_CLIENT_FD) */
}

static bool have_fix(struct subscriber_t *whoami)
{
    if (!whoami->device) {
	gpsd_report(LOG_PROG, "Client has no device\n");
	return false;
    }
#define VALIDATION_COMPLAINT(level, legend) \
	gpsd_report(level, legend " (status=%d, mode=%d).\n", \
		    whoami->device->gpsdata.status, whoami->fixbuffer.mode)
    if ((whoami->device->gpsdata.status == STATUS_NO_FIX) != (whoami->fixbuffer.mode == MODE_NO_FIX)) {
	VALIDATION_COMPLAINT(3, "GPS is confused about whether it has a fix");
	return false;
    }
    else if (whoami->device->gpsdata.status > STATUS_NO_FIX && whoami->fixbuffer.mode > MODE_NO_FIX) {
	VALIDATION_COMPLAINT(3, "GPS has a fix");
	return true;
    }
    VALIDATION_COMPLAINT(3, "GPS has no fix");
    return false;
#undef VALIDATION_COMPLAINT
}

static /*@null@*/ /*@observer@*/ struct subscriber_t* allocate_client(void)
{
    int cfd;
    for (cfd = 0; cfd < MAXSUBSCRIBERS; cfd++) {
	if (subscribers[cfd].device == NULL) 
	    return &subscribers[cfd];
    }
    return NULL;
}

static void detach_client(struct subscriber_t *sub)
{
    (void)close(sub->fd);
    gpsd_report(LOG_PROG, "detaching %d in detach_client\n", sub_index(sub));
    FD_CLR(sub->fd, &all_fds);
    adjust_max_fd(sub->fd, false);
    sub->raw = 0;
    sub->watcher = false;
    sub->active = 0;
    /*@i1@*/sub->device = NULL;
    sub->buffer_policy = casoc;
    sub->fd = -1;
}

static ssize_t throttled_write(struct subscriber_t *sub, char *buf, ssize_t len)
/* write to client -- throttle if it's gone or we're close to buffer overrun */
{
    ssize_t status;

    if (debuglevel >= 3) {
	if (isprint(buf[0]))
	    gpsd_report(LOG_IO, "=> client(%d): %s", sub_index(sub), buf);
	else {
	    char *cp, buf2[MAX_PACKET_LENGTH*3];
	    buf2[0] = '\0';
	    for (cp = buf; cp < buf + len; cp++)
		(void)snprintf(buf2 + strlen(buf2), 
			       sizeof(buf2)-strlen(buf2),
			      "%02x", (unsigned int)(*cp & 0xff));
	    gpsd_report(LOG_IO, "=> client(%d): =%s\r\n", sub_index(sub), buf2);
	}
    }

    if ((status = write(sub->fd, buf, (size_t)len)) > -1)
	return status;
    if (errno == EBADF)
	gpsd_report(LOG_WARN, "client(%d) has vanished.\n", sub_index(sub));
    else if (errno == EWOULDBLOCK && timestamp() - sub->active > NOREAD_TIMEOUT)
	gpsd_report(LOG_INF, "client(%d) timed out.\n", sub_index(sub));
    else
	gpsd_report(LOG_INF, "client(%d) write: %s\n", sub_index(sub), strerror(errno));
    detach_client(sub);
    return status;
}

static void notify_watchers(struct gps_device_t *device, char *sentence, ...)
/* notify all clients watching a given device of an event */
{
    struct subscriber_t *sub;
    va_list ap;
    char buf[BUFSIZ];

    va_start(ap, sentence) ;
    (void)vsnprintf(buf, sizeof(buf), sentence, ap);
    va_end(ap);

    for (sub = subscribers; sub < subscribers + MAXSUBSCRIBERS; sub++)
	if (sub->watcher != 0 && sub->device == device)
	    (void)throttled_write(sub, buf, (ssize_t)strlen(buf));
}

static void raw_hook(struct gps_data_t *ud, 
		     char *sentence, size_t len, int level)
/* hook to be executed on each incoming packet */
{
    struct subscriber_t *sub;

    for (sub = subscribers; sub < subscribers + MAXSUBSCRIBERS; sub++) {
	/* copy raw NMEA sentences from GPS to clients in raw mode */
	if (sub->raw == level && 
	    sub->device!=NULL &&
	    strcmp(ud->gps_device, sub->device->gpsdata.gps_device)==0)
	    (void)throttled_write(sub, sentence, (ssize_t)len);
    }
}

/*@ -globstate @*/
static /*@null@*/ /*@observer@*/struct gps_device_t *find_device(char *device_name)
/* find the channel block for an existing device name */
{
    struct gps_device_t *chp;

    for (chp = channels; chp < channels + MAXDEVICES; chp++)
	if (allocated_channel(chp) && strcmp(chp->gpsdata.gps_device, device_name)==0)
	    return chp;
    return NULL;
}

/*@ -statictrans @*/
static /*@null@*/ struct gps_device_t *open_device(char *device_name)
/* open and initialize a new channel block */
{
    struct gps_device_t *chp;

    /* special case: source may be a URL to differential-GPS service */
    if (dgnss_url(device_name)) {
	int dsock = dgnss_open(&context, device_name);
	if (dsock >= 0) {
	    FD_SET(dsock, &all_fds);
	    adjust_max_fd(dsock, true);
	}
	/* shaky, but only 0 versus nonzero is tested */
	return &channels[0];
    }

    /* normal case: set up GPS service */
    for (chp = channels; chp < channels + MAXDEVICES; chp++)
	if (!allocated_channel(chp)){
	    goto found;
        }
    return NULL;
found:
    gpsd_init(chp, &context, device_name);
    chp->gpsdata.raw_hook = raw_hook;
    /*
     * Bring the device all the way so we'll sniff packets from it and
     * discover up front whether it's a GPS source or an RTCM source.
     * Otherwise clients trying to bind to a specific type won't know
     * what source types are actually available.  If we're in nowait mode
     * the device has to be configured now; otherwise, it can wait.
     */
    if (gpsd_activate(chp, nowait) < 0)
	return NULL;
    FD_SET(chp->gpsdata.gps_fd, &all_fds);
    adjust_max_fd(chp->gpsdata.gps_fd, true);
    return chp;
}
/*@ +statictrans @*/
/*@ +globstate @*/

static bool allocation_policy(struct gps_device_t *channel,
			      struct subscriber_t *user,
			      double most_recent)
{
#ifdef __UNUSED__
    /* only allocate devices that we know the packet type of */
    if (channel->packet_type == BAD_PACKET)
	return false;
#endif /* __UNUSED__ */
    /* maybe we have already bound a more recently active device */
    if (user->device!=NULL && channel->gpsdata.sentence_time < most_recent)
	return false;
    gpsd_report(LOG_PROG, "User requires %d, channel type is %d\n", user->requires, channel->packet.type);
    /* we might have type constraints */
    if (user->requires == ANY)
	return true;
    else if (user->requires==RTCM104 && (channel->packet.type==RTCM_PACKET))
	return true;
    else if (user->requires == GPS 
	     && (channel->packet.type!=RTCM_PACKET) && (channel->packet.type!=BAD_PACKET))
	return true;
    else
	return false;
}

/*@ -branchstate -usedef -globstate @*/
static bool assign_channel(struct subscriber_t *user)
{
    bool was_unassigned = (user->device == NULL);
    /* if subscriber has no device... */
    if (was_unassigned) {
	double most_recent = 0;
	struct gps_device_t *channel;

	gpsd_report(LOG_PROG, "client(%d): assigning channel...\n", user-subscribers);
	/* ...connect him to the most recently active device */
	/*@ -mustfreeonly @*/
	for(channel = channels; channel<channels+MAXDEVICES; channel++)
	    if (allocated_channel(channel)) {
		if (allocation_policy(channel, user, most_recent)) {
		    user->device = channel;
		    most_recent = channel->gpsdata.sentence_time;
		}
	    }
	/*@ +mustfreeonly @*/
    }

    if (user->device == NULL) {
	gpsd_report(LOG_ERROR, "client(%d): channel assignment failed.\n", user-subscribers);
	return false;
    }

    /* and open that device */
    if (user->device->gpsdata.gps_fd != -1) 
	gpsd_report(LOG_PROG,"client(%d): channel %d already active.\n",
		    user-subscribers, user->device->gpsdata.gps_fd);
    else {
	if (gpsd_activate(user->device, true) < 0) {
	    
	    gpsd_report(LOG_ERROR, "client(%d): channel activation failed.\n", user-subscribers);
	    return false;
	} else {
	    gpsd_report(LOG_RAW, "flagging descriptor %d in assign_channel\n", user->device->gpsdata.gps_fd);
	    FD_SET(user->device->gpsdata.gps_fd, &all_fds);
	    adjust_max_fd(user->device->gpsdata.gps_fd, true);
	    if (user->watcher && !user->tied) {
		(void)write(user->fd, "GPSD,F=", 7);
		(void)write(user->fd, 
			    user->device->gpsdata.gps_device,
			    strlen(user->device->gpsdata.gps_device));
		(void)write(user->fd, "\r\n", 2);
	    }
	}
    }

    if (user->watcher && was_unassigned) {
	char buf[NMEA_MAX];
	(void)snprintf(buf, sizeof(buf), "GPSD,X=%f,I=%s\r\n", 
		       timestamp(), gpsd_id(user->device));
	(void)write(user->fd, buf, strlen(buf));
    }
    return true;
}
/*@ +branchstate +usedef +globstate @*/

#ifdef RTCM104_SERVICE
static int handle_rtcm_request(struct subscriber_t* sub UNUSED, char *buf UNUSED, int buflen UNUSED)
/* interpret a client request; cfd is the socket back to the client */
{
    return 0;	/* not actually interpreting these yet */
}
#endif /* RTCM104_SERVICE */

static /*@ observer @*/ char *snarfline(char *p, /*@out@*/char **out)
/* copy the rest of the command line, before CR-LF */
{
    char *q;
    static char	stash[BUFSIZ];

    /*@ -temptrans -mayaliasunique @*/
    for (q = p; isprint(*p) && !isspace(*p) && /*@i@*/(p-q < BUFSIZ-1); p++)
	continue;
    (void)memcpy(stash, q, (size_t)(p-q));
    stash[p-q] = '\0';
    *out = stash;
    return p;
    /*@ +temptrans +mayaliasunique @*/
}

static bool privileged_user(struct subscriber_t *who)
/* is this user privileged to change the GPS's behavior? */
{
    struct subscriber_t *sub;
    int subscribercount = 0;

    /* grant user privilege if he's the only one listening to the device */
    for (sub = subscribers; sub < subscribers + MAXSUBSCRIBERS; sub++)
	if (sub->device == who->device)
	    subscribercount++;
    return (subscribercount == 1);
}

static int handle_gpsd_request(struct subscriber_t* sub, char *buf, int buflen)
/* interpret a client request; cfd is the socket back to the client */
{
    char reply[BUFSIZ], phrase[BUFSIZ], *p, *stash;
    int i, j;
    struct gps_device_t *newchan;

    (void)strlcpy(reply, "GPSD", BUFSIZ);
    p = buf;
    while (*p != '\0' && p - buf < buflen) {
	phrase[0] = '\0';
	switch (toupper(*p++)) {
	case 'A':
	    if (assign_channel(sub) && have_fix(sub) && sub->fixbuffer.mode == MODE_3D)
		(void)snprintf(phrase, sizeof(phrase), ",A=%.3f", 
			sub->fixbuffer.altitude);
	    else
		(void)strlcpy(phrase, ",A=?", BUFSIZ);
	    break;
	case 'B':		/* change baud rate (SiRF/Zodiac only) */
#ifndef FIXED_PORT_SPEED
	    if (assign_channel(sub) && sub->device->device_type!=NULL && *p=='=' && privileged_user(sub)) {
		i = atoi(++p);
		while (isdigit(*p)) p++;
#ifdef ALLOW_RECONFIGURE
		if (sub->device->device_type->speed_switcher)
		    if (sub->device->device_type->speed_switcher(sub->device, (unsigned)i)) {
			/* 
			 * Allow the control string time to register at the
			 * GPS before we do the baud rate switch, which 
			 * effectively trashes the UART's buffer.
			 *
			 * This definitely fails below 40 milliseconds on a
			 * BU-303b. 50ms is also verified by Chris Kuethe on 
			 *        Pharos iGPS360 + GSW 2.3.1ES + prolific
			 *        Rayming TN-200 + GSW 2.3.1 + ftdi
			 *        Rayming TN-200 + GSW 2.3.2 + ftdi
			 * so it looks pretty solid.
			 *
			 * The minimum delay time is probably constant
			 * across any given type of UART.
			 */
			(void)tcdrain(sub->device->gpsdata.gps_fd);
			(void)usleep(50000);
			gpsd_set_speed(sub->device, (speed_t)i,
				(unsigned char)sub->device->gpsdata.parity,
				sub->device->gpsdata.stopbits);
		    }
#endif /* ALLOW_RECONFIGURE */
	    }
#endif /* FIXED_PORT_SPEED */
	    if (sub->device) {
		if ( sub->device->gpsdata.parity == 0 ) {
			/* zero parity breaks the next snprintf */
			sub->device->gpsdata.parity = (unsigned)'N';
		}
		(void)snprintf(phrase, sizeof(phrase), ",B=%d %d %c %u", 
		    (int)gpsd_get_speed(&sub->device->ttyset),
			9 - sub->device->gpsdata.stopbits, 
			(int)sub->device->gpsdata.parity,
			sub->device->gpsdata.stopbits);
	    } else {
		(void)strlcpy(phrase, ",B=?", BUFSIZ);
	    }
	    break;
	case 'C':
	    if (!assign_channel(sub) || sub->device->device_type==NULL)
		(void)strlcpy(phrase, ",C=?", BUFSIZ);
	    else {
		struct gps_type_t *dev = sub->device->device_type;
		double mincycle = (dev->cycle_chars * 10.0) / sub->device->gpsdata.baudrate;
		if (*p == '=' && privileged_user(sub)) {
		    double cycle = strtod(++p, &p);
		    if (cycle >= mincycle)
			if (dev->rate_switcher != NULL)
			    if (dev->rate_switcher(sub->device, cycle))
				dev->cycle = cycle;
		}
		if (dev->rate_switcher == NULL)
		    (void)snprintf(phrase, sizeof(phrase), 
				   ",C=%.2f", dev->cycle);
		else
		    (void)snprintf(phrase, sizeof(phrase), 
				   ",C=%.2f %.2f", dev->cycle, mincycle);
	    }
	    break;
	case 'D':
	    (void)strlcpy(phrase, ",D=", BUFSIZ);
	    if (assign_channel(sub) && isnan(sub->fixbuffer.time)==0)
		(void)unix_to_iso8601(sub->fixbuffer.time, 
				phrase+3, (int)(sizeof(phrase)-3));
	    else
		(void)strlcat(phrase, "?", BUFSIZ);
	    break;
	case 'E':
	    (void)strlcpy(phrase, ",E=", BUFSIZ);
	    if (assign_channel(sub) && have_fix(sub)) {
		if (finite(sub->device->gpsdata.epe))
		    (void)snprintf(phrase+strlen(phrase),
				   sizeof(phrase)-strlen(phrase),
				   "%.2f", sub->device->gpsdata.epe * (CEP50_SIGMA/GPSD_CONFIDENCE));
		else
		    (void)strlcat(phrase, "?", sizeof(phrase));
		if (finite(sub->device->gpsdata.fix.eph))
		    (void)snprintf(phrase+strlen(phrase),
				   sizeof(phrase)-strlen(phrase),
				   " %.2f", sub->device->gpsdata.fix.eph * (CEP50_SIGMA/GPSD_CONFIDENCE));
		else
		    (void)strlcat(phrase, " ?", sizeof(phrase));
		if (finite(sub->device->gpsdata.fix.epv))
		    (void)snprintf(phrase+strlen(phrase),
				   sizeof(phrase)-strlen(phrase),
				   " %.2f", sub->device->gpsdata.fix.epv * (CEP50_SIGMA/GPSD_CONFIDENCE));
		else
		    (void)strlcat(phrase, " ?", sizeof(phrase));
	    } else
		(void)strlcat(phrase, "?", sizeof(phrase));
	    break;
	case 'F':
	    /*@ -branchstate @*/
	    if (*p == '=') {
		p = snarfline(++p, &stash);
		gpsd_report(LOG_INF,"<= client(%d): switching to %s\n",sub_index(sub),stash);
		if ((newchan = find_device(stash))) {
		    /*@i@*/sub->device = newchan;
		    sub->tied = true;
		}
	    }
	    /*@ +branchstate @*/
	    if (sub->device != NULL)
		(void)snprintf(phrase, sizeof(phrase), ",F=%s", 
			 sub->device->gpsdata.gps_device);
	    else
		(void)strlcpy(phrase, ",F=?", BUFSIZ);
	    break;
	case 'G':
	    if (*p == '=') {
		gpsd_report(LOG_INF,"<= client(%d): requesting data type %s\n",sub_index(sub),++p);
		if (strncasecmp(p, "rtcm104", 7) == 0)
		    sub->requires = RTCM104;
		else if (strncasecmp(p, "gps", 3) == 0)
		    sub->requires = GPS;
		else
		    sub->requires = ANY;
		p += strcspn(p, ",\r\n");
	    }
	    (void)assign_channel(sub);
	    if (sub->device==NULL||sub->device->packet.type==BAD_PACKET)
		(void)strlcpy(phrase, ",G=?", BUFSIZ);
	    else if (sub->device->packet.type == RTCM_PACKET)
		(void)snprintf(phrase, sizeof(phrase), ",G=RTCM104");
	    else
		(void)snprintf(phrase, sizeof(phrase), ",G=GPS");
	    break;
	case 'I':
	    if (assign_channel(sub) && sub->device->device_type!=NULL) {
		(void)snprintf(phrase, sizeof(phrase), ",I=%s", 
			       gpsd_id(sub->device));
	    } else
		(void)strlcpy(phrase, ",I=?", BUFSIZ);
	    break;
	case 'J':
	    if (*p == '=') ++p;
	    if (*p == '1' || *p == '+') {
		sub->buffer_policy = nocasoc;
		p++;
	    } else if (*p == '0' || *p == '-') {
		sub->buffer_policy = casoc;
		p++;
	    }
	    (void)snprintf(phrase, sizeof(phrase), ",J=%u", sub->buffer_policy);
	    break;
	case 'K':
	    for (j = i = 0; i < MAXDEVICES; i++)
		if (allocated_channel(&channels[i]))
		    j++;
	    (void)snprintf(phrase, sizeof(phrase), ",K=%d ", j);
	    for (i = 0; i < MAXDEVICES; i++) {
		if (allocated_channel(&channels[i]) && strlen(phrase)+strlen(channels[i].gpsdata.gps_device)+1 < sizeof(phrase)) {
		    (void)strlcat(phrase, channels[i].gpsdata.gps_device, BUFSIZ);
		    (void)strlcat(phrase, " ", BUFSIZ);
		}
	    }
	    phrase[strlen(phrase)-1] = '\0';
	    break;
	case 'L':
	    (void)snprintf(phrase, sizeof(phrase), ",L=3 " VERSION " abcdefgijklmnopqrstuvwxyz");	//h
	    break;
	case 'M':
	    if (!assign_channel(sub) && (!sub->device || sub->fixbuffer.mode == MODE_NOT_SEEN))
		(void)strlcpy(phrase, ",M=?", BUFSIZ);
	    else
		(void)snprintf(phrase, sizeof(phrase), ",M=%d", sub->fixbuffer.mode);
	    break;
	case 'N':
	    if (!assign_channel(sub) || sub->device->device_type == NULL)
		(void)strlcpy(phrase, ",N=?", BUFSIZ);
	    else if (!sub->device->device_type->mode_switcher)
		(void)strlcpy(phrase, ",N=0", BUFSIZ);
#ifdef ALLOW_RECONFIGURE
	    else if (privileged_user(sub)) {
		if (*p == '=') ++p;
		if (*p == '1' || *p == '+') {
		    sub->device->device_type->mode_switcher(sub->device, 1);
		    p++;
		} else if (*p == '0' || *p == '-') {
		    sub->device->device_type->mode_switcher(sub->device, 0);
		    p++;
		}
	    }
#endif /* ALLOW_RECONFIGURE */
	    if (!sub->device)
		(void)snprintf(phrase, sizeof(phrase), ",N=?");
	    else
		(void)snprintf(phrase, sizeof(phrase), ",N=%u", sub->device->gpsdata.driver_mode);
	    break;
	case 'O':
	    if (!assign_channel(sub) || !have_fix(sub))
		(void)strlcpy(phrase, ",O=?", BUFSIZ);
	    else {
		(void)snprintf(phrase, sizeof(phrase), ",O=%s",
			       sub->device->gpsdata.tag[0]!='\0' ? sub->device->gpsdata.tag : "-");
		if (isnan(sub->fixbuffer.time)==0)
		    (void)snprintf(phrase+strlen(phrase),
				   sizeof(phrase)-strlen(phrase),
				   " %.3f",
				   sub->fixbuffer.time);
		else
		    (void)strlcat(phrase, " ?", BUFSIZ);
		if (isnan(sub->fixbuffer.ept)==0)
		    (void)snprintf(phrase+strlen(phrase),
				   sizeof(phrase)-strlen(phrase),
				   " %.3f",
				   sub->fixbuffer.ept);
		else
		    (void)strlcat(phrase, " ?", BUFSIZ);
		if (isnan(sub->fixbuffer.latitude)==0)
		    (void)snprintf(phrase+strlen(phrase),
				   sizeof(phrase)-strlen(phrase),
				   " %.6f",
				   sub->fixbuffer.latitude);
		else
		    (void)strlcat(phrase, " ?", BUFSIZ);
		if (isnan(sub->fixbuffer.longitude)==0)
		    (void)snprintf(phrase+strlen(phrase),
				   sizeof(phrase)-strlen(phrase),
				   " %.6f",
				   sub->fixbuffer.longitude);
		else
		    (void)strlcat(phrase, " ?", BUFSIZ);
		if (isnan(sub->fixbuffer.altitude)==0)
		    (void)snprintf(phrase+strlen(phrase),
				   sizeof(phrase)-strlen(phrase),
				   " %.2f",
				   sub->fixbuffer.altitude);
		else
		    (void)strlcat(phrase, " ?", BUFSIZ);
		if (isnan(sub->fixbuffer.eph)==0)
		    (void)snprintf(phrase+strlen(phrase), 
				   sizeof(phrase)-strlen(phrase),
				  " %.2f",  sub->fixbuffer.eph);
		else
		    (void)strlcat(phrase, " ?", BUFSIZ);
		if (isnan(sub->fixbuffer.epv)==0)
		    (void)snprintf(phrase+strlen(phrase), 
				   sizeof(phrase)-strlen(phrase),
				   " %.2f",  sub->fixbuffer.epv);
		else
		    (void)strlcat(phrase, " ?", BUFSIZ);
		if (isnan(sub->fixbuffer.track)==0)
		    (void)snprintf(phrase+strlen(phrase), 
				   sizeof(phrase)-strlen(phrase),
				   " %.4f %.3f",
				   sub->fixbuffer.track, 
				   sub->fixbuffer.speed);
		else
		    (void)strlcat(phrase, " ? ?", BUFSIZ);
		if (isnan(sub->fixbuffer.climb)==0)
		    (void)snprintf(phrase+strlen(phrase),
				   sizeof(phrase)-strlen(phrase),
				   " %.3f", 
				   sub->fixbuffer.climb);
		else
		    (void)strlcat(phrase, " ?", BUFSIZ);
		if (isnan(sub->fixbuffer.epd)==0)
		    (void)snprintf(phrase+strlen(phrase), 
				   sizeof(phrase)-strlen(phrase),
				   " %.4f",
				   sub->fixbuffer.epd);
		else
		    (void)strlcat(phrase, " ?", BUFSIZ);
		if (isnan(sub->fixbuffer.eps)==0)
		    (void)snprintf(phrase+strlen(phrase),
			     sizeof(phrase)-strlen(phrase),
			     " %.2f", sub->fixbuffer.eps);		    
		else
		    (void)strlcat(phrase, " ?", BUFSIZ);
		if (isnan(sub->fixbuffer.epc)==0)
		    (void)snprintf(phrase+strlen(phrase),
			     sizeof(phrase)-strlen(phrase),
			     " %.2f", sub->fixbuffer.epc);		    
		else
		    (void)strlcat(phrase, " ?", BUFSIZ);
		if (sub->fixbuffer.mode > 0)
		    (void)snprintf(phrase+strlen(phrase),
			     sizeof(phrase)-strlen(phrase),
			     " %d", sub->fixbuffer.mode);		    
		else
		    (void)strlcat(phrase, " ?", BUFSIZ);
	    }
	    break;
	case 'P':
	    if (assign_channel(sub) && have_fix(sub))
		(void)snprintf(phrase, sizeof(phrase), ",P=%.6f %.6f", 
			sub->fixbuffer.latitude, 
			sub->fixbuffer.longitude);
	    else
		(void)strlcpy(phrase, ",P=?", BUFSIZ);
	    break;
	case 'Q':
#define ZEROIZE(x)	(isnan(x)!=0 ? 0.0 : x)  
	    if (assign_channel(sub) && 
		(isnan(sub->device->gpsdata.pdop)==0
		 || isnan(sub->device->gpsdata.hdop)==0
		 || isnan(sub->device->gpsdata.vdop)==0))
		(void)snprintf(phrase, sizeof(phrase), ",Q=%d %.2f %.2f %.2f %.2f %.2f",
			sub->device->gpsdata.satellites_used, 
			ZEROIZE(sub->device->gpsdata.pdop), 
			ZEROIZE(sub->device->gpsdata.hdop), 
			ZEROIZE(sub->device->gpsdata.vdop),
			ZEROIZE(sub->device->gpsdata.tdop),
			ZEROIZE(sub->device->gpsdata.gdop));
	    else
		(void)strlcpy(phrase, ",Q=?", BUFSIZ);
#undef ZEROIZE
	    break;
	case 'R':
	    if (*p == '=') ++p;
	    if (*p == '2') {
		(void)assign_channel(sub);
		sub->raw = 2;
		gpsd_report(LOG_INF, "client(%d) turned on super-raw mode\n", sub_index(sub));
		(void)snprintf(phrase, sizeof(phrase), ",R=2");
		p++;
	    } else if (*p == '1' || *p == '+') {
		(void)assign_channel(sub);
		sub->raw = 1;
		gpsd_report(LOG_INF, "client(%d) turned on raw mode\n", sub_index(sub));
		(void)snprintf(phrase, sizeof(phrase), ",R=1");
		p++;
	    } else if (*p == '0' || *p == '-') {
		sub->raw = 0;
		gpsd_report(LOG_INF, "client(%d) turned off raw mode\n", sub_index(sub));
		(void)snprintf(phrase, sizeof(phrase), ",R=0");
		p++;
	    } else if (sub->raw) {
		sub->raw = 0;
		gpsd_report(LOG_INF, "client(%d) turned off raw mode\n", sub_index(sub));
		(void)snprintf(phrase, sizeof(phrase), ",R=0");
	    } else {
		(void)assign_channel(sub);
		sub->raw = 1;
		gpsd_report(LOG_INF, "client(%d) turned on raw mode\n", sub_index(sub));
		(void)snprintf(phrase, sizeof(phrase), ",R=1");
	    }
	    break;
	case 'S':
	    if (assign_channel(sub))
		(void)snprintf(phrase, sizeof(phrase), ",S=%d", sub->device->gpsdata.status);
	    else
		(void)strlcpy(phrase, ",S=?", BUFSIZ);
	    break;
	case 'T':
	    if (assign_channel(sub) && have_fix(sub) && isnan(sub->fixbuffer.track)==0)
		(void)snprintf(phrase, sizeof(phrase), ",T=%.4f", sub->fixbuffer.track);
	    else
		(void)strlcpy(phrase, ",T=?", BUFSIZ);
	    break;
	case 'U':
	    if (assign_channel(sub) && have_fix(sub) && sub->fixbuffer.mode == MODE_3D)
		(void)snprintf(phrase, sizeof(phrase), ",U=%.3f", sub->fixbuffer.climb);
	    else
		(void)strlcpy(phrase, ",U=?", BUFSIZ);
	    break;
	case 'V':
	    if (assign_channel(sub) && have_fix(sub) && isnan(sub->fixbuffer.speed)==0)
		(void)snprintf(phrase, sizeof(phrase), ",V=%.3f", sub->fixbuffer.speed * MPS_TO_KNOTS);
	    else
		(void)strlcpy(phrase, ",V=?", BUFSIZ);
	    break;
	case 'W':
	    if (*p == '=') ++p;
	    if (*p == '1' || *p == '+') {
		sub->watcher = true;
		(void)assign_channel(sub);
		(void)snprintf(phrase, sizeof(phrase), ",W=1");
		p++;
	    } else if (*p == '0' || *p == '-') {
		sub->watcher = false;
		(void)snprintf(phrase, sizeof(phrase), ",W=0");
		p++;
	    } else if (sub->watcher!=0) {
		sub->watcher = false;
		(void)snprintf(phrase, sizeof(phrase), ",W=0");
	    } else {
		sub->watcher = true;
		(void)assign_channel(sub);
		gpsd_report(LOG_INF, "client(%d) turned on watching\n", sub_index(sub));
		(void)snprintf(phrase, sizeof(phrase), ",W=1");
	    }
	    break;
        case 'X':
	    if (assign_channel(sub) && sub->device != NULL)
		(void)snprintf(phrase, sizeof(phrase), ",X=%f", sub->device->gpsdata.online);
	    else
		(void)strlcpy(phrase, ",X=?", BUFSIZ);
	    break;
	case 'Y':
	    if (assign_channel(sub) && sub->device->gpsdata.satellites > 0) {
		int used, reported = 0;
		(void)strlcpy(phrase, ",Y=", BUFSIZ);
		if (sub->device->gpsdata.tag[0] != '\0')
		    (void)strlcat(phrase, sub->device->gpsdata.tag, BUFSIZ);
		else
		    (void)strlcat(phrase, "-", BUFSIZ);
		if (isnan(sub->device->gpsdata.sentence_time)==0)
		    (void)snprintf(phrase+strlen(phrase), 
				   sizeof(phrase)-strlen(phrase),
				   " %.3f ",
				   sub->device->gpsdata.sentence_time);
		else
		    (void)strlcat(phrase, " ? ", BUFSIZ);
		/* insurance against flaky drivers */
		for (i = 0; i < sub->device->gpsdata.satellites; i++)
		    if (sub->device->gpsdata.PRN[i])
			reported++;
		(void)snprintf(phrase+strlen(phrase), 
			       sizeof(phrase)-strlen(phrase),
			       "%d:", reported);
		for (i = 0; i < sub->device->gpsdata.satellites; i++) {
		    used = 0;
		    for (j = 0; j < sub->device->gpsdata.satellites_used; j++)
			if (sub->device->gpsdata.used[j] == sub->device->gpsdata.PRN[i]) {
			    used = 1;
			    break;
			}
		    if (sub->device->gpsdata.PRN[i]) {
			(void)snprintf(phrase+strlen(phrase), 
				      sizeof(phrase)-strlen(phrase),
				      "%d %d %d %d %d:", 
				      sub->device->gpsdata.PRN[i], 
				      sub->device->gpsdata.elevation[i],sub->device->gpsdata.azimuth[i],
				      sub->device->gpsdata.ss[i],
				      used);
		    }
		}
		if (sub->device->gpsdata.satellites != reported)
		    gpsd_report(LOG_WARN,"Satellite count %d != PRN count %d\n",
				sub->device->gpsdata.satellites, reported);
	    } else
		(void)strlcpy(phrase, ",Y=?", BUFSIZ);
	    break;
	case 'Z':
	    (void)assign_channel(sub); 
	    if (*p == '=') ++p;
	    if (sub->device == NULL) {
		(void)snprintf(phrase, sizeof(phrase), ",Z=?");
		p++;		
	    } else if (*p == '1' || *p == '+') {
		sub->device->gpsdata.profiling = true;
		gpsd_report(LOG_INF, "client(%d) turned on profiling mode\n", sub_index(sub));
		(void)snprintf(phrase, sizeof(phrase), ",Z=1");
		p++;
	    } else if (*p == '0' || *p == '-') {
		sub->device->gpsdata.profiling = false;
		gpsd_report(LOG_INF, "client(%d) turned off profiling mode\n", sub_index(sub));
		(void)snprintf(phrase, sizeof(phrase), ",Z=0");
		p++;
	    } else {
		sub->device->gpsdata.profiling = !sub->device->gpsdata.profiling;
		gpsd_report(LOG_INF, "client(%d) toggled profiling mode\n", sub_index(sub));
		(void)snprintf(phrase, sizeof(phrase), ",Z=%d",
			       (int)sub->device->gpsdata.profiling);
	    }
	    break;
        case '$':
	    if (!assign_channel(sub))
		(void)strlcpy(phrase, ",$=?", BUFSIZ);
	    else if (sub->device->gpsdata.sentence_time!=0)
		(void)snprintf(phrase, sizeof(phrase), ",$=%s %d %lf %lf %lf %lf %lf %lf",
			sub->device->gpsdata.tag,
			(int)sub->device->gpsdata.sentence_length,
			sub->device->gpsdata.sentence_time,
			sub->device->gpsdata.d_xmit_time - sub->device->gpsdata.sentence_time,
			sub->device->gpsdata.d_recv_time - sub->device->gpsdata.sentence_time,
			sub->device->gpsdata.d_decode_time - sub->device->gpsdata.sentence_time,
			sub->device->poll_times[sub_index(sub)] - sub->device->gpsdata.sentence_time,
			timestamp() - sub->device->gpsdata.sentence_time);
	    else
		(void)snprintf(phrase, sizeof(phrase), ",$=%s %d 0 %lf %lf %lf %lf %lf",
			sub->device->gpsdata.tag,
			(int)sub->device->gpsdata.sentence_length,
			sub->device->gpsdata.d_xmit_time,
			sub->device->gpsdata.d_recv_time - sub->device->gpsdata.d_xmit_time,
			sub->device->gpsdata.d_decode_time - sub->device->gpsdata.d_xmit_time,
			sub->device->poll_times[sub_index(sub)] - sub->device->gpsdata.d_xmit_time,
			timestamp() - sub->device->gpsdata.d_xmit_time);
	    break;
	case '\r': case '\n':
	    goto breakout;
	}
	if (strlen(reply) + strlen(phrase) < sizeof(reply) - 1)
	    (void)strlcat(reply, phrase, BUFSIZ);
	else
	    return -1;	/* Buffer would overflow.  Just return an error */
    }
 breakout:
    (void)strlcat(reply, "\r\n", BUFSIZ);

    return (int)throttled_write(sub, reply, (ssize_t)strlen(reply));
}

static void handle_control(int sfd, char *buf)
/* handle privileged commands coming through the control socket */
{
    char	*p, *stash, *eq;
    struct gps_device_t	*chp;
    int cfd;

    if (buf[0] == '-') {
	p = snarfline(buf+1, &stash);
	gpsd_report(LOG_INF, "<= control(%d): removing %s\n", sfd, stash);
	if ((chp = find_device(stash))) {
	    if (chp->gpsdata.gps_fd > 0) {
		FD_CLR(chp->gpsdata.gps_fd, &all_fds);
		adjust_max_fd(chp->gpsdata.gps_fd, false);
	    }
	    notify_watchers(chp, "X=0\r\n");
	    for (cfd = 0; cfd < MAXSUBSCRIBERS; cfd++)
		if (subscribers[cfd].device == chp)
		    subscribers[cfd].device = NULL;
	    chp->gpsdata.gps_fd = -1;	/* device is already disconnected */
	    gpsd_wrap(chp);
	    /*@i@*/free_channel(chp);	/* modifying observer storage */
	    (void)write(sfd, "OK\n", 3);
	} else
	    (void)write(sfd, "ERROR\n", 6);
    } else if (buf[0] == '+') {
	p = snarfline(buf+1, &stash);
	if (find_device(stash)) {
	    gpsd_report(LOG_INF,"<= control(%d): %s already active \n", sfd, stash);
		(void)write(sfd, "ERROR\n", 6);
	} else {
	    gpsd_report(LOG_INF,"<= control(%d): adding %s \n", sfd, stash);
	    if (open_device(stash))
		(void)write(sfd, "OK\n", 3);
	    else
		(void)write(sfd, "ERROR\n", 6);
	}
    } else if (buf[0] == '!') {
	p = snarfline(buf+1, &stash);
	eq = strchr(stash, '=');
	if (eq == NULL) {
	    gpsd_report(LOG_WARN,"<= control(%d): ill-formed command \n", sfd);
	    (void)write(sfd, "ERROR\n", 3);
	} else {
	    *eq++ = '\0';
	    if ((chp = find_device(stash))) {
		gpsd_report(LOG_INF,"<= control(%d): writing to %s \n", sfd, stash);
		(void)write(chp->gpsdata.gps_fd, eq, strlen(eq));
		(void)write(sfd, "OK\n", 3);
	    } else {
		gpsd_report(LOG_INF,"<= control(%d): %s not active \n", sfd, stash);
		(void)write(sfd, "ERROR\n", 6);
	    }
	}
    }
}

/*@ -mustfreefresh @*/
int main(int argc, char *argv[])
{
    static char *pid_file = NULL;
    static int st, csock = -1;
    static gps_mask_t changed;
    static char *gpsd_service = NULL; 
#ifdef RTCM104_SERVICE
    static char *rtcm_service = NULL; 
    static int nsock;
#endif /* RTCM104_SERVICE */
    static char *control_socket = NULL;
    struct gps_device_t *channel;
    struct sockaddr_in fsin;
    fd_set rfds, control_fds;
    int i, option, msock, cfd, dfd; 
    bool go_background = true;
    struct timeval tv;
    int raw = 0;
#ifdef RTCM104_SERVICE
    struct gps_device_t *gps;
#endif /* RTCM104_SERVICE */
    struct subscriber_t *sub;

#ifdef PPS_ENABLE
    pthread_mutex_init(&report_mutex, NULL);
#endif /* PPS_ENABLE */

    debuglevel = 0;
    while ((option = getopt(argc, argv, "F:D:S:dfhNRnpP:V"
#ifdef RTCM104_SERVICE
			    "R:"
#endif /* RTCM104_SERVICE */
		)) != -1) {
	switch (option) {
	case 'R':
	    raw = 1;
	    break;
	case 'D':
	    debuglevel = (int) strtol(optarg, 0, 0);
	    break;
	case 'F':
	    control_socket = optarg;
	    break;
	case 'N':
	    go_background = false;
	    break;
#ifdef RTCM104_SERVICE
	case 'R':
	    rtcm_service = optarg;
	    break;
#endif /* RTCM104_SERVICE */
	case 'S':
	    gpsd_service = optarg;
	    break;
	case 'n':
	    nowait = true;
	    break;
	case 'd':
	case 'f':
	case 'p':
	    /* skip this option, treat following as argument */ 
	    break;
	case 'P':
	    pid_file = optarg;
	    break;
	case 'V':
	    (void)printf("gpsd %s\n", VERSION);
	    exit(0);
	case 'h': case '?':
	default:
	    usage();
	    exit(0);
	}
    }

#ifdef FIXED_PORT_SPEED
    /* Asssume that if we're running with FIXED_PORT_SPEED we're some sort
     * of embedded configuration where we don't want to wait for connect */
    nowait = true;
#endif

    if (!control_socket && optind >= argc) {
	gpsd_report(LOG_ERROR, "can't run with neither control socket nor devices\n");
	exit(1);
    }

    /*
     * Control socket has to be created before we go background in order to
     * avoid a race condition in which hotplug scripts can try opening
     * the socket before it's created.
     */
    if (control_socket) {
	(void)unlink(control_socket);
	if ((csock = filesock(control_socket)) < 0) {
	    gpsd_report(LOG_ERROR,"control socket create failed, netlib error %d\n",csock);
	    exit(2);
	}
	FD_SET(csock, &all_fds);
	adjust_max_fd(csock, true);
	gpsd_report(LOG_PROG, "control socket opened at %s\n", control_socket);
    }

    if (go_background)
	(void)daemonize();

    if (pid_file) {
	FILE *fp;

	if ((fp = fopen(pid_file, "w")) != NULL) {
	    (void)fprintf(fp, "%u\n", (unsigned int)getpid());
	    (void)fclose(fp);
	} else {
	    gpsd_report(LOG_ERROR, "Cannot create PID file: %s.\n", pid_file);
	}
    }

    openlog("gpsd", LOG_PID, LOG_USER);
    gpsd_report(LOG_INF, "launching (Version %s)\n", VERSION);
    /*@ -observertrans @*/
    if (!gpsd_service)
	gpsd_service = getservbyname("gpsd", "tcp") ? "gpsd" : DEFAULT_GPSD_PORT;
    /*@ +observertrans @*/
    if ((msock = passivesock(gpsd_service, "tcp", QLEN)) < 0) {
	gpsd_report(LOG_ERR,"command socket create failed, netlib error %d\n",msock);
	exit(2);
    }
    gpsd_report(LOG_INF, "listening on port %s\n", gpsd_service);
#ifdef RTCM104_SERVICE
    /*@ -observertrans @*/
    if (!rtcm_service)
	rtcm_service = getservbyname("rtcm", "tcp") ? "rtcm" : DEFAULT_RTCM_PORT;
    /*@ +observertrans @*/
    if ((nsock = passivesock(rtcm_service, "tcp", QLEN)) < 0) {
	gpsd_report(LOG_ERROR,"RTCM104 socket create failed, netlib error %d\n",nsock);
	exit(2);
    }
    gpsd_report(LOG_INF, "listening on port %s\n", rtcm_service);
#endif /* RTCM104_SERVICE */

#ifdef NTPSHM_ENABLE
    if (getuid() == 0) {
	(void)nice(-10);		/* for precise timekeeping increase priority */
	(void)ntpshm_init(&context, nowait);
    } else {
	gpsd_report (2, "Unable to start ntpshm.  gpsd must run as root.\n");
    }
#endif /* NTPSHM_ENABLE */

#ifdef DBUS_ENABLE
    /* we need to connect to dbus as root */
    if (initialize_dbus_connection()) {
	/* the connection could not be started */
	gpsd_report (2, "unable to connect to the DBUS system bus\n");
    } else
	gpsd_report (2, "successfully connected to the DBUS system bus\n");
#endif /* DBUS_ENABLE */

    if (getuid() == 0 && go_background) {
	struct passwd *pw;
	struct stat stb;

	/* make default devices accessible even after we drop privileges */
	for (i = optind; i < argc; i++) 
	    if (stat(argv[i], &stb) == 0)
		(void)chmod(argv[i], stb.st_mode|S_IRGRP|S_IWGRP);
	/*
	 * Drop privileges.  Up to now we've been running as root.  Instead,
	 * set the user ID to 'nobody' and the group ID to the owning group 
	 * of a prototypical TTY device.  This limits the scope of any
	 * compromises in the code.  It requires that all GPS devices have
	 * their group read/write permissions set.
	 */
	if ((optind<argc&&stat(argv[optind], &stb)==0)||stat(PROTO_TTY,&stb)==0) {
	    gpsd_report(LOG_PROG, "changing to group %d\n", stb.st_gid);
	    if (setgid(stb.st_gid) != 0)
		gpsd_report(LOG_ERROR, "setgid() failed, errno %s\n", strerror(errno));
	}
	pw = getpwnam("nobody");
	if (pw)
	    (void)seteuid(pw->pw_uid);
    }
    gpsd_report(LOG_INF, "running with effective group ID %d\n", getegid());
    gpsd_report(LOG_INF, "running with effective user ID %d\n", geteuid());

    for (sub = subscribers; sub < subscribers + MAXSUBSCRIBERS; sub++) {
	gps_clear_fix(&sub->fixbuffer);
	gps_clear_fix(&sub->oldfix);
    }

    /* daemon got termination or interrupt signal */
    if ((st = setjmp(restartbuf)) > 0) {
	/* try to undo all device configurations */
	for (dfd = 0; dfd < MAXDEVICES; dfd++) {
	    if (allocated_channel(&channels[dfd]))
		(void)gpsd_wrap(&channels[dfd]);
	}
	if (st == SIGHUP+1)
	    gpsd_report(LOG_WARN, "gpsd restarted by SIGHUP\n");
	else if (st > 0) {
	    gpsd_report(LOG_WARN, "Received terminating signal %d. Exiting...\n",st-1);
	    if (control_socket)
		(void)unlink(control_socket);
	    if (pid_file)
		(void)unlink(pid_file);
	    exit(10 + st);
	}
    }

    /* Handle some signals */
    (void)signal(SIGHUP, onsig);
    (void)signal(SIGINT, onsig);
    (void)signal(SIGTERM, onsig);
    (void)signal(SIGQUIT, onsig);
    (void)signal(SIGPIPE, SIG_IGN);

    FD_SET(msock, &all_fds);
    adjust_max_fd(msock, true);
#ifdef RTCM104_SERVICE
    FD_SET(nsock, &all_fds);
    adjust_max_fd(nsock, true);
#endif /* RTCM104_SERVICE */
    FD_ZERO(&control_fds);

    /* optimization hack to defer having to read subframe data */
    if (time(NULL) < START_SUBFRAME)
	context.valid |= LEAP_SECOND_VALID;

    for (i = optind; i < argc; i++) { 
	struct gps_device_t *device = open_device(argv[i]);
	if (!device) {
	    gpsd_report(LOG_ERROR, "GPS device %s nonexistent or can't be read\n", argv[i]);
	}
    }

    for (;;) {
        (void)memcpy((char *)&rfds, (char *)&all_fds, sizeof(rfds));

	gpsd_report(LOG_RAW+2, "select waits\n");
	/* 
	 * Poll for user commands or GPS data.  The timeout doesn't
	 * actually matter here since select returns whenever one of
	 * the file descriptors in the set goes ready.  The point
	 * of tracking maxfd is to keep the set of descriptors that
	 * select(2) has to poll here as small as possible (for
	 * low-clock-rate SBCs and the like).
	 */
	/*@ -usedef @*/
	tv.tv_sec = 1; tv.tv_usec = 0;
	if (select(maxfd+1, &rfds, NULL, NULL, &tv) < 0) {
	    if (errno == EINTR)
		continue;
	    gpsd_report(LOG_ERROR, "select: %s\n", strerror(errno));
	    exit(2);
	}
	/*@ +usedef @*/

#ifdef __UNUSED__
	{
	    char dbuf[BUFSIZ];
	    dbuf[0] = '\0';
	    for (sub = subscribers; sub < subscribers + MAXSUBSCRIBERS; sub++)
		if (FD_ISSET(sub->fd, &all_fds))
		    (void)snprintf(dbuf + strlen(dbuf), 
				   sizeof(dbuf)-strlen(dbuf),
				   " %d", sub->fd);
	    strlcat(dbuf, "} -> {", BUFSIZ);
	    for (sub = subscribers; sub < subscribers + MAXSUBSCRIBERS; sub++)
		if (FD_ISSET(sub->fd, &rfds))
		    (void)snprintf(dbuf + strlen(dbuf), 
				   sizeof(dbuf)-strlen(dbuf),
				   " %d", sub->fd);
	    gpsd_report(LOG_RAW, "Polling descriptor set: {%s}\n", dbuf);
	}
#endif /* UNUSED */

	/* always be open to new client connections */
	if (FD_ISSET(msock, &rfds)) {
	    socklen_t alen = (socklen_t)sizeof(fsin);
	    /*@i1@*/int ssock = accept(msock, (struct sockaddr *) &fsin, &alen);

	    if (ssock < 0)
		gpsd_report(LOG_ERROR, "accept: %s\n", strerror(errno));
	    else {
		struct subscriber_t *client = NULL;
		int opts = fcntl(ssock, F_GETFL);

		if (opts >= 0)
		    (void)fcntl(ssock, F_SETFL, opts | O_NONBLOCK);
		gpsd_report(LOG_INF, "client connect on %d\n", ssock);

		client = allocate_client();
		if (raw > 0 ) {
			gpsd_report(LOG_INF, "setting initial raw mode %d\n", raw);
			client->raw=raw;
			(void)assign_channel(client);
		}
		if (client == NULL) {
		    gpsd_report(LOG_ERROR, "No client subscriber slots available!\n");
		    (void)close(ssock);
		} else {
			FD_SET(ssock, &all_fds);
			adjust_max_fd(ssock, true);
			client->fd = ssock;
			client->active = timestamp();
			client->tied = false;
			client->requires = ANY;
		}
	    }
	    FD_CLR(msock, &rfds);
	}

#ifdef RTCM104_SERVICE
	/* also to RTCM client connections */
	if (FD_ISSET(nsock, &rfds)) {
	    socklen_t alen = (socklen_t)sizeof(fsin);
	    /*@i1@*/int ssock = accept(nsock, (struct sockaddr *)&fsin, &alen);

	    if (ssock < 0)
		gpsd_report(LOG_ERROR, "accept: %s\n", strerror(errno));
	    else {
		struct subscriber_t *client = NULL;
		int opts = fcntl(ssock, F_GETFL);

		if (opts >= 0)
		    (void)fcntl(ssock, F_SETFL, opts | O_NONBLOCK);
		gpsd_report(LOG_INF, "client connect on %d\n", ssock);
		client = allocate_client();
		if (client == NULL) {
		    gpsd_report(LOG_ERROR, "No client subscriber slots available!\n");
		    close(ssock);
		} else {
		    FD_SET(ssock, &all_fds);
		    adjust_max_fd(ssock, true);
		    client->active = true;
		    client->tied = false;
		    client->requires = RTCM104;
		    client->fd = ssock;
		}
	    }
	    FD_CLR(nsock, &rfds);
	}
#endif /* RTCM104_SERVICE */

	/* also be open to new control-socket connections */
	if (csock > -1 && FD_ISSET(csock, &rfds)) {
	    socklen_t alen = (socklen_t)sizeof(fsin);
	    /*@i1@*/int ssock = accept(csock, (struct sockaddr *) &fsin, &alen);

	    if (ssock < 0)
		gpsd_report(LOG_ERROR, "accept: %s\n", strerror(errno));
	    else {
		gpsd_report(LOG_INF, "control socket connect on %d\n", ssock);
		FD_SET(ssock, &all_fds);
		FD_SET(ssock, &control_fds);
		adjust_max_fd(ssock, true);
	    }
	    FD_CLR(csock, &rfds);
	}

	if (context.dsock >= 0 && FD_ISSET(context.dsock, &rfds)) {
	    /* be ready for DGPS reports */
	    if (dgnss_poll(&context) == -1){
		FD_CLR(context.dsock, &all_fds);
		FD_CLR(context.dsock, &rfds);
		context.dsock = -1;
	    }
	}
	/* read any commands that came in over control sockets */
	for (cfd = 0; cfd < FD_SETSIZE; cfd++)
	    if (FD_ISSET(cfd, &control_fds)) {
		char buf[BUFSIZ];

		while (read(cfd, buf, sizeof(buf)-1) > 0) {
		    gpsd_report(LOG_IO, "<= control(%d): %s\n", cfd, buf);
		    handle_control(cfd, buf);
		}
		(void)close(cfd);
		FD_CLR(cfd, &all_fds);
		FD_CLR(cfd, &control_fds);
		adjust_max_fd(cfd, false);
	    }

	/* poll all active devices */
	for (channel = channels; channel < channels + MAXDEVICES; channel++) {
	    if (!allocated_channel(channel))
		continue;

	    /* pass the current RTCM correction to the GPS if new */
	    if (channel->device_type)
		rtcm_relay(channel);

	    /* get data from the device */
	    changed = 0;
	    if (channel->gpsdata.gps_fd >= 0 && FD_ISSET(channel->gpsdata.gps_fd, &rfds))
	    {
		gpsd_report(LOG_RAW+1, "polling %d\n", channel->gpsdata.gps_fd);
		changed = gpsd_poll(channel);
		if (changed == ERROR_SET) {
		    gpsd_report(LOG_WARN, "packet sniffer failed to sync up\n");
		    FD_CLR(channel->gpsdata.gps_fd, &all_fds);
		    adjust_max_fd(channel->gpsdata.gps_fd, false);
		    gpsd_deactivate(channel);
		} else if ((changed & ONLINE_SET) == 0) {
/*
		    gpsd_report(LOG_INF, "GPS is offline (%lf sec since data)\n", 
				timestamp() - channel->gpsdata.online);
*/
		    FD_CLR(channel->gpsdata.gps_fd, &all_fds);
		    adjust_max_fd(channel->gpsdata.gps_fd, false);
		    gpsd_deactivate(channel);
		    notify_watchers(channel, "GPSD,X=0\r\n");
		}
		else {
		    /* handle laggy response to a firmware version query*/
		    if ((changed & DEVICEID_SET) != 0) {
			char id[NMEA_MAX];
			assert(channel->device_type != NULL);
			(void)snprintf(id, sizeof(id), "GPSD,I=%s", 
				       channel->device_type->typename);
			if (channel->subtype[0] != '\0') {
			    (void)strlcat(id, " ", sizeof(id));
			    (void)strlcat(id,channel->subtype,sizeof(id));
			}
			(void)strlcat(id, "\r\n", sizeof(id));
			notify_watchers(channel, id);
		    }
		    /* copy/merge channel data into subscriber fix buffers */
		    for (sub = subscribers;
			 sub < subscribers + MAXSUBSCRIBERS;
			 sub++) {
			if (sub->device == channel) {
			    if (sub->buffer_policy == casoc && (changed & CYCLE_START_SET)!=0)
				gps_clear_fix(&sub->fixbuffer);
			    /* don't downgrade mode if holding previous fix */
			    if (sub->fixbuffer.mode > sub->device->gpsdata.fix.mode)
				changed &=~ MODE_SET;
			    gps_merge_fix(&sub->fixbuffer, 
					  changed,
					  &sub->device->gpsdata.fix);
			    gpsd_error_model(sub->device, 
					     &sub->fixbuffer, &sub->oldfix);
			}
		    }
		}
#ifdef RTCM104_SERVICE
		/* copy each RTCM-104 correction to all GPSes */
		if ((changed & RTCM_SET) != 0) {
		    for (gps = channels; gps < channels + MAXDEVICES; gps++)
			if (gps->device_type != NULL && gps->device_type->rtcm_writer != NULL)
			    (void)gps->device_type->rtcm_writer(gps, (char *)channel->outbuffer, channel->outbuflen);
		}
#endif /* RTCM104_SERVICE */
	    }

	    for (sub = subscribers; sub < subscribers + MAXSUBSCRIBERS; sub++) {
		/* some listeners may be in watcher mode */
		if (sub->watcher) {
		    char cmds[4] = ""; 
		    channel->poll_times[sub - subscribers] = timestamp();
		    if (changed &~ ONLINE_SET) {
			if (changed & (LATLON_SET | MODE_SET))
			    (void)strlcat(cmds, "o", 4);
			if (changed & SATELLITE_SET)
			    (void)strlcat(cmds, "y", 4);
			if (channel->gpsdata.profiling!=0)
			    (void)strlcat(cmds, "$", 4);
		    }
		    if (cmds[0] != '\0')
			(void)handle_gpsd_request(sub, cmds, (int)strlen(cmds));
		}
	    }
#ifdef DBUS_ENABLE
	    if (changed &~ ONLINE_SET) {
		    if (changed & (LATLON_SET | MODE_SET)) 
			    send_dbus_fix (channel);
	    }
#endif
	}

#ifdef NOT_FIXED
	if (context.fixcnt > 0 && context.dsock == -1) {
	    for (channel=channels; channel < channels+MAXDEVICES; channel++) {
		if (channel->gpsdata.fix.mode > MODE_NO_FIX) {
		    dgnss_autoconnect(&context,
				      channel->gpsdata.fix.latitude,
				      channel->gpsdata.fix.longitude);
		    break;
		}
	    }
	}
#endif

	/* accept and execute commands for all clients */
	for (sub = subscribers; sub < subscribers + MAXSUBSCRIBERS; sub++) {
	    if (sub->active == 0) 
		continue;

	    if (FD_ISSET(sub->fd, &rfds)) {
		char buf[BUFSIZ];
		int buflen;

		gpsd_report(LOG_PROG, "checking client(%d)\n", sub_index(sub));
		if ((buflen = (int)read(sub->fd, buf, sizeof(buf) - 1)) <= 0) {
		    detach_client(sub);
		} else {
		    if (buf[buflen-1] != '\n')
			buf[buflen++] = '\n';
		    buf[buflen] = '\0';
		    gpsd_report(LOG_IO, "<= client(%d): %s", sub_index(sub), buf);

#ifdef RTCM104_SERVICE
		    if (sub->requires==RTCM104
			|| sub->requires==ANY) {
			if (handle_rtcm_request(sub, buf, buflen) < 0)
			    detach_client(sub);
		    } else
#endif /* RTCM104_SERVICE */
		    {
			if (sub->device){
		            /*
			     * when a command comes in, to update .active to
			     * timestamp() so we don't close the connection
			     * after POLLER_TIMEOUT seconds. This makes
			     * POLLER_TIMEOUT useful.
			     */
			    sub->active = sub->device->poll_times[sub_index(sub)] = timestamp();
                        }
			if (handle_gpsd_request(sub, buf, buflen) < 0)
			    detach_client(sub);
		    }
		}
	    } else if (sub->device == NULL && timestamp() - sub->active > ASSIGNMENT_TIMEOUT) {
		gpsd_report(LOG_WARN, "client(%d) timed out before assignment request.\n", sub_index(sub));
		detach_client(sub);
	    } else if (sub->device != NULL && !(sub->watcher || sub->raw>0) && timestamp() - sub->active > POLLER_TIMEOUT) {
		gpsd_report(LOG_WARN, "client(%d) timed out on command wait.\n", cfd);
		detach_client(sub);
	    }
	}

	/*
	 * Close devices with an identified packet type but no remaining 
	 * subscribers.  The reason the test has this particular form is 
	 * so that, immediately after device open, we'll keep reading 
	 * packets until a type is identified even though there are no
	 * subscribers yet.  We need this to happen so that subscribers 
	 * can later choose a device by packet type.
	 */
	if (!nowait)
	    for (channel=channels; channel < channels+MAXDEVICES; channel++) {
		if (allocated_channel(channel)) {
		    if (channel->packet.type != BAD_PACKET) {
			bool need_gps = false;

			for (cfd = 0; cfd < MAXSUBSCRIBERS; cfd++)
			    if (subscribers[cfd].device == channel)
				need_gps = true;

			if (!need_gps && channel->gpsdata.gps_fd > -1) {
			    gpsd_report(LOG_RAW, "unflagging descriptor %d\n", channel->gpsdata.gps_fd);
			    FD_CLR(channel->gpsdata.gps_fd, &all_fds);
			    adjust_max_fd(channel->gpsdata.gps_fd, false);
			    gpsd_deactivate(channel);
			}
		    }
		}
	    }
    }

    if (control_socket)
	(void)unlink(control_socket);
    if (pid_file)
	(void)unlink(pid_file);
    return 0;
}
/*@ +mustfreefresh @*/
