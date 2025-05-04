#!/bin/sh

mstpctl_parse_ports()
{
  while [ "${1+set}" = set ]
  do
    # For compatibility: the 'all' option.
    case $1 in
      all)
	shift &&
	set -- regex eth.\* em.\* 'p[0-9].*' noregex "$@"
	;;
    esac

    # Primitive state machine...
    case $1-$(uname -s) in
      regex-Linux)
	all_interfaces=$(sed -n 's%^[\ ]*\([^:]*\):.*$%\1%p' < /proc/net/dev)
	shift
	;;
      regex-*)
	echo "$0 needs to be ported for your $(uname -s) system.  " \
	     "Trying to continue nevertheless." >&2
	shift
	;;
      noregex-*)
	unset all_interfaces
	shift
	;;
    esac

    case ${all_interfaces+regex}-${1+set} in
      regex-set)
	# The following interface specification are to be parsed as regular
	# expressions against all interfaces the system provides.
	i=$(grep -E "^$1$" << EOAI
$all_interfaces
EOAI
)
	shift
	;;
      *-set)
	# Literal interfaces.
	i=$1
	shift
	;;
      *)
	# No interface specification is following.
	i=
	;;
    esac

    echo "$i"
  done
}
