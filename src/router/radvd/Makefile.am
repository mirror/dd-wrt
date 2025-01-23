#
#
#   Authors:
#    Lars Fenneberg		<lf@elemental.net>	 
#
#   This software is Copyright 1996,1997 by the above mentioned author(s), 
#   All Rights Reserved.
#
#   The license which is distributed with this software in the file COPYRIGHT
#   applies to this software. If your distribution is missing this file, you
#   may request it from <reubenhwk@gmail.com>.
#


# $(CC) $(DEFS) -I. [$(mumble_CPPFLAGS)|$(AM_CPPFLAGS)] $(CPPFLAGS) [$(mumble_CFLAGS)|$(AM_CFLAGS)] $(CFLAGS)
# mumble_CPPFLAGS override AM_CPPFLAGS
# CPP is C PreProcessor

AUTOMAKE_OPTIONS = 1.2 foreign subdir-objects

###  Compiler flags and such ###

DEFS = \
	-DINET6=1 \
	-DLOG_FACILITY=$(LOG_FACILITY) \
	-DPATH_RADVD_CONF=\"$(PATH_RADVD_CONF)\" \
	-DPATH_RADVD_LOG=\"$(PATH_RADVD_LOG)\" \
	-DPATH_RADVD_PID=\"$(PATH_RADVD_PID)\" \
	-DVERSION=\"$(VERSION)\"

ENABLE_WARNINGS = \
	-Wall \
	-Wcast-qual \
	-Wmissing-declarations \
	-Wmissing-prototypes \
	-Wpointer-arith \
	-Wstrict-prototypes

DISABLE_WARNINGS = \
	-Wno-strict-aliasing \
	-Wno-cast-align

AM_CFLAGS = \
	-fno-strict-aliasing \
	$(ENABLE_WARNINGS) \
	$(DISABLE_WARNINGS)

if HAVE_STACK_PROTECTOR
AM_CFLAGS += \
	-fstack-protector
endif

### CPP is C PreProcessor ###

AM_YFLAGS = -d

noinst_LIBRARIES = libradvd-parser.a

radvd_LDADD = \
	libradvd-parser.a \
	@CONDITIONAL_SOURCES@

### Programs and sources ###

scanner.c: gram.h
gram.h: gram.c

libradvd_parser_a_SOURCES = \
	gram.h \
	gram.y \
	scanner.l

sbin_PROGRAMS = \
	radvd \
	radvdump

EXTRA_radvd_SOURCES = \
	device-bsd44.c \
	device-linux.c \
	netlink.c \
	netlink.h \
	privsep-linux.c

radvd_SOURCES = \
	defaults.h \
	includes.h \
	log.c \
	log.h \
	pathnames.h \
	radvd.h \
	recv.c \
	socket.c \
	util.c \
	device-common.c \
	interface.c \
	process.c \
	radvd.c \
	send.c \
	timer.c

radvdump_SOURCES = \
	defaults.h \
	includes.h \
	log.c \
	log.h \
	pathnames.h \
	radvd.h \
	recv.c \
	socket.c \
	util.c \
	radvdump.c

### coverage ###

if HAVE_CHECK
cov: check
	gcov *.c *.h *.l *.y
	lcov --capture --directory . --output-file coverage.info
	genhtml coverage.info --output-directory cov
else
cov:
	$(error libcheck not available)
endif

if HAVE_CHECK
else
check:
	$(error libcheck not available)
endif

### man pages ###

SUFFIXES = .man

.man:
	$(AM_V_GEN)$(SED) \
		-e 's,@''VERSION''@,$(VERSION),g' \
		-e 's,@''sbindir''@,$(sbindir),g' \
		-e 's,@''PATH_RADVD_CONF''@,$(PATH_RADVD_CONF),g' \
		-e 's,@''PATH_RADVD_PID''@,$(PATH_RADVD_PID),g' \
		-e 's,@''PATH_RADVD_LOG''@,$(PATH_RADVD_LOG),g' \
		-e 's,@''LOG_FACILITY''@,$(LOG_FACILITY),g' \
		$< > $@

man_MANS = \
	radvd.8 \
	radvd.conf.5 \
	radvdump.8


### build support ###

