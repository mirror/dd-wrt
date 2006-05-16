#!/bin/sh

if [ "$#" -ne 1 ] ; then
	echo "Purpose:"
	echo "Locate all madwifi-related kernel modules in a given directory"
	echo "(including all its subdirectories)."
	echo
	echo "Usage:"
	echo "$0 <dir>"
	echo
	echo "<dir>: name of the directory used for the search"
	echo
	exit 1
fi

SCRIPTS=$(dirname $0)
if [ -z "$SCRIPTS" ]; then
	SCRIPTS=.
fi

if [ -e "${SCRIPTS}"/../noask ]; then
	QUIET="noask"
fi

SEARCH=${1}

if [ -d "${SEARCH}" ]; then
	PATTERN="^.*\/(ath_(hal|pci|rate_(onoe|amrr|sample))\.k?o)|(wlan(_(acl|ccmp|scan_(ap|sta)|tkip|wep|xauth))?\.k?o)$"
	OLD_MODULES=$(find ${SEARCH} -type f -regex '.*\.k?o' 2>/dev/null | grep -w -E "${PATTERN}")
fi

if [ -n "${OLD_MODULES}" ]; then
	if [ "${QUIET}" = "noask" ]; then
		rm -f ${OLD_MODULES}
		exit
	fi
	echo
	echo "WARNING:"
	echo "It seems that there are modules left from previous MadWifi installations."
	echo "If you are unistalling the MadWifi modules please press \"r\" to remove them."
	echo "If you are installing new MadWifi modules, you should consider removing those"
	echo "already installed, or else you may experience problems during operation."
	echo "Remove old modules?"
	
	while true; do
		echo
		echo -n "[l]ist, [r]emove, [i]gnore or e[x]it (l,r,i,[x]) ? "
		echo
		read REPLY
		case ${REPLY} in
			l|L)
				for m in ${OLD_MODULES}; do echo ${m}; done
				continue
				;;
			
			r|R)
				rm -f ${OLD_MODULES}
				exit
				;;
		
			i|I)
				exit 0
				;;
	
			x|X)
				exit 1
				;;

			*)
				continue
				;;
		esac
	done
fi

exit 0
