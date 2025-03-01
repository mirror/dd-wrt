# SPDX-License-Identifier: GPL-2.0
SSOBJ=libnetlink.o utils.o inet_proto.o rt_names.o json_print.o color.o ll_addr.o rose_ntop.o netrom_ntop.o ax25_ntop.o json_writer.o ll_map.o ll_proto.o mpls_pton.o mpls_ntop.o cg_map.o namespace.o fs.o
LNSTATOBJ=lnstat.o lnstat_util.o

TARGETS=ss nstat ifstat rtacct lnstat
CFLAGS += -Iinclude -Iinclude/uapi -D_GNU_SOURCE   
include config.mk

ifeq ($(HAVE_BERKELEY_DB),y)
	TARGETS += arpd
endif

all: $(TARGETS)

ss: ss.o ssfilter_check.o ssfilter.tab.o $(SSOBJ)
	$(QUIET_LINK)$(CC) $^ $(LDFLAGS) $(LDLIBS) -o $@

nstat: nstat.o $(SSOBJ)
	$(QUIET_CC)$(CC) $(CFLAGS) $(CPPFLAGS) $(LDFLAGS) -o nstat $^ $(LDLIBS) -lm

ifstat: ifstat.o $(SSOBJ)
	$(QUIET_CC)$(CC) $(CFLAGS) $(CPPFLAGS) $(LDFLAGS) -o ifstat $^ $(LDLIBS) -lm

rtacct: rtacct.o $(SSOBJ)
	$(QUIET_CC)$(CC) $(CFLAGS) $(CPPFLAGS) $(LDFLAGS) -o rtacct $^ $(LDLIBS) -lm

arpd: arpd.c
	$(QUIET_CC)$(CC) $(CFLAGS) -I$(DBM_INCLUDE) $(CPPFLAGS) $(LDFLAGS) -o arpd arpd.c $(LDLIBS) -ldb

ssfilter.tab.c: ssfilter.y
	$(QUIET_YACC)$(YACC) -b ssfilter ssfilter.y

lnstat: $(LNSTATOBJ) $(SSOBJ)
	$(QUIET_LINK)$(CC) $^ $(LDFLAGS) $(LDLIBS) -o $@

install: all
	install -m 0755 $(TARGETS) $(DESTDIR)$(SBINDIR)
	ln -sf lnstat $(DESTDIR)$(SBINDIR)/rtstat
	ln -sf lnstat $(DESTDIR)$(SBINDIR)/ctstat

clean:
	rm -f *.o $(TARGETS) ssfilter.c
