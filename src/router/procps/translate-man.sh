#!/bin/bash
#
# Run this script in the top level directory to create the translated man pages:
#
# Once the TP module has been created, this commands get the latest po files:
# echo "Getting the latest translations from translationproject.org..."
# rsync -Lrtvz  translationproject.org::tp/latest/procps-ng-man/  po-man
# rsync -Lrtvz  translationproject.org::tp/latest/procps-ng-man-ps/  po-man/ps
# rsync -Lrtvz  translationproject.org::tp/latest/procps-ng-man-top/  po-man/top

if [ -d po-man ] ; then
  echo "po-man: directory exists, will be reused"
  else
    mkdir -p po-man/{ps,top}
fi

cd po-man

langfiles=*.po
if [ $langfiles = '*.po' ] ; then
    echo No man po files found
    exit 0
fi
for lang in *.po
  do
  if [ -d ${lang%.*} ] ; then
  echo "${lang%.*}: directory exists, will be reused"
  else
    mkdir -p ${lang%.*}/{man1,man5,man8}
  fi
    po4a-translate -f man -m ../free.1 -p ${lang} -l ${lang%.*}/man1/free.1
    po4a-translate -f man -m ../kill.1 -p ${lang} -l ${lang%.*}/man1/kill.1
    po4a-translate -f man -m ../pgrep.1 -p ${lang} -l ${lang%.*}/man1/pgrep.1
    po4a-translate -f man -m ../pidof.1 -p ${lang} -l ${lang%.*}/man1/pidof.1
    po4a-translate -f man -m ../pkill.1 -p ${lang} -l ${lang%.*}/man1/pkill.1
    po4a-translate -f man -m ../pmap.1 -p ${lang} -l ${lang%.*}/man1/pmap.1
    po4a-translate -f man -m ../pwdx.1 -p ${lang} -l ${lang%.*}/man1/pwdx.1
    po4a-translate -f man -m ../skill.1 -p ${lang} -l ${lang%.*}/man1/skill.1
    po4a-translate -f man -m ../slabtop.1 -p ${lang} -l ${lang%.*}/man1/slabtop.1
    po4a-translate -f man -m ../sysctl.8 -p ${lang} -l ${lang%.*}/man8/sysctl.8
    po4a-translate -f man -m ../sysctl.conf.5 -p ${lang} -l ${lang%.*}/man5/sysctl.conf.5
    po4a-translate -f man -m ../tload.1 -p ${lang} -l ${lang%.*}/man1/tload.1
    po4a-translate -f man -m ../uptime.1 -p ${lang} -l ${lang%.*}/man1/uptime.1
    po4a-translate -f man -m ../vmstat.8 -p ${lang} -l ${lang%.*}/man8/vmstat.8
    po4a-translate -f man -m ../w.1 -p ${lang} -l ${lang%.*}/man1/w.1
    po4a-translate -f man -m ../watch.1 -p ${lang} -l ${lang%.*}/man1/watch.1
    if [ -f ps/${lang} ] ; then
	po4a-translate -f man -m ../ps/ps.1 -p ps/${lang} -l ${lang%.*}/man1/ps.1
    fi
    if [ -f top/${lang} ] ; then
	po4a-translate -f man -m ../top/top.1 -p top/${lang} -l ${lang%.*}/man1/top.1
    fi
done
