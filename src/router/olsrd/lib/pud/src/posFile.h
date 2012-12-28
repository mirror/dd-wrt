#ifndef _PUD_POSFILE_H_
#define _PUD_POSFILE_H_

/* Plugin includes */

/* OLSR includes */

/* System includes */
#include <stdbool.h>
#include <nmea/info.h>
#include <nmea/sentence.h>

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
#define POSFILE_DEFAULT_SMASK           (GPGGA | GPGSA | GPRMC | GPVTG)

/* no default utc: current time is always used */
#define POSFILE_DEFAULT_SIG             (NMEA_SIG_HIGH)
#define POSFILE_DEFAULT_FIX             (NMEA_FIX_BAD)
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
bool readPositionFile(char * fileName, nmeaINFO * nmeaInfo);

#endif /* _PUD_POSFILE_H_ */
