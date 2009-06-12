#!/bin/sh
DATE=$(date +%D_%T | sed -e "s|/|_|g" | sed -e "s/:/_/g")
DIFF=diff
TEE=tee
MAIL_LIST=vsubbiah@atheros.com
(./nightly_build.sh | $TEE /tmp/${USER}_make_out_$DATE) > /tmp/${USER}_make_log_$DATE 2>&1
BUILD_DIR=/home/$USER/tgt/build
cp /tmp/${USER}_make_out_$DATE $BUILD_DIR
cp /tmp/${USER}_make_log_$DATE $BUILD_DIR
$DIFF $BUILD_DIR/${USER}_make_log_$DATE $BUILD_DIR/${USER}_make_out_$DATE > $BUILD_DIR/make_errors
#echo "Build completed" | mail vsubbiah@atheros.com -s "Linux build completed"
