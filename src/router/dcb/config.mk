# Generated config based on /xfs/iproute/iproute2-6.4.0/include
# user can control verbosity similar to kernel builds (e.g., V=1)
ifeq ("$(origin V)", "command line")
  VERBOSE = $(V)
endif
ifndef VERBOSE
  VERBOSE = 0
endif
ifeq ($(VERBOSE),1)
  Q =
else
  Q = @
endif

ifeq ($(VERBOSE), 0)
    QUIET_CC       = @echo '    CC       '$@;
    QUIET_AR       = @echo '    AR       '$@;
    QUIET_LINK     = @echo '    LINK     '$@;
    QUIET_YACC     = @echo '    YACC     '$@;
    QUIET_LEX      = @echo '    LEX      '$@;
endif
PKG_CONFIG:=pkg-config
YACC:=bison
TC_CONFIG_IPSET:=y
LIBDIR:=/usr/lib
IPT_LIB_DIR:=/usr/lib/xtables
IP_CONFIG_SETNS:=y
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
CFLAGS += -DHAVE_LIBMNL
LDLIBS += -lmnl
HAVE_BERKELEY_DB:=y
#CFLAGS += -DNEED_STRLCPY
#HAVE_CAP:=y
#CFLAGS += -DHAVE_LIBCAP
#LDLIBS += -lcap

%.o: %.c
	$(QUIET_CC)$(CC) $(CFLAGS) $(EXTRA_CFLAGS) $(CPPFLAGS) -c -o $@ $<
