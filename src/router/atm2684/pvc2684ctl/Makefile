EXECS = pvc2684ctl pvc2684d

CFLAGS+=${BRCM_DRIVER_ATM_BONDING_DEFINES}

ifeq ($(strip $(BUILD_PVC2684CTL)),static)
CFLAGS += -DBUILD_STATIC
endif

INCLUDE = $(KERNEL_DIR)/include


CFLAGS += $(COPTS) -Wall -I$(INCLUDE)
LIBS= -L../atm/lib -latm

all: $(EXECS) 

dynamic: libatm $(EXECS) install

static: libatm pvc2684ctl.a pvc2684d.a
	
pvc2684d : pvc2684d.c brpvc.h
	$(CC) $(CFLAGS) -o $@ $< ${LIBS}

pvc2684ctl : pvc2684ctl.c brpvc.h
	$(CC) $(CFLAGS) -o $@ $< ${LIBS}

libatm:
	$(MAKE) -C ../atm/lib $(MFLAGS) all

pvc2684ctl.a: pvc2684ctl.o
	$(AR) rcs pvc2684ctl.a  pvc2684ctl.o

pvc2684ctl.o: pvc2684ctl.c
	$(CC) $(CFLAGS) $(INCLUDE) -Os -c -o pvc2684ctl.o pvc2684ctl.c

pvc2684d.a: pvc2684d.o
	$(AR) rcs pvc2684d.a  pvc2684d.o

pvc2684d.o: pvc2684d.c
	$(CC) $(CFLAGS) $(INCLUDE) -Os -c -o pvc2684d.o pvc2684d.c

install:
	mkdir -p $(INSTALLDIR)/usr/sbin
	install -m 755 pvc2684ctl $(INSTALLDIR)/usr/sbin
	$(STRIP) $(INSTALLDIR)/usr/sbin/pvc2684ctl
	install -m 755 pvc2684d $(INSTALLDIR)/usr/sbin
	$(STRIP) $(INSTALLDIR)/usr/sbin/pvc2684d
	
clean :
	rm -f *.o *~ pvc2684ctl pvc2684d core
	$(MAKE) -C ../atm/lib clean
