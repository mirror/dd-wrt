# Copyright (C) 2012 Igor Drach
# leaigor@gmail.com

DESCRIPTION = "xupnpd - eXtensible UPnP agent"
HOMEPAGE = "http://xupnpd.org"

LICENSE = "GPLv2"
LIC_FILES_CHKSUM = "file://../LICENSE;md5=751419260aa954499f7abaabaa882bbe"

DEPENDS += "lua5.1"
PR = "r12"
SRCREV = "388"
SRC_URI = "svn://tsdemuxer.googlecode.com/svn/trunk;proto=http;module=xupnpd \
	    file://xupnpd.init \
	    file://xupnpd.lua"
S = "${WORKDIR}/xupnpd/src"

inherit base


SRC = "main.cpp soap.cpp mem.cpp mcast.cpp luaxlib.cpp luaxcore.cpp luajson.cpp luajson_parser.cpp"

do_compile () {
  ${CC} -O2 -c -o md5.o md5c.c
  ${CC} ${CFLAGS} ${LDFLAGS} -DWITH_URANDOM -o xupnpd ${SRC} md5.o -llua -lm -ldl -lstdc++
}


do_install () {
  install -d ${D}/usr/bin ${D}/usr/share/xupnpd ${D}/usr/share/xupnpd/config ${D}/usr/share/xupnpd/playlists ${D}/etc/init.d
  cp ${WORKDIR}/xupnpd.init ${D}/etc/init.d/xupnpd
  cp ${S}/xupnpd ${D}/usr/bin/
  cp -r ${S}/plugins ${D}/usr/share/xupnpd/
  cp -r ${S}/profiles ${D}/usr/share/xupnpd/
  cp -r ${S}/ui ${D}/usr/share/xupnpd/
  cp -r ${S}/www ${D}/usr/share/xupnpd/
  cp  ${S}/*.lua ${D}/usr/share/xupnpd/
  cp ${WORKDIR}/xupnpd.lua ${D}/usr/share/xupnpd/
}