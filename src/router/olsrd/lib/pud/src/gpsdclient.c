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

#include "gpsdclient.h"

#include "configuration.h"

#include <nmealib/nmath.h>
#include <nmealib/sentence.h>
#include <errno.h>
#include <gps.h>
#include <math.h>
#include <string.h>
#include <syslog.h>

#if GPSD_API_MAJOR_VERSION < 6
  #if GPSD_API_MINOR_VERSION < 1
    #define GPSD_WHEEZY
  #else
    #define GPSD_JESSIE
  #endif
#else
  #define GPSD_NEW
#endif

/*
 * Loggers
 */

/* log gpsd log messages*/
static void gpsdLog(const char *s) {
  syslog(LOG_INFO, "gpsd log: %s", s);
}

/* log gpsd errors */
static void gpsdError(const char *s) {
  syslog(LOG_ERR, "gpsd error: %s", s);
}

/* standard parsing of a GPS data source spec */
void gpsdParseSourceSpec(char *arg, GpsDaemon *gpsDaemon) {
  if (!arg //
      || !gpsDaemon) {
    return;
  }

  gpsDaemon->source.server = (char *) (unsigned long) DEFAULT_GPSD_HOST;
  gpsDaemon->source.port = (char *) (unsigned long) DEFAULT_GPSD_PORT;
  gpsDaemon->source.device = NULL;

  if (*arg) {
    char *colon1;
    const char *skipto;
    char *rbrk;

    strncpy(gpsDaemon->source.spec, arg, PATH_MAX);
    gpsDaemon->source.spec[PATH_MAX - 1] = '\0';

    skipto = gpsDaemon->source.spec;
    if ((*skipto == '[') //
        && ((rbrk = strchr(skipto, ']')) != NULL )) {
      skipto = rbrk;
    }
    colon1 = strchr(skipto, ':');

    if (colon1 != NULL) {
      char *colon2;

      *colon1 = '\0';
      if (colon1 != gpsDaemon->source.spec) {
        gpsDaemon->source.server = gpsDaemon->source.spec;
      }

      gpsDaemon->source.port = colon1 + 1;
      colon2 = strchr(gpsDaemon->source.port, ':');
      if (colon2 != NULL) {
        *colon2 = '\0';
        gpsDaemon->source.device = colon2 + 1;
      }
    } else if (strchr(gpsDaemon->source.spec, '/') != NULL) {
      gpsDaemon->source.device = gpsDaemon->source.spec;
    } else {
      gpsDaemon->source.server = gpsDaemon->source.spec;
    }
  }

  if (*gpsDaemon->source.server == '[') {
    char *rbrk = strchr(gpsDaemon->source.server, ']');
    ++gpsDaemon->source.server;
    if (rbrk != NULL) {
      *rbrk = '\0';
    }
  }
}

bool gpsdConnect(GpsDaemon *gpsd, struct gps_data_t *gpsData, struct GpsdConnectionState *connectionTracking) {
  bool disconnectFirst;

  if (!gpsd //
      || !gpsData //
      || !connectionTracking) {
    return false;
  }

  disconnectFirst = connectionTracking->connected;

  if (disconnectFirst) {
    gpsdDisconnect(gpsData, connectionTracking);
  }

  gpsData->gps_fd = -1;
  if (gps_open(gpsd->source.server, gpsd->source.port, gpsData)) {
    if (!connectionTracking->connectionFailureReported) {
      syslog(LOG_WARNING, "%sonnecting to gpsd on %s:%s failed, starting retries", disconnectFirst ? "Rec" : "C", gpsd->source.server, gpsd->source.port);
      connectionTracking->connectionFailureReported = true;
    }
    connectionTracking->retryCount++;
    return false;
  }

  if (connectionTracking->connectionFailureReported) {
    syslog(LOG_WARNING, "%sonnecting to gpsd on %s:%s successful after %llu retr%s", disconnectFirst ? "Rec" : "C", gpsd->source.server, gpsd->source.port,
        connectionTracking->retryCount, connectionTracking->retryCount == 1 ? "y" : "ies");
    connectionTracking->connectionFailureReported = false;
  }
  connectionTracking->retryCount = 0;

  if (gpsd->source.device) {
    gpsd->gpsdStreamFlags |= WATCH_DEVICE;
  }

  /* instruct gpsd which data to receive */
  gps_stream(gpsData, gpsd->gpsdStreamFlags, gpsd->source.device);

  return true;
}

