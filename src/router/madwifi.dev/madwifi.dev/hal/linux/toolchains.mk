TOOLCHAIN_ROOT=/home/nbd/toolchains

ifneq (,$(findstring mipsisa32,$(TARGET)))
ifneq (,$(findstring -be-,$(TARGET)))
TOOLPREFIX=/xfs/toolchains/toolchain-mips_r2_gcc-4.7-linaro_uClibc-0.9.33.2/bin/mips-linux-
else
TOOLPREFIX=/opt/4.1.1/bin/mipsel-linux-
endif
endif

ifneq (,$(findstring i386,$(TARGET)))
TOOLPREFIX=/xfs/toolchains/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/bin/i486-linux-
endif
ifneq (,$(findstring mips64,$(TARGET)))
TOOLPREFIX=/xfs/toolchains/toolchain-mips64_octeonplus_64_gcc-7.4.0_musl/bin/mips64-linux-
endif
ifneq (,$(findstring x86_64,$(TARGET)))
TOOLPREFIX=/xfs/toolchains/toolchain-x86_64_gcc-10.2.0_musl/bin/x86_64-linux-
endif
ifneq (,$(findstring powerpc-boese,$(TARGET)))
TOOLPREFIX=/xfs/toolchains/toolchain-powerpc_e300c3_gcc-4.8-linaro_uClibc-0.9.33.2/bin/powerpc-linux-
#TOOLPREFIX=/opt/staging_dir_powerpc/bin/powerpc-linux-
endif
ifneq (,$(findstring powerpc-be,$(TARGET)))
TOOLPREFIX=/home/dd-wrt2/i+me/powerpc/bin/powerpc-603e-linux-gnu-
endif
ifneq (,$(findstring ap51,$(TARGET)))
TOOLPREFIX=/xfs/toolchains/toolchain-mips_mips32_gcc-8.2.0_musl/bin/mips-linux-
endif
ifneq (,$(findstring ar7100,$(TARGET)))
TOOLPREFIX=/xfs/toolchains/toolchain-mips_24kc_gcc-8.2.0_musl/bin/mips-linux-
endif
ifneq (,$(findstring ls2,$(TARGET)))
TOOLPREFIX=/xfs/toolchains/toolchain-mips_mips32_gcc-8.2.0_musl/bin/mips-linux-
endif
ifneq (,$(findstring bwrg1000,$(TARGET)))
TOOLPREFIX=/xfs/toolchains/toolchain-mips_mips32_gcc-8.2.0_musl/bin/mips-linux-
endif
ifneq (,$(findstring bs2,$(TARGET)))
TOOLPREFIX=/xfs/toolchains/toolchain-mips_mips32_gcc-8.2.0_musl/bin/mips-linux-
endif
ifneq (,$(findstring eoc2610,$(TARGET)))
TOOLPREFIX=/xfs/toolchains/toolchain-mips_mips32_gcc-8.2.0_musl/bin/mips-linux-
endif
ifneq (,$(findstring eoc5610,$(TARGET)))
TOOLPREFIX=/xfs/toolchains/toolchain-mips_mips32_gcc-8.2.0_musl/bin/mips-linux-
endif
ifneq (,$(findstring ls5,$(TARGET)))
TOOLPREFIX=/xfs/toolchains/toolchain-mips_mips32_gcc-8.2.0_musl/bin/mips-linux-
endif
ifneq (,$(findstring ap61,$(TARGET)))
TOOLPREFIX=/xfs/toolchains/toolchain-mips_mips32_gcc-8.2.0_musl/bin/mips-linux-
endif
ifneq (,$(findstring ap30,$(TARGET)))
TOOLPREFIX=/xfs/toolchains/toolchain-mips_mips32_gcc-8.2.0_musl/bin/mips-linux-
endif
ifneq (,$(findstring ap43,$(TARGET)))
TOOLPREFIX=/xfs/toolchains/toolchain-mips_mips32_gcc-8.2.0_musl/bin/mips-linux-
endif
ifneq (,$(findstring xscale,$(TARGET)))
TOOLPREFIX=/xfs/toolchains/toolchain-armeb_xscale_gcc-8.2.0_musl/bin/armeb-linux-
endif
ifneq (,$(findstring arm,$(TARGET)))
TOOLPREFIX=/xfs/toolchains/toolchain-arm_gcc4.2.3/bin/arm-linux-
endif
ifneq (,$(findstring laguna,$(TARGET)))
TOOLPREFIX=/xfs/toolchains/toolchain-arm_mpcore+vfp_gcc-12.1.0_musl_eabi/bin/arm-linux-
endif
ifneq (,$(findstring adm5120,$(TARGET)))
TOOLPREFIX=/xfs/toolchains/toolchain-mipsel_mips32_gcc-8.2.0_musl/bin/mipsel-linux-
endif
ifneq (,$(findstring arm64,$(TARGET)))
TOOLPREFIX=/xfs/toolchains/toolchain-aarch64_generic_gcc-12.1.0_musl/bin/aarch64-linux-
endif

