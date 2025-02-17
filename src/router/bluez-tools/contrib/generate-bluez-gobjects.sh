#!/bin/sh

# BlueZ API

API_VERSION=5.20-fixed

# adapter-api.txt
echo "adapter-api.txt"
echo "Generating adapter header"
./gen-dbus-gobject.pl -header bluez-api-${API_VERSION}/adapter-api.txt > out/adapter.h
echo "Generating adapter source"
./gen-dbus-gobject.pl -source bluez-api-${API_VERSION}/adapter-api.txt > out/adapter.c

# agent-api.txt
echo "agent-api.txt"
echo "Generating agent manager header"
./gen-dbus-gobject.pl -header bluez-api-${API_VERSION}/agent-api.txt 1 > out/agent_manager.h
echo "Generating agent manager source"
./gen-dbus-gobject.pl -source bluez-api-${API_VERSION}/agent-api.txt 1 > out/agent_manager.c

# echo "Generating agent header"
# ./gen-dbus-gobject.pl -header bluez-api-${API_VERSION}/agent-api.txt 2 > out/agent.h
# echo "Generating agent source"
# ./gen-dbus-gobject.pl -source bluez-api-${API_VERSION}/agent-api.txt 2 > out/agent.c

# alert-api.txt
echo "alert-api.txt"
echo "Generating alert header"
./gen-dbus-gobject.pl -header bluez-api-${API_VERSION}/alert-api.txt 1 > out/alert.h
echo "Generating alert source"
./gen-dbus-gobject.pl -source bluez-api-${API_VERSION}/alert-api.txt 1 > out/alert.c

echo "Generating alert agent header"
./gen-dbus-gobject.pl -header bluez-api-${API_VERSION}/alert-api.txt 2 > out/alert_agent.h
echo "Generating alert agent source"
./gen-dbus-gobject.pl -source bluez-api-${API_VERSION}/alert-api.txt 2 > out/alert_agent.c

# cyclingspeed-api.txt
echo "cyclingspeed-api.txt"
echo "Generating cycling speed manager header"
./gen-dbus-gobject.pl -header bluez-api-${API_VERSION}/cyclingspeed-api.txt 1 > out/cycling_speed_manager.h
echo "Generating cycling speed manager source"
./gen-dbus-gobject.pl -source bluez-api-${API_VERSION}/cyclingspeed-api.txt 1 > out/cycling_speed_manager.c

echo "Generating cycling speed header"
./gen-dbus-gobject.pl -header bluez-api-${API_VERSION}/cyclingspeed-api.txt 2 > out/cycling_speed.h
echo "Generating cycling speed source"
./gen-dbus-gobject.pl -source bluez-api-${API_VERSION}/cyclingspeed-api.txt 2 > out/cycling_speed.c

# echo "Generating cycling watcher header"
# ./gen-dbus-gobject.pl -header bluez-api-${API_VERSION}/cyclingspeed-api.txt 3 > out/cycling_speed_watcher.h
# echo "Generating cycling watcher source"
# ./gen-dbus-gobject.pl -source bluez-api-${API_VERSION}/cyclingspeed-api.txt 3 > out/cycling_speed_watcher.c

# device-api.txt
echo "device-api.txt"
echo "Generating device header"
./gen-dbus-gobject.pl -header bluez-api-${API_VERSION}/device-api.txt > out/device.h
echo "Generating device source"
./gen-dbus-gobject.pl -source bluez-api-${API_VERSION}/device-api.txt > out/device.c

# health-api.txt
echo "health-api.txt"
echo "Generating health manager header"
./gen-dbus-gobject.pl -header bluez-api-${API_VERSION}/health-api.txt 1 > out/health_manager.h
echo "Generating health manager source"
./gen-dbus-gobject.pl -source bluez-api-${API_VERSION}/health-api.txt 1 > out/health_manager.c

