deps_config := \
	extra/Configs/Config.in.arch \
	extra/Configs/Config.v850 \
	extra/Configs/Config.sparc \
	extra/Configs/Config.sh \
	extra/Configs/Config.powerpc \
	extra/Configs/Config.mips \
	extra/Configs/Config.microblaze \
	extra/Configs/Config.m68k \
	extra/Configs/Config.i960 \
	extra/Configs/Config.i386 \
	extra/Configs/Config.h8300 \
	extra/Configs/Config.e1 \
	extra/Configs/Config.cris \
	extra/Configs/Config.arm \
	extra/Configs/Config.alpha \
	extra/Configs/Config.in

.config include/bits/uClibc_config.h: $(deps_config)

$(deps_config):
