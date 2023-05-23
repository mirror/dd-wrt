SRCS = apfs-snap.c
OBJS = $(SRCS:.c=.o)
DEPS = $(SRCS:.c=.d)

DESTDIR ?= ~
BINDIR = /bin
MANDIR = /share/man/man8

SPARSE_VERSION := $(shell sparse --version 2>/dev/null)

override CFLAGS += -Wall -Wno-address-of-packed-member -fno-strict-aliasing

apfs-snap: $(OBJS)
	@echo '  Linking...'
	@gcc $(CFLAGS) $(LDFLAGS) -o apfs-snap $(OBJS)
	@echo '  Build complete'

%.o: %.c
	@echo '  Compiling $<...'
	@gcc $(CFLAGS) -o $@ -MMD -MP -c $<
ifdef SPARSE_VERSION
	@sparse $(CFLAGS) $<
endif

-include $(DEPS)

clean:
	rm -f $(OBJS) $(DEPS) apfs-snap
install:
	install -d $(DESTDIR)$(BINDIR)
	install -t $(DESTDIR)$(BINDIR) apfs-snap
	install -d $(DESTDIR)$(MANDIR)
	install -m 644 -t $(DESTDIR)$(MANDIR) apfs-snap.8
