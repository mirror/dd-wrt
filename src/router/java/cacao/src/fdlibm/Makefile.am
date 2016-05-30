## src/fdlibm/Makefile.am
##
## Copyright (C) 1996-2013
## CACAOVM - Verein zur Foerderung der freien virtuellen Maschine CACAO
##
## This file is part of CACAO.
##
## This program is free software; you can redistribute it and/or
## modify it under the terms of the GNU General Public License as
## published by the Free Software Foundation; either version 2, or (at
## your option) any later version.
##
## This program is distributed in the hope that it will be useful, but
## WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
## General Public License for more details.
##
## You should have received a copy of the GNU General Public License
## along with this program; if not, write to the Free Software
## Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
## 02110-1301, USA.
##
## Contact: cacao@cacaojvm.org
##
## Authors: Christian Thalinger
##
##
AM_CPPFLAGS += -I$(top_builddir)

LIBS =

noinst_LTLIBRARIES = \
	libfdlibm.la 

libfdlibm_la_SOURCES = \
	e_fmod.c \
	fdlibm.h \
	ieeefp.h \
	mprec.h \
	s_copysign.c \
	s_finite.c \
	w_fmod.c

if ENABLE_JAVAME_CLDC1_1
libfdlibm_la_SOURCES += \
	e_rem_pio2.c \
	e_sqrt.c \
	k_cos.c \
	k_rem_pio2.c \
	k_sin.c \
	k_tan.c \
	s_ceil.c \
	s_cos.c \
	s_floor.c \
	s_sin.c \
	s_tan.c \
	w_sqrt.c
endif


## Local variables:
## mode: Makefile
## indent-tabs-mode: t
## c-basic-offset: 4
## tab-width: 8
## compile-command: "automake --add-missing"
## End:
