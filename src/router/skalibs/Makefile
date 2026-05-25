#
# This Makefile requires GNU make.
#
# Do not make changes here.
# Use the included .mak files.
#

it: all

make_need := 3.81
ifeq "" "$(strip $(filter $(make_need), $(firstword $(sort $(make_need) $(MAKE_VERSION)))))"
$(error Your make ($(MAKE_VERSION)) is too old. You need $(make_need) or newer)
endif

deduprec = $(if $(1),$(firstword $(1)) $(call deduprec,$(filter-out $(firstword $(1)),$(wordlist 2,$(words $(1)),$(1)))),)
dedup = $(strip $(call deduprec,$(1)))

-include config.mak
include package/deps.mak

version_m := $(basename $(version))
version_M := $(basename $(version_m))
version_l := $(basename $(version_M))
CPPFLAGS_ALL := $(CPPFLAGS_AUTO) $(CPPFLAGS)
CFLAGS_ALL := $(CFLAGS_AUTO) $(CFLAGS)
LDFLAGS_ALL := $(LDFLAGS_AUTO) $(LDFLAGS)
LDLIBS_ALL := $(LDLIBS_AUTO) $(LDLIBS)
AR := $(CROSS_COMPILE)ar
RANLIB := $(CROSS_COMPILE)ranlib
STRIP := $(CROSS_COMPILE)strip
INSTALL := ./tools/install.sh

TYPES := size uid gid pid time dev ino

