#!/bin/sh

if [ $# != 1 ]; then
echo "usage: release_rootfs.sh  apType "
echo "ex   : release_rootfs.sh  ap30 "
exit 1
fi

if [ -z $TFTPPATH ]; then
echo "TFTPPATH is not defined! "
exit 1
fi

if [ -z $KERNELPATH ]; then
echo "KERNELPATH is not defined! "
exit 1
fi

if [ -z $INSTALLPATH ]; then
echo "INSTALLPATH is not defined! "
exit 1
fi

HAS_4MB_FLASH=`grep "CONFIG_FLASH_4MB=y" $KERNELPATH/.config`
HAS_2MB_FLASH=`grep "CONFIG_FLASH_2MB=y" $KERNELPATH/.config`

if [ $1 == "ap51" -o $1 == "ap51-debug" -o  $1 == "ap48" -o $1 == "ap48-debug" -o $1 == "ap43" -o $1 == "ap43-debug" -o $1 == "ap61" -o $1 == "ap61-debug" -o $1 == "ap65" -o $1 == "ap65-debug" -o $1 == "ap63" -o $1 == "ap63-debug" -o $1 == "pb32" ]; then
    ERASEBLOCK=0x10000
else
    ERASEBLOCK=0x1000
fi

FLASHSIZE="2MB"
if [ $1 == "ap51" -o $1 == "ap51-debug" -o $1 == "ap48" -o $1 == "ap48-debug" -o $1 == "ap43" -o $1 == "ap43-debug" -o $1 == "ap63" -o $1 == "ap63-debug" ]; then
    PADSIZE=0x140000
else
    PADSIZE=0x141000
fi

if [ ! -z $HAS_4MB_FLASH ]; then
    PADSIZE=0x300000
    FLASHSIZE="4MB"
fi
mkdir -p $INSTALLPATH
pushd $KERNELPATH/arch/mips/ar531x/ROOTDISK/rootdir
echo ""
echo "**** Creating jffs2 rootfs for $FLASHSIZE flash part ****"
mkfs.jffs2 -r . -o $INSTALLPATH/jffs2.$1.bin -b --squash --pad=$PADSIZE --eraseblock=$ERASEBLOCK
sudo cp $INSTALLPATH/jffs2.$1.bin $TFTPPATH 
echo "  rootfs partition length = $PADSIZE"
echo ""
popd

echo ""
echo "  Flash using RedBoot commands:"
echo "    RedBoot> load -r -b %{FREEMEMLO} jffs2.$1.bin"
echo "    RedBoot> fis create -f 0xbfc30000 -e 0 rootfs"
echo ""


sudo cp -r $KERNELPATH/arch/mips/ar531x/ROOTDISK/rootdir/lib/modules $TFTPPATH/${1}_modules
