#
# Makefile for libmtd
#

SUBDIRS =

# CFLAGS += -Werror
CPPFLAGS += -I../include
LIBS = libmtd
override TARGETS = libmtd.a

include ../common.mk

$(BUILDDIR)/libmtd.a: $(addprefix $(BUILDDIR)/,\
       libmtd.o libmtd_legacy.o libcrc32.o libfec.o)

clean::
	rm -f $(addsuffix .a, $(LIBS))

install::

uninstall:
