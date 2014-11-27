#!/bin/sh
# orig: http://ubuntuforums.org/showthread.php?t=464883
VERBOSE=""
LOGG="yes"
SOURCEPATH="/etc/arpalert/"
MACOWNERLIST="${SOURCEPATH}maclist.dat"
LOGFILE="${SOURCEPATH}alert.log"
TIMESTAMP="`date`"
OWNER="`grep $1 $MACOWNERLIST | cut -f 2`"
NEWNAME="`nbtscan -s ';' $2 | cut -d ';' -f 2 | tr -d '<>'`"
HEADER=""

case "$5" in
        0)      HEADER="0 ip change"
                ;;
        1)      HEADER="1 mac address only detected but not in whithe list"
                ;;
        2)      HEADER="2 mac address in black list"
                ;;
        3)      HEADER="3 new mac address"
                ;;
        4)      HEADER="4 unauthorized arp request"
                ;;
        5)      HEADER="5 abusive number of arp request detected"
                ;;
        6)      HEADER="6 ethernet mac address different from arp mac address"
                ;;
        7)      HEADER="7 global flood detection"
                ;;
        8)      HEADER="8 new mac adress without ip"
                ;;
        9)      HEADER="9 mac change"
                ;;
esac

if [ "$HEADER" ] ; then
/usr/bin/zenity --warning --title="${HEADER}" --text="OWNER =${OWNER}
COMPUTERNAME = ${NEWNAME}
----------
TIMESTAMP = ${TIMESTAMP}
MAC = ${1}
NEW IP = ${2}
OLD IP = ${3}
REPORTER = ${4}
VENDOR = ${6}"
fi

[ "$LOGG" ] && echo "$TIMESTAMP - $* (${OWNER})" >> $LOGFILE

exit 0
