#!/bin/sh
#
#   libnet support file
#   Ensures a target directory will exist for the installer.
#   Unknown author.
#

pathname=$1
mode=$2
OLDIFS="${IFS}"; IFS=/; set $pathname; IFS="${OLDIFS}"

case $pathname in
       /*)     partial=/; ;;
       *)      partial=""; ;;
esac

for i do
       case i in "") continue; ;; esac
       partial="${partial}${i}"
       if [ ! -d ${partial} ]; then
               mkdir ${partial} || exit 1;
               chmod ${mode} ${partial}
       fi
       partial="${partial}/"
done
