# This Makefile.am is in the public domain

SUBDIRS = include microhttpd .

if RUN_LIBCURL_TESTS
SUBDIRS += testcurl
if RUN_ZZUF_TESTS
SUBDIRS += testzzuf
endif
endif

# Finally (last!) also build experimental lib...
if HAVE_EXPERIMENTAL
SUBDIRS += microhttpd_ws lib
endif

if BUILD_EXAMPLES
SUBDIRS += examples
endif

if BUILD_TOOLS
SUBDIRS += tools
endif

EXTRA_DIST = \
 datadir/cert-and-key.pem \
 datadir/cert-and-key-for-wireshark.pem

.NOTPARALLEL:
