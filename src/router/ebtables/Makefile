# ebtables Makefile

PROGNAME:=ebtables
PROGVERSION:=2.0.8-rc1
PROGDATE:=November\ 2005

# default paths
LIBDIR:=/usr/lib
MANDIR:=/usr/local/man
BINDIR:=/usr/sbin
ETCDIR:=/etc
INITDIR:=/etc/rc.d/init.d
SYSCONFIGDIR:=/etc/sysconfig
DESTDIR:=

# include DESTDIR param
override LIBDIR:=$(DESTDIR)$(LIBDIR)/$(PROGNAME)
override MANDIR:=$(DESTDIR)$(MANDIR)
override BINDIR:=$(DESTDIR)$(BINDIR)
override ETCDIR:=$(DESTDIR)$(ETCDIR)
override INITDIR:=$(DESTDIR)$(INITDIR)
override SYSCONFIGDIR:=$(DESTDIR)$(SYSCONFIGDIR)


#CFLAGS:=-Wall -Wunused
#CC:=gcc
#LD:=ld

ifeq ($(shell uname -m),sparc64)
CFLAGS+=-DEBT_MIN_ALIGN=8 -DKERNEL_64_USERSPACE_32
endif

include extensions/Makefile

OBJECTS2:=getethertype.o communication.o libebtc.o \
useful_functions.o ebtables.o

OBJECTS:=$(OBJECTS2) ebtables.o $(EXT_OBJS) $(EXT_LIBS)

KERNEL_INCLUDES?=include/

ETHERTYPESPATH?=$(ETCDIR)
ETHERTYPESFILE:=$(ETHERTYPESPATH)/ethertypes

PIPE_DIR?=$(DESTDIR)/tmp/$(PROGNAME)-v$(PROGVERSION)
PIPE=$(PIPE_DIR)/ebtablesd_pipe
EBTD_CMDLINE_MAXLN?=2048
EBTD_ARGC_MAX?=50

PROGSPECS:=-DPROGVERSION=\"$(PROGVERSION)\" \
	-DPROGNAME=\"$(PROGNAME)\" \
	-DPROGDATE=\"$(PROGDATE)\" \
	-D_PATH_ETHERTYPES=\"$(ETHERTYPESFILE)\" \
	-DEBTD_ARGC_MAX=$(EBTD_ARGC_MAX) \
	-DEBTD_CMDLINE_MAXLN=$(EBTD_CMDLINE_MAXLN)

# You can probably ignore this, ebtables{u,d} are normally not used
PROGSPECSD:=-DPROGVERSION=\"$(PROGVERSION)\" \
	-DPROGNAME=\"$(PROGNAME)\" \
	-DPROGDATE=\"$(PROGDATE)\" \
	-D_PATH_ETHERTYPES=\"$(ETHERTYPESFILE)\" \
	-DEBTD_CMDLINE_MAXLN=$(EBTD_CMDLINE_MAXLN) \
	-DEBTD_ARGC_MAX=$(EBTD_ARGC_MAX) \
	-DEBTD_PIPE=\"$(PIPE)\" \
	-DEBTD_PIPE_DIR=\"$(PIPE_DIR)\"

# Uncomment for debugging (slower)
#PROGSPECS+=-DEBT_DEBUG
#PROGSPECSD+=-DEBT_DEBUG
#CFLAGS+=-ggdb

all: ebtables ebtables-restore

communication.o: communication.c include/ebtables_u.h
	$(CC) $(CFLAGS) $(PROGSPECS) -c -o $@ $< -I$(KERNEL_INCLUDES)

libebtc.o: libebtc.c include/ebtables_u.h
	$(CC) $(CFLAGS) $(PROGSPECS) -c -o $@ $< -I$(KERNEL_INCLUDES)

useful_functions.o: useful_functions.c include/ebtables_u.h
	$(CC) $(CFLAGS) $(PROGSPECS) -c -o $@ $< -I$(KERNEL_INCLUDES)

getethertype.o: getethertype.c include/ethernetdb.h
	$(CC) $(CFLAGS) $(PROGSPECS) -c -o $@ $< -Iinclude/

ebtables.o: ebtables.c include/ebtables_u.h
	$(CC) $(CFLAGS) $(PROGSPECS) -c -o $@ $< -I$(KERNEL_INCLUDES)

ebtables-standalone.o: ebtables-standalone.c include/ebtables_u.h
	$(CC) $(CFLAGS) $(PROGSPECS) -c $< -o $@ -I$(KERNEL_INCLUDES)

.PHONY: libebtc
libebtc: $(OBJECTS2)
	$(LD) -shared -soname libebtc.so -o libebtc.so -lc $(OBJECTS2)

