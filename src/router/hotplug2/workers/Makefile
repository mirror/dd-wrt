# vim:set sw=8 nosta:

BINS=worker_example.so worker_single.so worker_fork.so
SUBDIRS=
DESTDIR=


all: $(BINS)

install:
	$(INSTALL_BIN) $(BINS) $(DESTDIR)/lib/hotplug2/

worker_single.so: ../action.o ../uevent.o ../xmemutils.o \
		  ../rules/ruleset.o ../rules/rule.o ../rules/condition.o \
		  ../rules/expression.o ../rules/execution.o ../rules/command.o

worker_fork.so: ../action.o ../uevent.o ../xmemutils.o \
		  ../rules/ruleset.o ../rules/rule.o ../rules/condition.o \
		  ../rules/expression.o ../rules/execution.o ../rules/command.o

include ../common.mak
