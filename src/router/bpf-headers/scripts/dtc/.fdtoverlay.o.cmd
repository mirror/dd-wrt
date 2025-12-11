savedcmd_scripts/dtc/fdtoverlay.o := /home/xfs/openwrt/lede/openwrt/staging_dir/host/bin/gcc -Wp,-MMD,scripts/dtc/.fdtoverlay.o.d -Wall -Wmissing-prototypes -Wstrict-prototypes -O2 -fomit-frame-pointer -std=gnu11   -I ./scripts/include -I/home/xfs/openwrt/lede/openwrt/staging_dir/host/include -I scripts/dtc/libfdt -DNO_YAML  -c -o scripts/dtc/fdtoverlay.o scripts/dtc/fdtoverlay.c

source_scripts/dtc/fdtoverlay.o := scripts/dtc/fdtoverlay.c

deps_scripts/dtc/fdtoverlay.o := \
  scripts/dtc/libfdt/libfdt.h \
  scripts/dtc/libfdt/libfdt_env.h \
  scripts/dtc/libfdt/fdt.h \
  scripts/dtc/util.h \

scripts/dtc/fdtoverlay.o: $(deps_scripts/dtc/fdtoverlay.o)

$(deps_scripts/dtc/fdtoverlay.o):