ALL_SRCS := $(sort $(wildcard src/lib*/*.c))
ALL_DOBJS := $(ALL_SRCS:%.c=%.lo)
ifeq ($(strip $(STATIC_LIBS_ARE_PIC)),)
ALL_SOBJS := $(ALL_SRCS:%.c=%.o)
CFLAGS_SHARED := -fPIC
else
ALL_SOBJS := $(ALL_DOBJS)
CFLAGS_SHARED :=
endif
ALL_LIBS := $(SHARED_LIBS) $(STATIC_LIBS)
SYSTEM_LIBS := $(call dedup,$(SOCKET_LIB) $(UTIL_LIB) $(PTHREAD_LIB) $(SYSCLOCK_LIB) $(TIMER_LIB) $(SPAWN_LIB))
BUILT_INCLUDES := \
src/include/$(package)/sysdeps.h \
src/include/$(package)/uint16.h \
src/include/$(package)/uint32.h \
src/include/$(package)/uint64.h \
src/include/$(package)/types.h \
src/include/$(package)/ip46.h
ALL_INCLUDES := $(sort $(BUILT_INCLUDES) $(wildcard src/include/$(package)/*.h))
ALL_SYSDEPS := $(wildcard $(sysdeps)/*)
ALL_DATA := $(wildcard src/etc/*)
PC_TARGETS :=
ifdef DO_PKGCONFIG
PC_TARGETS += libskarnet.pc
endif

all: config.mak $(ALL_LIBS) $(ALL_INCLUDES) $(ALL_SYSDEPS) $(ALL_DATA) $(PC_TARGETS)

clean:
	@exec rm -f $(ALL_LIBS) $(ALL_BINS) $(ALL_SOBJS) $(ALL_DOBJS) $(BUILT_INCLUDES) $(PC_TARGETS)

distclean: clean
	@exec rm -rf config.mak src/include/$(package)/config.h sysdeps.cfg

tgz: distclean
	@. package/info && \
	rm -rf /tmp/$$package-$$version && \
	cp -a . /tmp/$$package-$$version && \
	cd /tmp && \
	tar -zpcv --owner=0 --group=0 --numeric-owner --exclude=.git* -f /tmp/$$package-$$version.tar.gz $$package-$$version && \
	sha256sum $$package-$$version.tar.gz > $$package-$$version.tar.gz.sha256 && \
	exec rm -rf /tmp/$$package-$$version

strip: $(ALL_LIBS)
ifneq ($(strip $(STATIC_LIBS)),)
	exec $(STRIP) -x -R .note -R .comment $(STATIC_LIBS)
endif
ifneq ($(strip $(SHARED_LIBS)),)
	exec $(STRIP) -R .note -R .comment $(SHARED_LIBS)
endif

install: install-sysconf install-sysdeps install-dynlib install-lib install-include install-pkgconfig
install-sysconf: $(ALL_DATA:src/etc/%=$(DESTDIR)$(sysconfdir)/%)
install-sysdeps: $(ALL_SYSDEPS:$(sysdeps)/%=$(DESTDIR)$(sysdepdir)/%)
install-dynlib: $(SHARED_LIBS:lib%.$(SHLIB_EXT).xyzzy=$(DESTDIR)$(dynlibdir)/lib%.$(SHLIB_EXT))
install-lib: $(STATIC_LIBS:lib%.a.xyzzy=$(DESTDIR)$(libdir)/lib%.a)
install-include: $(ALL_INCLUDES:src/include/$(package)/%.h=$(DESTDIR)$(includedir)/$(package)/%.h)
install-pkgconfig: $(PC_TARGETS:%=$(DESTDIR)$(pkgconfdir)/%)

ifneq ($(exthome),)

$(DESTDIR)$(exthome): $(DESTDIR)$(home)
	exec $(INSTALL) -l $(notdir $(home)) $(DESTDIR)$(exthome)

update: $(DESTDIR)$(exthome)

global-links: $(DESTDIR)$(exthome) $(SHARED_LIBS:lib%.so.xyzzy=$(DESTDIR)$(sproot)/library.so/lib%.so.$(version_M))

$(DESTDIR)$(sproot)/library.so/lib%.so.$(version_M): $(DESTDIR)$(dynlibdir)/lib%.so.$(version_M)
	exec $(INSTALL) -D -l ..$(subst $(sproot),,$(exthome))/library.so/$(<F) $@

.PHONY: update global-links

endif

$(DESTDIR)$(sysconfdir)/%: src/etc/%
	exec $(INSTALL) -D -m 644 $< $@

$(DESTDIR)$(sysdepdir)/%: $(sysdeps)/%
	exec $(INSTALL) -D -m 644 $< $@

$(DESTDIR)$(dynlibdir)/lib%.so $(DESTDIR)$(dynlibdir)/lib%.so.$(version_M) $(DESTDIR)$(dynlibdir)/lib%.so.$(version): lib%.so.xyzzy
	$(INSTALL) -D -m 755 $< $(@D)/lib$(*F).so.$(version) && \
	$(INSTALL) -l lib$(*F).so.$(version) $(@D)/lib$(*F).so.$(version_M) && \
	exec $(INSTALL) -l lib$(*F).so.$(version_M) $(@D)/lib$(*F).so

$(DESTDIR)$(libdir)/lib%.a: lib%.a.xyzzy
	exec $(INSTALL) -D -m 644 $< $@

$(DESTDIR)$(includedir)/$(package)/%.h: src/include/$(package)/%.h
	exec $(INSTALL) -D -m 644 $< $@

$(DESTDIR)$(pkgconfdir)/lib%.pc: lib%.pc
	exec $(INSTALL) -D -m 644 $< $@

%.o: %.c
	exec $(CC) $(CPPFLAGS_ALL) $(CFLAGS_ALL) -c -o $@ $<

%.lo: %.c
	exec $(CC) $(CPPFLAGS_ALL) $(CFLAGS_ALL) $(CFLAGS_SHARED) -c -o $@ $<

libskarnet.a.xyzzy: $(ALL_SOBJS)
	exec $(AR) rc $@ $^
	exec $(RANLIB) $@

libskarnet.so.xyzzy: $(ALL_DOBJS)
	exec $(CC) -o $@ $(CFLAGS_ALL) $(CFLAGS_SHARED) $(LDFLAGS_ALL) $(LDFLAGS_SHARED) -Wl,-soname,libskarnet.so.$(version_M) $(LDFLAGS_RPATH) $^ $(SYSTEM_LIBS)

libskarnet.pc:
	exec env \
	  prefix="$(prefix)" \
	  library="skarnet" \
	  includedir="$(includedir)" \
	  dynlibdir="$(dynlibdir)" \
	  libdir="$(libdir)" \
	  extra_includedirs="$(extra_includedirs)" \
	  extra_libdirs="$(extra_libdirs)" \
	  extra_libs="$(SYSTEM_LIBS)" \
	  description="The skarnet.org C programming library (skalibs)" \
	  url="https://skarnet.org/software/skalibs/" \
	  ldlibs="$(LDLIBS)" \
	  ./tools/gen-dotpc.sh > $@.tmp
	exec mv -f $@.tmp $@

.PHONY: it all clean distclean tgz strip install install-data install-sysdeps install-dynlib install-lib install-include

.DELETE_ON_ERROR:

src/include/$(package)/sysdeps.h: $(sysdeps)/sysdeps $(sysdeps)/target
	exec tools/gen-sysdepsh.sh `cat $(sysdeps)/target` < $(sysdeps)/sysdeps > $@

src/include/$(package)/uint16.h: $(sysdeps)/sysdeps src/headers/uint16-bswap src/headers/bits-header src/headers/bits-footer src/headers/bits-lendian src/headers/bits-bendian src/headers/bits-template src/headers/uint64-include src/include/$(package)/uint64.h
	exec tools/gen-bits.sh $(sysdeps)/sysdeps 16 6 7 5 17 > $@

src/include/$(package)/uint32.h: $(sysdeps)/sysdeps src/headers/uint32-bswap src/headers/bits-header src/headers/bits-footer src/headers/bits-lendian src/headers/bits-bendian src/headers/bits-template src/headers/uint64-include src/include/$(package)/uint64.h
	exec tools/gen-bits.sh $(sysdeps)/sysdeps 32 11 13 9 33 > $@

src/include/$(package)/uint64.h: $(sysdeps)/sysdeps src/headers/uint64-bswap src/headers/bits-header src/headers/bits-footer src/headers/bits-lendian src/headers/bits-bendian src/headers/bits-template src/headers/uint64-ulong64 src/headers/uint64-noulong64 src/headers/uint64-defs src/headers/uint64-macros
	exec tools/gen-bits.sh $(sysdeps)/sysdeps 64 21 25 17 65 > $@

src/include/$(package)/types.h: src/include/$(package)/uint16.h src/include/$(package)/uint32.h src/include/$(package)/uint64.h $(sysdeps)/sysdeps src/headers/types-header src/headers/types-footer src/headers/unsigned-template src/headers/signed-template
	exec tools/gen-types.sh $(sysdeps)/sysdeps $(TYPES) > $@

src/include/$(package)/ip46.h: src/include/$(package)/fmtscan.h src/include/$(package)/tai.h src/include/$(package)/socket.h $(sysdeps)/sysdeps src/headers/ip46-header src/headers/ip46-footer src/headers/ip46-with src/headers/ip46-without
	@{ \
	  cat src/headers/ip46-header ; \
	  if $(ipv6) && grep -qF 'ipv6: yes' $(sysdeps)/sysdeps ; then cat src/headers/ip46-with ; \
	  else cat src/headers/ip46-without ; \
	  fi ; \
	  exec cat src/headers/ip46-footer ; \
	} > $@

ifeq ($(SHLIB_EXT),dylib)
# MacOS weirdness and 3-semver forcing

version_X := $(shell exec expr 256 '*' $(version_l) + $(subst .,,$(suffix $(version_M))))
version_XY := $(version_X)$(suffix $(version_m))
version_XYZ := $(version_XY)$(suffix $(version))

ifneq ($(exthome),)
global-links: $(SHARED_LIBS:lib%.dylib.xyzzy=$(DESTDIR)$(sproot)/library.so/lib%.$(version_X).dylib)

$(DESTDIR)$(sproot)/library.so/lib%.$(version_X).dylib: $(DESTDIR)$(sproot)/library.so/lib%.$(version_M).dylib
	exec $(INSTALL) -l $(<F) $@
endif

$(DESTDIR)$(dynlibdir)/lib%.dylib $(DESTDIR)$(dynlibdir)/lib%.$(version_X).dylib $(DESTDIR)$(dynlibdir)/lib%.$(version_M).dylib $(DESTDIR)$(dynlibdir)/lib%.$(version).dylib: lib%.dylib.xyzzy
	$(INSTALL) -D -m 755 $< $(@D)/lib$(*F).$(version).dylib && \
	$(INSTALL) -l lib$(*F).$(version).dylib $(@D)/lib$(*F).$(version_M).dylib && \
	$(INSTALL) -l lib$(*F).$(version_M).dylib $(@D)/lib$(*F).$(version_X).dylib && \
	exec $(INSTALL) -l lib$(*F).$(version_X).dylib $(@D)/lib$(*F).dylib

libskarnet.dylib.xyzzy: $(ALL_DOBJS)
	exec $(CC) -o $@ $(CFLAGS_ALL) $(CFLAGS_SHARED) $(LDFLAGS_ALL) $(LDFLAGS_SHARED) -Wl,-dylib_install_name,$(dynlibdir)/libskarnet.$(version_M).dylib -Wl,-dylib_compatibility_version,$(version_XY) -Wl,-dylib_current_version,$(version_XYZ) $^ $(SYSTEM_LIBS)
endif
