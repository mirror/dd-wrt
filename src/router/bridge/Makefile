# SPDX-License-Identifier: GPL-2.0
BROBJ = bridge.o fdb.o monitor.o link.o mdb.o vlan.o vni.o utils.o json_print.o json_writer.o color.o rt_names.o libnetlink.o ll_proto.o ll_addr.o ll_map.o mpls_ntop.o mpls_pton.o ax25_ntop.o netrom_ntop.o rose_ntop.o namespace.o

CFLAGS += -Iinclude -Iinclude/uapi -D_GNU_SOURCE $(COPTS) $(MIPS16_OPT) $(LTO) -DNEED_PRINTF
CFLAGS += -DHAVE_SETNS
CFLAGS += -DHAVE_HANDLE_AT
#HAVE_SELINUX:=y
#LDLIBS += -lselinux
#CFLAGS += -DHAVE_SELINUX
HAVE_RPC:=y
#LDLIBS += -ltirpc
#CFLAGS += -DHAVE_RPC
HAVE_ELF:=y
CFLAGS += -DHAVE_ELF
#LDLIBS +=  -lelf
#HAVE_MNL:=y
#CFLAGS += -DHAVE_LIBMNL
#LDLIBS += -lmnl
HAVE_BERKELEY_DB:=y
#CFLAGS += -DNEED_STRLCPY
#HAVE_CAP:=y
#CFLAGS += -DHAVE_LIBCAP
#LDLIBS += -lcap

all: bridge

bridge: $(BROBJ) $(LIBNETLINK)
	$(CC) $^ $(LDFLAGS) $(LDLIBS) -o $@

install: all
	install -m 0755 bridge $(INSTALLDIR)/usr/sbin

clean:
	rm -f $(BROBJ) bridge
