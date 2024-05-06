#!/bin/bash

. config
STATUS=0

cd $JOOL_DIR
cd mod
make
sudo make install
if [ $? -ne 0 ]; then
	STATUS=$((STATUS+1))
fi
cd ../usr
./autogen.sh
./configure
make
sudo make install
if [ $? -ne 0 ]; then
	STATUS=$((STATUS+1))
fi
exit $STATUS
