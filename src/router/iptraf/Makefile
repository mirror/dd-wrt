# The default target of this Makefile is this
all::

# Define V=1 to have a more verbose compile.
#
# Define NO_PANEL if you don't want to use -lpanel.
#
# Define NEEDS_NCURSES5 if you need linking with ncurses5.
#
# Define NEEDS_NCURSESW5 if you need linking with ncursesw5.
#
# Define NEEDS_NCURSES6 if you need linking with ncurses6.
#
# Define NEEDS_NCURSESW6 if you need linking with ncursesw6.


VERSION-FILE: FORCE
	@$(SHELL_PATH) ./GEN-VERSION-FILE
-include VERSION-FILE

CFLAGS = $(COPTS) $(MIPS16_OPT) $(THUMB) -D_GNU_SOURCE -Wall -W -std=gnu99
LDFLAGS =
ALL_CFLAGS = $(CPPFLAGS) $(CFLAGS)
ALL_LDFLAGS = $(LDFLAGS)
STRIP ?= strip

prefix =
sbindir_relative = sbin
sbindir = $(prefix)/$(sbindir_relative)
mandir = $(prefix)/share/man
sharedir = $(prefix)/share
localedir = $(sharedir)/locale
lib = lib
man8dir = $(mandir)/man8

# DESTDIR=
pathsep = :

export prefix bindir sharedir sysconfdir gitwebdir localedir

#CC = cc
RM = rm -f
INSTALL = install
RPMBUILD = rpmbuild
TAR = tar

### --- END CONFIGURATION SECTION ---


# Those must not be GNU-specific; they are shared with perl/ which may
# be built by a different compiler. (Note that this is an artifact now
# but it still might be nice to keep that distinction.)
BASIC_CFLAGS = -I. -Isrc/
BASIC_LDFLAGS =

# Guard against environment variables
iptraf-h :=
iptraf-o :=

ALL_PROGRAMS := iptraf-ng

ifndef SHELL_PATH
	SHELL_PATH = /bin/sh
endif

iptraf-h += src/tui/input.h
iptraf-h += src/tui/labels.h
iptraf-h += src/tui/listbox.h
iptraf-h += src/tui/menurt.h
iptraf-h += src/tui/msgboxes.h
iptraf-h += src/tui/winops.h
iptraf-h += src/iptraf-ng-compat.h
iptraf-h += src/parse-options.h
iptraf-h += src/packet.h
iptraf-h += src/tcptable.h
iptraf-h += src/othptab.h
iptraf-h += src/ifstats.h
iptraf-h += src/deskman.h
iptraf-h += src/hostmon.h
iptraf-h += src/fltedit.h
iptraf-h += src/cidr.h
iptraf-h += src/fltselect.h
iptraf-h += src/ipfilter.h
iptraf-h += src/fltmgr.h
iptraf-h += src/ipfrag.h
iptraf-h += src/serv.h
iptraf-h += src/servname.h
iptraf-h += src/timer.h
iptraf-h += src/ifaces.h
iptraf-h += src/error.h
iptraf-h += src/revname.h
iptraf-h += src/log.h
iptraf-h += src/pktsize.h
iptraf-h += src/landesc.h
iptraf-h += src/dirs.h
iptraf-h += src/getpath.h
iptraf-h += src/options.h
iptraf-h += src/promisc.h
iptraf-h += src/parseproto.h
iptraf-h += src/addproto.h
iptraf-h += src/arphdr.h
iptraf-h += src/attrs.h
iptraf-h += src/fltdefs.h
iptraf-h += src/logvars.h
iptraf-h += src/list.h
iptraf-h += src/counters.h
iptraf-h += src/rate.h
iptraf-h += src/built-in.h
iptraf-h += src/sockaddr.h
iptraf-h += src/capt.h
iptraf-h += src/capt-recvmsg.h
iptraf-h += src/capt-recvmmsg.h
iptraf-h += src/capt-mmap-v2.h
iptraf-h += src/capt-mmap-v3.h

