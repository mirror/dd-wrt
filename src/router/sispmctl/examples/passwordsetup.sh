#!/bin/bash

set -e

echo 'SiS PM Control - Password Setup'
echo
FILE=/etc/sispmctl/password
which base64 > /dev/null || (echo base64 is missing; false)
id | grep '^uid=0(' > /dev/null || \
(echo This scripts must be run as root; false)
echo -n 'User name: '
read UNAME
while true
do
  echo -n 'Password: '
  read -s PASSWD
  echo
  echo -n 'Repeat password: '
  read -s PASSWD2
  echo
  if [[ "x$PASSWD" == "x$PASSWD2" ]]; then
    break;
  fi
  echo
  echo The password inputs did not match.
done
SECRET=$(echo -n "$UNAME:$PASSWD" | base64)
mkdir -p /etc/sispmctl
rm -f $FILE
echo $SECRET > $FILE
chmod 400 $FILE
chown sispmctl $FILE
echo
echo To enable the new password restart the sispmctl services.
