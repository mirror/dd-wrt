#!/bin/sh
    pids="$(ps | grep 'Plex Media Server' | grep -v grep | awk '{print $1}')"
    kill -15 $pids

    sleep 5

    # Stuck
    pids="$(ps | grep /usr/lib/plexmediaserver | grep -v grep | awk '{print $1}')"

    if [ "$pids" != "" ]; then
      kill -9 $pids
    sleep 2
    fi

