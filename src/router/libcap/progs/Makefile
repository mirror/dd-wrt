
topdir=$(shell pwd)/..
include $(topdir)/Make.Rules
#
# Programs: all of the examples that we will compile
#
PROGS=getpcaps capsh getcap setcap

BUILD=$(PROGS)

ifneq ($(DYNAMIC),yes)
LDFLAGS += --static
endif
LDLIBS += -L../libcap -lcap

all: $(BUILD)

$(BUILD): %: %.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $< $(LDLIBS)

%.o: %.c $(INCS)
	$(CC) $(IPATH) $(CFLAGS) -c $< -o $@

install: all
	mkdir -p -m 0755 $(FAKEROOT)$(SBINDIR)
	for p in $(PROGS) ; do \
		install -m 0755 $$p $(FAKEROOT)$(SBINDIR) ; \
	done
ifeq ($(RAISE_SETFCAP),yes)
	$(FAKEROOT)$(SBINDIR)/setcap cap_setfcap=i $(FAKEROOT)$(SBINDIR)/setcap
endif

clean:
	$(LOCALCLEAN)
	rm -f *.o $(BUILD) tcapsh ping hack.sh
