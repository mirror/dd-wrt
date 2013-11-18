# Automake description for gpsd
# $Id: Makefile.am 4135 2006-12-14 19:12:29Z ckuethe $

#SUBDIRS = contrib

#
# Build stuff depending on Motif
#
if HAVE_MOTIF
MOTIF_PROGS = xgps xgpsspeed
endif

XMLTO = xmlto

if HAVE_DBUS
INCLUDES = $(DBUS_CFLAGS) $(DBUS_GLIB_CFLAGS) -DDBUS_API_SUBJECT_TO_CHANGE=1
DBUSPROGS = gpxlogger
gpxlogger_LDADD = $(DBUS_GLIB_LIBS) libgps.la  -lm
endif

if HAVE_RTCM104
RTCM104PROGS = rtcmdecode
RTCM104PAGES = rtcmdecode.1
endif

bin_PROGRAMS = $(DBUSPROGS) $(MOTIF_PROGS) $(RTCM104PROGS) gpsctl gpspipe gpsflash cgps cgpxlogger sirfmon
sbin_PROGRAMS = gpsd
check_PROGRAMS = bits_test gpsmm_test packet_test
if HAVE_PYTHON
dist_bin_SCRIPTS = gpsprof gpsfake gpscat
python_PYTHON = gps.py gpsfake.py gpspacket.so
PYTHONPAGES = gpsprof.1 gpsfake.1
endif

#
# Build xgps
#
xgps_SOURCES = display.c display.h xgps.c
xgps_LDADD = $(LIBM) $(LIBC) $(LIBNSL) $(LIBSOCKET) $(XM_LIBS) $(XT_LIBS) $(X_LIBS) $(X_PRE_LIBS) libgps.la -lm $(LIBPTHREAD)

#
# Build xgpsspeed
#
xgpsspeed_c_sources = \
	xgpsspeed.c \
	Tachometer.c \
	TachometerP.h \
	Tachometer.h
xgpsspeed_SOURCES = \
	$(xgpsspeed_c_sources) \
	xgpsspeed.icon
xgpsspeed_LDADD = $(LIBM) $(LIBC) $(LIBNSL) $(LIBSOCKET) $(XAW_LIBS) $(XM_LIBS) $(X_LIBS) $(X_PRE_LIBS) libgps.la -lm $(LIBPTHREAD)

#
# Build cgps
#
cgps_SOURCES = cgps.c
cgps_LDADD = $(LIBM) $(LIBC) $(LIBNSL) $(LIBSOCKET) -lncurses libgps.la -lm $(LIBPTHREAD)

#
# Build cgpxlogger
#
cgpxlogger_SOURCES = cgpxlogger.c
cgpxlogger_LDADD = $(LIBC) $(LIBNSL) $(LIBSOCKET) libgps.la -lm $(LIBPTHREAD)

#
# Build gpsd
#
gpsd_SOURCES = gpsd_dbus.h gpsd_dbus.c gpsd.c
gpsd_LDADD = $(DBUS_LIBS) $(LIBM) libgps.la -lm $(LIBPTHREAD)

#
# Build gpsctl
#
gpsctl_SOURCES = gpsctl.c
gpsctl_LDADD = $(LIBM) libgps.la -lm $(LIBPTHREAD)

#
# Build gpspipe
#
gpspipe_SOURCES = gpspipe.c
gpspipe_LDADD = $(DBUS_LIBS) $(LIBM) libgps.la -lm

#
# Build sirfmon
#
sirfmon_SOURCES = sirfmon.c
sirfmon_LDADD = $(LIBM) -lncurses libgps.la -lm $(LIBPTHREAD)

#
# Build rtcmdecode
#
rtcmdecode_SOURCES = rtcmdecode.c
rtcmdecode_LDADD = $(LIBM) -lncurses libgps.la -lm $(LIBPTHREAD)

#
# Build gpsflash
#
gpsflash_SOURCES = gpsflash.c gpsflash.h sirfflash.c
gpsflash_LDADD = $(LIBM) libgps.la -lm

#
# Build shared libraries
#
libgps_la_LDFLAGS = -version-number 16:0:0
lib_LTLIBRARIES = libgps.la

