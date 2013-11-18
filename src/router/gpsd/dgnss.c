/* $Id: dgnss.c 3771 2006-11-02 05:15:20Z esr $ */
/* dgnss.c -- common interface to a number of Differential GNSS services */

#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include "gpsd_config.h"
#include "gpsd.h"

#define DGNSS_PROTO_DGPSIP	"dgpsip://"
#define DGNSS_PROTO_NTRIP	"ntrip://"

/* Where to find the list of DGPSIP correction servers, if there is one */
#define DGPSIP_SERVER_LIST	"/usr/share/gpsd/dgpsip-servers"

bool dgnss_url(char *name)
/* is given string a valid URL for DGPS service? */
{
    return 
	strncmp(name,DGNSS_PROTO_NTRIP,strlen(DGNSS_PROTO_NTRIP))==0
	|| strncmp(name,DGNSS_PROTO_DGPSIP,strlen(DGNSS_PROTO_DGPSIP))==0;
}


/*@ -branchstate */
int dgnss_open(struct gps_context_t *context, char *dgnss_service)
/* open a connection to a DGNSS service */
{
#ifdef NTRIP_ENABLE
    if (strncmp(dgnss_service,DGNSS_PROTO_NTRIP,strlen(DGNSS_PROTO_NTRIP))==0)
        return ntrip_open(context, dgnss_service + strlen(DGNSS_PROTO_NTRIP));
#endif

    if (strncmp(dgnss_service,DGNSS_PROTO_DGPSIP,strlen(DGNSS_PROTO_DGPSIP))==0)
        return dgpsip_open(context, dgnss_service + strlen(DGNSS_PROTO_DGPSIP));

#ifndef REQUIRE_DGNSS_PROTO
    return dgpsip_open(context, dgnss_service);
#else
    gpsd_report(LOG_ERROR, "Unknown or unspecified DGNSS protocol for service %s\n",
		dgnss_service);
    return -1;
#endif
}
/*@ +branchstate */

int dgnss_poll(struct gps_context_t *context)
/* poll the DGNSS service for a correction report */
{
    if (context->dsock > -1) {
	context->rtcmbytes = read(context->dsock, context->rtcmbuf, sizeof(context->rtcmbuf));
	if ((context->rtcmbytes == -1 && errno != EAGAIN) ||
	    (context->rtcmbytes == 0)) {
	    (void)shutdown(context->dsock, SHUT_RDWR);
	    (void)close(context->dsock);
	    return -1;
	} else
	    context->rtcmtime = timestamp();
    }
    return 0;
}

void dgnss_report(struct gps_device_t *session)
/* may be time to ship a usage report to the DGNSS service */
{
    if (session->context->dgnss_service == dgnss_dgpsip)
	dgpsip_report(session);
#ifdef NTRIP_ENABLE
    else if (session->context->dgnss_service == dgnss_ntrip)
	ntrip_report(session);
#endif
}

void dgnss_autoconnect(struct gps_context_t *context, double lat, double lon)
{
    if (context->dgnss_service != dgnss_ntrip) {
	dgpsip_autoconnect(context, lat, lon, DGPSIP_SERVER_LIST);
    }
}

void rtcm_relay(struct gps_device_t *session)
/* pass a DGNSS connection report to a session */
{
    if (session->gpsdata.gps_fd !=-1 
	&& session->context->rtcmbytes > -1
	&& session->rtcmtime < session->context->rtcmtime
	&& session->device_type->rtcm_writer != NULL) {
	if (session->device_type->rtcm_writer(session, 
					      session->context->rtcmbuf, 
					      (size_t)session->context->rtcmbytes) == 0)
	    gpsd_report(LOG_ERROR, "Write to rtcm sink failed\n");
	else { 
	    session->rtcmtime = timestamp();
	    gpsd_report(LOG_IO, "<= DGPS: %d bytes of RTCM relayed.\n", session->context->rtcmbytes);
	}
    }
}

