#!/usr/bin/make -f
# SPDX-License-Identifier: GPL-2.0
# -*- makefile -*-
#
# Copyright (C) B.A.T.M.A.N. contributors
#
# License-Filename: LICENSES/preferred/GPL-2.0

# just for backward compatibility - please use CONFIG_bisect_iv instead
export CONFIG_BATCTL_BISECT=n

# batctl build
BINARY_NAME = batctl

obj-y += bat-hosts.o
obj-y += debug.o
obj-y += functions.o
obj-y += genl.o
obj-y += genl_json.o
obj-y += hash.o
obj-y += icmp_helper.o
obj-y += main.o
obj-y += netlink.o
obj-y += sys.o

define add_command
  CONFIG_$(1):=$(2)
  ifneq ($$(CONFIG_$(1)),y)
    ifneq ($$(CONFIG_$(1)),n)
      $$(warning invalid value for parameter CONFIG_$(1): $$(CONFIG_$(1)))
    endif
  endif

  obj-$$(CONFIG_$(1)) += $(1).o
endef # add_command

# using the make parameter CONFIG_* (e.g. CONFIG_bisect_iv) with the value 'y'
# enables the related feature and 'n' disables it
$(eval $(call add_command,aggregation,y))
$(eval $(call add_command,ap_isolation,y))
$(eval $(call add_command,backbonetable,y))
$(eval $(call add_command,bisect_iv,$(CONFIG_BATCTL_BISECT)))
$(eval $(call add_command,bla_backbone_json,y))
$(eval $(call add_command,bla_claim_json,y))
$(eval $(call add_command,bonding,y))
$(eval $(call add_command,bridge_loop_avoidance,y))
$(eval $(call add_command,claimtable,y))
$(eval $(call add_command,dat_cache,y))
$(eval $(call add_command,dat_cache_json,y))
$(eval $(call add_command,distributed_arp_table,y))
$(eval $(call add_command,elp_interval,y))
$(eval $(call add_command,event,y))
$(eval $(call add_command,fragmentation,y))
$(eval $(call add_command,gateways,y))
$(eval $(call add_command,gateways_json,y))
$(eval $(call add_command,gw_mode,y))
$(eval $(call add_command,hardif_json,y))
$(eval $(call add_command,hardifs_json,y))
$(eval $(call add_command,hop_penalty,y))
$(eval $(call add_command,interface,y))
$(eval $(call add_command,isolation_mark,y))
$(eval $(call add_command,loglevel,y))
$(eval $(call add_command,mcast_flags,y))
$(eval $(call add_command,mcast_flags_json,y))
$(eval $(call add_command,mesh_json,y))
$(eval $(call add_command,multicast_fanout,y))
$(eval $(call add_command,multicast_forceflood,y))
$(eval $(call add_command,multicast_mode,y))
$(eval $(call add_command,neighbors,y))
$(eval $(call add_command,neighbors_json,y))
$(eval $(call add_command,network_coding,y))
$(eval $(call add_command,orig_interval,y))
$(eval $(call add_command,originators,y))
$(eval $(call add_command,originators_json,y))
$(eval $(call add_command,ping,y))
$(eval $(call add_command,routing_algo,y))
$(eval $(call add_command,statistics,y))
$(eval $(call add_command,tcpdump,y))
$(eval $(call add_command,throughput_override,y))
$(eval $(call add_command,throughputmeter,y))
$(eval $(call add_command,traceroute,y))
$(eval $(call add_command,transglobal,y))
$(eval $(call add_command,translate,y))
$(eval $(call add_command,translocal,y))
$(eval $(call add_command,transtable_global_json,y))
$(eval $(call add_command,transtable_local_json,y))
$(eval $(call add_command,vlan_json,y))

MANPAGE = man/batctl.8

# batctl flags and options
CFLAGS += -Wall -W -std=gnu99 -fno-strict-aliasing -MD -MP
CPPFLAGS += -D_GNU_SOURCE
LDLIBS += -lm -lrt

# disable verbose output
ifneq ($(findstring $(MAKEFLAGS),s),s)
ifndef V
	Q_CC = @echo '   ' CC $@;
	Q_LD = @echo '   ' LD $@;
	export Q_CC
	export Q_LD
endif
endif

ifeq ($(origin PKG_CONFIG), undefined)
  PKG_CONFIG = pkg-config
  ifeq ($(shell which $(PKG_CONFIG) 2>/dev/null),)
    $(error $(PKG_CONFIG) not found)
  endif
endif

CFLAGS += $(LIBNL_CFLAGS)
LDLIBS += $(LIBNL_LDLIBS)

CFLAGS += $(LIBNL_GENL_CFLAGS)
LDLIBS += $(LIBNL_GENL_LDLIBS)

# standard build tools
CC ?= gcc
RM ?= rm -f
INSTALL ?= install
MKDIR ?= mkdir -p
COMPILE.c = $(Q_CC)$(CC) $(CFLAGS) $(CPPFLAGS) $(TARGET_ARCH) -c
LINK.o = $(Q_LD)$(CC) $(CFLAGS) $(LDFLAGS) $(TARGET_ARCH)

# standard install paths
PREFIX = /usr/local
SBINDIR = $(PREFIX)/sbin
MANDIR = $(PREFIX)/share/man

# try to generate revision
REVISION= $(shell	if [ -d .git ]; then \
				echo $$(git describe --always --dirty --match "v*" |sed 's/^v//' 2> /dev/null || echo "[unknown]"); \
			fi)
ifneq ($(REVISION),)
CPPFLAGS += -DSOURCE_VERSION=\"$(REVISION)\"
endif

# default target
all: $(BINARY_NAME)

# standard build rules
.SUFFIXES: .o .c
.c.o:
	$(COMPILE.c) -o $@ $<

$(BINARY_NAME): $(obj-y)
	$(LINK.o) $^ $(LDLIBS) -o $@

clean:
	$(RM) $(BINARY_NAME) $(obj-y) $(obj-n) $(DEP)

install: $(BINARY_NAME)
	$(MKDIR) $(DESTDIR)$(SBINDIR)
	$(MKDIR) $(DESTDIR)$(MANDIR)/man8
	$(INSTALL) -m 0755 $(BINARY_NAME) $(DESTDIR)$(SBINDIR)
	$(INSTALL) -m 0644 $(MANPAGE) $(DESTDIR)$(MANDIR)/man8

# load dependencies
DEP = $(obj-y:.o=.d) $(obj-n:.o=.d)
-include $(DEP)

.PHONY: all clean install
