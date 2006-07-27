#!/bin/sh

# Report ARCH and TARGET for the given Linux .config file.
# Argument 1 should be path to .config

test -r "$1" || { echo "" ""; exit 1; }
. "$1"

# Calculate ARCH first.  This covers all architectures supported by
# Linux 2.4 and 2.6, whether they are supported by HAL or not.
# Note that more specific entries must follow less specific ones, e.g.
# CONFIG_X86_64 overrides CONFIG_X86.
eval ARCH_${CONFIG_ALPHA}=alpha
eval ARCH_${CONFIG_ARM}=arm
eval ARCH_${CONFIG_ARM26}=arm26
eval ARCH_${CONFIG_CRIS}=cris
eval ARCH_${CONFIG_FRV}=frv
eval ARCH_${CONFIG_H8300}=h8300
eval ARCH_${CONFIG_X86}=i386
eval ARCH_${CONFIG_IA64}=ia64
eval ARCH_${CONFIG_M32R}=m32r
eval ARCH_${CONFIG_M68K}=m68k
eval ARCH_${CONFIG_M68KNOMMU}=m68knommu
eval ARCH_${CONFIG_MIPS}=mips
eval ARCH_${CONFIG_MIPS64}=mips64
eval ARCH_${CONFIG_PARISC}=parisc
eval ARCH_${CONFIG_PPC}=ppc
eval ARCH_${CONFIG_PPC64}=ppc64
eval ARCH_${CONFIG_PPC_MERGE}=powerpc
eval ARCH_${CONFIG_ARCH_S390}=s390
eval ARCH_${CONFIG_ARCH_S390X}=s390x
eval ARCH_${CONFIG_SUPERH}=sh
eval ARCH_${CONFIG_CPU_SH5}=sh64
eval ARCH_${CONFIG_SUPERH64}=sh64
eval ARCH_${CONFIG_SPARC32}=sparc
eval ARCH_${CONFIG_SPARC64}=sparc64
eval ARCH_${CONFIG_UML}=um
eval ARCH_${CONFIG_V850}=v850
eval ARCH_${CONFIG_X86_64}=x86_64
eval ARCH_${CONFIG_XTENSA}=xtensa
ARCH=${ARCH_y}
test -z "${ARCH}" && { echo "" ""; exit 1; }


# Determine endianess.  Note that it's not indicated for some CPUs at
# all, so this value is only valid for certain processors.
case ${ARCH} in
	arm*) ENDIAN=le;;
	mips*) ENDIAN=be;;
esac

if test -n "${CONFIG_CPU_BIG_ENDIAN}${CONFIG_BIG_ENDIAN}"; then
	ENDIAN=be
fi

if test -n "${CONFIG_CPU_LITTLE_ENDIAN}${CONFIG_LITTLE_ENDIAN}"; then
	ENDIAN=le
fi


# Determine the target (i.e. which HAL to use).
# The default is ${ARCH}-elf
eval TARGET_${CONFIG_CPU_32v4}=armv4-${ENDIAN}-elf
eval TARGET_${CONFIG_CPU_MIPS32_R1}=mips1-${ENDIAN}-elf
eval TARGET_${CONFIG_CPU_MIPS32_R2}=mips-${ENDIAN}-elf
eval TARGET_${CONFIG_CPU_R4X00}=mipsisa32-${ENDIAN}-elf
eval TARGET_${CONFIG_CPU_TX49XX}=mipsisa32-${ENDIAN}-elf
eval TARGET_${CONFIG_PPC32}=powerpc-be-elf
eval TARGET_${CONFIG_CPU_SH4}=sh4-le-elf
eval TARGET_${CONFIG_SPARC32}=sparc-be-elf
eval TARGET_${CONFIG_SPARC64}=sparc64-be-elf
eval TARGET_${CONFIG_CPU_SA110}=xscale-${ENDIAN}-elf
eval TARGET_${CONFIG_CPU_SA1100}=xscale-${ENDIAN}-elf
eval TARGET_${CONFIG_CPU_XSCALE}=xscale-${ENDIAN}-elf

if test -n "${TARGET_y}"; then
	TARGET="${TARGET_y}"
else
	TARGET="${ARCH}-elf"
fi

echo "${ARCH}" "${TARGET}"
exit 0
