TOPDIR ?= ../..
include $(TOPDIR)/Makefile.inc

# hardening flags: -fPIE -Wl,-z,now -pie
CFLAGS += $(shell pkg-config --cflags gtk+-2.0) -fPIE -Wno-strict-prototypes \
          -Wno-cast-qual
CPPFLAGS += $(shell pkg-config --cflags gtk+-2.0)
LDFLAGS += -pie
#LDFLAGS +=	-Wl,--warn-common


LIBS := $(shell pkg-config --libs gtk+-2.0) -lm

# 32/64 cross compilation
ifdef M32
CFLAGS +=	-m32
LDFLAGS +=	-m32
endif
ifdef M64
CFLAGS +=	-m64
LDFLAGS +=	-m64
endif


all: default_target

default_target: olsrd-gui

olsrd-gui:	$(OBJS)
ifeq ($(VERBOSE),0)
	@echo "[LD] $@"
endif
	$(MAKECMDPREFIX)$(CC) $(LDFLAGS) -o $@ $(OBJS) $(LIBS)

.PHONY: clean install

clean:
	rm -f $(OBJS) olsrd-gui

install:
	mkdir -p "$(DESTDIR)/usr/bin"
	cp ./olsrd-gui "$(DESTDIR)/usr/bin/"
