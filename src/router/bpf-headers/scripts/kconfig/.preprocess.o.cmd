savedcmd_scripts/kconfig/preprocess.o := /home/xfs/openwrt/lede/openwrt/staging_dir/host/bin/gcc -Wp,-MMD,scripts/kconfig/.preprocess.o.d -Wall -Wmissing-prototypes -Wstrict-prototypes -O2 -fomit-frame-pointer -std=gnu11   -I ./scripts/include -I/home/xfs/openwrt/lede/openwrt/staging_dir/host/include  -c -o scripts/kconfig/preprocess.o scripts/kconfig/preprocess.c

source_scripts/kconfig/preprocess.o := scripts/kconfig/preprocess.c

deps_scripts/kconfig/preprocess.o := \
  scripts/include/array_size.h \
  scripts/include/list.h \
  scripts/include/list_types.h \
  scripts/include/xalloc.h \
  scripts/kconfig/internal.h \
  scripts/include/hashtable.h \
  scripts/kconfig/lkc.h \
    $(wildcard include/config/prefix) \
  scripts/kconfig/expr.h \
  scripts/kconfig/lkc_proto.h \
  scripts/kconfig/preprocess.h \

scripts/kconfig/preprocess.o: $(deps_scripts/kconfig/preprocess.o)

$(deps_scripts/kconfig/preprocess.o):
