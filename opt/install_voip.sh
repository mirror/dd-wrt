cd ../src
#cp cy_voip.mak cy_conf.mak
#cp cy_voip.h cy_conf.h
#make clean
cd router
make clean

cp .config_voip .config
#cp Makefile.standard Makefile
#copy config.normal .config
rm -rf mipsel-uclibc/install
make rc-clean
make services-clean
make shared-clean
make httpd-clean
rm busybox/busybox
rm busybox/applets/busybox.o
cd ..
make
cd ../opt
mkdir ../src/router/mipsel-uclibc/target/etc/config
#mkdir ../src/router/mipsel-uclibc/target/etc/langpack
./sstrip/sstrip ../src/router/mipsel-uclibc/target/bin/*
./sstrip/sstrip ../src/router/mipsel-uclibc/target/sbin/*
./sstrip/sstrip ../src/router/mipsel-uclibc/target/usr/sbin/*

cp ./bin/ipkg ../src/router/mipsel-uclibc/target/bin

cp ./libgcc/* ../src/router/mipsel-uclibc/target/lib
cd ../src/router/mipsel-uclibc/target/lib
ln -s libgcc_s.so.1 libgcc_s.so
cd ../../../../../opt
cp ./etc/preinit ../src/router/mipsel-uclibc/target/etc
cp ./etc/postinit ../src/router/mipsel-uclibc/target/etc
cp ./etc/ipkg.conf ../src/router/mipsel-uclibc/target/etc
cp ./etc/config/* ../src/router/mipsel-uclibc/target/etc/config
cp ./usr/lib/smb.conf ../src/router/mipsel-uclibc/target/usr/lib
cd ../src/router/mipsel-uclibc/target/www

ln -s ../tmp/smbshare smb

cd ../../../../../opt

./strip_libs.sh

# make language packs
#diff -r -a -w ./lang/spanish/www ../src/router/www > lang/langpacks/spanish.diff
#gzip -9 lang/langpacks/spanish.diff

#copy language packs to destination
#cp ./lang/langpacks/* ../src/router/mipsel-uclibc/target/langpacks
#cp ./lang/* ../src/router/mipsel-uclibc/target/etc/langpack
../src/linux/brcm/linux.v23/scripts/squashfs/mksquashfs-lzma ../src/router/mipsel-uclibc/target target.squashfs -noappend -root-owned -le
./make_kernel.sh
../tools/trx -o dd-wrt.v23_voip.trx ./loader-0.02/loader.gz ../src/router/mipsel-uclibc/vmlinuz -a 1024 target.squashfs
../tools/trx_gs -o dd-wrt.v23_voip_gs.trx ./loader-0.02/loader.gz ../src/router/mipsel-uclibc/vmlinuz target.squashfs
./asus/asustrx -p WL500gx -v 1.9.2.7 -o dd-wrt.v23_voip_asus.trx ./loader-0.02/loader.gz ../src/router/mipsel-uclibc/vmlinuz target.squashfs
#add pattern

./tools/addpattern -4 -p W54G -v v4.20.6 -i dd-wrt.v23_voip.trx -o dd-wrt.v23_voip_wrt54g.bin -g
./tools/addpattern -4 -p W54S -v v4.70.6 -i dd-wrt.v23_voip.trx -o dd-wrt.v23_voip_wrt54gs.bin -g
./tools/addpattern -4 -p W54U -v v4.70.6 -i dd-wrt.v23_voip.trx -o dd-wrt.v23_voip_wrtsl54gs.bin -g
./tools/addpattern -4 -p W54s -v v1.05.0 -i dd-wrt.v23_voip.trx -o dd-wrt.v23_voip_wrt54gsv4.bin -g


cp dd-wrt.v23_voip.trx ~/GruppenLW/dd-wrt.v23_voip.bin
#cp dd-wrt.v23.prefinal5_asus.trx ~/GruppenLW