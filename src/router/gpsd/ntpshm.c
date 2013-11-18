/* $Id: ntpshm.c 3771 2006-11-02 05:15:20Z esr $ */
/* 
 * ntpshm.c - put time information in SHM segment for xntpd
 * struct shmTime and getShmTime from file in the xntp distribution:
 *	sht.c - Testprogram for shared memory refclock
 */

#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#include "gpsd_config.h"
#include "gpsd.h"
#ifdef NTPSHM_ENABLE

#if defined(HAVE_SYS_TIME_H)
#include <sys/time.h>
#endif

#include <sys/ipc.h>
#include <sys/shm.h>

#define PPS_MAX_OFFSET	100000		/* microseconds the PPS can 'pull' */
#define PUT_MAX_OFFSET	500000		/* microseconds for lost lock */

#define NTPD_BASE	0x4e545030	/* "NTP0" */
#define SHM_UNIT	0		/* SHM driver unit number (0..3) */

struct shmTime {
    int    mode; /* 0 - if valid set
		  *       use values, 
		  *       clear valid
		  * 1 - if valid set 
		  *       if count before and after read of values is equal,
		  *         use values 
		  *       clear valid
		  */
    int    count;
    time_t clockTimeStampSec;
    int    clockTimeStampUSec;
    time_t receiveTimeStampSec;
    long   receiveTimeStampUSec;
    int    leap;
    int    precision;
    int    nsamples;
    int    valid;
    int    pad[10];
};

static /*@null@*/ struct shmTime *getShmTime(int unit)
{
    int shmid=shmget ((key_t)(NTPD_BASE+unit), 
		      sizeof (struct shmTime), IPC_CREAT|0644);
    if (shmid == -1) {
	gpsd_report(LOG_ERROR, "shmget failed\n");
	return NULL;
    } else {
	struct shmTime *p=(struct shmTime *)shmat (shmid, 0, 0);
	/*@ -mustfreefresh */
	if ((int)(long)p == -1) {
	    gpsd_report(LOG_ERROR, "shmat failed\n");
	    return NULL;
	}
        gpsd_report(LOG_PROG, "shmat(%d,0,0) succeeded\n");
	return p;
	/*@ +mustfreefresh */
    }
}

void ntpshm_init(struct gps_context_t *context, bool enablepps)
/* attach all NTP SHM segments.  called once at startup, while still root */
{
    int i;

    for (i = 0; i < NTPSHMSEGS; i++)
	context->shmTime[i] = getShmTime(i);
    memset(context->shmTimeInuse,0,sizeof(context->shmTimeInuse));
# ifdef PPS_ENABLE
    context->shmTimePPS = enablepps;
# endif /* PPS_ENABLE */
    context->enable_ntpshm = true;
}

int ntpshm_alloc(struct gps_context_t *context)
/* allocate NTP SHM segment.  return its segment number, or -1 */
{
    int i;

    for (i = 0; i < NTPSHMSEGS; i++)
	if (context->shmTime[i] != NULL && !context->shmTimeInuse[i]) {
	    context->shmTimeInuse[i] = true;

	    memset((void *)context->shmTime[i],0,sizeof(struct shmTime));
	    context->shmTime[i]->mode = 1;
	    context->shmTime[i]->precision = -1; /* initially 0.5 sec */
	    context->shmTime[i]->nsamples = 3;	/* stages of median filter */

	    return i;
	}

    return -1;
}

bool ntpshm_free(struct gps_context_t *context, int segment)
/* free NTP SHM segment */
{
    if (segment < 0 || segment >= NTPSHMSEGS)
	return false;

    context->shmTimeInuse[segment] = false;
    return true;
}


int ntpshm_put(struct gps_device_t *session, double fixtime)
/* put a received fix time into shared memory for NTP */
{
    struct shmTime *shmTime = NULL;
    struct timeval tv;
    double seconds,microseconds;

    if (session->shmindex < 0 ||
	(shmTime = session->context->shmTime[session->shmindex]) == NULL)
	return 0;

    (void)gettimeofday(&tv,NULL);
    microseconds = 1000000.0 * modf(fixtime,&seconds);

    shmTime->count++;
    shmTime->clockTimeStampSec = (time_t)seconds;
    shmTime->clockTimeStampUSec = (int)microseconds;
    shmTime->receiveTimeStampSec = (time_t)tv.tv_sec;
    shmTime->receiveTimeStampUSec = tv.tv_usec;
    /* setting the precision here does not seem to help anything, too
       hard to calculate properly anyway.  Let ntpd figure it out.
       Any NMEA will be about -1 or -2. 
       Garmin GPS-18/USB is around -6 or -7.
    */
    shmTime->count++;
    shmTime->valid = 1;

    return 1;
}

#ifdef PPS_ENABLE
/* put NTP shared memory info based on received PPS pulse */

int ntpshm_pps(struct gps_device_t *session, struct timeval *tv)
{
    struct shmTime *shmTime = NULL, *shmTimeP = NULL;
    time_t seconds;
    double offset;
    long l_offset;

    if (session->shmindex < 0 || session->shmTimeP < 0 ||
	(shmTime = session->context->shmTime[session->shmindex]) == NULL ||
	(shmTimeP = session->context->shmTime[session->shmTimeP]) == NULL)
	return 0;

    /* check if received time messages are within locking range */

#ifdef S_SPLINT_S	/* avoids an internal error in splint 3.1.1 */
    l_offset = 0;
#else
    l_offset = shmTime->receiveTimeStampSec - shmTime->clockTimeStampSec;
#endif
    /*@ -ignorequals @*/
    l_offset *= 1000000;
    l_offset += shmTime->receiveTimeStampUSec - shmTime->clockTimeStampUSec;
    /*@ +ignorequals @*/
    if (labs( l_offset ) > PUT_MAX_OFFSET) {
        gpsd_report(LOG_RAW, "ntpshm_pps: not in locking range: %ld\n"
		, (long)l_offset);
	return -1;
    }
    /*@ -ignorequals @*/

    if (tv->tv_usec < PPS_MAX_OFFSET) {
	seconds = (time_t)tv->tv_sec;
	offset = (double)tv->tv_usec / 1000000.0;
    } else {
	if (tv->tv_usec > (1000000 - PPS_MAX_OFFSET)) {
	    seconds = (time_t)(tv->tv_sec + 1);
	    offset = 1 - ((double)tv->tv_usec / 1000000.0);
	} else {
	    shmTimeP->precision = -1;	/* lost lock */
	    gpsd_report(LOG_INF, "ntpshm_pps: lost PPS lock\n");
	    return -1;
	}
    }

    shmTimeP->count++;
    shmTimeP->clockTimeStampSec = seconds;
    shmTimeP->clockTimeStampUSec = 0;
    shmTimeP->receiveTimeStampSec = (time_t)tv->tv_sec;
    shmTimeP->receiveTimeStampUSec = tv->tv_usec;
    shmTimeP->precision = offset != 0 ? (int)(ceil(log(offset) / M_LN2)) : -20;
    shmTimeP->count++;
    shmTimeP->valid = 1;

    gpsd_report(LOG_RAW, "ntpshm_pps: clock: %lu @ %lu.%06lu, precision %d\n"
	, (unsigned long)seconds, (unsigned long)tv->tv_sec
        , (unsigned long)tv->tv_usec, shmTimeP->precision);
    return 1;
}
#endif /* PPS_ENABLE */
#endif /* NTPSHM_ENABLE */
