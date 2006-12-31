
EXECS = br2684ctl
ifeq ($(strip $(BUILD_BR2684CTL)),static)
CFLAGS = -DBUILD_STATIC
endif

CFLAGS += $(COPTS)
INCLUDE = -I$(KERNEL_DIR)/include

all: $(EXECS) 

dynamic: libatm $(EXECS) install

static: libatm  br2684ctl.a

libatm:
	$(MAKE) -C ../atm/lib $(MFLAGS) all

install:
	install -m 755 br2684ctl $(INSTALL_DIR)/bin
	$(STRIP) $(INSTALL_DIR)/bin/br2684ctl

br2684ctl.a: br2684ctl.o
	$(AR) rcs br2684ctl.a  br2684ctl.o

br2684ctl.o: br2684ctl.c
	$(CC) $(CFLAGS) $(INCLUDE) -Os -c -o br2684ctl.o br2684ctl.c

$(EXECS): br2684ctl.c
	$(CC) $(CFLAGS) -o br2684ctl -Os br2684ctl.c -L../atm/lib -latm $(INCLUDE)

clean:
	$(MAKE) -C ../atm/lib clean
	rm -f *.o *.a $(EXECS)
