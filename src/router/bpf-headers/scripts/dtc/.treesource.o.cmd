savedcmd_scripts/dtc/treesource.o := /home/xfs/openwrt/lede/openwrt/staging_dir/host/bin/gcc -Wp,-MMD,scripts/dtc/.treesource.o.d -Wall -Wmissing-prototypes -Wstrict-prototypes -O2 -fomit-frame-pointer -std=gnu11   -I ./scripts/include -I/home/xfs/openwrt/lede/openwrt/staging_dir/host/include -I scripts/dtc/libfdt -DNO_YAML  -c -o scripts/dtc/treesource.o scripts/dtc/treesource.c

source_scripts/dtc/treesource.o := scripts/dtc/treesource.c

deps_scripts/dtc/treesource.o := \
  scripts/dtc/dtc.h \
  scripts/dtc/libfdt/libfdt_env.h \
  scripts/dtc/libfdt/fdt.h \
  scripts/dtc/util.h \
  scripts/dtc/srcpos.h \

scripts/dtc/treesource.o: $(deps_scripts/dtc/treesource.o)

$(deps_scripts/dtc/treesource.o):
