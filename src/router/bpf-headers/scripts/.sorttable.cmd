savedcmd_scripts/sorttable := /home/xfs/openwrt/lede/openwrt/staging_dir/host/bin/gcc -Wp,-MMD,scripts/.sorttable.d -Wall -Wmissing-prototypes -Wstrict-prototypes -O2 -fomit-frame-pointer -std=gnu11   -I ./scripts/include -I/home/xfs/openwrt/lede/openwrt/staging_dir/host/include -I./tools/include   -o scripts/sorttable scripts/sorttable.c -L/home/xfs/openwrt/lede/openwrt/staging_dir/host/lib -lpthread

source_scripts/sorttable := scripts/sorttable.c

deps_scripts/sorttable := \
  /home/xfs/openwrt/lede/openwrt/staging_dir/host/include/elf.h \
  tools/include/tools/be_byteshift.h \
  tools/include/tools/le_byteshift.h \
  scripts/sorttable.h \

scripts/sorttable: $(deps_scripts/sorttable)

$(deps_scripts/sorttable):
