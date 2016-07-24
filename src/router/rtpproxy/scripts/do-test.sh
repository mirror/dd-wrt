#!/bin/sh

set -e

uname -a
${CC} --version
./configure
make
make clean
sudo DEBIAN_FRONTEND=noninteractive apt-get install libgsm1-dev libsndfile1-dev tcpdump
tcpdump --version || true
mkdir deps
cd deps
wget http://download-mirror.savannah.gnu.org/releases/linphone/plugins/sources/bcg729-1.0.0.tar.gz
tar xfz bcg729-1.0.0.tar.gz
cd bcg729-1.0.0
perl -pi -e 's|BASICOPERATIONSMACROS__H|BASICOPERATIONSMACROS_H|g' include/basicOperationsMacros.h
./configure
make
sudo make install
cd ..
git clone git://github.com/sippy/libg722 libg722
cd libg722
make
sudo make install
cd ../..
sudo ldconfig
autoreconf --force --install --verbose
./configure
make
make check || (cat tests/test-suite.log; exit 1)
