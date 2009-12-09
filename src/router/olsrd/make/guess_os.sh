#!/bin/sh

if test -n "$OS"; then
	echo $OS;
	exit;
else
	# OS not specified.
	# get it from uname
	arch=`uname -s | tr '[A-Z]' '[a-z]'`
	case "$arch" in
		linux)		     arch=linux ;;
		freebsd*)	     arch=fbsd ;;
		gnu/kfreebsd*) arch=kfbsd ;;
		netbsd*)	     arch=nbsd ;;
		openbsd*)	     arch=obsd ;;
		darwin*)	     arch=osx ;;
		cygwin_*)	     arch=win32 ;;
		Windows_*)	   arch=win32 ;;
		*)		         arch="UNKNOWN" ;;
	esac
	echo $arch
fi
