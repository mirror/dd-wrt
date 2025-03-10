# SPDX-License-Identifier: GPL-2.0
include config.mk

SSOBJ=libnetlink.o utils.o inet_proto.o rt_names.o json_print.o color.o ll_addr.o rose_ntop.o netrom_ntop.o ax25_ntop.o json_writer.o ll_map.o ll_proto.o mpls_pton.o mpls_ntop.o cg_map.o namespace.o fs.o utils_math.o mnl_utils.o json_print_math.o

CFLAGS += -Iinclude -Iinclude/uapi -I$(TOP)/libmnl/include -D_GNU_SOURCE $(COPTS) $(MIPS16_OPT) $(LTO) -DNEED_PRINTF
LDFLAGS += -L$(TOP)/libmnl/src/.libs

DCBOBJ = dcb.o \
         dcb_app.o \
         dcb_buffer.o \
         dcb_dcbx.o \
         dcb_ets.o \
         dcb_maxrate.o \
         dcb_pfc.o \
         dcb_apptrust.o

TARGETS += dcb
LDLIBS += -lm

all: $(TARGETS) $(LIBS)

dcb: $(DCBOBJ) $(SSOBJ) $(LIBNETLINK)
	$(QUIET_LINK)$(CC) $^ $(LDFLAGS) $(LDLIBS) -o $@

install: all
	for i in $(TARGETS); \
	do install -m 0755 $$i $(INSTALLDIR)$(SBINDIR); \
	done

clean:
	rm -f $(DCBOBJ) $(TARGETS) $(SSOBJ)
