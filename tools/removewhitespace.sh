#!/bin/sh

for i in $2/*.asp $2/*.htm $2/*.html $2/*.js $2/*.css $2/*.webservices $2/*.webconfig  $2/*.webconfig_release $2/*.webhotspot $2/*.webalive $2/*.websecurity $2/*.webvpn $2/*.ipv6config $2/*.p2pwebconfig $2/*.webproxy $2/*.webnas $2/*.webserver $2/*.webusb; do
	if test -e $i; then
		echo $i
		$1/removewhitespace $i
	fi
done
