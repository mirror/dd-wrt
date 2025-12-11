savedcmd_scripts/kallsyms := /home/xfs/openwrt/lede/openwrt/staging_dir/host/bin/gcc -Wp,-MMD,scripts/.kallsyms.d -Wall -Wmissing-prototypes -Wstrict-prototypes -O2 -fomit-frame-pointer -std=gnu11   -I ./scripts/include -I/home/xfs/openwrt/lede/openwrt/staging_dir/host/include    -o scripts/kallsyms scripts/kallsyms.c -L/home/xfs/openwrt/lede/openwrt/staging_dir/host/lib 

source_scripts/kallsyms := scripts/kallsyms.c

deps_scripts/kallsyms := \
  scripts/include/xalloc.h \

scripts/kallsyms: $(deps_scripts/kallsyms)

$(deps_scripts/kallsyms):
