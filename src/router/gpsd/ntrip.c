/* $Id: ntrip.c 3771 2006-11-02 05:15:20Z esr $ */
/* ntrip.c -- gather and dispatch DGNSS data from Ntrip broadcasters */
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <netdb.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include "gpsd_config.h"

#ifdef NTRIP_ENABLE
#include "gpsd.h"
#include "bsd-base64.h"

struct ntrip_stream_t {
    char mountpoint[101];
    enum { fmt_rtcm2, fmt_rtcm2_0, fmt_rtcm2_1, fmt_rtcm2_2, fmt_rtcm2_3, fmt_unknown } format;
    int carrier;
    double latitude;
    double longitude;
    int nmea;
    enum { cmp_enc_none, cmp_enc_unknown } compr_encryp;
    enum { auth_none, auth_basic, auth_digest, auth_unknown } authentication;
    int fee;
    int bitrate;
};

#define NTRIP_SOURCETABLE	"SOURCETABLE 200 OK\r\n"
#define NTRIP_ENDSOURCETABLE	"ENDSOURCETABLE"
#define NTRIP_CAS		"CAS;"
#define NTRIP_NET		"NET;"
#define NTRIP_STR		"STR;"
#define NTRIP_BR		"\r\n"
#define NTRIP_QSC		"\";\""
#define NTRIP_ICY		"ICY 200 OK"
#define NTRIP_UNAUTH		"401 Unauthorized"

static struct ntrip_stream_t ntrip_stream;

/*@ -temptrans -mustfreefresh @*/
static char *ntrip_field_iterate(char *start, /*@null@*/char *prev, const char *eol)
{
    char *s, *t, *u;

    if (start)
	    s = start;
    else {
	if (!prev)
	    return NULL;
	s = prev + strlen(prev) + 1;
	if (s >= eol)
	    return NULL;
    }

    /* ignore any quoted ; chars as they are part of the field content */
    t = s;
    while ((u = strstr(t, NTRIP_QSC)))
	t = u + strlen(NTRIP_QSC);

    if ((t = strstr(t, ";")))
	*t = '\0';

    gpsd_report(LOG_RAW, "Next Ntrip source table field %s\n", s);	

    return s;
}
/*@ +temptrans +mustfreefresh @*/

/*@ -mustfreefresh @*/
static void ntrip_str_parse(char *str, size_t len,
			    /*@out@*/struct ntrip_stream_t *hold)
{
    char *s, *eol = str + len;

    memset(hold, 0, sizeof(*hold));

    /* <mountpoint> */
    if ((s = ntrip_field_iterate(str, NULL, eol)))
	strncpy(hold->mountpoint, s, sizeof(hold->mountpoint) - 1);
    /* <identifier> */
    s = ntrip_field_iterate(NULL, s, eol);
    /* <format> */
    if ((s = ntrip_field_iterate(NULL, s, eol))) {
	if (strcasecmp("RTCM 2", s)==0)
	    hold->format = fmt_rtcm2;
	else if (strcasecmp("RTCM 2.0", s)==0)
	    hold->format = fmt_rtcm2_0;
	else if (strcasecmp("RTCM 2.1", s)==0)
	    hold->format = fmt_rtcm2_1;
	else if (strcasecmp("RTCM 2.2", s)==0)
	    hold->format = fmt_rtcm2_2;
	else if (strcasecmp("RTCM 2.3", s)==0)
	    hold->format = fmt_rtcm2_3;
	else
	    hold->format = fmt_unknown;
    }
    /* <format-details> */
    s = ntrip_field_iterate(NULL, s, eol);
    /* <carrier> */
    if ((s = ntrip_field_iterate(NULL, s, eol)))
	(void)sscanf(s, "%d", &hold->carrier);
    /* <nav-system> */
    s = ntrip_field_iterate(NULL, s, eol);
    /* <network> */
    s = ntrip_field_iterate(NULL, s, eol);
    /* <country> */
    s = ntrip_field_iterate(NULL, s, eol);
    /* <latitude> */
    hold->latitude = NAN;
    if ((s = ntrip_field_iterate(NULL, s, eol)))
	(void)sscanf(s, "%lf", &hold->latitude);
    /* <longitude> */
    hold->longitude = NAN;
    if ((s = ntrip_field_iterate(NULL, s, eol)))
	(void)sscanf(s, "%lf", &hold->longitude);
    /* <nmea> */
    if ((s = ntrip_field_iterate(NULL, s, eol))) {
	(void)sscanf(s, "%d", &hold->nmea);
    }
    /* <solution> */
    s = ntrip_field_iterate(NULL, s, eol);
    /* <generator> */
    s = ntrip_field_iterate(NULL, s, eol);
    /* <compr-encryp> */
    if ((s = ntrip_field_iterate(NULL, s, eol))) {
	if (strcasecmp("none", s)==0)
	    hold->compr_encryp = cmp_enc_none;
	else
	    hold->compr_encryp = cmp_enc_unknown;
    }
    /* <authentication> */
    if ((s = ntrip_field_iterate(NULL, s, eol))) {
	if (strcasecmp("N", s)==0)
	    hold->authentication = auth_none;
	else if (strcasecmp("B", s)==0)
	    hold->authentication = auth_basic;
	else if (strcasecmp("D", s)==0)
	    hold->authentication = auth_digest;
	else
	    hold->authentication = auth_unknown;
    }
    /* <fee> */
    if ((s = ntrip_field_iterate(NULL, s, eol))) {
	(void)sscanf(s, "%d", &hold->fee);
    }
    /* <bitrate> */
    if ((s = ntrip_field_iterate(NULL, s, eol))) {
	(void)sscanf(s, "%d", &hold->bitrate);
    }
    /* ...<misc> */
    while ((s = ntrip_field_iterate(NULL, s, eol)));
}
/*@ +mustfreefresh @*/

