/*
 * The olsr.org Optimized Link-State Routing daemon (olsrd)
 *
 * (c) by the OLSR project
 *
 * See our Git repository to find out who worked on this file
 * and thus is a copyright holder on it.
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * * Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 * * Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in
 *   the documentation and/or other materials provided with the
 *   distribution.
 * * Neither the name of olsr.org, olsrd nor the names of its
 *   contributors may be used to endorse or promote products derived
 *   from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * Visit http://www.olsr.org for more information.
 *
 * If you find this software useful feel free to make a donation
 * to the project. For more information see the website or contact
 * the copyright holders.
 *
 */

#ifndef _PUD_GPSD_GPSDCLIENT_H_
#define _PUD_GPSD_GPSDCLIENT_H_

#include <gps.h>
#include <stdbool.h>
#include <nmealib/info.h>

struct GpsdConnectionState {
    bool connected;
    bool connectionFailureReported;
    unsigned long long retryCount;

    bool version;
    bool devSeen[MAXUSERDEVS];
    struct devconfig_t dev[MAXUSERDEVS];
};

/* describe a data source */
struct fixsource_t {
    char spec[PATH_MAX]; /* working space, will be modified */
    char *server; /* pointer into spec field */
    char *port; /* pointer into spec field */
    char *device; /* pointer into spec field */
};

/**
 * The gpsd daemon spec
 */
typedef struct _GpsDaemon {
    struct fixsource_t source;
    unsigned int gpsdStreamFlags; /**< the stream flags for the gpsd connection */
} GpsDaemon;

/** The default gpsd host */
#define DEFAULT_GPSD_HOST "localhost"

/** The default gpsd port */
#define DEFAULT_GPSD_PORT "2947"

/** The default gpsd source spec */
#define PUD_GPSD_DEFAULT (DEFAULT_GPSD_HOST ":" DEFAULT_GPSD_PORT)

void gpsdParseSourceSpec(char *fromstring, GpsDaemon *source);

bool gpsdConnect(GpsDaemon *gpsd, struct gps_data_t *gpsData, struct GpsdConnectionState *connectionTracking);
void gpsdDisconnect(struct gps_data_t *gpsdata, struct GpsdConnectionState *connectionTracking);

float true2magnetic(double lat, double lon, double heading);

void nmeaInfoFromGpsd(struct gps_data_t *gpsdata, NmeaInfo *info, struct GpsdConnectionState *connectionTracking);

void readFromGpsd(GpsDaemon *configuration, struct gps_data_t *gpsdata, struct GpsdConnectionState *connectionTracking, NmeaInfo *nmeaInfo);

#endif /* _PUD_GPSDCLIENT_H_ */