void gpsdDisconnect(struct gps_data_t *gpsdata, struct GpsdConnectionState *connectionTracking) {
  if (!gpsdata //
      || !connectionTracking) {
    return;
  }

  if (gpsdata->gps_fd >= 0) {
    gps_close(gpsdata);
    gpsdata->gps_fd = -1;
    syslog(LOG_INFO, "Closed gpsd connection");
  }

  memset(connectionTracking, 0, sizeof(*connectionTracking));
}

/**
 * Convert true heading to magnetic.  Taken from the Aviation
 * Formulary v1.43.  Valid to within two degrees within the
 * continiental USA except for the following airports: MO49 MO86 MO50
 * 3K6 02K and KOOA.  AK correct to better than one degree.  Western
 * Europe correct to within 0.2 deg.
 *
 * If you're not in one of these areas, I apologize, I don't have the
 * math to compute your varation.  This is obviously extremely
 * floating-point heavy, so embedded people, beware of using.
 *
 * Note that there are issues with using magnetic heading.  This code
 * does not account for the possibility of travelling into or out of
 * an area of valid calculation beyond forcing the magnetic conversion
 * off.  A better way to communicate this to the user is probably
 * desirable (in case the don't notice the subtle change from "(mag)"
 * to "(true)" on their display).
 */
float true2magnetic(double lat, double lon, double heading) {
  /* Western Europe */
  if ((lat > 36.0) && (lat < 68.0) && (lon > -10.0) && (lon < 28.0)) {
    heading = (10.4768771667158 - (0.507385322418858 * lon) + (0.00753170031703826 * pow(lon, 2)) - (1.40596203924748e-05 * pow(lon, 3))
        - (0.535560699962353 * lat) + (0.0154348808069955 * lat * lon) - (8.07756425110592e-05 * lat * pow(lon, 2)) + (0.00976887198864442 * pow(lat, 2))
        - (0.000259163929798334 * lon * pow(lat, 2)) - (3.69056939266123e-05 * pow(lat, 3)) + heading);
  }
  /* USA */
  else if ((lat > 24.0) && (lat < 50.0) && (lon > 66.0) && (lon < 125.0)) {
    lon = 0.0 - lon;
    heading = ((-65.6811) + (0.99 * lat) + (0.0128899 * pow(lat, 2)) - (0.0000905928 * pow(lat, 3)) + (2.87622 * lon) - (0.0116268 * lat * lon)
        - (0.00000603925 * lon * pow(lat, 2)) - (0.0389806 * pow(lon, 2)) - (0.0000403488 * lat * pow(lon, 2)) + (0.000168556 * pow(lon, 3)) + heading);
  }
  /* AK */
  else if ((lat > 54.0) && (lon > 130.0) && (lon < 172.0)) {
    lon = 0.0 - lon;
    heading = (618.854 + (2.76049 * lat) - (0.556206 * pow(lat, 2)) + (0.00251582 * pow(lat, 3)) - (12.7974 * lon) + (0.408161 * lat * lon)
        + (0.000434097 * lon * pow(lat, 2)) - (0.00602173 * pow(lon, 2)) - (0.00144712 * lat * pow(lon, 2)) + (0.000222521 * pow(lon, 3)) + heading);
  } else {
    /* We don't know how to compute magnetic heading for this
     * location. */
    heading = NAN;
  }

  /* No negative headings. */
  if (isnan(heading) == 0 && heading < 0.0) {
    heading += 360.0;
  }

  return (float) (heading);
}

