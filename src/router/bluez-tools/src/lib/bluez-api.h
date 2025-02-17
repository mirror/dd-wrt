#ifndef BLUEZ_API_H
#define	BLUEZ_API_H

#ifdef	__cplusplus
extern "C" {
#endif

#define BLUEZ_DBUS_SERVICE_NAME "org.bluez"
#define BLUEZ_OBEX_DBUS_SERVICE_NAME "org.bluez.obex"
#define BLUEZ_DBUS_BASE_PATH "/org/bluez"
#define BLUEZ_OBEX_DBUS_BASE_PATH "/org/bluez/obex"

#include "manager.h"
#include "obex_agent.h"

#include "bluez/adapter.h"
#include "bluez/agent_manager.h"
#include "bluez/alert.h"
#include "bluez/alert_agent.h"
#include "bluez/cycling_speed.h"
#include "bluez/cycling_speed_manager.h"
#include "bluez/device.h"
#include "bluez/health_channel.h"
#include "bluez/health_device.h"
#include "bluez/health_manager.h"
#include "bluez/heart_rate.h"
#include "bluez/heart_rate_manager.h"
#include "bluez/media.h"
#include "bluez/media_control.h"
#include "bluez/media_player.h"
#include "bluez/network.h"
#include "bluez/network_server.h"
#include "bluez/obex/obex_agent_manager.h"
#include "bluez/obex/obex_client.h"
#include "bluez/obex/obex_file_transfer.h"
#include "bluez/obex/obex_message.h"
#include "bluez/obex/obex_message_access.h"
#include "bluez/obex/obex_object_push.h"
#include "bluez/obex/obex_phonebook_access.h"
#include "bluez/obex/obex_session.h"
#include "bluez/obex/obex_synchronization.h"
#include "bluez/obex/obex_transfer.h"
#include "bluez/profile_manager.h"
#include "bluez/proximity_monitor.h"
#include "bluez/proximity_reporter.h"
#include "bluez/sim_access.h"
#include "bluez/thermometer.h"
#include "bluez/thermometer_manager.h"
    
#ifdef	__cplusplus
}
#endif

#endif	/* BLUEZ_H */

