#!/bin/sh

mstpctl_parse_ports()
{
  while [ x"${1+set}" = xset ]
  do
    # For compatibility: the `all' option.
    case $1 in
      all)
	shift &&
	set regex eth.\* em.\* 'p[0-9].*' noregex "$@"
	;;
    esac

    # Primitive state machine...
    case $1-`uname -s` in
      regex-Linux)
	all_interfaces=`sed -n 's%^[\ ]*\([^:]*\):.*$%\1%p' < /proc/net/dev`
	shift
	;;
      regex-*)
	echo -n "$0 needs to be ported for your `uname -s` system.  " >&2
	echo "Trying to continue nevertheless." >&2
	shift
	;;
      noregex-*)
	all_interfaces=
	unset all_interfaces
	shift
	;;
    esac

    case ${all_interfaces+regex}-${1+set} in
      regex-set)
	# The following interface specification are to be parsed as regular
	# expressions against all interfaces the system provides.
	i=`egrep "^$1$" << EOAI
$all_interfaces
EOAI
`
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

    echo $i
  done
}