void nmeaInfoFromGpsd(struct gps_data_t *gpsdata, NmeaInfo *info, struct GpsdConnectionState *connectionTracking) {
  if (!gpsdata //
      || !info //
      || !connectionTracking) {
    return;
  }

  if (gpsdata->set & VERSION_SET) {
    if (!connectionTracking->version) {
      size_t releaseLength = strlen(gpsdata->version.release);
      size_t revLength = strlen(gpsdata->version.rev);
      size_t remoteLength = strlen(gpsdata->version.remote);
      syslog(LOG_INFO, "Connected to gpsd%s%s%s%s%s%s on %s, protocol %d.%d", //
          !releaseLength ? "" : " ", //
          !releaseLength ? "" : gpsdata->version.release, //
          !releaseLength ? "" : " ", //
          !revLength ? "" : "(", //
          !revLength ? "" : gpsdata->version.rev, //
          !revLength ? "" : ")", //
          !remoteLength ? "localhost" : gpsdata->version.remote, //
          gpsdata->version.proto_major, //
          gpsdata->version.proto_minor);

      connectionTracking->version = true;
    }

    gpsdata->set &= ~VERSION_SET;
  }

  if (gpsdata->set & LOGMESSAGE_SET) {
    gpsdLog(gpsdata->error);
    gpsdata->set &= ~LOGMESSAGE_SET;
  }

  if (gpsdata->set & ERROR_SET) {
    gpsdError(gpsdata->error);
    gpsdata->set &= ~ERROR_SET;
  }

  if (gpsdata->set & (DEVICELIST_SET | DEVICE_SET | DEVICEID_SET)) {
    int i = 0;
    while (i < MAXUSERDEVS) {
      struct devconfig_t *dev = &gpsdata->devices.list[i];
      if (dev->flags //
          && !connectionTracking->devSeen[i]) {
        size_t subtypeLength = strlen(dev->subtype);
        syslog(LOG_INFO, "Using %s device%s%s on %s in %s mode at %u baud (%u%c%u), refresh time %.3f (min. %.3f)", //
            dev->driver, //
            !subtypeLength ? "" : " ", //
            !subtypeLength ? "" : dev->subtype, //
            dev->path, //
            dev->driver_mode ? "native" : "compatibility", //
            dev->baudrate, //
            8, //
            dev->parity, //
            dev->stopbits, //
            dev->cycle, //
            dev->mincycle);

        connectionTracking->devSeen[i] = true;
        connectionTracking->dev[i] = *dev;
      } else if (connectionTracking->devSeen[i]) {
        size_t subtypeLength;

        dev = &connectionTracking->dev[i];
        subtypeLength = strlen(dev->subtype);
        syslog(LOG_INFO, "No longer using %s device%s%s on %s", //
            dev->driver, //
            !subtypeLength ? "" : " ", //
            !subtypeLength ? "" : dev->subtype, //
            dev->path);

        connectionTracking->devSeen[i] = false;
        memset(&connectionTracking->dev[i], 0, sizeof(connectionTracking->dev[i]));
      }

      i++;
    }

    gpsdata->set &= ~(DEVICELIST_SET | DEVICE_SET | DEVICEID_SET);
  }

  /* ignored */
  gpsdata->set &= ~( //
          ONLINE_SET // unreliable
          | TIMERR_SET //
          | CLIMB_SET //
          | DOP_SET // using dop from fix
          | ATTITUDE_SET //
          | SPEEDERR_SET //
          | TRACKERR_SET //
          | CLIMBERR_SET //
          | RTCM2_SET //
          | RTCM3_SET //
          | AIS_SET //
          | PACKET_SET //
          | SUBFRAME_SET //
          | GST_SET //
          | POLICY_SET //
#ifdef GPSD_JESSIE
          | TIMEDRIFT_SET //
          | EOF_SET //
#endif
#ifdef GPSD_NEW
          | TOFF_SET //
          | PPS_SET //
          | NAVDATA_SET //
#endif
          );

  gpsdata->set &= ~STATUS_SET; /* always valid */
  if (gpsdata->status == STATUS_NO_FIX) {
    nmeaInfoClear(info);
    nmeaTimeSet(&info->utc, &info->present, NULL);
    return;
  }

  if (!gpsdata->set) {
    return;
  }

  info->smask = NMEALIB_SENTENCE_MASK;
  nmeaInfoSetPresent(&info->present, NMEALIB_PRESENT_SMASK);

  /* date & time */
  if (!isNaN(gpsdata->fix.time)) {
    double seconds;
    double fraction = modf(fabs(gpsdata->fix.time), &seconds);
    long sec = lrint(seconds);
    struct tm *time = gmtime(&sec);
    if (time) {
      info->utc.year = (unsigned int) time->tm_year + 1900;
      info->utc.mon = (unsigned int) time->tm_mon + 1;
      info->utc.day = (unsigned int) time->tm_mday;
      info->utc.hour = (unsigned int) time->tm_hour;
      info->utc.min = (unsigned int) time->tm_min;
      info->utc.sec = (unsigned int) time->tm_sec;
      info->utc.hsec = (unsigned int) lrint(fraction * 100);

      nmeaInfoSetPresent(&info->present, NMEALIB_PRESENT_UTCDATE | NMEALIB_PRESENT_UTCTIME);
    }
  }
  gpsdata->set &= ~TIME_SET;

  /* sig & fix */
  if (!gpsdata->online) {
    gpsdata->fix.mode = MODE_NO_FIX;
  }

  switch (gpsdata->fix.mode) {
    case MODE_3D:
      info->fix = NMEALIB_FIX_3D;
      info->sig = NMEALIB_SIG_FIX;
      break;

    case MODE_2D:
      info->fix = NMEALIB_FIX_2D;
      info->sig = NMEALIB_SIG_FIX;
      break;

    case MODE_NOT_SEEN:
    case MODE_NO_FIX:
    default:
      info->fix = NMEALIB_FIX_BAD;
      info->sig = NMEALIB_SIG_INVALID;
      break;
  }
  nmeaInfoSetPresent(&info->present, NMEALIB_PRESENT_FIX | NMEALIB_PRESENT_SIG);
  gpsdata->set &= ~MODE_SET;

  /* hdop */
  if (!isNaN(gpsdata->fix.epx) //
      && !isNaN(gpsdata->fix.epy)) {
    info->hdop = nmeaMathPdopCalculate(gpsdata->fix.epx, gpsdata->fix.epy);
    nmeaInfoSetPresent(&info->present, NMEALIB_PRESENT_HDOP);
  }
  gpsdata->set &= ~HERR_SET;

  /* vdop */
  if (!isNaN(gpsdata->fix.epv)) {
    info->vdop = gpsdata->fix.epv;
    nmeaInfoSetPresent(&info->present, NMEALIB_PRESENT_VDOP);
  }
  gpsdata->set &= ~VERR_SET;

  /* pdop */
  if (nmeaInfoIsPresentAll(info->present, NMEALIB_PRESENT_HDOP | NMEALIB_PRESENT_VDOP)) {
    info->pdop = nmeaMathPdopCalculate(info->hdop, info->vdop);
    nmeaInfoSetPresent(&info->present, NMEALIB_PRESENT_PDOP);
  }

  /* lat */
  if ((gpsdata->fix.mode >= MODE_2D) //
      && !isNaN(gpsdata->fix.latitude)) {
    info->latitude = nmeaMathDegreeToNdeg(gpsdata->fix.latitude);
    nmeaInfoSetPresent(&info->present, NMEALIB_PRESENT_LAT);
  }

  /* lon */
  if ((gpsdata->fix.mode >= MODE_2D) //
      && !isNaN(gpsdata->fix.longitude)) {
    info->longitude = nmeaMathDegreeToNdeg(gpsdata->fix.longitude);
    nmeaInfoSetPresent(&info->present, NMEALIB_PRESENT_LON);
  }

  /* lat & lon */
  gpsdata->set &= ~LATLON_SET;

  /* elv */
  if ((gpsdata->fix.mode >= MODE_3D) //
      && !isNaN(gpsdata->fix.altitude)) {
    info->elevation = gpsdata->fix.altitude;
    info->height = gpsdata->separation;
    nmeaInfoSetPresent(&info->present, NMEALIB_PRESENT_ELV | NMEALIB_PRESENT_HEIGHT);
  }
  gpsdata->set &= ~ALTITUDE_SET;

  /* speed */
  if ((gpsdata->fix.mode >= MODE_2D) //
      && !isNaN(gpsdata->fix.speed)) {
    info->speed = gpsdata->fix.speed * MPS_TO_KPH;
    nmeaInfoSetPresent(&info->present, NMEALIB_PRESENT_SPEED);
  }
  gpsdata->set &= ~SPEED_SET;

  /* track & mtrack */
  if ((gpsdata->fix.mode >= MODE_2D) //
      && !isNaN(gpsdata->fix.track)) {
    double magheading;

    info->track = gpsdata->fix.track;
    nmeaInfoSetPresent(&info->present, NMEALIB_PRESENT_TRACK);

    magheading = true2magnetic(gpsdata->fix.latitude, gpsdata->fix.longitude, gpsdata->fix.track);
    if (!isNaN(magheading)) {
      info->mtrack = magheading;
      nmeaInfoSetPresent(&info->present, NMEALIB_PRESENT_MTRACK);
    }
  }
  gpsdata->set &= ~TRACK_SET;

  /* magvar: not available */

  /* satellites */

  info->satellites.inUseCount = 0;
  memset(&info->satellites.inUse, 0, sizeof(info->satellites.inUse));
  info->satellites.inViewCount = 0;
  memset(&info->satellites.inView, 0, sizeof(info->satellites.inView));

  if (gpsdata->satellites_visible > 0) {
    int iGpsd;

#ifndef GPSD_NEW
    bool usedFlags[MAXCHANNELS];

    memset(usedFlags, 0, sizeof(usedFlags));

    /* build a bitmap of which satellites are used */
    for (iGpsd = 0; iGpsd < MAXCHANNELS; iGpsd++) {
      size_t iGpsdUsed;
      for (iGpsdUsed = 0; iGpsdUsed < (size_t) gpsdata->satellites_used; iGpsdUsed++) {
        if (gpsdata->used[iGpsdUsed] == gpsdata->PRN[iGpsd]) {
          usedFlags[iGpsd] = true;
          iGpsdUsed = (size_t) gpsdata->satellites_used;
        }
      }
    }
#endif

    for (iGpsd = 0; //
        (iGpsd < gpsdata->satellites_visible) && (iGpsd < MAXCHANNELS) && (iGpsd < (int) NMEALIB_MAX_SATELLITES); //
        iGpsd++) {
      NmeaSatellite *infoSatellite = &info->satellites.inView[iGpsd];
      unsigned int prn;
      int elevation;
      unsigned int azimuth;
      unsigned int snr;
      bool inUse;

#ifndef GPSD_NEW
      prn = (unsigned int) gpsdata->PRN[iGpsd];
      elevation = gpsdata->elevation[iGpsd];
      azimuth = (unsigned int) gpsdata->azimuth[iGpsd];
      snr = (unsigned int) lrint(gpsdata->ss[iGpsd]);
      inUse = usedFlags[iGpsd];
#else
      struct satellite_t *gpsdSatellite = &gpsdata->skyview[iGpsd];

      prn = (unsigned int) gpsdSatellite->PRN;
      elevation = gpsdSatellite->elevation;
      azimuth = (unsigned int) gpsdSatellite->azimuth;
      snr = (unsigned int) lrint(gpsdSatellite->ss);
      inUse = gpsdSatellite->used;
#endif

      infoSatellite->prn = prn;
      infoSatellite->elevation = elevation;
      infoSatellite->azimuth = azimuth;
      infoSatellite->snr = snr;
      info->satellites.inViewCount++;

      if (inUse) {
        info->satellites.inUse[iGpsd] = prn;
        info->satellites.inUseCount++;
      }
    }
  }
  nmeaInfoSetPresent(&info->present, //
      NMEALIB_PRESENT_SATINUSECOUNT //
      | NMEALIB_PRESENT_SATINUSE //
      | NMEALIB_PRESENT_SATINVIEWCOUNT //
      | NMEALIB_PRESENT_SATINVIEW);
  gpsdata->set &= ~SATELLITE_SET;

  nmeaInfoSanitise(info);

  if (gpsdata->set) {
    syslog(LOG_WARNING, "Unhandled bits in gpsdata->set: %llx", (long long unsigned int) gpsdata->set);
  }
}

