#!/bin/sh
# Copyright 2022 Collabora Ltd.
# SPDX-License-Identifier: MIT

# Usage: check-runstatedir.sh /run/dbus/system_bus_socket

set -e
set -u

same_file () {
    # Many shells implement test -ef (test whether two names point to
    # the same file), but POSIX doesn't require it, so fall back to
    # comparing realpath output if necessary. We prefer test -ef if
    # available, since it does the right thing for bind-mounts, not just
    # symlinks
    if test / -ef / 2>/dev/null; then
        test  "$1" -ef "$2"
    else
        test "$(realpath "$1")" = "$(realpath "$2")"
    fi
}

if [ -e /run ] && [ -e /var/run ] && ! same_file /run /var/run; then
    echo
    echo "WARNING: /run and /var/run are not the same directory."
    echo "| Tools that do not agree on whether a socket is in /run or in"
    echo "| /var/run will fail to interoperate."
    echo "| Ask your OS distributor to make these two directories equivalent"
    echo "| via a symbolic link or bind mount: there is no useful reason to"
    echo "| make them different."
    echo
fi

system_socket="$1"

case "$system_socket" in
    (/run/dbus/system_bus_socket)
        # --with-system-socket=/run/dbus/system_bus_socket
        if ! same_file /run /var/run; then
            echo
            echo "WARNING: system bus socket: /run/dbus/system_bus_socket"
            echo "| The system bus has been configured to listen on"
            echo "| /run/dbus/system_bus_socket, but /run is not the same"
            echo "| as /var/run on this system."
            echo "|"
            echo "| Most D-Bus implementations will expect to find the D-Bus"
            echo "| system bus socket at /var/run/dbus/system_bus_socket."
            echo "| Consider creating a symbolic link."
            echo
        fi
        ;;

    (/var/run/dbus/system_bus_socket)
        # e.g. --localstatedir=/var
        if ! same_file /run /var/run; then
            echo
            echo "NOTE: system bus socket: /var/run/dbus/system_bus_socket"
            echo "| The system bus has been configured to listen on"
            echo "| /var/run/dbus/system_bus_socket, but /run is not the same"
            echo "| as /var/run on this system."
            echo "|"
            echo "| Some D-Bus implementations might expect to find the"
            echo "| D-Bus system bus socket at /run/dbus/system_bus_socket."
            echo "| Consider creating a symbolic link."
            echo
        fi
        ;;

    (*)
        # e.g. --prefix=/opt/dbus
        echo
        echo "NOTE: system bus listens on $system_socket"
        echo "| This build of dbus will not interoperate with the well-known"
        echo "| system bus socket, /var/run/dbus/system_bus_socket."
        echo
        ;;
esac
