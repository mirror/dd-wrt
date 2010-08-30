# vim:set sw=8 nosta:

COPTS=-Os -Wall -g
LDFLAGS=-g

CFLAGS=$(COPTS)
FPIC=-fPIC

INSTALL=install -c -m 644
INSTALL_BIN=install -c -m 755


.PHONY: all clean dep install install-recursive clean-recursive \
	dep-recursive all-recursive

MAKEDEP=-$(CC) $(CFLAGS) -MM $(wildcard *.c *.cc) > .depend
dep: dep-recursive
	$(MAKEDEP)
.depend:
	$(MAKEDEP)
dep-recursive:
	@for i in $(SUBDIRS); do $(MAKE) -C $$i dep; done
	
-include .depend

clean: clean-recursive
	$(RM) $(wildcard *.o *.so *.a $(BINS)) .depend
clean-recursive:
	@for i in $(SUBDIRS); do $(MAKE) -C $$i clean; done

all: all-recursive
all-recursive:
	@for i in $(SUBDIRS); do $(MAKE) -C $$i all; done

install: all install-recursive
install-recursive:
	@for i in $(SUBDIRS); do $(MAKE) -C $$i install; done

%.so: LDFLAGS += -shared -nostartfiles
%.so: %.o
	$(LINK.o) $^ $(LOADLIBES) $(LDLIBS) -o $@