ebtables: $(OBJECTS) ebtables-standalone.o libebtc
	$(CC) $(CFLAGS) -o $@ ebtables-standalone.o -I$(KERNEL_INCLUDES) -L. -Lextensions -lebtc $(EXT_LIBSI) \
	-Wl,-rpath,$(LIBDIR)

ebtablesu: ebtablesu.c
	$(CC) $(CFLAGS) $(PROGSPECSD) $< -o $@

ebtablesd.o: ebtablesd.c include/ebtables_u.h
	$(CC) $(CFLAGS) $(PROGSPECSD) -c $< -o $@  -I$(KERNEL_INCLUDES)

ebtablesd: $(OBJECTS) ebtablesd.o libebtc
	$(CC) $(CFLAGS) -o $@ ebtablesd.o -I$(KERNEL_INCLUDES) -L. -Lextensions -lebtc $(EXT_LIBSI) \
	-Wl,-rpath,$(LIBDIR)

ebtables-restore.o: ebtables-restore.c include/ebtables_u.h
	$(CC) $(CFLAGS) $(PROGSPECS) -c $< -o $@  -I$(KERNEL_INCLUDES)

ebtables-restore: $(OBJECTS) ebtables-restore.o libebtc
	$(CC) $(CFLAGS) -o $@ ebtables-restore.o -I$(KERNEL_INCLUDES) -L. -Lextensions -lebtc $(EXT_LIBSI) \
	-Wl,-rpath,$(LIBDIR)

.PHONY: daemon
daemon: ebtablesd ebtablesu

tmp1:=$(shell printf $(BINDIR) | sed 's/\//\\\//g')
tmp2:=$(shell printf $(SYSCONFIGDIR) | sed 's/\//\\\//g')
tmp3:=$(shell printf $(PIPE) | sed 's/\//\\\//g')
.PHONY: scripts
scripts: ebtables-save ebtables.sysv ebtables-config
	cat ebtables-save | sed 's/__EXEC_PATH__/$(tmp1)/g' > ebtables-save_
	install -m 0755 -o root -g root ebtables-save_ $(BINDIR)/ebtables-save
	cat ebtables.sysv | sed 's/__EXEC_PATH__/$(tmp1)/g' | sed 's/__SYSCONFIG__/$(tmp2)/g' > ebtables.sysv_
	install -m 0755 -o root -g root ebtables.sysv_ $(INITDIR)/ebtables
	cat ebtables-config | sed 's/__SYSCONFIG__/$(tmp2)/g' > ebtables-config_
	install -m 0600 -o root -g root ebtables-config_ $(SYSCONFIGDIR)/ebtables-config
	rm -f ebtables-save_ ebtables.sysv_ ebtables-config_

$(MANDIR)/man8/ebtables.8: ebtables.8
	mkdir -p $(@D)
	install -m 0644 -o root -g root $< $@

$(ETHERTYPESFILE): ethertypes
	mkdir -p $(@D)
	install -m 0644 -o root -g root $< $@

.PHONY: exec
exec: ebtables ebtables-restore
	mkdir -p $(BINDIR)
	install -m 0755 -o root -g root $(PROGNAME) $(BINDIR)/$(PROGNAME)
	install -m 0755 -o root -g root ebtables-restore $(BINDIR)/ebtables-restore

.PHONY: install
install: $(MANDIR)/man8/ebtables.8 $(ETHERTYPESFILE) exec scripts
	mkdir -p $(LIBDIR)
	install -m 0755 extensions/*.so $(LIBDIR)
	install -m 0755 *.so $(LIBDIR)

.PHONY: clean
clean:
	rm -f ebtables ebtables-restore ebtablesd ebtablesu
	rm -f *.o *~ *.so
	rm -f extensions/*.o extensions/*.c~ extensions/*.so include/*~

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
	cd ..;tar -c $(DIR) | gzip >$(DIR).tar.gz; cd -
	rm -rf include/linux

# This will make the rpm and put it in /usr/src/redhat/RPMS
# (do this as root after make release)
.PHONY: rpmbuild
rpmbuild:
	cp ../$(DIR).tar.gz /usr/src/redhat/SOURCES/
	rpmbuild --buildroot $(shell mktemp -td $(DIR)-XXXXX) -bb ebtables.spec

.PHONY: test_ulog
test_ulog: examples/ulog/test_ulog.c getethertype.o
	$(CC) $(CFLAGS)  $< -o test_ulog -I$(KERNEL_INCLUDES) -lc \
	getethertype.o
	mv test_ulog examples/ulog/

.PHONY: examples
examples: test_ulog
