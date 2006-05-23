#
# Makefile for MIPS-specific library files..
#

lib-y	+= csum_partial_copy.o memcpy.o promlib.o strlen_user.o strncpy_user.o \
	   strnlen_user.o uncached.o

obj-y	+= iomap.o

#ugly hack. Why is this needed?
#lib-$(CONFIG_MIKROTIK_RB500) += ashldi3.o ashrdi3.o lshrdi3.o

EXTRA_AFLAGS := $(CFLAGS)
