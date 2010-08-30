# vim:set sw=8 nosta:

SOFTWARE=hotplug2
VERSION=1.0-alpha

BINS=hotplug2 hotplug2-modwrap
SUBDIRS=parser rules

hotplug2-objs := \
	hotplug2.o netlink.o seqnum.o settings.o uevent.o xmemutils.o \
	workers/loader.o parser/parser.o parser/buffer.o parser/token.o \
	parser/token_queue.o parser/lexer.o rules/ruleset.o rules/rule.o \
	rules/condition.o rules/expression.o rules/execution.o \
	rules/command.o

ifdef STATIC_WORKER
  ifeq ($(wildcard workers/worker_$(STATIC_WORKER).c),)
    $(error Worker source worker/worker_$(STATIC_WORKER).c not found)
  endif
  hotplug2-objs += action.o workers/worker_$(STATIC_WORKER).o
else
  SUBDIRS += workers
endif

DESTDIR=


all: $(BINS)

install:
	$(INSTALL_BIN) $(BINS) $(DESTDIR)/sbin/

hotplug2: $(hotplug2-objs)

coldplug2: coldplug2.o

include common.mak

ifdef STATIC_WORKER
  CFLAGS += -DSTATIC_WORKER=1
else
  CFLAGS += $(FPIC)
  LDFLAGS += -ldl
endif

