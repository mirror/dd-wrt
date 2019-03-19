#!/usr/bin/env bash

LOCAL_LC_TIME=$LC_TIME
export LC_TIME="en_US.UTF-8"
cd "/home/seg/DEV/mpc83xx/src/router/libyang" && tar --exclude="." --exclude=./.git* -zcvf "/home/seg/DEV/mpc83xx/src/router/libyang/master.tar.gz" . --transform 's/./libyang-master/'
cd /home/seg/DEV/mpc83xx/src/router/libyang
mkdir -p rpms/{BUILD,RPMS,SOURCES,SPECS,SRPMS,tmp}
mv master.tar.gz rpms/SOURCES
sed -e '7d' build-packages/libyang.spec > rpms/SPECS/libyang.spec
git log -1 --date=format:'%a %b %d %Y' --pretty=format:"* %ad  %aN <%aE>" 2>/dev/null >>rpms/SPECS/libyang.spec || printf "* `date +"%a %b %d %Y"`  ${USER} <${USER}@`hostname`>" >>rpms/SPECS/libyang.spec
echo " 0.16.105" >>rpms/SPECS/libyang.spec
git log -10 --pretty=format:"- %s (%aN)" >>rpms/SPECS/libyang.spec 2>/dev/null || echo "- unknown changes" >>rpms/SPECS/libyang.spec
rpmbuild --ba rpms/SPECS/libyang.spec --define "%_topdir /home/seg/DEV/mpc83xx/src/router/libyang/rpms"
export LC_TIME=$LOCAL_LC_TIME