libgps_c_sources = \
	netlib.c \
	nmea_parse.c \
	serial.c \
	drivers.c \
	zodiac.c \
	garmin.c \
	tsip.c \
	evermore.c \
	italk.c \
	libgpsd_core.c \
	subframe.c \
	ntpshm.c \
	libgps.c \
	packet.c \
	gpsutils.c \
	geoid.c \
	dgnss.c \
	dgpsip.c \
	ntrip.c \
	sirf.c \
	report.c \
	isgps.c \
	rtcm.c \
	rtcm.h \
	srecord.c \
	bits.h \
	timebase.h \
	bsd-base64.c \
	bsd-base64.h \
	strl.c \
	hex.c

BUILT_SOURCES = packet_names.h
packet_names.h: packet_states.h
	sed -e '/^ *\([A-Z][A-Z0-9_]*\),/s//   "\1",/' -e '/_states/s//_names/' <packet_states.h >packet_names.h

libgps_la_SOURCES = $(libgps_c_sources) rtcm.h gpsd_config.h libgpsmm.cpp packet_states.h
nodist_libgps_la_SOURCES = packet_names.h
libgps_la_LIBADD = $(LIBM) $(LIBC) $(LIBNSL) $(LIBSOCKET) $(LIBPTHREAD)

#
# Build Python binding
#
#PYEXTENSIONS = gpspacket.so

#noinst_SCRIPTS = gpspacket.so setup.py

#gpspacket.so: gpspacket.c libgps.la
#	python setup.py build_ext --inplace

#
# Build gpsmm_test
#
gpsmm_test_SOURCES = gpsmm_test.cpp
gpsmm_test_LDADD = $(LIBC) libgps.la -lm

#
# Build bits_test tester
#
bits_test_SOURCES = bits_test.c
bits_test_LDADD = $(LIBC) libgps.la

#
# Build packets tester
#
packet_test_SOURCES = packet_test.c
packet_test_LDADD = $(LIBC) libgps.la -lm

if HAVE_XSLT_PROCESSOR
#
# Create Manpages
#
man_MANS = \
	gpsd.8 \
	gps.1 \
	xgps.1 \
	xgpsspeed.1 \
	cgps.1 \
	cgpxlogger.1 \
	libgps.3 \
	libgpsmm.3 \
	libgpsd.3 \
	sirfmon.1 \
	$(RTCM104PAGES) \
	$(PYTHONPAGES) \
	gpsctl.1 \
	gpscat.1 \
	gpspipe.1 \
	gpsflash.1 \
	rtcm-104.5 \
	srec.5

.xml.1:
	$(MANGENERATOR) $(MANFLAGS) $(MANTARGET) $<

.xml.3:
	$(MANGENERATOR) $(MANFLAGS) $(MANTARGET) $<

.xml.5:
	$(MANGENERATOR) $(MANFLAGS) $(MANTARGET) $<

.xml.8:
	$(MANGENERATOR) $(MANFLAGS) $(MANTARGET) $<

gpsd.8: gpsd.xml

gps.1 xgps.1 xgpsspeed.1 cgps.1 cgpxlogger.1: gps.xml

libgps.3: libgps.xml

libgpsmm.3: libgpsmm.xml

libgpsd.3: libgpsd.xml

gpsctl.1: gpsctl.xml

gpscat.1: gpsctl.xml

gpsprof.1: gpsprof.xml

gpsfake.1: gpsfake.xml

sirfmon.1: sirfmon.xml

gpspipe.1: gpspipe.xml

rtcmdecode.1: rtcmdecode.xml

rtcm-104.5: rtcm-104.xml

srec.5: srec.xml

gpsflash.1: gpsflash.xml

endif

include_HEADERS = gps.h gpsd.h libgpsmm.h 

EXTRA_DIST = \
	autogen.sh \
	README \
	INSTALL \
	COPYING \
	TODO \
	AUTHORS \
	dgpsip-servers \
	floattest.c \
	gpsd.php \
	gpsd.xml \
	gps.xml \
	libgpsd.xml \
	libgps.xml \
	libgpsmm.xml \
	gpsprof.xml \
	gpsfake.xml \
	gpsctl.xml \
	sirfmon.xml \
	gpspipe.xml \
	rtcmdecode.xml \
	rtcm-104.xml \
	srec.xml \
	gpsflash.xml \
	$(man_MANS) \
	xgps.ad \
	xgpsspeed.ad \
	gpsd.spec.in \
	gpsd.spec \
	gpsd.hotplug \
	gpsd.usermap \
	valgrind-audit \
	valgrind-suppressions \
	gpspacket.c \
	driver_proto.c \
	logextract \
	striplog \
	setup.py \
	configure \
	Makefile.in

