savedcmd_arch/mips/boot/tools/relocs_main.o := /home/xfs/openwrt/lede/openwrt/staging_dir/host/bin/gcc -Wp,-MMD,arch/mips/boot/tools/.relocs_main.o.d -Wall -Wmissing-prototypes -Wstrict-prototypes -O2 -fomit-frame-pointer -std=gnu11   -I ./scripts/include -I/home/xfs/openwrt/lede/openwrt/staging_dir/host/include  -c -o arch/mips/boot/tools/relocs_main.o arch/mips/boot/tools/relocs_main.c

source_arch/mips/boot/tools/relocs_main.o := arch/mips/boot/tools/relocs_main.c

deps_arch/mips/boot/tools/relocs_main.o := \
  /home/xfs/openwrt/lede/openwrt/staging_dir/host/include/elf.h \
  arch/mips/boot/tools/relocs.h \

arch/mips/boot/tools/relocs_main.o: $(deps_arch/mips/boot/tools/relocs_main.o)

$(deps_arch/mips/boot/tools/relocs_main.o):
