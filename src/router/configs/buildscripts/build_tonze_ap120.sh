#!/bin/sh
OLDPATH=$PATH
DATE=$(date +%m-%d-%Y)
DATE+="-r"
DATE+=$(svnversion -n adm5120/src/router/httpd)
export PATH=/xfs/toolchains/toolchain-mipsel_gcc4.1.2/bin:$OLDPATH
cd adm5120/src/router
[ -n "$DO_UPDATE" ] && svn update
cp .config_ap120_2M .config
make -f Makefile.adm5120 kernel clean all install
mkdir -p /GruppenLW/releases/$DATE/Tonze-AP120
mkdir -p "/GruppenLW/releases/$DATE/OSBRiDGE 5XLi"
mkdir -p "/GruppenLW/releases/$DATE/OSBRiDGE 5GXi v1"
cd ../../../
cp adm5120/src/router/mipsel-uclibc/vmlinus.gz /GruppenLW/releases/$DATE/Tonze-AP120/image.bin
cp adm5120/src/router/mipsel-uclibc/adm5120-webflash.bin /GruppenLW/releases/$DATE/Tonze-AP120/AP120-webflash.bin

cp adm5120/src/router/mipsel-uclibc/adm5120-webflash-osbridge.bin "/GruppenLW/releases/$DATE/OSBRiDGE 5XLi/dd-wrt-webflash-osbridge.bin"
#cp adm5120/src/router/mipsel-uclibc/vmlinus.csys "/GruppenLW/releases/$DATE/OSBRiDGE 5XLi/image-osbridge.bin"
cp adm5120/src/router/mipsel-uclibc/vmlinus.osbridge "/GruppenLW/releases/$DATE/OSBRiDGE 5XLi/osbridge-to-ddwrt.bin"

cp adm5120/src/router/mipsel-uclibc/adm5120-webflash-osbridge.bin "/GruppenLW/releases/$DATE/OSBRiDGE 5GXi v1/dd-wrt-webflash-osbridge.bin"
#cp adm5120/src/router/mipsel-uclibc/vmlinus.csys "/GruppenLW/releases/$DATE/OSBRiDGE 5GXi v1/image-osbridge.bin"
cp adm5120/src/router/mipsel-uclibc/vmlinus.osbridge "/GruppenLW/releases/$DATE/OSBRiDGE 5GXi v1/osbridge-to-ddwrt.bin"