iptraf-o += src/tui/input.o
iptraf-o += src/tui/labels.o
iptraf-o += src/tui/listbox.o
iptraf-o += src/tui/menurt.o
iptraf-o += src/tui/msgboxes.o
iptraf-o += src/tui/winops.o
iptraf-o += src/error.o
iptraf-o += src/log.o
iptraf-o += src/getpath.o
iptraf-o += src/parseproto.o
iptraf-o += src/fltselect.o
iptraf-o += src/ipfilter.o
iptraf-o += src/fltmgr.o
iptraf-o += src/ipfrag.o
iptraf-o += src/serv.o
iptraf-o += src/servname.o
iptraf-o += src/timer.o
iptraf-o += src/revname.o
iptraf-o += src/pktsize.o
iptraf-o += src/landesc.o
iptraf-o += src/options.o
iptraf-o += src/promisc.o
iptraf-o += src/ifaces.o
iptraf-o += src/usage.o
iptraf-o += src/iptraf.o
iptraf-o += src/itrafmon.o
iptraf-o += src/wrapper.o
iptraf-o += src/parse-options.o
iptraf-o += src/packet.o
iptraf-o += src/tcptable.o
iptraf-o += src/othptab.o
iptraf-o += src/ifstats.o
iptraf-o += src/detstats.o
iptraf-o += src/deskman.o
iptraf-o += src/hostmon.o
iptraf-o += src/fltedit.o
iptraf-o += src/cidr.o
iptraf-o += src/counters.o
iptraf-o += src/rate.o
iptraf-o += src/capture-pkt.o
iptraf-o += src/sockaddr.o
iptraf-o += src/capt.o
iptraf-o += src/capt-recvmsg.o
iptraf-o += src/capt-recvmmsg.o
iptraf-o += src/capt-mmap-v2.o
iptraf-o += src/capt-mmap-v3.o
iptraf-o += src/rvnamed.o

ifndef sysconfdir
ifeq ($(prefix),/usr)
sysconfdir = /etc
else
sysconfdir = etc
endif
endif

ifdef CHECK_HEADER_DEPENDENCIES
COMPUTE_HEADER_DEPENDENCIES = no
USE_COMPUTED_HEADER_DEPENDENCIES =
endif

ifndef COMPUTE_HEADER_DEPENDENCIES
COMPUTE_HEADER_DEPENDENCIES = auto
endif

ifeq ($(COMPUTE_HEADER_DEPENDENCIES),auto)
dep_check = $(shell $(CC) $(ALL_CFLAGS) \
	-c -MF /dev/null -MMD -MP -x c /dev/null -o /dev/null 2>&1; \
	echo $$?)
ifeq ($(dep_check),0)
override COMPUTE_HEADER_DEPENDENCIES = yes
else
override COMPUTE_HEADER_DEPENDENCIES = no
endif
endif

ifeq ($(COMPUTE_HEADER_DEPENDENCIES),yes)
USE_COMPUTED_HEADER_DEPENDENCIES = YesPlease
else
ifneq ($(COMPUTE_HEADER_DEPENDENCIES),no)
$(error please set COMPUTE_HEADER_DEPENDENCIES to yes, no, or auto \
(not "$(COMPUTE_HEADER_DEPENDENCIES)"))
endif
endif

NCURSES_CFLAGS := -I$(TOP)/ncurses/include
NCURSES_LDFLAGS := -L$(TOP)/ncurses/lib -lncurses
ifndef NO_PANEL
	NCURSES_LDFLAGS += -lpanel
endif

QUIET_SUBDIR0  = +$(MAKE) -C # space to separate -C and subdir
QUIET_SUBDIR1  =

ifneq ($(findstring $(MAKEFLAGS),w),w)
PRINT_DIR = --no-print-directory
else # "make -w"
NO_SUBDIR = :
endif

ifneq ($(findstring $(MAKEFLAGS),s),s)
ifndef V
	QUIET_CC       = @echo '   ' CC $@;
	QUIET_LINK     = @echo '   ' LINK $@;
	QUIET_GEN      = @echo '   ' GEN $@;
	QUIET_SUBDIR0  = +@subdir=
	QUIET_SUBDIR1  = ;$(NO_SUBDIR) echo '   ' SUBDIR $$subdir; \
			 $(MAKE) $(PRINT_DIR) -C $$subdir
	export V
	export QUIET_GEN
	export QUIET_BUILT_IN
endif
endif


DESTDIR_SQ = $(subst ','\'',$(DESTDIR))
sbindir_SQ = $(subst ','\'',$(sbindir))

ALL_CFLAGS += $(BASIC_CFLAGS)
ALL_LDFLAGS += $(BASIC_LDFLAGS)

export TAR INSTALL DESTDIR SHELL_PATH

### Build rules

SHELL = $(SHELL_PATH)

#all:: shell_compatibility_test
#please_set_SHELL_PATH_to_a_more_modern_shell:
#	@$$(:)
#shell_compatibility_test: please_set_SHELL_PATH_to_a_more_modern_shell


all:: $(ALL_PROGRAMS)

iptraf-ng: $(iptraf-o)
	$(QUIET_LINK)$(CC) $(ALL_CFLAGS) -o $@ \
		$(iptraf-o) $(ALL_LDFLAGS) $(NCURSES_LDFLAGS)

src/deskman.o src/iptraf.o: VERSION-FILE
src/deskman.o src/iptraf.o src/capture-pkt.o: EXTRA_CPPFLAGS = \
	-DIPTRAF_VERSION='"$(IPTRAF_VERSION)"' \
	-DIPTRAF_NAME='"iptraf-ng"'

OBJECTS := $(sort $(iptraf-o))

dep_files := $(foreach f,$(OBJECTS),$(dir $f).depend/$(notdir $f).d)
dep_dirs := $(addsuffix .depend,$(sort $(dir $(OBJECTS))))

