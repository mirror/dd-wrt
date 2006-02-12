#! /bin/sh

set -e

#
# Shell script to integrate madwifi sources into a Linux
# source tree so it can be built statically.  Typically this
# is done to simplify debugging with tools like kgdb.
#
KERNEL_VERSION=`uname -r`
KERNEL_PATH=${1:-/lib/modules/${KERNEL_VERSION}/build}

DEPTH=../..

MKDIR()
{
	DIR=$1
	test -d $DIR || { echo "Creating $DIR"; mkdir $DIR; }
}

PATCH()
{
	patch -N $1 < $2
}

INSTALL()
{
	DEST=$1; shift
	cp $* $DEST
}

INSTALLX()
{
	DEST=$1; shift
	sed -e 's/^##2.4##//' -e '/^##2.6##/d' $1 > $DEST
}

#
# Location of various pieces.  These mimic what is in Makefile.inc
# and can be overridden from the environment.
#
SRC_HAL=${HAL:-${DEPTH}/hal}
test -d ${SRC_HAL} || { echo "No hal directory ${SRC_HAL}!"; exit 1; }
SRC_ATH_HAL=${ATH_HAL:-${DEPTH}/ath_hal}
test -d ${SRC_ATH_HAL} ||
	{ echo "No ath_hal directory ${SRC_ATH_HAL}!"; exit 1; }
SRC_NET80211=${WLAN:-${DEPTH}/net80211}
test -d ${SRC_NET80211} ||
	{ echo "No net80211 directory ${SRC_NET80211}!"; exit 1; }
SRC_ATH=${ATH:-${DEPTH}/ath}
test -d ${SRC_ATH} || { echo "No ath directory ${SRC_ATH}!"; exit 1; }
SRC_ATH_RATE=${ATH_RATE:-${DEPTH}/ath_rate/onoe}
test -d ${SRC_ATH_RATE} ||
	{ echo "No rate control algorithm directory ${SRC_ATH_RATE}!"; exit 1; }
SRC_COMPAT=${DEPTH}/include
test -d ${SRC_COMPAT} || { echo "No compat directory ${SRC_COMPAT}!"; exit 1; }

WIRELESS=${KERNEL_PATH}/drivers/net/wireless
test -d ${WIRELESS} || { echo "No wireless directory ${WIRELESS}!"; exit 1; }

DST_ATH=${WIRELESS}/ath
MKDIR ${DST_ATH}
echo "Copy ath driver bits..."
FILES=`ls ${SRC_ATH}/*.[ch] | sed '/mod.c/d'`
INSTALL ${DST_ATH} ${FILES}
INSTALL ${DST_ATH} ${SRC_ATH}/Kconfig
INSTALLX ${DST_ATH}/Makefile ${SRC_ATH}/Makefile.kernel

# NB: use leading '_' to ensure it's built before the driver
DST_ATH_HAL=${WIRELESS}/_ath_hal
MKDIR ${DST_ATH_HAL}
echo "Copy ath_hal bits..."
INSTALL ${DST_ATH_HAL} ${SRC_ATH_HAL}/Kconfig
INSTALLX ${DST_ATH_HAL}/Makefile ${SRC_ATH_HAL}/Makefile.kernel

# NB: use leading '_' to ensure it's built before the driver
DST_ATH_RATE=${WIRELESS}/_ath_rate
MKDIR ${DST_ATH_RATE}
echo "Copy $SRC_ATH_RATE bits..."
FILES=`ls ${SRC_ATH_RATE}/*.[ch] | sed '/mod.c/d'`
INSTALL ${DST_ATH_RATE} ${FILES}
INSTALL ${DST_ATH_RATE} ${SRC_ATH_RATE}/Kconfig
INSTALLX ${DST_ATH_RATE}/Makefile ${SRC_ATH_RATE}/Makefile.kernel

DST_HAL=${WIRELESS}/hal
MKDIR ${DST_HAL}
echo "Copy hal bits..."
INSTALL ${DST_HAL} ${SRC_HAL}/ah.h
INSTALL ${DST_HAL} ${SRC_HAL}/ah_desc.h
INSTALL ${DST_HAL} ${SRC_HAL}/ah_devid.h
INSTALL ${DST_HAL} ${SRC_HAL}/version.h
MKDIR ${DST_HAL}/linux
INSTALL ${DST_HAL}/linux ${SRC_HAL}/linux/ah_osdep.c
INSTALL ${DST_HAL}/linux ${SRC_HAL}/linux/ah_osdep.h
# XXX copy only target or use arch? 
INSTALL ${DST_HAL}/linux ${SRC_HAL}/public/*.inc
INSTALL ${DST_HAL}/linux ${SRC_HAL}/public/*.opt_ah.h
INSTALL ${DST_HAL}/linux ${SRC_HAL}/public/*.hal.o.uu
if [ -d ${SRC_HAL}/ar5212 ]; then
	MKDIR ${DST_HAL}/ar5212
	INSTALL ${DST_HAL}/ar5212 ${SRC_HAL}/ar5212/ar5212desc.h
fi

DST_NET80211=${WIRELESS}/net80211
MKDIR ${DST_NET80211}
echo "Copy net80211 bits..."
FILES=`ls ${SRC_NET80211}/*.[ch] | sed '/mod.c/d'`
INSTALL ${DST_NET80211} ${FILES}
INSTALL ${DST_NET80211} ${SRC_NET80211}/Kconfig
INSTALLX ${DST_NET80211}/Makefile ${SRC_NET80211}/Makefile.kernel

MKDIR ${DST_NET80211}/compat
echo "Setting up compatibility bits..."
INSTALL ${DST_NET80211}/compat ${SRC_COMPAT}/compat.h
MKDIR ${DST_NET80211}/compat/sys
INSTALL ${DST_NET80211}/compat/sys ${SRC_COMPAT}/sys/*.h

grep -q 'CONFIG_ATHEROS' ${WIRELESS}/Config.in || \
	PATCH ${WIRELESS}/Config.in Config.in.patch
grep -q 'CONFIG_ATHEROS' ${WIRELESS}/Makefile || \
	PATCH ${WIRELESS}/Makefile Makefile.patch
DST_DOC=${KERNEL_PATH}/Documentation
grep -q 'CONFIG_ATHEROS' ${DST_DOC}/Configure.help || \
	PATCH ${DST_DOC}/Configure.help Configure.help.patch
