# ebtables Makefile

PROGNAME:=ebtables
PROGVERSION:=2.0.6
PROGDATE:=November\ 2003

MANDIR?=/usr/local/man
CFLAGS:=-Wall -Wunused $(COPTS)
#CC:=gcc

ifeq ($(shell uname -m),sparc64)
CFLAGS+=-DEBT_MIN_ALIGN=8 -DKERNEL_64_USERSPACE_32
endif

include extensions/Makefile

OBJECTS:=getethertype.o ebtables.o communication.o $(EXT_OBJS)

#KERNEL_INCLUDES?=include/
KERNEL_INCLUDES=../../linux/linux.v23/include

ETHERTYPESPATH?=/etc
ETHERTYPESFILE:=$(ETHERTYPESPATH)/ethertypes

#BINPATH?=/sbin/
BINPATH=$(INSTALLDIR)/ebtables/usr/sbin/
BINFILE:=$(BINPATH)ebtables

PROGSPECS:=-DPROGVERSION=\"$(PROGVERSION)\" \
	-DPROGNAME=\"$(PROGNAME)\" \
	-DPROGDATE=\"$(PROGDATE)\" \
	-D_PATH_ETHERTYPES=\"$(ETHERTYPESFILE)\"


all: ebtables

communication.o: communication.c include/ebtables_u.h
	$(CC) $(CFLAGS) $(PROGSPECS) -c -o $@ $< -I$(KERNEL_INCLUDES)

getethertype.o: getethertype.c include/ethernetdb.h
	$(CC) $(CFLAGS) $(PROGSPECS) -c -o $@ $< -Iinclude/

ebtables.o: ebtables.c include/ebtables_u.h
	$(CC) $(CFLAGS) $(PROGSPECS) -c -o $@ $< -I$(KERNEL_INCLUDES)

ebtables: $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^ -I$(KERNEL_INCLUDES)

$(MANDIR)/man8/ebtables.8: ebtables.8
	mkdir -p $(@D)
	install -m 0644 -o root -g root $< $@

$(ETHERTYPESFILE): ethertypes
	mkdir -p $(@D)
	install -m 0644 -o root -g root $< $@

.PHONY: exec
exec: ebtables
	install -m 0755 -o root -g root $< $(BINFILE)

.PHONY: install
install: $(MANDIR)/man8/ebtables.8 $(ETHERTYPESFILE) exec

.PHONY: clean
clean:
	rm -f ebtables
	rm -f *.o *.c~
	rm -f extensions/*.o extensions/*.c~

DIR:=$(PROGNAME)-v$(PROGVERSION)
# This is used to make a new userspace release
.PHONY: release
release:
	mkdir -p include/linux/netfilter_bridge
	install -m 0644 -o root -g root \
		$(KERNEL_INCLUDES)/linux/netfilter_bridge.h include/linux/
# To keep possible compile error complaints about undefined ETH_P_8021Q
# off my back
	install -m 0644 -o root -g root \
		$(KERNEL_INCLUDES)/linux/if_ether.h include/linux/
	install -m 0644 -o root -g root \
		$(KERNEL_INCLUDES)/linux/netfilter_bridge/*.h \
		include/linux/netfilter_bridge/
	install -m 0644 -o root -g root \
		include/ebtables.h include/linux/netfilter_bridge/
	make clean
	touch *
	touch extensions/*
	touch include/*
	touch include/linux/*
	touch include/linux/netfilter_bridge/*
	cd ..;tar -c $(DIR) | gzip >$(DIR).tar.gz
