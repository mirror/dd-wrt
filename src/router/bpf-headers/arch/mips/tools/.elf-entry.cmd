savedcmd_arch/mips/tools/elf-entry := /home/xfs/openwrt/lede/openwrt/staging_dir/host/bin/gcc -Wp,-MMD,arch/mips/tools/.elf-entry.d -Wall -Wmissing-prototypes -Wstrict-prototypes -O2 -fomit-frame-pointer -std=gnu11   -I ./scripts/include -I/home/xfs/openwrt/lede/openwrt/staging_dir/host/include    -o arch/mips/tools/elf-entry arch/mips/tools/elf-entry.c -L/home/xfs/openwrt/lede/openwrt/staging_dir/host/lib 

source_arch/mips/tools/elf-entry := arch/mips/tools/elf-entry.c

deps_arch/mips/tools/elf-entry := \
  /home/xfs/openwrt/lede/openwrt/staging_dir/host/include/byteswap.h \
  /home/xfs/openwrt/lede/openwrt/staging_dir/host/include/elf.h \
  /home/xfs/openwrt/lede/openwrt/staging_dir/host/include/endian.h \

arch/mips/tools/elf-entry: $(deps_arch/mips/tools/elf-entry)

$(deps_arch/mips/tools/elf-entry):
