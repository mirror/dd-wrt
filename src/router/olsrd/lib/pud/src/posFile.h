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
 field/sentence GPGGA   GPRMC
 utc:           x       x
 sig:           x       x
 fix:                   x
 PDOP:                          =sqrt(2)*HDOP  (GPGSA)
 HDOP:          x
 VDOP:                          =HDOP          (GPGSA)
 lat:           x       x
 lon:           x       x
 elv:           x
 speed:                 x
 direction:             x
 </pre>
 */
#define POSFILE_DEFAULT_SMASK           (GPGGA | GPRMC)
#define POSFILE_SANITISE_SMASK          (GPGGA | GPRMC | GPGSA)

/* no default utc: current time is always used */
#define POSFILE_DEFAULT_SIG             (NMEA_SIG_HIGH)
#define POSFILE_DEFAULT_FIX             (NMEA_FIX_BAD)
#define POSFILE_DEFAULT_HDOP            (0.0)
#define POSFILE_DEFAULT_LAT             (0.0)
#define POSFILE_DEFAULT_LON             (0.0)
#define POSFILE_DEFAULT_ELV             (0.0)
#define POSFILE_DEFAULT_SPEED           (0.0)
#define POSFILE_DEFAULT_DIRECTION       (0.0)

#define POSFILE_CALCULATED_VDOP(hdop)   (hdop)
#define POSFILE_CALCULATED_PDOP(hdop)   (hdop * 1.414213562)

#define POSFILE_NAME_SIG                "sig"
#define POSFILE_NAME_FIX                "fix"
#define POSFILE_NAME_HDOP               "hdop"
#define POSFILE_NAME_LAT                "lat"
#define POSFILE_NAME_LON                "lon"
#define POSFILE_NAME_ELV                "elv"
#define POSFILE_NAME_SPEED              "speed"
#define POSFILE_NAME_DIRECTION          "direction"

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
