# Makefile for uClibc's math library
#
# Copyright (C) 2000-2002 Erik Andersen <andersen@uclibc.org>
#
# The routines included in this math library are derived from the
# math library for Apple's MacOS X/Darwin math library, which was
# itself swiped from FreeBSD.  The original copyright information
# is as follows:
# 
#     Copyright (C) 1993 by Sun Microsystems, Inc. All rights reserved.
# 
#     Developed at SunPro, a Sun Microsystems, Inc. business.
#     Permission to use, copy, modify, and distribute this
#     software is freely granted, provided that this notice
#     is preserved.
# 
# It has been ported to work with uClibc and generally behave
# by Erik Andersen <andersen@codepoet.org>
#
# This program is free software; you can redistribute it and/or modify it under
# the terms of the GNU Library General Public License as published by the Free
# Software Foundation; either version 2 of the License, or (at your option) any
# later version.
#
# This program is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE. See the GNU Library General Public License for more
# details.
#
# You should have received a copy of the GNU Library General Public License
# along with this program; if not, write to the Free Software Foundation, Inc.,
# 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA

TOPDIR=../
include $(TOPDIR)Rules.mak

DIRS = 
ifeq ($(strip $(HAS_FPU)),y)
ifeq ($(TARGET_ARCH),$(wildcard $(TARGET_ARCH)))
DIRS = $(TARGET_ARCH)
endif
endif
ALL_SUBDIRS = powerpc


LIBM=libm.a
LIBM_SHARED=libm.so
LIBM_SHARED_FULLNAME=libm-$(MAJOR_VERSION).$(MINOR_VERSION).$(SUBLEVEL).so
CFLAGS+=-D_IEEE_LIBM -D_ISOC99_SOURCE -D_SVID_SOURCE

ifeq ($(strip $(DO_C99_MATH)),y)
CSRC =   e_acos.c e_acosh.c e_asin.c e_atan2.c e_atanh.c e_cosh.c\
         e_exp.c e_fmod.c e_gamma.c e_gamma_r.c e_hypot.c e_j0.c\
         e_j1.c e_jn.c e_lgamma.c e_lgamma_r.c e_log.c e_log10.c\
         e_pow.c e_remainder.c e_rem_pio2.c e_scalb.c e_sinh.c\
         e_sqrt.c k_cos.c k_rem_pio2.c k_sin.c k_standard.c k_tan.c\
         s_asinh.c s_atan.c s_cbrt.c s_ceil.c s_copysign.c s_cos.c\
         s_erf.c s_expm1.c s_fabs.c s_finite.c s_floor.c s_frexp.c\
         s_ilogb.c s_ldexp.c s_lib_version.c s_log1p.c s_logb.c\
         s_matherr.c s_modf.c s_nextafter.c s_rint.c s_scalbn.c\
         s_signgam.c s_significand.c s_sin.c s_tan.c s_tanh.c\
         w_acos.c w_acosh.c w_asin.c w_atan2.c w_atanh.c w_cabs.c\
         w_cosh.c w_drem.c w_exp.c w_fmod.c w_gamma.c w_gamma_r.c\
         w_hypot.c w_j0.c w_j1.c w_jn.c w_lgamma.c w_lgamma_r.c\
         w_log.c w_log10.c w_pow.c w_remainder.c w_scalb.c w_sinh.c\
         w_sqrt.c w_sqrtf.c fpmacros.c nan.c s_ceilf.c s_floorf.c
else
# This list of math functions was taken from POSIX/IEEE 1003.1b-1993
CSRC =   w_acos.c w_asin.c s_atan.c w_atan2.c s_ceil.c s_cos.c \
	 w_cosh.c w_exp.c s_fabs.c s_floor.c w_fmod.c s_frexp.c \
	 s_ldexp.c w_log.c w_log10.c s_modf.c w_pow.c s_sin.c \
	 w_sinh.c w_sqrt.c s_tan.c s_tanh.c 
CSRC+=   s_expm1.c s_scalbn.c s_copysign.c e_acos.c e_asin.c e_atan2.c \
	 k_cos.c e_cosh.c e_exp.c e_fmod.c e_log.c e_log10.c e_pow.c \
	 k_sin.c e_sinh.c e_sqrt.c k_tan.c e_rem_pio2.c k_rem_pio2.c \
	 s_finite.c
endif
COBJS=$(patsubst %.c,%.o, $(CSRC))
OBJS=$(COBJS)


ifneq ($(strip $(UCLIBC_HAS_FLOATS)),y)
all:  clean subdirs
else
all: $(LIBM) subdirs
endif

$(LIBM): ar-target
	@if [ -f $(LIBM) ] ; then \
		set -x -e; \
		$(INSTALL) -d $(TOPDIR)lib; \
		$(RM) $(TOPDIR)lib/$(LIBM); \
		$(INSTALL) -m 644 $(LIBM) $(TOPDIR)lib; \
	fi;

shared: all
	@if [ -f $(LIBM) ] ; then \
	    set -x -e; \
	    $(LD) $(LDFLAGS) -soname=$(LIBM_SHARED).$(MAJOR_VERSION) \
		-o $(LIBM_SHARED_FULLNAME) --whole-archive $(LIBM) \
		--no-whole-archive $(TOPDIR)libc/misc/internals/interp.o \
		-L$(TOPDIR)lib -lc; \
	    $(INSTALL) -d $(TOPDIR)lib; \
	    $(RM) $(TOPDIR)lib/$(LIBM_SHARED_FULLNAME) $(TOPDIR)lib/$(LIBM_SHARED).$(MAJOR_VERSION); \
	    $(INSTALL) -m 644 $(LIBM_SHARED_FULLNAME) $(TOPDIR)lib; \
	    $(LN) -sf $(LIBM_SHARED_FULLNAME) $(TOPDIR)lib/$(LIBM_SHARED); \
	    $(LN) -sf $(LIBM_SHARED_FULLNAME) $(TOPDIR)lib/$(LIBM_SHARED).$(MAJOR_VERSION); \
	fi;

ar-target: $(OBJS)
	$(AR) $(ARFLAGS) $(LIBM) $(OBJS)

$(COBJS): %.o : %.c
	$(CC) $(CFLAGS) -c $< -o $@
	$(STRIPTOOL) -x -R .note -R .comment $*.o

$(OBJ): Makefile

tags:
	ctags -R

clean: subdirs_clean
	$(RM) *.[oa] *~ core $(LIBM_SHARED)* $(LIBM_SHARED_FULLNAME)*

subdirs: $(patsubst %, _dir_%, $(DIRS))
subdirs_clean: $(patsubst %, _dirclean_%, $(ALL_SUBDIRS))

$(patsubst %, _dir_%, $(DIRS)) : $(OBJS)
	$(MAKE) -C $(patsubst _dir_%, %, $@)

$(patsubst %, _dirclean_%, $(ALL_SUBDIRS)) : dummy
	$(MAKE) -C $(patsubst _dirclean_%, %, $@) clean

.PHONY: dummy
