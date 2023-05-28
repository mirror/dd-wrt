# This Makefile.am is in the public domain
if HAVE_CURL
curltests = testcurl
if HAVE_ZZUF
if HAVE_SOCAT
zzuftests = testzzuf
endif
endif
endif

SUBDIRS = include microhttpd $(curltests) $(zzuftests) .

# Finally (last!) also build experimental lib...
if HAVE_EXPERIMENTAL
SUBDIRS += microhttpd_ws lib
endif

if BUILD_EXAMPLES
SUBDIRS += examples
endif


EXTRA_DIST = \
 datadir/cert-and-key.pem \
 datadir/cert-and-key-for-wireshark.pem

.NOTPARALLEL:
