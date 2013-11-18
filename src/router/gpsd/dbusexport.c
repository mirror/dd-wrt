/*
 * This file is Copyright (c) 2010 by the GPSD project
 * BSD terms apply: see the file COPYING in the distribution root for details.
 */
#include "gpsd.h"
#if defined(DBUS_EXPORT_ENABLE) && !defined(S_SPLINT_S)
#include <dbus/dbus.h>

static DBusConnection *connection = NULL;

/*
 * Does what is required to initialize the dbus connection
 * This is pretty basic at this point, as we don't receive commands via dbus.
 * Returns 0 when everything is OK.
 */
int initialize_dbus_connection(void)
{
    DBusError error;

    dbus_error_init(&error);
    connection = dbus_bus_get(DBUS_BUS_SYSTEM, &error);
    if (connection == NULL) {
	/* report error */
	return 1;
    }
    return 0;
}

void send_dbus_fix(struct gps_device_t *channel)
{
/* sends the current fix data for this channel via dbus */
    struct gps_data_t *gpsdata;
    struct gps_fix_t *gpsfix;
    DBusMessage *message;
    /*DBusMessageIter   iter; */
    dbus_uint32_t serial;	/* collected, but not used */
    char *gpsd_devname;
    /* this packet format was designed before we split eph */
    double eph;

    /* if the connection is non existent, return without doing anything */
    if (connection == NULL)
	return;

    gpsdata = &(channel->gpsdata);
    gpsfix = &(gpsdata->fix);
    /* this packet format was designed before we split eph */
    eph = EMIX(gpsfix->epx, gpsfix->epy);
    gpsd_devname = gpsdata->dev.path;

    /* Send the named signal.  */
    message = dbus_message_new_signal("/org/gpsd", "org.gpsd", "fix");
    dbus_message_append_args(message,
			     DBUS_TYPE_DOUBLE, &(gpsfix->time),
			     DBUS_TYPE_INT32, &(gpsfix->mode),
			     DBUS_TYPE_DOUBLE, &(gpsfix->ept),
			     DBUS_TYPE_DOUBLE, &(gpsfix->latitude),
			     DBUS_TYPE_DOUBLE, &(gpsfix->longitude),
			     DBUS_TYPE_DOUBLE, &(eph),
			     DBUS_TYPE_DOUBLE, &(gpsfix->altitude),
			     DBUS_TYPE_DOUBLE, &(gpsfix->epv),
			     DBUS_TYPE_DOUBLE, &(gpsfix->track),
			     DBUS_TYPE_DOUBLE, &(gpsfix->epd),
			     DBUS_TYPE_DOUBLE, &(gpsfix->speed),
			     DBUS_TYPE_DOUBLE, &(gpsfix->eps),
			     DBUS_TYPE_DOUBLE, &(gpsfix->climb),
			     DBUS_TYPE_DOUBLE, &(gpsfix->epc),
			     DBUS_TYPE_STRING, &gpsd_devname,
			     DBUS_TYPE_INVALID);
    dbus_message_set_no_reply(message, TRUE);
    dbus_connection_send(connection, message, &serial);
    dbus_message_unref(message);
}

#endif /* defined(DBUS_EXPORT_ENABLE) && !defined(S_SPLINT_S) */
