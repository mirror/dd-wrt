savedcmd_scripts/dtc/libfdt/fdt_ro.o := /home/xfs/openwrt/lede/openwrt/staging_dir/host/bin/gcc -Wp,-MMD,scripts/dtc/libfdt/.fdt_ro.o.d -Wall -Wmissing-prototypes -Wstrict-prototypes -O2 -fomit-frame-pointer -std=gnu11   -I ./scripts/include -I/home/xfs/openwrt/lede/openwrt/staging_dir/host/include -I scripts/dtc/libfdt -DNO_YAML  -c -o scripts/dtc/libfdt/fdt_ro.o scripts/dtc/libfdt/fdt_ro.c

source_scripts/dtc/libfdt/fdt_ro.o := scripts/dtc/libfdt/fdt_ro.c

deps_scripts/dtc/libfdt/fdt_ro.o := \
  scripts/dtc/libfdt/libfdt_env.h \
  scripts/dtc/libfdt/fdt.h \
  scripts/dtc/libfdt/libfdt.h \
  scripts/dtc/libfdt/libfdt_internal.h \

scripts/dtc/libfdt/fdt_ro.o: $(deps_scripts/dtc/libfdt/fdt_ro.o)

$(deps_scripts/dtc/libfdt/fdt_ro.o):
