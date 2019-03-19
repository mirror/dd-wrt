#!/usr/bin/env bash

if [ "$TRAVIS_PULL_REQUEST" == "true" -o "$TRAVIS_EVENT_TYPE" != "cron" ] ; then
    exit 0
fi
# check osb_user and osb_pass
if [ -z "${osb_user}" -o -z "${osb_pass}" ]; then
    exit 0
fi

# fill username and password for opensuse build and downlaod last package information
echo -e "[general]\napiurl = https://api.opensuse.org\n\n[https://api.opensuse.org]\nuser = ${osb_user}\npass = ${osb_pass}" >~/.oscrc
cd ./build
if [ $TRAVIS_BRANCH == "devel" ]; then
	package="home:liberouter/libyang-experimental"
	name="libyang-experimental"
else
	package="home:liberouter/libyang"
	name="libyang"
fi
osc checkout home:liberouter
cp $package/libyang.spec $package/debian.changelog home:liberouter
cp build-packages/* $package
cd $package

# check versions
VERSION=$(cat libyang.spec | grep "Version: " | awk '{print $NF}')
OLDVERSION=$(cat ../libyang.spec | grep "Version: " | awk '{print $NF}')
if [ "$VERSION" == "$OLDVERSION" ]; then
    exit 0
fi

# create new changelog and paste old changelog
logtime=$(git log -i --grep="VERSION .* $OLDVERSION" | grep "Date: " | sed 's/Date:[ ]*//')
echo -e "$name ($VERSION) stable; urgency=low\n" >debian.changelog
git log --since="$logtime" --pretty=format:"  * %s (%aN)%n" | grep "BUGFIX\|CHANGE\|FEATURE" >>debian.changelog
git log -1  --pretty=format:"%n -- %aN <%aE>  %aD%n" >>debian.changelog
echo -e "\n" >>debian.changelog
cat ../debian.changelog >>debian.changelog
git log -1 --date=format:'%a %b %d %Y' --pretty=format:"* %ad  %aN <%aE>" | tr -d "\n" >>libyang.spec
echo " $VERSION" >>libyang.spec
git log --since="$logtime" --pretty=format:"- %s (%aN)"  | grep "BUGFIX\|CHANGE\|FEATURE" >>libyang.spec
echo -e "\n" >>libyang.spec
cat ../libyang.spec | sed -e '1,/%changelog/d' >>libyang.spec

# download source and update to opensuse build
wget "https://github.com/CESNET/libyang/archive/$TRAVIS_BRANCH.tar.gz" -O $TRAVIS_BRANCH.tar.gz
osc commit -m travis-update
