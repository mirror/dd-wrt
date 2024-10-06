SRCS = apfs-label.c
OBJS = $(SRCS:.c=.o)
DEPS = $(SRCS:.c=.d)

LIBDIR = ../lib
LIBRARY = $(LIBDIR)/libapfs.a

DESTDIR ?= ~
BINDIR = /bin
MANDIR = /share/man/man8

SPARSE_VERSION := $(shell sparse --version 2>/dev/null)

GIT_COMMIT = $(shell git describe --always HEAD | tail -c 9)

override CFLAGS += -Wall -Wno-address-of-packed-member -fno-strict-aliasing -I$(CURDIR)/../include

apfs-label: $(OBJS) $(LIBRARY)
	@echo '  Linking...'
	@$(CC) $(CFLAGS) $(LDFLAGS) -o apfs-label $(OBJS) $(LIBRARY)
	@echo '  Build complete'

# Build the common libraries
$(LIBRARY): FORCE
	@echo '  Building libraries...'
	@$(MAKE) -C $(LIBDIR) --silent --no-print-directory
	@echo '  Library build complete'
FORCE:

%.o: %.c
	@echo '  Compiling $<...'
	@$(CC) $(CFLAGS) -o $@ -MMD -MP -c $<
ifdef SPARSE_VERSION
	@sparse $(CFLAGS) $<
endif

apfs-label.o: version.h
version.h: FORCE
	@printf '#define GIT_COMMIT\t"%s"\n' $(GIT_COMMIT) > version.h

-include $(DEPS)

clean:
	rm -f $(OBJS) $(DEPS) apfs-label version.h
install:
	install -d $(DESTDIR)$(BINDIR)
	install -t $(DESTDIR)$(BINDIR) apfs-label
	install -d $(DESTDIR)$(MANDIR)
	install -m 644 -t $(DESTDIR)$(MANDIR) apfs-label.8
