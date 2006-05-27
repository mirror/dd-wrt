#!/bin/sh

# Report ARCH and TARGET for the given Linux .config file.
# Argument 1 should be path to .config

report () {
	echo "$1 ${2-$1-elf}"
	exit 0
}

test -r "$1" || report "" ""
. "$1"

test "$CONFIG_X86_64" && report x86_64
test "$CONFIG_X86" && report i386
test "$CONFIG_ALPHA" && report alpha
test "$CONFIG_SPARC64" && report sparc64 sparc64-be-elf
test "$CONFIG_PPC32" && test "$CONFIG_PPC_MERGE" && report powerpc powerpc-be-elf
test "$CONFIG_PPC32" && report ppc powerpc-be-elf
test "$CONFIG_CPU_SH4" && report sh sh4-le-elf
test "$CONFIG_CPU_XSCALE" && test "$CONFIG_CPU_BIG_ENDIAN" && report arm xscale-be-elf
test "$CONFIG_CPU_XSCALE" && report arm xscale-le-elf
test "$CONFIG_CPU_32v4" && test "$CONFIG_CPU_BIG_ENDIAN" && report arm armv4-be-elf
test "$CONFIG_CPU_32v4" && report arm armv4-le-elf
test "$CONFIG_MIPS" && test "$CONFIG_CPU_LITTLE_ENDIAN" && report mips mips1-le-elf
test "$CONFIG_MIPS" && report mips mips1-be-elf
report "" ""
