#! /bin/sh
#
# Shell script to integrate madwifi sources into a Linux
# source tree so it can be built statically.  Typically this
# is done to simplify debugging with tools like kgdb.
#

set -e

die()
{
	echo "FATAL ERROR: $1" >&2
	exit 1
}

SRC=..
KERNEL_VERSION=`uname -r`

if test -n "$1"; then
	KERNEL_PATH="$1"
else if test -e /lib/modules/${KERNEL_VERSION}/source; then
	KERNEL_PATH="/lib/modules/${KERNEL_VERSION}/source"
else if test -e /lib/modules/${KERNEL_VERSION}/build; then
	KERNEL_PATH="/lib/modules/${KERNEL_VERSION}/build"
else
	die "Cannot guess kernel source location"
fi
fi
fi

test -d ${KERNEL_PATH} || die "No kernel directory ${KERNEL_PATH}"

PATCH()
{
	patch -N $1 < $2
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
SRC_ATH_RATE=${DEPTH}/ath_rate
test -d ${SRC_ATH_RATE} ||
	{ echo "No rate control algorithm directory ${SRC_ATH_RATE}!"; exit 1; }
SRC_COMPAT=${DEPTH}/include
test -d ${SRC_COMPAT} || { echo "No compat directory ${SRC_COMPAT}!"; exit 1; }

WIRELESS=${KERNEL_PATH}/drivers/net/wireless
test -d ${WIRELESS} || { echo "No wireless directory ${WIRELESS}!"; exit 1; }

if test -f ${WIRELESS}/Kconfig; then
	kbuild=2.6
	kbuildconf=Kconfig
	makedef=LINUX26
else if test -f ${WIRELESS}/Config.in; then
	kbuild=2.4
	kbuildconf=Config.in
	makedef=LINUX24
else
	die "Kernel build system is not supported"
fi
fi

MADWIFI=${WIRELESS}/madwifi
rm -rf ${MADWIFI}
MKDIR ${MADWIFI}

DST_ATH=${MADWIFI}/ath
MKDIR ${DST_ATH}
echo "Copy ath driver bits..."
FILES=`ls ${SRC_ATH}/*.[ch] | sed '/mod.c/d'`
make -C ${DEPTH} svnversion.h
INSTALL ${DST_ATH} ${FILES} ${DEPTH}/*.h
INSTALLX ${DST_ATH}/Makefile ${SRC_ATH}/Makefile.kernel

# NB: use leading '_' to ensure it's built before the driver
DST_ATH_HAL=${MADWIFI}/_ath_hal
MKDIR ${DST_ATH_HAL}
echo "Copy ath_hal bits..."

$makedef := 1
INSTALLX ${DST_ATH_HAL}/Makefile ${SRC_ATH_HAL}/Makefile.kernel
INSTALL ${DST_ATH_HAL} ${SRC_ATH_HAL}/ah_osdep.c
INSTALL ${DST_ATH_HAL} ${SRC_ATH_HAL}/uudecode.c

DST_ATH_RATE=${MADWIFI}/ath_rate
MKDIR ${DST_ATH_RATE}
echo "Copy $SRC_ATH_RATE bits..."
RATEALGS="amrr onoe sample"
for ralg in $RATEALGS; do
	MKDIR ${DST_ATH_RATE}/$ralg
	FILES=`ls ${SRC_ATH_RATE}/$ralg/*.[ch] | sed '/mod.c/d'`
	INSTALL ${DST_ATH_RATE}/$ralg ${FILES}
	INSTALL ${DST_ATH_RATE}/$ralg/Makefile ${SRC_ATH_RATE}/$ralg/Makefile.kernel
done

echo "Copying Atheros HAL files"
DST_HAL=${MADWIFI}/hal
mkdir -p ${DST_HAL}
INSTALL ${DST_HAL} ${SRC_HAL}/ah.h
INSTALL ${DST_HAL} ${SRC_HAL}/ah_desc.h
INSTALL ${DST_HAL} ${SRC_HAL}/ah_devid.h
INSTALL ${DST_HAL} ${SRC_HAL}/version.h
mkdir -p ${DST_HAL}/linux
INSTALL ${DST_HAL}/linux ${SRC_HAL}/linux/ah_osdep.c
INSTALL ${DST_HAL}/linux ${SRC_HAL}/linux/ah_osdep.h
mkdir -p ${DST_HAL}/public
INSTALL ${DST_HAL}/public ${SRC_HAL}/public/*.opt_ah.h
INSTALL ${DST_HAL}/public ${SRC_HAL}/public/*.hal.o.uu


echo "Copying net80211 files"
DST_NET80211=${MADWIFI}/net80211
mkdir -p ${DST_NET80211}
FILES=`ls ${SRC_NET80211}/*.[ch] | sed '/mod.c/d'`
INSTALL ${DST_NET80211} ${FILES} ${DEPTH}/*.h
INSTALL ${DST_NET80211}/Makefile ${SRC_NET80211}/Makefile.kernel


echo "Copying compatibility files"
DST_COMPAT=${MADWIFI}/include
mkdir -p ${DST_COMPAT}
INSTALL ${DST_COMPAT} ${SRC_COMPAT}/*.h
mkdir -p ${DST_COMPAT}/sys
INSTALL ${DST_COMPAT}/sys ${SRC_COMPAT}/sys/*.h


echo "Patching the build system"
INSTALL ${MADWIFI} $kbuild/Makefile
if test "$kbuild" = 2.6; then
INSTALL ${MADWIFI} $kbuild/Kconfig
sed -i '/madwifi/d;/^endmenu/i\
source "drivers/net/wireless/madwifi/Kconfig"' ${WIRELESS}/Kconfig
sed -i '$a\
obj-$(CONFIG_ATHEROS) += madwifi/
/madwifi/d;' ${WIRELESS}/Makefile
else
cp -f $kbuild/Config.in ${MADWIFI}
sed -i '$a\
source drivers/net/wireless/madwifi/Config.in
/madwifi/d' ${WIRELESS}/Config.in
sed -i '/madwifi/d;/include/i\
subdir-$(CONFIG_ATHEROS) += madwifi\
obj-$(CONFIG_ATHEROS) += madwifi/madwifi.o' ${WIRELESS}/Makefile
fi

DST_DOC=${KERNEL_PATH}/Documentation
if test -f $kbuild/Configure.help.patch; then
	grep -q 'CONFIG_ATHEROS' ${DST_DOC}/Configure.help || \
		PATCH ${DST_DOC}/Configure.help $kbuild/Configure.help.patch
fi


echo "Done"
