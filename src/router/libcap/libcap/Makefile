#
# defines
#
topdir=$(shell pwd)/..
include ../Make.Rules
#
# Library version
#
LIBNAME=$(LIBTITLE).so
STALIBNAME=$(LIBTITLE).a
#

FILES=cap_alloc cap_proc cap_extint cap_flag cap_text cap_file

INCLS=libcap.h cap_names.h $(INCS)
OBJS=$(addsuffix .o, $(FILES))
MAJLIBNAME=$(LIBNAME).$(VERSION)
MINLIBNAME=$(MAJLIBNAME).$(MINOR)
GPERF_OUTPUT = _caps_output.gperf

all: $(MINLIBNAME) $(STALIBNAME) libcap.pc

ifeq ($(BUILD_GPERF),yes)
USE_GPERF_OUTPUT = $(GPERF_OUTPUT)
INCLUDE_GPERF_OUTPUT = -DINCLUDE_GPERF_OUTPUT='"$(GPERF_OUTPUT)"'
endif

libcap.pc: libcap.pc.in
	sed -e 's,@prefix@,$(prefix),' \
		-e 's,@exec_prefix@,$(exec_prefix),' \
		-e 's,@libdir@,$(LIBDIR),' \
		-e 's,@includedir@,$(inc_prefix)/include,' \
		-e 's,@VERSION@,$(VERSION).$(MINOR),' \
		-e 's,@deps@,$(DEPS),' \
		$< >$@

_makenames: _makenames.c cap_names.list.h
	gcc -Iinclude -O2 $< -o $@

cap_names.h: _makenames
	./_makenames > cap_names.h

$(GPERF_OUTPUT): cap_names.list.h
	perl -e 'print "struct __cap_token_s { const char *name; int index; };\n%{\nconst struct __cap_token_s *__cap_lookup_name(const char *, size_t);\n%}\n%%\n"; while ($$l = <>) { $$l =~ s/[\{\"]//g; $$l =~ s/\}.*// ; print $$l; }' < $< | gperf --ignore-case --language=ANSI-C --readonly --null-strings --global-table --hash-function-name=__cap_hash_name --lookup-function-name="__cap_lookup_name" -c -t -m20 $(INDENT) > $@

cap_names.list.h: Makefile $(KERNEL_HEADERS)/linux/capability.h
	@echo "=> making $@ from $(KERNEL_HEADERS)/linux/capability.h"
	perl -e 'while ($$l=<>) { if ($$l =~ /^\#define[ \t](CAP[_A-Z]+)[ \t]+([0-9]+)\s+$$/) { $$tok=$$1; $$val=$$2; $$tok =~ tr/A-Z/a-z/; print "{\"$$tok\",$$val},\n"; } }' $(KERNEL_HEADERS)/linux/capability.h | fgrep -v 0x > $@

$(STALIBNAME): $(OBJS)
	$(AR) rcs $@ $^
	$(RANLIB) $@

$(MINLIBNAME): $(OBJS)
	$(LD) $(CFLAGS) $(LDFLAGS) -Wl,-soname,$(MAJLIBNAME) -o $@ $^
	ln -sf $(MINLIBNAME) $(MAJLIBNAME)
	ln -sf $(MAJLIBNAME) $(LIBNAME)

%.o: %.c $(INCLS)
	$(CC) $(CFLAGS) $(IPATH) -c $< -o $@

cap_text.o: cap_text.c $(USE_GPERF_OUTPUT) $(INCLS)
	$(CC) $(CFLAGS) $(IPATH) $(INCLUDE_GPERF_OUTPUT) -c $< -o $@

install: all
	mkdir -p -m 0755 $(FAKEROOT)$(INCDIR)/sys
	install -m 0644 include/sys/capability.h $(FAKEROOT)$(INCDIR)/sys
	mkdir -p -m 0755 $(FAKEROOT)$(LIBDIR)
	install -m 0644 $(STALIBNAME) $(FAKEROOT)$(LIBDIR)/$(STALIBNAME)
	install -m 0644 $(MINLIBNAME) $(FAKEROOT)$(LIBDIR)/$(MINLIBNAME)
	ln -sf $(MINLIBNAME) $(FAKEROOT)$(LIBDIR)/$(MAJLIBNAME)
	ln -sf $(MAJLIBNAME) $(FAKEROOT)$(LIBDIR)/$(LIBNAME)
ifeq ($(FAKEROOT),)
	-/sbin/ldconfig
endif
#	mkdir -p -m 0755 $(FAKEROOT)$(PKGCONFIGDIR)
#	install -m 0644 libcap.pc $(FAKEROOT)$(PKGCONFIGDIR)/libcap.pc

clean:
	$(LOCALCLEAN)
	rm -f $(OBJS) $(LIBNAME)* $(STALIBNAME) libcap.pc
	rm -f cap_names.h cap_names.list.h _makenames $(GPERF_OUTPUT)
	cd include/sys && $(LOCALCLEAN)
