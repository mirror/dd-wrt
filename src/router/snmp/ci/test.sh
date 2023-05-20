#!/bin/bash

scriptdir="$(dirname "$0")"
case $(uname) in
    MinGW)
	;;
    *)
	"${scriptdir}"/net-snmp-run-tests
	;;
esac
