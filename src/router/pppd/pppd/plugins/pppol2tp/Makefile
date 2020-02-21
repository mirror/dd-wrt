#CC	= gcc
#COPTS	= $(COPTS)
CFLAGS	= $(COPTS) $(MIPS16_OPT) $(LTOMIN) -I$(TOP)/kernel_headers/$(KERNELRELEASE)/include -I$(LINUXDIR)/include -I. -I../.. -I../../../include -fPIC

LDFLAGS	= -shared
INSTALL	= install

#***********************************************************************

DESTDIR = @DESTDIR@
LIBDIR = $(DESTDIR)/lib/pppd/$(VERSION)

VERSION = $(shell awk -F '"' '/VERSION/ { print $$2; }' ../../patchlevel.h)

PLUGINS := pppol2tp.so openl2tp.so

all: $(PLUGINS)

%.so: %.o
	$(CC) $(CFLAGS) $(LDLTO) -o $@ -shared $^ $(LIBS)

install: all
	$(INSTALL) -d -m 755 $(LIBDIR)
	$(INSTALL) -c -m 4550 $(PLUGINS) $(LIBDIR)

clean:
	rm -f *.o *.so

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<
