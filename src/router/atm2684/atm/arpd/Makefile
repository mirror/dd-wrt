# brcm
#LIBS=-latmd
#LIBDEPS=../lib/libatmd.a
#LIBS=-lfl # lex may want  -ll  here
#INCLUDES=
include ../Rules.make

EXECS = rt2684d 
ifeq ($(strip $(BUILD_RT2684D)),static)
CFLAGS += -DBUILD_STATIC
endif
LDFLAGS += -Wl,-allow-shlib-undefined
OBJS=rt2684d.o arp.o io.o itf.o table.o
#BOOTPGMS=atmarp atmarpd
MAN8=atmarp.8 atmarpd.8
SYSHDR=atmarp.h atmarpd.h


all: $(EXECS) 

dynamic: lib-atm-atmd $(EXECS) install

static: lib-atm-atmd rt2684d.a

lib-atm-atmd:
	$(MAKE) -C ../lib $(MFLAGS) all

install:
	install -m 755 rt2684d $(INSTALL_DIR)/bin
	$(STRIP) $(INSTALL_DIR)/bin/rt2684d

rt2684d.a: $(OBJS)
	$(AR) rcs rt2684d.a $(OBJS)

$(EXECS): $(OBJS)
	$(CC) -o $@ $^ $(CFLAGS) $(LDFLAGS) -L../lib -latmd -latm


#.c.o:
#	$(CC) -c $(CFLAGS) $<

clean:
	$(MAKE) -C ../lib clean
	rm -f *.o *.a $(EXECS)