CLEANFILES = packet_names.h floattest *.core $(PYEXTENSIONS)

# This is not distributed
libgps: libgps.c .libs/libgps.a
	$(CC) $(CFLAGS) -o libgps -lm -DTESTMAIN $(LIBPTHREAD) -g libgps.c .libs/libgps.a

# Report splint warnings
splint:
	@echo "Running splint on daemon and libraries..."
	-splint -redef $(gpsd_SOURCES) $(libgps_c_sources)
	@echo "Running splint on xgps..."
	-splint -exportlocal $(xgps_SOURCES)
	@echo "Running splint on cgps..."
	-splint -exportlocal $(cgps_SOURCES)
	@echo "Running splint on xgpsspeed..."
	-splint -exportlocal $(xgpsspeed_c_sources)
	@echo "Running splint on gpsctl..."
	-splint $(gpsctl_SOURCES)
	@echo "Running splint on sirfmon..."
	-splint $(sirfmon_SOURCES)
	@echo "Running splint on gpspipe..."
	-splint $(gpspipe_SOURCES)
	@echo "Running splint on rtcmdecode..."
	-splint -exportlocal rtcmdecode.c
	@echo "Running splint on gpsflash..."
	-splint $(gpsflash_SOURCES)
	@echo "Running splint on bits_test test harness..."
	-splint $(bits_test_SOURCES)
	@echo "Running splint on packet_test test harness..."
	-splint $(packet_test_SOURCES)

version:
	@echo $(VERSION)

#
# Regression tests begin here
#

# Regression-test the daemon
# This requires gcc4; use of the math coprocessor's on-board trig functions
# apparently increases the accuracy of computation in a way that affects
# the low-order digits of the track field in the O response.  Alas, it looks
# like moving between 32- and 64-bit FPUs can also break regressions.
gps-regress: gpsd
	@echo "Testing the daemon..."
	@for f in test/*.log; do gpsfake -s 57600 -1 -b -p $${f} | grep -v "^GPSD,X" >test/test.chk; diff -ub $${f}.chk test/test.chk; done; rm test/test.chk

# Test that super-raw mode works. Compare each logfile against itself 
# dumped through the daemon running in R=2 mode.  (This test is not
# included in the normal regressions.)
raw-regress:
	@echo "Testing super-raw mode..."
	@for f in test/*.log; do gpsfake -s 57600 -1 -b -p -r "r=2" $${f} | ./striplog -1 >test/test1.chk; ./striplog <$${f} >test/test2.chk; cmp test/test[12].chk; done; rm test/test[12].chk

# Build the regression tests for the daemon.
gps-makeregress: gpsd
	@for f in test/*.log; do gpsfake -s 57600 -1 -b -p $${f} | grep -v "^GPSD,X" >$${f}.chk; done

# Regression-test the RTCM decoder.
rtcm-regress: rtcmdecode
	@echo "Testing RTCM decoding..."
	@for f in test/*.rtcm; do rtcmdecode <$${f} >test/test.chk; diff -ub $${f}.chk test/test.chk; done; rm test/test.chk

# Rebuild the RTCM regression tests.
rtcm-makeregress: rtcmdecode
	@for f in test/*.rtcm; do rtcmdecode <$${f} >$${f}.chk; done

# Regression-test the packet getter.
packet-regress: packet_test
	@echo "Testing detection of invalid packets..."
	@packet_test | diff -u test/packet.test.chk -

# Rebuild the packet-getter regression test
packet-makeregress: packet_test
	packet_test >test/packet.test.chk

# Test RTCM encoding using passthrough mode.
undump-regress: rtcmdecode
	@echo "Testing RTCM encoding..."
	@rtcmdecode -p <test/undump.sample >/tmp/undump.test
	@diff -u test/undump.sample /tmp/undump.test
	@rm /tmp/undump.test

# Do all normal regression tests
testregress: gps-regress rtcm-regress packet-regress undump-regress
	@echo "Regressions complete."
