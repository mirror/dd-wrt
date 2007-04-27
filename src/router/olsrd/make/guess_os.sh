#!/bin/sh

if test -n "$OS"; then
	# cygwin exports the OS environment vaiable - fix it. 
	# otherwiese accept the user supplied OS
	case "$OS" in
		Windows*)	OS=win32 ;;
	esac
	echo $OS;
	exit;
else
	# OS not specified.
	# get it from uname
	arch=`uname -s | tr '[A-Z]' '[a-z]'`
	case "$arch" in
		linux)		arch=linux ;;
		freebsd*)	arch=fbsd ;;
		netbsd*)	arch=nbsd ;;
		openbsd*)	arch=obsd ;;
		darwin*)		arch=osx ;;
		cygwin_*)	arch=win32 ;;
		Windows_*)	arch=win32 ;;
		*)		arch="UNKNOWN" ;;
	esac
	echo $arch
fi