echo "Generating health device header"
./gen-dbus-gobject.pl -header bluez-api-${API_VERSION}/health-api.txt 2 > out/health_device.h
echo "Generating health device source"
./gen-dbus-gobject.pl -source bluez-api-${API_VERSION}/health-api.txt 2 > out/health_device.c

echo "Generating health channel header"
./gen-dbus-gobject.pl -header bluez-api-${API_VERSION}/health-api.txt 3 > out/health_channel.h
echo "Generating health channel source"
./gen-dbus-gobject.pl -source bluez-api-${API_VERSION}/health-api.txt 3 > out/health_channel.c

# heartrate-api.txt
echo "heartrate-api.txt"
echo "Generating heart rate manager header"
./gen-dbus-gobject.pl -header bluez-api-${API_VERSION}/heartrate-api.txt 1 > out/heart_rate_manager.h
echo "Generating heart rate manager source"
./gen-dbus-gobject.pl -source bluez-api-${API_VERSION}/heartrate-api.txt 1 > out/heart_rate_manager.c

echo "Generating heart rate header"
./gen-dbus-gobject.pl -header bluez-api-${API_VERSION}/heartrate-api.txt 2 > out/heart_rate.h
echo "Generating heart rate source"
./gen-dbus-gobject.pl -source bluez-api-${API_VERSION}/heartrate-api.txt 2 > out/heart_rate.c

# echo "Generating heart rate watcher header"
# ./gen-dbus-gobject.pl -header bluez-api-${API_VERSION}/heartrate-api.txt 3 > out/heart_rate_watcher.h
# echo "Generating heart rate watcher source"
# ./gen-dbus-gobject.pl -source bluez-api-${API_VERSION}/heartrate-api.txt 3 > out/heart_rate_watcher.c

# media-api.txt
echo "media-api.txt"
echo "Generating media header"
./gen-dbus-gobject.pl -header bluez-api-${API_VERSION}/media-api.txt 1 > out/media.h
echo "Generating media source"
./gen-dbus-gobject.pl -source bluez-api-${API_VERSION}/media-api.txt 1 > out/media.c

echo "Generating media control header"
./gen-dbus-gobject.pl -header bluez-api-${API_VERSION}/media-api.txt 2 > out/media_control.h
echo "Generating media control source"
./gen-dbus-gobject.pl -source bluez-api-${API_VERSION}/media-api.txt 2 > out/media_control.c

echo "Generating media player header"
./gen-dbus-gobject.pl -header bluez-api-${API_VERSION}/media-api.txt 3 > out/media_player.h
echo "Generating media player source"
./gen-dbus-gobject.pl -source bluez-api-${API_VERSION}/media-api.txt 3 > out/media_player.c

# echo "Generating media folder header"
# ./gen-dbus-gobject.pl -header bluez-api-${API_VERSION}/media-api.txt 4 > out/media_folder.h
# echo "Generating media folder source"
# ./gen-dbus-gobject.pl -source bluez-api-${API_VERSION}/media-api.txt 4 > out/media_folder.c

# echo "Generating media item header"
# ./gen-dbus-gobject.pl -header bluez-api-${API_VERSION}/media-api.txt 5 > out/media_item.h
# echo "Generating media item source"
# ./gen-dbus-gobject.pl -source bluez-api-${API_VERSION}/media-api.txt 5 > out/media_item.c

# echo "Generating media endpoint header"
# ./gen-dbus-gobject.pl -header bluez-api-${API_VERSION}/media-api.txt 6 > out/media_endpoint.h
# echo "Generating media endpoint source"
# ./gen-dbus-gobject.pl -source bluez-api-${API_VERSION}/media-api.txt 6 > out/media_endpoint.c

# echo "Generating media transport header"
# ./gen-dbus-gobject.pl -header bluez-api-${API_VERSION}/media-api.txt 7 > out/media_transport.h
# echo "Generating media transport source"
# ./gen-dbus-gobject.pl -source bluez-api-${API_VERSION}/media-api.txt 7 > out/media_transport.c

