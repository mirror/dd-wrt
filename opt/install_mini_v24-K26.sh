echo "#define BUILD_DATE \"$(date +%D)\"" > build.h 

REV=`svn info |grep Revision | cut -f 2 -d ' '`

export PATH=/xfs/toolchains/toolchain-mipsel_gcc-linaro_uClibc-0.9.32/bin:$PATH


cd ../src/router


## use your desired config file here... 
cp ./configs/broadcom_K26/.config_mini.v24-K26 .config

## uncomment next line for 60K nvram builds, e.g. e2000, e3000, e3200, e4200...
# echo CONFIG_NVRAM_60K=y >> .config
## uncomment next line for 64K nvram builds, e.g. wndr4000...
# echo CONFIG_NVRAM_64K=y >> .config


make -f Makefile.brcm26 clean all install


cd mipsel-uclibc

## uncomment next line for normal (32k nvram) builds
cp dd-wrt.v24-K26.trx ~/dd-wrt/GruppenLW/dd-wrt.v24-${REV}_NEWD-2_K2.6_mini.bin

## uncomment next line for 60K nvram builds, e.g. e2000, e3000, e3200, e4200...
# cp dd-wrt.v24-K26_nv60k.bin ~/dd-wrt/GruppenLW/dd-wrt.v24-${REV}_NEWD-2_K2.6_mini-nv60k.bin

## uncomment next line for 64K nvram builds, e.g. wndr4000...
# cp dd-wrt.v24-K26_nv64k.bin ~/dd-wrt/GruppenLW/dd-wrt.v24-${REV}_NEWD-2_K2.6_mini-nv64k.bin