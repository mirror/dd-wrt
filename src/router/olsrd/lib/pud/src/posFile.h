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

#ifndef _PUD_POSFILE_H_
#define _PUD_POSFILE_H_

/* Plugin includes */

/* OLSR includes */

/* System includes */
#include <stdbool.h>
#include <nmealib/info.h>
#include <nmealib/sentence.h>

/**
 <pre>
 field/sentence       GPGGA   GPGSA   GPGSV   GPRMC   GPVTG
 present:               x       x       x       x       x
 smask:                 x       x       x       x       x
 utc (date):                                    x
 utc (time):            x                       x
 sig:                   x                       x1
 fix:                           x               x1
 PDOP:                          x                           =sqrt(2)*HDOP  (GPGSA)
 HDOP:                  x       x
 VDOP:                          x                           =HDOP          (GPGSA)
 lat:                   x                       x
 lon:                   x                       x
 elv:                   x
 speed:                                         x       x
 track:                                         x       x
 mtrack:                                                x
 magvar:                                        x
 satinfo (inuse count): x       x1
 satinfo (inuse):               x
 satinfo (inview):                      x

 x1 = not present in the sentence but the library sets it up.
 </pre>
 */
#define POSFILE_DEFAULT_SMASK           (NMEALIB_SENTENCE_GPGGA | NMEALIB_SENTENCE_GPGSA | NMEALIB_SENTENCE_GPRMC | NMEALIB_SENTENCE_GPVTG)

/* no default utc: current time is always used */
#define POSFILE_DEFAULT_SIG             (NMEALIB_SIG_SENSITIVE)
#define POSFILE_DEFAULT_FIX             (NMEALIB_FIX_BAD)
#define POSFILE_DEFAULT_HDOP            (0.0)
#define POSFILE_DEFAULT_LAT             (0.0)
#define POSFILE_DEFAULT_LON             (0.0)
#define POSFILE_DEFAULT_ELV             (0.0)
#define POSFILE_DEFAULT_SPEED           (0.0)
#define POSFILE_DEFAULT_TRACK           (0.0)
#define POSFILE_DEFAULT_MTRACK          (0.0)
#define POSFILE_DEFAULT_MAGVAR          (0.0)

#define POSFILE_CALCULATED_VDOP(hdop)   (hdop)
#define POSFILE_CALCULATED_PDOP(hdop)   (hdop * 1.414213562)

#define POSFILE_NAME_SIG                "sig"
#define POSFILE_NAME_FIX                "fix"
#define POSFILE_NAME_HDOP               "hdop"
#define POSFILE_NAME_LAT                "lat"
#define POSFILE_NAME_LON                "lon"
#define POSFILE_NAME_ELV                "elv"
#define POSFILE_NAME_SPEED              "speed"
#define POSFILE_NAME_TRACK              "track"
#define POSFILE_NAME_MTRACK             "mtrack"
#define POSFILE_NAME_MAGVAR             "magvar"

#define POSFILE_VALUE_SIG_BAD           "bad"
#define POSFILE_VALUE_SIG_LOW           "low"
#define POSFILE_VALUE_SIG_MID           "mid"
#define POSFILE_VALUE_SIG_HIGH          "high"

#define POSFILE_VALUE_FIX_BAD           "bad"
#define POSFILE_VALUE_FIX_2D            "2d"
#define POSFILE_VALUE_FIX_3D            "3d"

bool startPositionFile(void);
void stopPositionFile(void);
bool readPositionFile(char * fileName, NmeaInfo * nmeaInfo);

#endif /* _PUD_POSFILE_H_ */
