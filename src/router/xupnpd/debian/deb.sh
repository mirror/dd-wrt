#!/bin/bash

ROOT=./debian-pkg
DEBIAN=$ROOT/DEBIAN/
CONTENTS=$ROOT/usr/share/xupnpd/
BIN=$ROOT/usr/bin/
ETC=$ROOT/etc/
SRC=../

mkdir -p $DEBIAN
mkdir -p $BIN
mkdir -p $ETC
mkdir -p $CONTENTS
mkdir -p $CONTENTS/config
mkdir -p $CONTENTS/config/postinit
mkdir -p $CONTENTS/localmedia
mkdir -p $CONTENTS/playlists
mkdir -p $CONTENTS/playlists/example
mkdir -p $CONTENTS/plugins
mkdir -p $CONTENTS/plugins/skel
mkdir -p $CONTENTS/profiles
mkdir -p $CONTENTS/profiles/skel
mkdir -p $CONTENTS/ui
mkdir -p $CONTENTS/www

cp control $DEBIAN

cp $SRC/../LICENSE $CONTENTS
cp $SRC/xupnpd_*.lua $CONTENTS
sed "s/cfg.ssdp_interface='lo'/cfg.ssdp_interface='eth0'/g;s/cfg.ssdp_loop=1/cfg.ssdp_loop=0/g;s/cfg.daemon=false/cfg.daemon=false/g;s/cfg.mcast_interface='eth1'/cfg.mcast_interface='eth0'/g" < $SRC/xupnpd.lua > $CONTENTS/xupnpd.lua
cp $SRC/xupnpd $BIN
cp $SRC/playlists/*.m3u $CONTENTS/playlists/
cp $SRC/playlists/example/*.m3u $CONTENTS/playlists/example/
cp $SRC/plugins/*.lua $CONTENTS/plugins/
cp $SRC/plugins/skel/*.lua $CONTENTS/plugins/skel/
cp $SRC/profiles/*.lua $CONTENTS/profiles/
cp $SRC/profiles/skel/*.lua $CONTENTS/profiles/skel/
cp $SRC/ui/*.lua $CONTENTS/ui/
cp $SRC/ui/*.css $CONTENTS/ui/
cp $SRC/ui/*.html $CONTENTS/ui/
cp $SRC/ui/*.txt $CONTENTS/ui/
cp $SRC/www/*.html $CONTENTS/www/
cp $SRC/www/*.xml $CONTENTS/www/
cp $SRC/www/*.mp4 $CONTENTS/www/
cp $SRC/www/*.ico $CONTENTS/www/
cp $SRC/www/icon-48x48.png $CONTENTS/www/
touch $CONTENTS/config/.empty
touch $CONTENTS/config/postinit/.empty
touch $CONTENTS/localmedia/.empty
ln -s /usr/share/xupnpd/xupnpd.lua $ETC

fakeroot -- dpkg -b $ROOT debian-pkg.deb
dpkg-name -o debian-pkg.deb

rm -rf $ROOT