void readFromGpsd(GpsDaemon *gpsd, struct gps_data_t *gpsdata, struct GpsdConnectionState *connectionTracking, NmeaInfo *nmeaInfo) {
  int gpsReadCode;

  if (!gpsd //
      || !gpsdata //
      || !connectionTracking //
      || !nmeaInfo) {
    return;
  }

  errno = 0;
  if (!connectionTracking->connected) {
    gpsReadCode = -1;
  } else {
#if ((GPSD_API_MAJOR_VERSION >= 7) && (GPSD_API_MINOR_VERSION >= 0))
    char msg[] = "\0";
    int msg_len = 1;
    gpsReadCode = gps_read(gpsdata, msg, msg_len);
#else
    gpsReadCode = gps_read(gpsdata);
#endif
  }

  if (gpsReadCode > 0) {
    /* data received from gpsd */
    nmeaInfoFromGpsd(gpsdata, nmeaInfo, connectionTracking);
  } else if (gpsReadCode < 0) {
    /* failed to receive data from gpsd */

    if (connectionTracking->connected) {
      if (errno) {
        syslog(LOG_WARNING, "Disconnected from gpsd: %s", strerror(errno));
      } else {
        syslog(LOG_WARNING, "Disconnected from gpsd");
      }

      gpsdDisconnect(gpsdata, connectionTracking);
      nmeaInfoClear(nmeaInfo);
    }

    connectionTracking->connected = gpsdConnect(gpsd, gpsdata, connectionTracking);
    nmeaTimeSet(&nmeaInfo->utc, &nmeaInfo->present, NULL);
  } else {
    /* no data received from gpsd */
  }
}
