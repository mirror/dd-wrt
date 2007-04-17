#!/bin/sh
#
#  The purpose of this script is to forcibly load the *correct* version
#  of OpenSSL for FreeRADIUS, when you have more than one version of OpenSSL
#  installed on your system.
#
#  You'll have to edit the directories to the correct location
#  for your local system.
#
#	$Id: radiusd.sh,v 1.1 2004/01/07 17:07:41 aland Exp $
#

LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:/usr/local/ssl/lib:/usr/local/radius/lib
LD_PRELOAD=/usr/local/ssl/lib/libcrypto.so

export LD_LIBRARY_PATH LD_PRELOAD
exec /usr/local/radius/sbin/radiusd $@
