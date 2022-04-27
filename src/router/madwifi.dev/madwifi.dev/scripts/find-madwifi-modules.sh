#!/bin/sh

if [ -z "${1}" ] ; then
	echo "Purpose:"
	echo "Locate all madwifi-related kernel modules for a given kernel"
	echo "(including all its subdirectories)."
	echo
	echo "Usage:"
	echo "$0 [-l] [-r] <kernelrelease> [destdir]"
	echo
	echo "<kernelrelease>: the kernel release that madwifi has been compiled for"
	echo "[destdir]: destination directory for the compiled madwifi modules (optional)"
	echo "-l list the modules we find."
	echo "-r remove the modules we find."
	echo "If neither -l or -r is specified, the user will be prompted."
	echo
	exit 1
fi

LIST_CMD=0
REMOVE_CMD=0
while [ 1 ]; do
	case $1 in
		-l)
			LIST_CMD=1
			shift;
			;;
		-r)
			REMOVE_CMD=1
			shift;
			;;
		*) 	
			break;
			;;
	esac;
done;

KVERS="${1}"

if [ -n "${2}" ]; then
	KDEST="${2}"
else
	KDEST=""
fi

SEARCH="${KDEST}/lib/modules/${KVERS}"

PATTERN="^.*\/(ath_(hal|pci|rate_[^.]*)\.k?o)|(wlan(_(acl|ccmp|scan_(ap|sta)|tkip|wep|xauth))?\.k?o)$"
OLD_MODULES=$(find ${SEARCH} -type f -regex '.*\.k?o' 2>/dev/null | grep -w -E "${PATTERN}")



if [ -n "${OLD_MODULES}" ]; then
	if [ "$LIST_CMD" -eq 1 ] || [ "$REMOVE_CMD" -eq 1 ]; then
		if [ "$LIST_CMD" -eq 1 ]; then
			for m in ${OLD_MODULES}; do echo ${m}; done
		fi;
		if [ "$REMOVE_CMD" -eq 1 ]; then
			rm -f ${OLD_MODULES}
		fi;
	else
		echo ""
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
	fi;
fi

exit 0