ifeq ($(COMPUTE_HEADER_DEPENDENCIES),yes)
$(dep_dirs):
	@mkdir -p $@

missing_dep_dirs := $(filter-out $(wildcard $(dep_dirs)),$(dep_dirs))
dep_file = $(dir $@).depend/$(notdir $@).d
dep_args = -MF $(dep_file) -MMD -MP
ifdef CHECK_HEADER_DEPENDENCIES
$(error cannot compute header dependencies outside a normal build. \
Please unset CHECK_HEADER_DEPENDENCIES and try again)
endif
endif

.SUFFIXES:

ifdef PRINT_HEADER_DEPENDENCIES
$(OBJECTS): %.o: %.c FORCE
	echo $^

ifndef CHECK_HEADER_DEPENDENCIES
$(error cannot print header dependencies during a normal build. \
Please set CHECK_HEADER_DEPENDENCIES and try again)
endif
endif

ifndef PRINT_HEADER_DEPENDENCIES
ifdef CHECK_HEADER_DEPENDENCIES
$(OBJECTS): %.o: %.c $(dep_files) FORCE
	@set -e; echo CHECK $@; \
	missing_deps="$(missing_deps)"; \
	if test "$$missing_deps"; \
	then \
		echo missing dependencies: $$missing_deps; \
		false; \
	fi
endif
endif

ifndef CHECK_HEADER_DEPENDENCIES
$(OBJECTS): %.o: %.c $(missing_dep_dirs)
	$(QUIET_CC)$(CC) -o $*.o -c $(dep_args) $(NCURSES_CFLAGS) $(ALL_CFLAGS) $(EXTRA_CPPFLAGS) $<
endif

ifdef USE_COMPUTED_HEADER_DEPENDENCIES
# Take advantage of gcc's on-the-fly dependency generation
# See <http://gcc.gnu.org/gcc-3.0/features.html>.
dep_files_present := $(wildcard $(dep_files))
ifneq ($(dep_files_present),)
include $(dep_files_present)
endif
else
# Dependencies on header files, for platforms that do not support
# the gcc -MMD option.
#
# Dependencies on automatically generated headers such as common-cmds.h
# should _not_ be included here, since they are necessary even when
# building an object for the first time.
#
# XXX. Please check occasionally that these include all dependencies
# gcc detects!

$(OBJECTS): $(iptraf-h)
endif


### Maintainer's dist rules

IPTRAF_TARNAME = iptraf-ng-$(IPTRAF_VERSION)
dist:
	@mkdir -p $(IPTRAF_TARNAME)
	@cp --parents `git ls-files` $(IPTRAF_TARNAME)
	$(TAR) cf $(IPTRAF_TARNAME).tar $(IPTRAF_TARNAME)
	@$(RM) -rf $(IPTRAF_TARNAME)
	gzip -f -9 $(IPTRAF_TARNAME).tar

### Documentation rules
html: Documentation/book1.html
pdf: Documentation/manual.pdf

Documentation/book1.html: Documentation/manual.sgml
	cd Documentation && docbook2html manual.sgml

Documentation/manual.pdf: Documentation/manual.sgml
	cd Documentation && docbook2pdf manual.sgml

Documentation/manual.sgml: Documentation/manual.sgml.in VERSION-FILE
	cat $< | sed \
		-e s/@@version@@/`echo $(IPTRAF_VERSION) | cut -d. -f1-3`/ \
		-e s/@@major@@/`echo $(IPTRAF_VERSION) | cut -d. -f1-2`/ \
	> $@

## TODO: use asciidoc to generate mans

### Installation rules
install: all
#	@echo $(DESTDIR_SQ)$(man8dir)
	$(INSTALL) -d -m 755 '$(DESTDIR_SQ)$(sbindir_SQ)'
	$(INSTALL) $(ALL_PROGRAMS) '$(DESTDIR_SQ)$(sbindir_SQ)'
#	$(INSTALL) -d -m 755 $(DESTDIR)$(man8dir)
#	$(INSTALL) -m 644 src/iptraf-ng.8  $(DESTDIR)$(man8dir)
#	$(INSTALL) -m 644 src/rvnamed-ng.8  $(DESTDIR)$(man8dir)

### Cleaning rules

distclean: clean
	$(RM) configure

clean:
	$(RM) src/*.o src/tui/*.o
	$(RM) $(ALL_PROGRAMS)
	$(RM) -r autom4te.cache
	$(RM) -r $(dep_dirs)
	$(RM) *.spec
	$(RM) $(IPTRAF_TARNAME).tar.gz
	$(RM) config.log config.mak.autogen config.mak.append config.status config.cache
	$(RM) VERSION-FILE

.PHONY: gtags
gtags:
	$(QUIET_GEN) gtags

.PHONY: clean distclean all install FORCE

