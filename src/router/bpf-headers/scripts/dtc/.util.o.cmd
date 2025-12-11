savedcmd_scripts/dtc/util.o := /home/xfs/openwrt/lede/openwrt/staging_dir/host/bin/gcc -Wp,-MMD,scripts/dtc/.util.o.d -Wall -Wmissing-prototypes -Wstrict-prototypes -O2 -fomit-frame-pointer -std=gnu11   -I ./scripts/include -I/home/xfs/openwrt/lede/openwrt/staging_dir/host/include -I scripts/dtc/libfdt -DNO_YAML  -c -o scripts/dtc/util.o scripts/dtc/util.c

source_scripts/dtc/util.o := scripts/dtc/util.c

deps_scripts/dtc/util.o := \
  scripts/dtc/libfdt/libfdt.h \
  scripts/dtc/libfdt/libfdt_env.h \
  scripts/dtc/libfdt/fdt.h \
  scripts/dtc/util.h \
  scripts/dtc/version_gen.h \

scripts/dtc/util.o: $(deps_scripts/dtc/util.o)

$(deps_scripts/dtc/util.o):