static int ntrip_sourcetable_parse(int fd, char *buf, ssize_t blen,
				   const char *stream,
				   struct ntrip_stream_t *keep)
{
    struct ntrip_stream_t hold;
    ssize_t llen, len = 0;
    char *line;
    bool srctbl = false;
    bool match = false;

    for (;;) {
	char *eol;
	ssize_t rlen;

	memset(&buf[len], 0, (size_t)(blen - len));

	if ((rlen = recv(fd, &buf[len], (size_t)(blen - 1 - len), 0)) < 0) {
	    if (errno == EINTR)
		continue;
	    return -1;
	}
	if (rlen == 0)
	    continue;

	line = buf;
	rlen = len += rlen;

	gpsd_report(LOG_RAW, "Ntrip source table buffer %s\n", buf);	
	
	if (!srctbl) {
	    /* parse SOURCETABLE */
	    if (strncmp(line,NTRIP_SOURCETABLE,strlen(NTRIP_SOURCETABLE))==0) {
		srctbl = true;
		llen = (ssize_t)strlen(NTRIP_SOURCETABLE);
		line += llen;
		len -= llen;
	    } else {
		gpsd_report(LOG_WARN, "Received unexpexted Ntrip reply %s.\n", buf);
		return -1;
	    }
	}
	if (!srctbl)
		return -1;

	while (len > 0) {
	    /* parse ENDSOURCETABLE */
	    if (strncmp(line, NTRIP_ENDSOURCETABLE, strlen(NTRIP_ENDSOURCETABLE))==0)
		goto done;

	    if (!(eol = strstr(line, NTRIP_BR)))
		break;

	    gpsd_report(LOG_IO, "next Ntrip source table line %s\n", line);	

	    *eol = '\0';
	    llen = (ssize_t)(eol - line);

	    /* todo: parse headers */

	    /* parse STR */
	    if (strncmp(line, NTRIP_STR, strlen(NTRIP_STR))==0) {
		ntrip_str_parse(line + strlen(NTRIP_STR), (size_t)(llen - strlen(NTRIP_STR)), &hold);
		if (stream!=NULL && strcmp(stream, hold.mountpoint)==0) {
		    /* todo: support for RTCM 3.0, SBAS (WAAS, EGNOS), ... */
		    if (hold.format == fmt_unknown) {
			gpsd_report(LOG_ERROR,
				    "Ntrip stream %s format not supported\n",
				    line);	
			return -1;
		    }
		    /* todo: support encryption and compression algorithms */
		    if (hold.compr_encryp != cmp_enc_none) {
			gpsd_report(LOG_ERROR,
				    "Ntrip stream %s compression/encryption algorithm not supported\n",
				    line);	
			return -1;
		    }
		    /* todo: support digest authentication */
		    if (hold.authentication != auth_none 
			&& hold.authentication != auth_basic) {
			gpsd_report(LOG_ERROR,
				    "Ntrip stream %s authentication method not supported\n",
				    line);	
			return -1;
		    }
		    memcpy(keep, &hold, sizeof(hold));
		    match = true;
		}
		/* todo: compare stream location to own location to
		   find nearest stream if user hasn't provided one */
	    }
	    /* todo: parse CAS */
	    /* else if (strncmp(line, NTRIP_CAS, strlen(NTRIP_CAS))==0); */

	    /* todo: parse NET */
	    /* else if (strncmp(line, NTRIP_NET, strlen(NTRIP_NET))==0); */

	    llen += strlen(NTRIP_BR);
	    line += llen;
	    len -= llen;
	    gpsd_report(LOG_RAW, "Remaining Ntrip source table buffer %d %s\n", len, line);
	}
	/* message too big to fit into buffer */
	if (len == blen - 1)
	    return -1;

	if (len > 0)
	    memcpy(buf, &buf[rlen-len], (size_t)len);
    }

done:
    return match ? 0 : -1;
}