# network-api.txt
echo "network-api.txt"
echo "Generating network header"
./gen-dbus-gobject.pl -header bluez-api-${API_VERSION}/network-api.txt 1 > out/network.h
echo "Generating network source"
./gen-dbus-gobject.pl -source bluez-api-${API_VERSION}/network-api.txt 1 > out/network.c

echo "Generating network server header"
./gen-dbus-gobject.pl -header bluez-api-${API_VERSION}/network-api.txt 2 > out/network_server.h
echo "Generating network server source"
./gen-dbus-gobject.pl -source bluez-api-${API_VERSION}/network-api.txt 2 > out/network_server.c

# obex-agent-api.txt
echo "obex-agent-api.txt"
echo "Generating obex agent manager header"
./gen-dbus-gobject.pl -header bluez-api-${API_VERSION}/obex-agent-api.txt 1 > out/obex/obex_agent_manager.h
echo "Generating obex agent manager source"
./gen-dbus-gobject.pl -source bluez-api-${API_VERSION}/obex-agent-api.txt 1 > out/obex/obex_agent_manager.c

# echo "Generating obex agent header"
# ./gen-dbus-gobject.pl -header bluez-api-${API_VERSION}/obex-agent-api.txt 2 > out/obex/obex_agent.h
# echo "Generating obex agent source"
# ./gen-dbus-gobject.pl -source bluez-api-${API_VERSION}/obex-agent-api.txt 2 > out/obex/obex_agent.c

# obex-api.txt
echo "obex-api.txt"
echo "Generating obex client header"
./gen-dbus-gobject.pl -header bluez-api-${API_VERSION}/obex-api.txt 1 > out/obex/obex_client.h
echo "Generating obex client source"
./gen-dbus-gobject.pl -source bluez-api-${API_VERSION}/obex-api.txt 1 > out/obex/obex_client.c

echo "Generating obex session header"
./gen-dbus-gobject.pl -header bluez-api-${API_VERSION}/obex-api.txt 2 > out/obex/obex_session.h
echo "Generating obex session source"
./gen-dbus-gobject.pl -source bluez-api-${API_VERSION}/obex-api.txt 2 > out/obex/obex_session.c

echo "Generating obex transfer header"
./gen-dbus-gobject.pl -header bluez-api-${API_VERSION}/obex-api.txt 3 > out/obex/obex_transfer.h
echo "Generating obex transfer source"
./gen-dbus-gobject.pl -source bluez-api-${API_VERSION}/obex-api.txt 3 > out/obex/obex_transfer.c

echo "Generating obex object push header"
./gen-dbus-gobject.pl -header bluez-api-${API_VERSION}/obex-api.txt 4 > out/obex/obex_object_push.h
echo "Generating obex object push source"
./gen-dbus-gobject.pl -source bluez-api-${API_VERSION}/obex-api.txt 4 > out/obex/obex_object_push.c

echo "Generating obex file transfer header"
./gen-dbus-gobject.pl -header bluez-api-${API_VERSION}/obex-api.txt 5 > out/obex/obex_file_transfer.h
echo "Generating obex file transfer source"
./gen-dbus-gobject.pl -source bluez-api-${API_VERSION}/obex-api.txt 5 > out/obex/obex_file_transfer.c

echo "Generating obex phonebook access header"
./gen-dbus-gobject.pl -header bluez-api-${API_VERSION}/obex-api.txt 6 > out/obex/obex_phonebook_access.h
echo "Generating obex phonebook access source"
./gen-dbus-gobject.pl -source bluez-api-${API_VERSION}/obex-api.txt 6 > out/obex/obex_phonebook_access.c