indent:
	clang-format -i *.c *.h test/*.c test/*.h

EXTRA_DIST = \
	autogen-container.sh \
	autogen.sh \
	CHANGES \
	.clang-format \
	contrib/95-radvd-gen \
	contrib/unset_addrs.sh \
	COPYRIGHT \
	copyright.blurb \
	Docker.autogen \
	.github/workflows/bsd.yaml \
	.github/workflows/buildroot.yaml \
	.github/workflows/linux.yaml \
	.gitignore \
	INTRO.html \
	radvd.8.man \
	radvd.conf.5.man \
	radvd.conf.example \
	radvd.service.in \
	radvdump.8.man \
	README \
	RELEASE-PROCESS.md \
	redhat/systemd/radvd.service \
	redhat/systemd/radvd.spec \
	redhat/systemd/radvd-tmpfs.conf \
	redhat/SysV/radvd.conf.empty \
	redhat/SysV/radvd.init \
	redhat/SysV/radvd.spec \
	redhat/SysV/radvd.sysconfig \
	redhat/SysV/radvd-tmpfs.conf \
	test/check.c \
	test/print_safe_buffer.c \
	test/print_safe_buffer.h \
	test/send.c \
	test/test1.conf \
	test/test_build.sh \
	test/test_dnssl1.conf \
	test/test_dnssl2.conf \
	test/test_dnssl3.conf \
	test/test_dnssl4.conf \
	test/test_dnssl5.conf \
	test/test_dnssl6.conf \
	test/test_rdnss.conf \
	test/test_rdnss_long.conf \
	test/util.c \
	TODO \
	.travis.yml

EXTENSIONS = gz bz2 lz lzma xz tarZ shar zip
HASHES = sha256 sha512

# Generate clearsigned checksum files.
# Generate detached signatures of the tarballs.
sign:
	$(AM_V_GEN)for e in $(EXTENSIONS); do \
		if [ -f radvd-$(VERSION).tar.$$e ]; then \
			gpg --armor --sign --detach-sign radvd-$(VERSION).tar.$$e; \
			for h in $(HASHES); do \
				$${h}sum --tag radvd-$(VERSION).tar.$$e > radvd-$(VERSION).tar.$$e.$$h || exit 1; \
				gpg --clear-sign radvd-$(VERSION).tar.$$e.$$h || exit 1; \
				mv -f radvd-$(VERSION).tar.$$e.$$h.asc radvd-$(VERSION).tar.$$e.$$h || exit 1; \
			done; \
		fi; \
	done

# Verify clearsigned checksum files.
# Verify detached signatures of the tarballs.
#
# Be careful to verify the clearsign, take ONLY the signed part, and then
# verify the checksum contained in that (ignore checksums OUTSIDE the
# clearsigned part).
verify:
	$(AM_V_GEN)for e in $(EXTENSIONS); do \
		if [ -f radvd-$(VERSION).tar.$$e ]; then \
			gpg --verify radvd-$(VERSION).tar.$$e.asc; \
			for h in $(HASHES); do \
				rm -f radvd-$(VERSION).tar.$$e.$$h.verified || exit 1; \
				gpg --output radvd-$(VERSION).tar.$$e.$$h.verified --verify radvd-$(VERSION).tar.$$e.$$h || exit 1; \
				if ! $${h}sum -c radvd-$(VERSION).tar.$$e.$$h.verified ; then \
					rm -f radvd-$(VERSION).tar.$$e.$$h.verified; \
					exit 1; \
				fi; \
				rm -f radvd-$(VERSION).tar.$$e.$$h.verified; \
			done; \
		fi; \
	done

html:
	$(AM_V_GEN)echo "<ul>"; \
	for e in $(EXTENSIONS); do \
		if [ -f radvd-$(VERSION).tar.$$e ]; then \
			echo "<li>"; \
			echo "<a href=\"dist/radvd-$(VERSION).tar.$$e\">radvd-$(VERSION).tar.$$e</a>"; \
			for h in $(HASHES); do \
				echo "<a href=\"dist/radvd-$(VERSION).tar.$$e.$$h\">[$$h]</a>"; \
			done; \
			echo "<a href=\"dist/radvd-$(VERSION).tar.$$e.asc\">[GPG]</a>"; \
			echo "</li>"; \
		fi; \
	done; \
	echo "</ul>"

packages:
	@if [[ "$$(git diff | wc -l)" != "0" ]] ; then printf "\n\n\tYou have local changes in the working copy...\n\n\n" && git diff && false ; fi
	@if [[ "$$(git rev-parse HEAD)" != "$$(git rev-parse v$(VERSION)^{commit})" ]] ; then printf "\n\n\tv$(VERSION) tag missing, or not checked out...\n\n\n" && false ; fi
	rm -f radvd-$(VERSION).tar*
	$(MAKE) dist-gzip
	rm -rf radvd-$(VERSION)
	tar zfx radvd-$(VERSION).tar.gz
	cd radvd-$(VERSION) && ./configure --with-check && $(MAKE) check
	rm -rf radvd-$(VERSION)
	$(MAKE) dist-xz
	$(MAKE) sign
	$(MAKE) verify
	$(MAKE) html
	@printf "\n\n\tDont forget to push the v$(VERSION) tag and this branch to origin (git push origin v$(VERSION) master)\n\n\n"


dist-hook:
	rm -f $(distdir)/gram.c
	rm -f $(distdir)/gram.h
	rm -f $(distdir)/scanner.c

CLEANFILES = \
	radvd.8 \
	radvd.conf.5 \
	radvdump.8 \
	gram.c \
	gram.h \
	gram.dot \
	scanner.c \
	scanner.h \
	gram.dot

### Explicit Dependencies ###

radvd_DEPENDENCIES = \
	@CONDITIONAL_SOURCES@ \
	libradvd-parser.a

### make check ###

TESTS = check_all

check_PROGRAMS = check_all

EXTRA_check_all_SOURCES = \
	device-bsd44.c \
	device-linux.c \
	netlink.c \
	netlink.h \
	privsep-linux.c

check_all_SOURCES = \
	test/print_safe_buffer.h \
	test/print_safe_buffer.c \
	test/check.c \
	device-common.c \
	interface.c \
	log.c \
	send.c \
	timer.c \
	util.c

check_all_CFLAGS = \
	@CHECK_CFLAGS@ \
	-DUNIT_TEST

check_all_LDADD = \
	@CHECK_LIBS@ \
	@CONDITIONAL_SOURCES@ \
	libradvd-parser.a

DISTCHECK_CONFIGURE_FLAGS = \
  --with-systemdsystemunitdir=$$dc_install_base/$(systemdsystemunitdir)

if HAVE_SYSTEMD
systemdsystemunit_DATA = radvd.service
endif

