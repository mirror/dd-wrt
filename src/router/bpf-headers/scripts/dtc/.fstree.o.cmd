savedcmd_scripts/dtc/fstree.o := /home/xfs/openwrt/lede/openwrt/staging_dir/host/bin/gcc -Wp,-MMD,scripts/dtc/.fstree.o.d -Wall -Wmissing-prototypes -Wstrict-prototypes -O2 -fomit-frame-pointer -std=gnu11   -I ./scripts/include -I/home/xfs/openwrt/lede/openwrt/staging_dir/host/include -I scripts/dtc/libfdt -DNO_YAML  -c -o scripts/dtc/fstree.o scripts/dtc/fstree.c

source_scripts/dtc/fstree.o := scripts/dtc/fstree.c

deps_scripts/dtc/fstree.o := \
  scripts/dtc/dtc.h \
  scripts/dtc/libfdt/libfdt_env.h \
  scripts/dtc/libfdt/fdt.h \
  scripts/dtc/util.h \

scripts/dtc/fstree.o: $(deps_scripts/dtc/fstree.o)

$(deps_scripts/dtc/fstree.o):
