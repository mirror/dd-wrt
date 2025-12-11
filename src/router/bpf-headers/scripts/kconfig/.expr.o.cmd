savedcmd_scripts/kconfig/expr.o := /home/xfs/openwrt/lede/openwrt/staging_dir/host/bin/gcc -Wp,-MMD,scripts/kconfig/.expr.o.d -Wall -Wmissing-prototypes -Wstrict-prototypes -O2 -fomit-frame-pointer -std=gnu11   -I ./scripts/include -I/home/xfs/openwrt/lede/openwrt/staging_dir/host/include  -c -o scripts/kconfig/expr.o scripts/kconfig/expr.c

source_scripts/kconfig/expr.o := scripts/kconfig/expr.c

deps_scripts/kconfig/expr.o := \
  scripts/include/hash.h \
  scripts/include/xalloc.h \
  scripts/kconfig/internal.h \
  scripts/include/hashtable.h \
  scripts/include/array_size.h \
  scripts/include/list.h \
  scripts/include/list_types.h \
  scripts/kconfig/lkc.h \
    $(wildcard include/config/prefix) \
  scripts/kconfig/expr.h \
  scripts/kconfig/lkc_proto.h \

scripts/kconfig/expr.o: $(deps_scripts/kconfig/expr.o)

$(deps_scripts/kconfig/expr.o):
