#!/bin/sh
MKHASH=./mkhash
SOURCE_DATE_EPOCH=$(perl -e 'print((stat $ARGV[0])[9])' "$0")

IMG_PART_SIGNATURE=$(echo $SOURCE_DATE_EPOCH$ | $MKHASH md5 | cut -b1-8)
IMG_PART_DISKGUID=$(echo $SOURCE_DATE_EPOCH$ | $MKHASH md5 | sed -E 's/(.{8})(.{4})(.{4})(.{4})(.{10})../\1-\2-\3-\4-\500/')

echo $IMG_PART_SIGNATURE
echo $IMG_PART_DISKGUID
rm -rf tmp
mkdir -p tmp
cp -urv boot tmp
cp -urv efi tmp
cp ../x86_64-uclibc/root.grub/boot/vmlinuz tmp/boot
cp grub.cfg tmp/boot/grub
#sed -i 's/\01182520-ef7b-24db-37da-d10ea5c4d702/$IMG_PART_DISKGUID/g' tmp/boot/grub/grub.cfg

echo "menuentry \"DD-WRT\" {" >> tmp/boot/grub/grub.cfg
echo "	linux /boot/vmlinuz root=/dev/hda2 nohz_full=all rootfstype=squashfs noinitrd video=vga16fb:off nofb console=ttyS0,115200n8 initcall_blacklist=acpi_cpufreq_init reboot=bios rootdelay=5" >> tmp/boot/grub/grub.cfg
echo "}" >> tmp/boot/grub/grub.cfg

echo "menuentry \"MEMTEST86\" {" >> tmp/boot/grub/grub.cfg
echo "	set root='hd0,gpt1'" >> tmp/boot/grub/grub.cfg
echo "	chainloader (\$root)/efi/memtest86/memtest64.efi" >> tmp/boot/grub/grub.cfg
echo "}" >> tmp/boot/grub/grub.cfg
PADDING="1" SIGNATURE="$IMG_PART_SIGNATURE" GUID="$IMG_PART_DISKGUID" ./scripts/gen_image_generic.sh efi.img 32 tmp 320 ../x86_64-uclibc/root.fs 256 128

rm -rf imp
mkdir -p img
cp boot/grub/boot.img img
cp boot/grub/core.img img
echo "(hd0) `pwd`/efi.img" > img/device.map

grub-tools/grub-bios-setup -m "img/device.map" -d "img" \
		-r "hd0,gpt1" \
		efi.img

./trunc efi.img efi.tmp
mv -f efi.tmp efi.img 

PADDING="1" SIGNATURE="$IMG_PART_SIGNATURE" GUID="$IMG_PART_DISKGUID" ./scripts/gen_image_generic.sh efi-big.img 32 tmp 320 ../x86_64-uclibc/root.fs 256 1780

rm -rf imp
mkdir -p img
cp boot/grub/boot.img img
cp boot/grub/core.img img
echo "(hd0) `pwd`/efi-big.img" > img/device.map

grub-tools/grub-bios-setup -m "img/device.map" -d "img" \
		-r "hd0,gpt1" \
		efi-big.img

./trunc efi-big.img efi-big.tmp
mv -f efi-big.tmp efi-big.img 


rm -rf tmp
mkdir -p tmp
cp -urv boot tmp
cp -urv efi tmp
cp ../x86_64-uclibc/root.grub/boot/vmlinuz tmp/boot
cp grub-vga.cfg tmp/boot/grub/grub.cfg
echo "menuentry \"DD-WRT\" {" >> tmp/boot/grub/grub.cfg
echo "	set gfxpayload=keep" >> tmp/boot/grub/grub.cfg
echo "	linux /boot/vmlinuz root=/dev/hda2 nohz_full=all rootfstype=squashfs noinitrd fbcon=nodefer vga=0x305 video=efifb:1024x768x32 initcall_blacklist=acpi_cpufreq_init reboot=bios rootdelay=5" >> tmp/boot/grub/grub.cfg
echo "}" >> tmp/boot/grub/grub.cfg

echo "menuentry \"MEMTEST86\" {" >> tmp/boot/grub/grub.cfg
echo "	set root='hd0,gpt1'" >> tmp/boot/grub/grub.cfg
echo "	chainloader (\$root)/efi/memtest86/memtest64.efi" >> tmp/boot/grub/grub.cfg
echo "}" >> tmp/boot/grub/grub.cfg

PADDING="1" SIGNATURE="$IMG_PART_SIGNATURE" GUID="$IMG_PART_DISKGUID" ./scripts/gen_image_generic.sh efi-vga.img 32 tmp 320 ../x86_64-uclibc/root.fs 256 128
rm -rf imp
mkdir -p img
cp boot/grub/boot.img img
cp boot/grub/core.img img
echo "(hd0) `pwd`/efi-vga.img" > img/device.map

grub-tools/grub-bios-setup -m "img/device.map" -d "img" \
		-r "hd0,gpt1" \
		efi-vga.img

./trunc efi-vga.img efi-vga.tmp
mv -f efi-vga.tmp efi-vga.img 


PADDING="1" SIGNATURE="$IMG_PART_SIGNATURE" GUID="$IMG_PART_DISKGUID" ./scripts/gen_image_generic.sh efi-vga-big.img 32 tmp 320 ../x86_64-uclibc/root.fs 256 1780
rm -rf imp
mkdir -p img
cp boot/grub/boot.img img
cp boot/grub/core.img img
echo "(hd0) `pwd`/efi-vga-big.img" > img/device.map

grub-tools/grub-bios-setup -m "img/device.map" -d "img" \
		-r "hd0,gpt1" \
		efi-vga-big.img

./trunc efi-vga-big.img efi-vga-big.tmp
mv -f efi-vga-big.tmp efi-vga-big.img 
