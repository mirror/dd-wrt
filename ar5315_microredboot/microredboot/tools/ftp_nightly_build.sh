#!/bin/sh
HOST='10.10.18.177'
USER='spe'
PASSWD='t-span'
TFTP_DIR=/tftpboot
DIR=/home/build/tgt/build
echo $DIR
sudo mv $TFTP_DIR/ $TFTP_DIR_$(date +%D_%T | sed -e "s|/|_|g" | sed -e "s/:/_/g")
mkdir $TFTP_DIR
#cd /home/build/tgt/04_14_05
ftp -i -n  $HOST <<END_SCRIPT
user $USER $PASSWD
cd $DIR
lcd $TFTP_DIR
bin
mget *
quit
END_SCRIPT
echo "Ftp done"

