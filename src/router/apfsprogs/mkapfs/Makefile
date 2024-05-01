SRCS = btree.c dir.c mkapfs.c object.c spaceman.c super.c
OBJS = $(SRCS:.c=.o)
DEPS = $(SRCS:.c=.d)

LIBDIR = ../lib
LIBRARY = $(LIBDIR)/libapfs.a

BINDIR = $(INSTALLDIR)/bin
MANDIR = $(INSTALLDIR)/share/man/man8

SPARSE_VERSION := $(shell sparse --version 2>/dev/null)

override CFLAGS += -Wall -Wno-address-of-packed-member -fno-strict-aliasing -I$(CURDIR)/../include $(COPTS) $(MIPS16_OPT) $(THUMB) -ffunction-sections -fdata-sections $(LTO) -DNEED_PRINTF

mkapfs: $(OBJS) $(LIBRARY)
	@echo '  Linking...'
	$(CC) $(CFLAGS) -o mkapfs $(OBJS) $(LIBRARY) $(LDLTO)
	@echo '  Build complete'

# Build the common libraries
$(LIBRARY): FORCE
	@echo '  Building libraries...'
	@$(MAKE) -C $(LIBDIR) --silent --no-print-directory
	@echo '  Library build complete'
FORCE:

%.o: %.c
	@echo '  Compiling $<...'
	$(CC) $(CFLAGS) -o $@ -MMD -MP -c $<
ifdef SPARSE_VERSION
	@sparse $(CFLAGS) $<
endif

-include $(DEPS)

clean:
	rm -f $(OBJS) $(DEPS) mkapfs
install:
	install -d $(BINDIR)
	install -t $(BINDIR) mkapfs
	cd $(BINDIR) && ln -fs -T mkapfs mkfs.apfs
#	ln -fs -T $(BINDIR)/mkapfs $(BINDIR)/mkfs.apfs
#	install -d $(MANDIR)
#	install -m 644 -t $(MANDIR) mkapfs.8
#	ln -fs -T $(MANDIR)/mkapfs.8 $(MANDIR)/mkfs.apfs.8
