SRCS = apfs-snap.c
OBJS = $(SRCS:.c=.o)
DEPS = $(SRCS:.c=.d)

DESTDIR = $(INSTALLDIR)
BINDIR = /bin
MANDIR = /share/man/man8

SPARSE_VERSION := $(shell sparse --version 2>/dev/null)

GIT_COMMIT = $(shell git describe --always HEAD | tail -c 9)

override CFLAGS += -Wall -Wno-address-of-packed-member -fno-strict-aliasing -I$(CURDIR)/../include $(COPTS) $(MIPS16_OPT) $(THUMB) -ffunction-sections -fdata-sections $(LTO) -DNEED_PRINTF

apfs-snap: $(OBJS)
	@echo '  Linking...'
	@$(CC) $(CFLAGS) $(LDFLAGS) -o apfs-snap $(OBJS)
	@echo '  Build complete'

%.o: %.c
	@echo '  Compiling $<...'
	@$(CC) $(CFLAGS) -o $@ -MMD -MP -c $<
ifdef SPARSE_VERSION
	@sparse $(CFLAGS) $<
endif

apfs-snap.o: version.h
version.h: FORCE
	@printf '#define GIT_COMMIT\t"%s"\n' $(GIT_COMMIT) > version.h
FORCE:

-include $(DEPS)

clean:
	rm -f $(OBJS) $(DEPS) apfs-snap version.h
install:
	install -d $(DESTDIR)$(BINDIR)
	install -t $(DESTDIR)$(BINDIR) apfs-snap
#	install -d $(DESTDIR)$(MANDIR)
#	install -m 644 -t $(DESTDIR)$(MANDIR) apfs-snap.8
