/*
 * NMEA2000 over CAN.
 *
 * The entry points for driver_nmea2000
 *
 * This file is Copyright (c) 2012 by the GPSD project
 * BSD terms apply: see the file COPYING in the distribution root for details.
 */

#ifndef _DRIVER_NMEA2000_H_
#define _DRIVER_NMEA2000_H_

#if defined(NMEA2000_ENABLE)

int nmea2000_open(struct gps_device_t *session);

void nmea2000_close(struct gps_device_t *session);

#endif /* of defined(NMEA2000_ENABLE) */

#endif /* of ifndef _DRIVER_NMEA2000_H_ */
