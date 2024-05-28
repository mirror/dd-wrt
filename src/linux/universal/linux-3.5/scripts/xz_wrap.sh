#!/bin/sh
#
# This is a wrapper for xz to compress the kernel image using appropriate
# compression options depending on the architecture.
#
# Author: Lasse Collin <lasse.collin@tukaani.org>
#
# This file has been put into the public domain.
# You can do whatever you want with this file.
#

BCJ=
LZMA2OPTS=

case $SRCARCH in
	x86)            BCJ=--x86; LZMA2OPTS=pb=2,lp=2,lc=1 ;;
	x86_64)         BCJ=--x86; LZMA2OPTS=pb=2,lp=2,lc=1 ;;
	powerpc)        BCJ=--powerpc; LZMA2OPTS=pb=2,lp=2,lc=1 ;;
	ia64)           BCJ=--ia64; LZMA2OPTS=pb=4 ;;
	arm)            BCJ=--arm; LZMA2OPTS=pb=2,lp=2,lc=1 ;;
	armeb)          BCJ=--arm; LZMA2OPTS=pb=2,lp=2,lc=1 ;;
	arm64)          BCJ=--arm64; LZMA2OPTS=pb=2,lp=2,lc=1 ;;
	riscv)          BCJ=--riscv; LZMA2OPTS=pb=2,lp=2,lc=1 ;;
	sparc)          BCJ=--sparc; LZMA2OPTS=pb=2,lp=2,lc=1 ;;
	mips)           LZMA2OPTS=pb=2,lp=2,lc=1 ;;
	mips64)         LZMA2OPTS=pb=2,lp=2,lc=1 ;;
esac

exec xz --check=crc32 -9 -e $BCJ --lzma2=$LZMA2OPTS,dict=32MiB