echo "Generating obex synchronization header"
./gen-dbus-gobject.pl -header bluez-api-${API_VERSION}/obex-api.txt 7 > out/obex/obex_synchronization.h
echo "Generating obex synchronization source"
./gen-dbus-gobject.pl -source bluez-api-${API_VERSION}/obex-api.txt 7 > out/obex/obex_synchronization.c

echo "Generating obex message access header"
./gen-dbus-gobject.pl -header bluez-api-${API_VERSION}/obex-api.txt 8 > out/obex/obex_message_access.h
echo "Generating obex message access source"
./gen-dbus-gobject.pl -source bluez-api-${API_VERSION}/obex-api.txt 8 > out/obex/obex_message_access.c

echo "Generating obex message header"
./gen-dbus-gobject.pl -header bluez-api-${API_VERSION}/obex-api.txt 9 > out/obex/obex_message.h
echo "Generating obex message source"
./gen-dbus-gobject.pl -source bluez-api-${API_VERSION}/obex-api.txt 9 > out/obex/obex_message.c

# profile-api.txt
echo "profile-api.txt"
echo "Generating profile manager header"
./gen-dbus-gobject.pl -header bluez-api-${API_VERSION}/profile-api.txt 1 > out/profile_manager.h
echo "Generating profile manager source"
./gen-dbus-gobject.pl -source bluez-api-${API_VERSION}/profile-api.txt 1 > out/profile_manager.c

# echo "Generating profile header"
# ./gen-dbus-gobject.pl -header bluez-api-${API_VERSION}/profile-api.txt 2 > out/profile.h
# echo "Generating profile source"
# ./gen-dbus-gobject.pl -source bluez-api-${API_VERSION}/profile-api.txt 2 > out/profile.c

# proximity-api.txt
echo "proximity-api.txt"
echo "Generating proximity monitor header"
./gen-dbus-gobject.pl -header bluez-api-${API_VERSION}/proximity-api.txt 1 > out/proximity_monitor.h
echo "Generating proximity monitor source"
./gen-dbus-gobject.pl -source bluez-api-${API_VERSION}/proximity-api.txt 1 > out/proximity_monitor.c

echo "Generating proximity reporter header"
./gen-dbus-gobject.pl -header bluez-api-${API_VERSION}/proximity-api.txt 2 > out/proximity_reporter.h
echo "Generating proximity reporter source"
./gen-dbus-gobject.pl -source bluez-api-${API_VERSION}/proximity-api.txt 2 > out/proximity_reporter.c

# sap-api.txt
echo "sap-api.txt"
echo "Generating sim access header"
./gen-dbus-gobject.pl -header bluez-api-${API_VERSION}/sap-api.txt > out/sim_access.h
echo "Generating sim access source"
./gen-dbus-gobject.pl -source bluez-api-${API_VERSION}/sap-api.txt > out/sim_access.c

# thermometer-api.txt
echo "thermometer-api.txt"
echo "Generating thermometer manager header"
./gen-dbus-gobject.pl -header bluez-api-${API_VERSION}/thermometer-api.txt 1 > out/thermometer_manager.h
echo "Generating thermometer manager source"
./gen-dbus-gobject.pl -source bluez-api-${API_VERSION}/thermometer-api.txt 1 > out/thermometer_manager.c

echo "Generating thermometer header"
./gen-dbus-gobject.pl -header bluez-api-${API_VERSION}/thermometer-api.txt 2 > out/thermometer.h
echo "Generating thermometer source"
./gen-dbus-gobject.pl -source bluez-api-${API_VERSION}/thermometer-api.txt 2 > out/thermometer.c

# echo "Generating thermometer watcher header"
# ./gen-dbus-gobject.pl -header bluez-api-${API_VERSION}/thermometer-api.txt 3 > out/thermometer_watcher.h
# echo "Generating thermometer watcher source"
# ./gen-dbus-gobject.pl -source bluez-api-${API_VERSION}/thermometer-api.txt 3 > out/thermometer_watcher.c