static int ntrip_stream_probe(const char *caster,
			      const char *port,
			      const char *stream,
			      struct ntrip_stream_t *keep)
{
    int ret;
    int dsock;
    char buf[BUFSIZ];

    if ((dsock = netlib_connectsock(caster, port, "tcp")) < 0) {
	    printf("error %d\n", dsock);
	    return -1;
    }
    (void)snprintf(buf, sizeof(buf),
		   "GET / HTTP/1.1\r\n"
		   "User-Agent: NTRIP gpsd/%s\r\n"
		   "Connection: close\r\n"
		   "\r\n",
		   VERSION);
    (void)write(dsock, buf, strlen(buf));
    ret = ntrip_sourcetable_parse(dsock, buf, (ssize_t)sizeof(buf), stream, keep);
    (void)close(dsock);
    return ret;
}

static int ntrip_auth_encode(const struct ntrip_stream_t *stream,
			     const char *auth, 
			     /*@out@*/char buf[], 
			     size_t size)
{
    memset(buf, 0, size);
    if (stream->authentication == auth_none)
	return 0;
    else if (stream->authentication == auth_basic) {
	char authenc[64];
	if (!auth)
	    return -1;
	memset(authenc, 0, sizeof(authenc));
	if (b64_ntop((u_char *) auth, strlen(auth), authenc, sizeof(authenc) - 1) < 0)
	    return -1;
	(void)snprintf(buf, size - 1, "Authorization: Basic %s\r\n", authenc);
    } else {
	/* todo: support digest authentication */
    }
    return 0;
}

/*@ -nullpass @*/ /* work around a splint bug */
static int ntrip_stream_open(const char *caster,
			     const char *port,
			     const char *auth,
			     struct gps_context_t *context, 
			     struct ntrip_stream_t *stream)
{
    char buf[BUFSIZ];
    char authstr[128];
    int opts;

    if (ntrip_auth_encode(stream, auth, authstr, sizeof(authstr)) < 0) {
	gpsd_report(LOG_ERROR, "User-ID and password needed for %s:%s/%s\n",
		    caster, port, stream->mountpoint);	
	return -1;
    }
    if ((context->dsock = netlib_connectsock(caster, port, "tcp")) < 0)
	return -1;

    (void)snprintf(buf, sizeof(buf),
		   "GET /%s HTTP/1.1\r\n"
		   "User-Agent: NTRIP gpsd/%s\r\n"
		   "Accept: rtk/rtcm, dgps/rtcm\r\n"
		   "%s"
		   "Connection: close\r\n"
		   "\r\n",
		   stream->mountpoint, VERSION, authstr);
    (void)write(context->dsock, buf, strlen(buf));

    memset(buf, 0, sizeof(buf));
    if (read(context->dsock, buf, sizeof(buf) - 1) < 0)
	goto close;

