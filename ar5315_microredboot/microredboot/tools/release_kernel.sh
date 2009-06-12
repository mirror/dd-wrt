#!/bin/sh

if [ $# != 1 ]; then
echo "usage: release_kernel.sh  apType "
echo "ex   : release_kernel.sh  ap30 "
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

if [ -z $TOPDIR ]; then
echo "TOPDIR is not defined! "
exit 1
fi


HAS_4MB_FLASH=`grep "CONFIG_FLASH_4MB=y" $KERNELPATH/.config`
HAS_2MB_FLASH=`grep "CONFIG_FLASH_2MB=y" $KERNELPATH/.config`
LZMA=$TOPDIR/tools/lzma/sdk4.17/lzma

mkdir -p $INSTALLPATH
cp $KERNELPATH/vmlinux $INSTALLPATH
pushd $INSTALLPATH > /dev/null
entry=`mips-linux-objdump -f vmlinux | grep start | cut -f3 -d' '`
mem=`mips-linux-nm vmlinux | grep _ftext | cut -f1 -d' '`
cp vmlinux vmlinux.$1 
mips-linux-objcopy -O binary -g vmlinux vmlinux.$1.bin
$LZMA e vmlinux.$1.bin vmlinux.$1.bin.l7
gzip -f vmlinux.$1.bin

echo
echo "Flash using RedBoot commands:"
echo "  RedBoot> load -r -b %{FREEMEMLO} vmlinux.$1.bin.l7"
if [ ! -z $HAS_4MB_FLASH ]; then
  if [ $1 == "ap61" -o $1 == "pb32" -o $1 == "ap65" ]; then
  	LENGTH=0xb0000
  else
	LENGTH=0xa0000
  fi
  FLASH_ADDR=0xbff30000
  GZIP_BS=720896
  #589824=0x90000
  L7_BS=589824
elif [ ! -z $HAS_2MB_FLASH ]; then
  if [ $1 == "ap51" -o $1 == "ap51-debug" -o $1 == "ap48" -o $1 == "ap48-debug" -o $1 == "ap43" -o $1 == "ap43-debug" -o $1 == "ap61" -o $1 == "ap61-debug" -o $1 == "ap65" -o $1 == "ap65-debug" ]; then
    LENGTH=0x70000
    FLASH_ADDR=0xbfd70000
    GZIP_BS=589824
    #458752=0x70000
    L7_BS=458752
  else   
    LENGTH=0x6d000
    FLASH_ADDR=0xbfd71000
    GZIP_BS=610304
    #458752=0x70000
    L7_BS=446464
 
  fi
fi

echo "  RedBoot> fis create -l $LENGTH -f $FLASH_ADDR -e $entry -r 0x$mem vmlinux.bin.l7"
echo

#dd if=vmlinux.$1.bin.gz of=burnit bs=$GZIP_BS count=1 conv=sync
dd if=vmlinux.$1.bin.l7 of=burnit.l7 bs=$L7_BS count=1 conv=sync

#mv -f burnit vmlinux.$1.bin.gz
mv -f burnit.l7 vmlinux.$1.bin.l7
#sudo cp vmlinux.$1.bin.gz $TFTPPATH
sudo cp vmlinux.$1.bin.l7 $TFTPPATH
sudo cp vmlinux.$1 $TFTPPATH
echo
popd > /dev/null
