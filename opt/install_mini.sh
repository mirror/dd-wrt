cd ../src
#cp cy_std.mak cy_conf.mak
#cp cy_std.h cy_conf.h

cd router

#make clean
cp .config_mini .config
#cp Makefile.standard Makefile

cd ..
make clean
make
cd ../opt
mkdir ../src/router/mipsel-uclibc/target/etc/config
mkdir ../src/router/mipsel-uclibc/target/etc/langpack
./sstrip/sstrip ../src/router/mipsel-uclibc/target/bin/busybox
./sstrip/sstrip ../src/router/mipsel-uclibc/target/sbin/rc
./sstrip/sstrip ../src/router/mipsel-uclibc/target/usr/sbin/*

cp ./bin/ipkg ../src/router/mipsel-uclibc/target/bin

cp ./libgcc/* ../src/router/mipsel-uclibc/target/lib
cd ../src/router/mipsel-uclibc/target/lib
ln -s libgcc_s.so.1 libgcc_s.so
cd ../../../../../opt
cp ./etc/preinit ../src/router/mipsel-uclibc/target/etc
cp ./etc/postinit ../src/router/mipsel-uclibc/target/etc
cp ./etc/ipkg.conf ../src/router/mipsel-uclibc/target/etc
cp ./etc/mini/config/* ../src/router/mipsel-uclibc/target/etc/config
cd ../src/router/mipsel-uclibc/target/www

cd ../../../../../opt

rm ../src/router/mipsel-uclibc/target/lib/modules/2.4.33-pre1/kernel/fs/mmc/mmc.o
rm ../src/router/mipsel-uclibc/target/lib/modules/2.4.33-pre1/kernel/fs/fat/*
rm ../src/router/mipsel-uclibc/target/lib/modules/2.4.33-pre1/kernel/fs/smbfs/*
rm ../src/router/mipsel-uclibc/target/lib/modules/2.4.33-pre1/kernel/fs/vfat/*
rm ../src/router/mipsel-uclibc/target/lib/modules/2.4.33-pre1/kernel/fs/msdos/*
rm ../src/router/mipsel-uclibc/target/lib/modules/2.4.33-pre1/kernel/net/ipv6/*


./strip_libs.sh


# make language packs
#diff -r -a -w ./lang/spanish/www ../src/router/www > lang/langpacks/spanish.diff
#gzip -9 lang/langpacks/spanish.diff

#copy language packs to destination
#cp ./lang/langpacks/* ../src/router/mipsel-uclibc/target/langpacks
cp ./lang/* ../src/router/mipsel-uclibc/target/etc/langpack

#vfs option
#cd ../src/router/mipsel-uclibc/target/www
#mkvfs ../usr/lib/vfs.lib *.asp
#rm *.asp
#cd ../../../../../opt


../src/linux/linux.v23/scripts/squashfs/mksquashfs-lzma ../src/router/mipsel-uclibc/target target.squashfs -noappend -root-owned -le
./make_kernel.sh
../tools/trx -o dd-wrt.v23_mini.trx ./loader-0.02/loader.gz ../src/router/mipsel-uclibc/vmlinuz target.squashfs
../tools/trx_gs -o dd-wrt.v23_gs_mini.trx ./loader-0.02/loader.gz ../src/router/mipsel-uclibc/vmlinuz target.squashfs
./asus/asustrx -p WL500gx -v 1.9.2.7 -o dd-wrt.v23_mini_asus.trx ./loader-0.02/loader.gz ../src/router/mipsel-uclibc/vmlinuz target.squashfs
#add pattern
./tools/addpattern -4 -p W54G -v v4.20.6 -i dd-wrt.v23_mini.trx -o dd-wrt.v23_mini_wrt54g.bin -g
./tools/addpattern -4 -p W54S -v v4.70.6 -i dd-wrt.v23_mini.trx -o dd-wrt.v23_mini_wrt54gs.bin -g
./tools/addpattern -4 -p W54s -v v1.05.0 -i dd-wrt.v23_mini.trx -o dd-wrt.v23_mini_wrt54gsv4.bin -g

#./tools/addpattern -i dd-wrt.v23_mini.trx -o dd-wrt.v23_mini_wrt54gs.bin -2 -g
#sed -e  1s,^W54S,W54G, < dd-wrt.v23_mini_wrt54gs.bin > dd-wrt.v23_mini_wrt54g.bin 

cp dd-wrt.v23_mini_asus.trx /GruppenLW
cp dd-wrt.v23_mini_wrt54g.bin /GruppenLW
cp dd-wrt.v23_mini_wrt54gs.bin /GruppenLW
cp dd-wrt.v23_mini_wrt54gsv4.bin /GruppenLW
cp dd-wrt.v23_mini.trx /GruppenLW/dd-wrt.v23_mini_generic.bin
./tools/motorola-bin dd-wrt.v23_mini.trx dd-wrt.bin
cp dd-wrt.bin /GruppenLW/dd-wrt.v23_mini_moto.bin

#cp dd-wrt.v23.prefinal5_asus.trx /GruppenLW