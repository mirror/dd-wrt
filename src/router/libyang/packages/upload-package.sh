#!/usr/bin/env bash

GIT_PATH=`git rev-parse --show-cdup`
if [ "$GIT_PATH" != "" ]; then
    echo "Must be run from repository root"
    exit
fi

if [ $# -ne 1 ]; then
    echo "Usage $0 <osb-user>"
    exit
fi

# for correct git log command output used in changelog
LC_TIME=en_US.UTF-8
# upload package even if the version is not newer
FORCEVERSION=1
# needed so that the package is even created
TRAVIS_EVENT_TYPE=cron

# OSB login
osb_user=$1

# OSB password
echo -n Password:
read -s osb_pass
echo

# create and upload the package
. ./packages/create-package.sh
