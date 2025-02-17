#!/bin/sh

pod2man -n bt-adapter -c "bluez-tools" -r "" man/bt-adapter.pod > ../src/bt-adapter.1
pod2man -n bt-agent -c "bluez-tools" -r "" man/bt-agent.pod > ../src/bt-agent.1
pod2man -n bt-device -c "bluez-tools" -r "" man/bt-device.pod > ../src/bt-device.1

# pod2man -n bt-monitor -c "bluez-tools" -r "" man/bt-monitor.pod > ../src/bt-monitor.1

# pod2man -n bt-audio -c "bluez-tools" -r "" man/bt-audio.pod > ../src/bt-audio.1
# pod2man -n bt-input -c "bluez-tools" -r "" man/bt-input.pod > ../src/bt-input.1
pod2man -n bt-network -c "bluez-tools" -r "" man/bt-network.pod > ../src/bt-network.1
# pod2man -n bt-serial -c "bluez-tools" -r "" man/bt-serial.pod > ../src/bt-serial.1

pod2man -n bt-obex -c "bluez-tools" -r "" man/bt-obex.pod > ../src/bt-obex.1
