echo "#define BUILD_DATE \"$(date +%D)\"" > build.h 

REV=`svn info |grep Revision | cut -f 2 -d ' '`

export PATH=/xfs/toolchains/toolchain-mipsel_gcc4.1.2/bin/:$PATH


cd ../src

cd router

## use your desired config file here
cp ./configs/broadcom_K26/.config_mini.v24-K26 .config

## uncomment next line for 60K nvram builds, e.g. e2000, e3000...
# echo CONGIG_NVRAM_60K=y >> .config


make -f Makefile.brcm26 clean all install


cd mipsel-uclibc

cp dd-wrt.v24-K26.trx ~/dd-wrt/GruppenLW/dd-wrt.v24-${REV}_NEWD-2_K2.6_mini.bin

## uncomment next line for 60K nvram builds, e.g. e2000, e3000...
# cp dd-wrt.v24-K26.trx ~/dd-wrt/GruppenLW/dd-wrt.v24-${REV}_NEWD-2_K2.6_mini-e2k-e3k.bin