    /* parse 401 Unauthorized */
    if (strstr(buf, NTRIP_UNAUTH)) {
	gpsd_report(LOG_ERROR, "%s not authorized for Ntrip stream %s:%s/%s\n",
		    auth, caster, port, stream->mountpoint);	
	goto close;
    }
    /* parse SOURCETABLE */
    if (strstr(buf, NTRIP_SOURCETABLE)) {
	gpsd_report(LOG_ERROR, "Broadcaster doesn't recognize Ntrip stream %s:%s/%s\n",
		    caster, port, stream->mountpoint);	
	goto close;
    }
    /* parse ICY 200 OK */
    if (!strstr(buf, NTRIP_ICY)) {
	gpsd_report(LOG_ERROR, "Unknown reply %s from Ntrip service %s:%s/%s\n",
		    buf, caster, port, stream->mountpoint);	
	goto close;
    }
    opts = fcntl(context->dsock, F_GETFL);

    if (opts >= 0)
	(void)fcntl(context->dsock, F_SETFL, opts | O_NONBLOCK);

    context->dgnss_service = dgnss_ntrip;
#ifndef S_SPLINT_S
    context->dgnss_privdata = stream;
#endif
    return context->dsock;
close:
    (void)close(context->dsock);
    return -1;
}
/*@ +nullpass @*/

/*@ -branchstate @*/
int ntrip_open(struct gps_context_t *context, char *caster)
/* open a connection to a Ntrip broadcaster */
{
    char *amp, *colon, *slash;
    char *auth = NULL;
    char *port = NULL;
    char *stream = NULL;
    int ret;

    /*@ -boolops @*/
    if ((amp = strchr(caster, '@')) != NULL) {
	if (((colon = strchr(caster, ':')) != NULL) &&  colon < amp) { 
	    auth = caster;
	    *amp = '\0';
	    caster = amp + 1;
	} else {
	    gpsd_report(LOG_ERROR, "can't extract user-ID and password from %s\n",
			caster);
	    return -1;
	}
    }
    /*@ +boolops @*/
    if ((slash = strchr(caster, '/')) != NULL) {
	*slash = '\0';
	stream = slash + 1;
    } else {
	/* todo: add autoconnect like in dgpsip.c */
	gpsd_report(LOG_ERROR, "can't extract Ntrip stream from %s\n", caster);
	return -1;
    }
    if ((colon = strchr(caster, ':')) != NULL) {
	port = colon + 1;
	*colon = '\0';
    }
    if (!port) {
	port = "rtcm-sc104";
	if (!getservbyname(port, "tcp"))
	    port = DEFAULT_RTCM_PORT;
    }
    if (ntrip_stream_probe(caster, port, stream, &ntrip_stream)) {
	gpsd_report(LOG_ERROR, "unable to probe for data about stream %s:%s/%s\n",
		    caster, port, stream);
	return -1;
    }
    ret = ntrip_stream_open(caster, port, auth, context, &ntrip_stream);
    if (ret >= 0)
	gpsd_report(LOG_PROG,"connection to Ntrip broadcaster %s established.\n",
		    caster);
    else
	gpsd_report(LOG_ERROR, "can't connect to Ntrip stream %s:%s/%s.\n",
		    caster, port, stream);
    return ret;
}
/*@ +branchstate @*/

void ntrip_report(struct gps_device_t *session)
/* may be time to ship a usage report to the Ntrip caster */
{
    struct ntrip_stream_t *stream = session->context->dgnss_privdata;
    /*
     * 10 is an arbitrary number, the point is to have gotten several good
     * fixes before reporting usage to our Ntrip caster.
     */
    if (stream!=NULL && stream->nmea!=0
	&& session->context->fixcnt > 10 && !session->context->sentdgps) {
	session->context->sentdgps = true;
	if (session->context->dsock > -1) {
	    char buf[BUFSIZ];
	    gpsd_position_fix_dump(session, buf, sizeof(buf));
	    (void)write(session->context->dsock, buf, strlen(buf));
	    gpsd_report(LOG_IO, "=> dgps %s", buf);
	}
    }
}
#endif /* NTRIP_ENABLE */
