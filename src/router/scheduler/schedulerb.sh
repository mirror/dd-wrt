#!/bin/sh
SCHEDULE_TIME=$(nvram get schedule_time)

sleep $SCHEDULE_TIME ; reboot &