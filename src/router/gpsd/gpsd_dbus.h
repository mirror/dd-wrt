/* $Id: gpsd_dbus.h 3874 2006-11-13 05:17:19Z esr $ */
#include "gpsd_config.h"
#ifdef DBUS_ENABLE

#ifndef _gpsd_dbus_h_
#define _gpsd_dbus_h_

//#define DBUS_API_SUBJECT_TO_CHANGE
#include <dbus/dbus.h>
#include "gpsd.h"

int initialize_dbus_connection (void);
void send_dbus_fix (struct gps_device_t* channel);

#endif /* _gpsd_dbus_h_ */

#endif
