#
# linux/arch/arm/fastfpe/Makefile
#
# Copyright (C) Peter Teichmann
#

AFLAGS_CPDO.o := -march=armv3m

obj-y	:=
obj-m	:=
obj-n	:=
obj-	:=

fastfpe-objs := module.o round.o CPDT.o CPRT.o CPDO.o entry.o

obj-$(CONFIG_FPE_FASTFPE) += fastfpe.o
