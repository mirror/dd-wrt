## $Id
AUTOMAKE_OPTIONS=foreign no-dependencies

noinst_LIBRARIES = libspp.a

SUBDIRS = HttpInspect Stream5

if BUILD_PROCPIDSTATS
PROCPIDSTATS_SOURCE = sfprocpidstats.c sfprocpidstats.h
endif

libspp_a_SOURCES = spp_arpspoof.c spp_arpspoof.h spp_bo.c spp_bo.h \
spp_rpc_decode.c spp_rpc_decode.h  \
stream_ignore.c stream_ignore.h \
spp_perfmonitor.c spp_perfmonitor.h \
perf.c perf.h \
perf-base.c perf-base.h \
perf-flow.c perf-flow.h \
perf-event.c perf-event.h \
$(PROCPIDSTATS_SOURCE) \
spp_httpinspect.c spp_httpinspect.h \
snort_httpinspect.c snort_httpinspect.h \
portscan.c portscan.h \
spp_sfportscan.c spp_sfportscan.h \
spp_frag3.c spp_frag3.h \
str_search.c str_search.h \
spp_stream5.c spp_stream5.h \
stream_api.c stream_api.h \
spp_normalize.c spp_normalize.h \
normalize.c normalize.h 


INCLUDES = @INCLUDES@
