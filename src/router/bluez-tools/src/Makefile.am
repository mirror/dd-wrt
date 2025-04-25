AM_CPPFLAGS = $(GLIB_CFLAGS) $(GIO_CFLAGS)
LDADD = $(GLIB_LIBS) $(GIO_LIBS)

bluez_sources =	lib/bluez/adapter.c lib/bluez/adapter.h \
		lib/bluez/agent_manager.c lib/bluez/agent_manager.h \
		lib/bluez/alert_agent.c lib/bluez/alert_agent.h \
		lib/bluez/alert.c lib/bluez/alert.h \
		lib/bluez/cycling_speed.c lib/bluez/cycling_speed.h \
		lib/bluez/cycling_speed_manager.c lib/bluez/cycling_speed_manager.h \
		lib/bluez/device.c lib/bluez/device.h \
		lib/bluez/health_channel.c lib/bluez/health_channel.h \
		lib/bluez/health_device.c lib/bluez/health_device.h \
		lib/bluez/health_manager.c lib/bluez/health_manager.h \
		lib/bluez/heart_rate.c lib/bluez/heart_rate.h \
		lib/bluez/heart_rate_manager.c lib/bluez/heart_rate_manager.h \
		lib/bluez/media.c lib/bluez/media.h \
		lib/bluez/media_control.c lib/bluez/media_control.h \
		lib/bluez/media_player.c lib/bluez/media_player.h \
		lib/bluez/network.c lib/bluez/network.h \
		lib/bluez/network_server.c lib/bluez/network_server.h \
		lib/bluez/obex/obex_agent_manager.c lib/bluez/obex/obex_agent_manager.h \
		lib/bluez/obex/obex_client.c lib/bluez/obex/obex_client.h \
		lib/bluez/obex/obex_file_transfer.c lib/bluez/obex/obex_file_transfer.h \
		lib/bluez/obex/obex_message_access.c lib/bluez/obex/obex_message_access.h \
		lib/bluez/obex/obex_message.c lib/bluez/obex/obex_message.h \
		lib/bluez/obex/obex_object_push.c lib/bluez/obex/obex_object_push.h \
		lib/bluez/obex/obex_phonebook_access.c lib/bluez/obex/obex_phonebook_access.h \
		lib/bluez/obex/obex_session.c lib/bluez/obex/obex_session.h \
		lib/bluez/obex/obex_synchronization.c lib/bluez/obex/obex_synchronization.h \
		lib/bluez/obex/obex_transfer.c lib/bluez/obex/obex_transfer.h \
		lib/bluez/profile_manager.c lib/bluez/profile_manager.h \
		lib/bluez/proximity_monitor.c lib/bluez/proximity_monitor.h \
		lib/bluez/proximity_reporter.c lib/bluez/proximity_reporter.h \
		lib/bluez/sim_access.c lib/bluez/sim_access.h \
		lib/bluez/thermometer.c lib/bluez/thermometer.h \
		lib/bluez/thermometer_manager.c lib/bluez/thermometer_manager.h

lib_sources = 	lib/agent-helper.c lib/agent-helper.h \
		lib/dbus-common.c lib/dbus-common.h \
		lib/helpers.c lib/helpers.h \
		lib/manager.c lib/manager.h \
		lib/obex_agent.c lib/obex_agent.h \
		lib/properties.c lib/properties.h \
		lib/sdp.c lib/sdp.h \
		lib/bluez-api.h

bin_PROGRAMS = bt-adapter bt-agent bt-audio bt-device bt-network bt-obex
bt_adapter_SOURCES = $(lib_sources) $(bluez_sources) bt-adapter.c
bt_agent_SOURCES = $(lib_sources) $(bluez_sources) bt-agent.c
bt_audio_SOURCES = $(lib_sources) $(bluez_sources) bt-audio.c
bt_device_SOURCES = $(lib_sources) $(bluez_sources) bt-device.c
bt_network_SOURCES = $(lib_sources) $(bluez_sources) bt-network.c
bt_obex_SOURCES = $(lib_sources) $(bluez_sources) bt-obex.c
bt_obex_LDADD = $(LDADD) $(LIBREADLINE)
