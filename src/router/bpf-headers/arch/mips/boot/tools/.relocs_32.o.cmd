savedcmd_arch/mips/boot/tools/relocs_32.o := /home/xfs/openwrt/lede/openwrt/staging_dir/host/bin/gcc -Wp,-MMD,arch/mips/boot/tools/.relocs_32.o.d -Wall -Wmissing-prototypes -Wstrict-prototypes -O2 -fomit-frame-pointer -std=gnu11   -I ./scripts/include -I/home/xfs/openwrt/lede/openwrt/staging_dir/host/include  -c -o arch/mips/boot/tools/relocs_32.o arch/mips/boot/tools/relocs_32.c

source_arch/mips/boot/tools/relocs_32.o := arch/mips/boot/tools/relocs_32.c

deps_arch/mips/boot/tools/relocs_32.o := \
  arch/mips/boot/tools/relocs.h \
  /home/xfs/openwrt/lede/openwrt/staging_dir/host/include/elf.h \
  arch/mips/boot/tools/relocs.c \
    $(wildcard include/config/RELOCATION_TABLE_SIZE) \

arch/mips/boot/tools/relocs_32.o: $(deps_arch/mips/boot/tools/relocs_32.o)

$(deps_arch/mips/boot/tools/relocs_32.o):
