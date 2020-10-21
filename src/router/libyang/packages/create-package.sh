#!/usr/bin/env bash

package="libyang"

if [ "$TRAVIS_PULL_REQUEST" == "true" -o "$TRAVIS_EVENT_TYPE" != "cron" ] ; then
    exit 0
fi

#install OSC
sudo apt-get install -y osc

# check osb_user and osb_pass
if [ -z "${osb_user}" -o -z "${osb_pass}" ]; then
    exit 0
fi

# fill username and password for opensuse build and downlaod last package information
echo -e "[general]\napiurl = https://api.opensuse.org\n\n[https://api.opensuse.org]\nuser = ${osb_user}\npass = ${osb_pass}" >~/.oscrc
cd ./build

osc checkout home:liberouter
cp home:liberouter/$package/$package.spec home:liberouter/$package/debian.changelog home:liberouter
cp build-packages/debian* build-packages/$package* home:liberouter/$package
cd home:liberouter/$package

# check versions
VERSION=$(cat $package.spec | grep "Version: " | awk '{print $NF}')
OLDVERSION=$(cat ../$package.spec | grep "Version: " | awk '{print $NF}')
if [ -z "$FORCEVERSION" -a "$VERSION" == "$OLDVERSION" ]; then
    exit 0
fi

# create new changelog and paste old changelog
if [ "$VERSION" != "$OLDVERSION" ]; then
    logtime=$(git log -i --grep="VERSION .* $OLDVERSION" | grep "Date: " | sed 's/Date:[ ]*//')
    echo -e "$package ($VERSION) stable; urgency=low\n" >debian.changelog
    git log --since="$logtime" --pretty=format:"  * %s (%aN)%n" | grep "BUGFIX\|CHANGE\|FEATURE" >>debian.changelog
    git log -1  --pretty=format:"%n -- %aN <%aE>  %aD%n" >>debian.changelog
    echo -e "\n" >>debian.changelog
    cat ../debian.changelog >>debian.changelog
fi

if [ "$VERSION" != "$OLDVERSION" ]; then
    git log -1 --date=format:'%a %b %d %Y' --pretty=format:"* %ad  %aN <%aE>" | tr -d "\n" >>$package.spec
    echo " $VERSION" >>$package.spec
    git log --since="$logtime" --pretty=format:"- %s (%aN)"  | grep "BUGFIX\|CHANGE\|FEATURE" >>$package.spec
    echo -e "\n" >>$package.spec
fi
cat ../$package.spec | sed -e '1,/%changelog/d' >>$package.spec

# download source and update to opensuse build
wget "https://github.com/CESNET/libyang/archive/master.tar.gz" -O master.tar.gz
osc commit -m travis-update
