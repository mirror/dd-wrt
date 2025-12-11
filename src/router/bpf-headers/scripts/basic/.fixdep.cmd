savedcmd_scripts/basic/fixdep := /home/xfs/openwrt/lede/openwrt/staging_dir/host/bin/gcc -Wp,-MMD,scripts/basic/.fixdep.d -Wall -Wmissing-prototypes -Wstrict-prototypes -O2 -fomit-frame-pointer -std=gnu11   -I ./scripts/include -I/home/xfs/openwrt/lede/openwrt/staging_dir/host/include    -o scripts/basic/fixdep scripts/basic/fixdep.c -L/home/xfs/openwrt/lede/openwrt/staging_dir/host/lib 

source_scripts/basic/fixdep := scripts/basic/fixdep.c

deps_scripts/basic/fixdep := \
    $(wildcard include/config/HIS_DRIVER) \
    $(wildcard include/config/MY_OPTION) \
    $(wildcard include/config/FOO) \
  scripts/include/xalloc.h \

scripts/basic/fixdep: $(deps_scripts/basic/fixdep)

$(deps_scripts/basic/fixdep):
