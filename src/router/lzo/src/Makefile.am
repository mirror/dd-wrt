## Process this file with automake to create Makefile.in
#
# Copyright (C) 1996-2002 Markus F.X.J. Oberhumer
#

AUTOMAKE_OPTIONS = 1.6


# /***********************************************************************
# // compiler and architecture specific CFLAGS
# ************************************************************************/

LZO_CFLAGS_GCC   =
LZO_CFLAGS_GCC_O =
LZO_CFLAGS_GCC_W = \
    -Wpointer-arith -Wstrict-prototypes -Wmissing-prototypes \
    -Wno-uninitialized

LZO_CFLAGS_GCC_i386   =
LZO_CFLAGS_GCC_i386_O = $(CFLAGS_GCC_OMIT_FRAME_POINTER) # -fforce-addr
LZO_CFLAGS_GCC_i386_W =

L_CFLAGS   = $(LZO_CFLAGS_@MFX_CC@) $(LZO_CFLAGS_@MFX_CC@_@MFX_ARCH@)
L_CFLAGS_O = $(LZO_CFLAGS_@MFX_CC@_O) $(LZO_CFLAGS_@MFX_CC@_@MFX_ARCH@_O)
L_CFLAGS_W = $(LZO_CFLAGS_@MFX_CC@_W) $(LZO_CFLAGS_@MFX_CC@_@MFX_ARCH@_W)


# /***********************************************************************
# //
# ************************************************************************/

### LZO_DEBUG = -DLZO_DEBUG
### LZO_DEBUG = -DLZO_DEBUG -Wno-error
### LZO_DEBUG = -DLZO_DEBUG -g -fno-omit-frame-pointer -Wno-error
### LZO_EXTRA_CFLAGS = -fbranch-probabilities -Wno-error
### LZO_EXTRA_CFLAGS = -fprofile-arcs -ftest-coverage -O0

INCLUDES = -I$(top_srcdir)/include

ALL_CFLAGS = $(L_CFLAGS) $(CFLAGS_O) $(L_CFLAGS_O) $(CFLAGS_W) $(L_CFLAGS_W) $(CFLAGS_W_PEDANTIC) $(LZO_CFLAGS) $(LZO_EXTRA_CFLAGS) $(LZO_DEBUG)

CFLAGS = @CFLAGS@ -DLZO_BUILD $(ALL_CFLAGS)
CXXFLAGS = @CXXFLAGS@ -DLZO_BUILD $(ALL_CFLAGS)


VPATH = @srcdir@:@srcdir@@SRC_VPATH@


# /***********************************************************************
# // assembler support
# ************************************************************************/

SUFFIXES = .asm .c .i .s .sh

COMPILE_S   = $(CC) $(ASFLAGS) $(DEFS) $(DEFAULT_INCLUDES) $(INCLUDES) $(CPPFLAGS) $(LZO_DEBUG)
COMPILE_ASM = $(NASM) -f elf

.s.o:
	$(COMPILE_S) -c $<

.s.lo:
	$(LIBTOOL) --mode=compile $(COMPILE_S) -c $<

.asm.o:
	$(COMPILE_ASM) -o $@ $<

.asm.lo:
	$(LIBTOOL) --mode=compile $(COMPILE_ASM) -o $@ $<


# /***********************************************************************
# // library files
# ************************************************************************/

lib_LTLIBRARIES = liblzo.la

liblzo_la_LDFLAGS = -version-info 1:0:0

liblzo_la_SOURCES = \
    alloc.c io.c lzo_crc.c lzo_dll.c lzo_init.c lzo_ptr.c lzo_str.c lzo_util.c \
    lzo1.c lzo1_99.c \
    lzo1a.c lzo1a_99.c \
    lzo1b_1.c lzo1b_2.c lzo1b_3.c lzo1b_4.c lzo1b_5.c lzo1b_6.c \
    lzo1b_7.c lzo1b_8.c lzo1b_9.c lzo1b_99.c lzo1b_9x.c \
    lzo1b_cc.c lzo1b_d1.c lzo1b_d2.c lzo1b_rr.c lzo1b_xx.c \
    lzo1c_1.c lzo1c_2.c lzo1c_3.c lzo1c_4.c lzo1c_5.c lzo1c_6.c \
    lzo1c_7.c lzo1c_8.c lzo1c_9.c lzo1c_99.c lzo1c_9x.c \
    lzo1c_cc.c lzo1c_d1.c lzo1c_d2.c lzo1c_rr.c lzo1c_xx.c \
    lzo1f_1.c lzo1f_9x.c lzo1f_d1.c lzo1f_d2.c \
    lzo1x_1.c lzo1x_9x.c lzo1x_d1.c lzo1x_d2.c lzo1x_d3.c \
    lzo1x_o.c lzo1x_1k.c lzo1x_1l.c lzo1x_1o.c \
    lzo1y_1.c lzo1y_9x.c lzo1y_d1.c lzo1y_d2.c lzo1y_d3.c lzo1y_o.c \
    lzo1z_9x.c lzo1z_d1.c lzo1z_d2.c lzo1z_d3.c \
    lzo2a_9x.c lzo2a_d1.c lzo2a_d2.c \
     lzo1_cm.ch lzo1_d.ch lzo1a_cm.ch lzo1a_cr.ch lzo1b_c.ch \
     lzo1b_cm.ch lzo1b_cr.ch lzo1b_d.ch lzo1b_r.ch \
     lzo1b_sm.ch lzo1b_tm.ch lzo1f_d.ch lzo1x_c.ch \
     lzo1x_d.ch lzo1x_oo.ch lzo2a_d.ch lzo_mchw.ch lzo_swd.ch \
      compr1b.h compr1c.h config1.h config1a.h config1b.h config1c.h \
      config1f.h config1x.h config1y.h config1z.h config2a.h fake16.h \
      lzo1a_de.h lzo1b_cc.h lzo1b_de.h lzo1c_cc.h lzo_conf.h lzo_dict.h \
      lzo_ptr.h lzo_util.h stats1a.h stats1b.h stats1c.h

LZO_ASM_OBJECTS_GCC_i386 = \
    lzo1c_s1.lo lzo1c_s2.lo \
    lzo1f_f1.lo lzo1f_f2.lo \
    lzo1x_f1.lo lzo1x_f2.lo lzo1x_s1.lo lzo1x_s2.lo \
    lzo1y_f1.lo lzo1y_f2.lo lzo1y_s1.lo lzo1y_s2.lo

LZO_ASM_OBJECTS = @LZO_ASM_OBJECTS@

liblzo_la_LIBADD = $(LZO_ASM_OBJECTS)
liblzo_la_DEPENDENCIES = $(liblzo_la_LIBADD)


# /***********************************************************************
# // extra targets and dependencies
# ************************************************************************/

.c.i:
	$(CPP) $(DEFS) $(DEFAULT_INCLUDES) $(INCLUDES) $(CPPFLAGS) -o $@ $<

.s.i:
	$(CPP) $(ASFLAGS) $(DEFS) $(DEFAULT_INCLUDES) $(INCLUDES) $(CPPFLAGS) -o $@ $<

dist-hook:
	cp -rpi $(srcdir)/i386 $(distdir)/

@MAINT@@GMAKE_TRUE@override o=.lo
@MAINT@@GMAKE_TRUE@-include $(top_srcdir)/maint/make/src.mk

