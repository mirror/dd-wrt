## $Id$
AUTOMAKE_OPTIONS=foreign no-dependencies

bin_PROGRAMS = snort

if BUILD_SNPRINTF
SNPRINTF_SOURCES = snprintf.c snprintf.h
endif

snort_SOURCES = cdefs.h \
event.h \
generators.h \
protocols.h \
plugin_enum.h \
rules.h \
treenodes.h \
checksum.h \
debug.c debug.h \
decode.c decode.h \
encode.c encode.h \
active.c active.h \
log.c log.h \
mstring.c mstring.h \
parser.c parser.h \
profiler.c profiler.h \
plugbase.c plugbase.h \
preprocids.h \
snort.c snort.h \
build.h \
$(SNPRINTF_SOURCES) \
strlcatu.c strlcatu.h \
strlcpyu.c strlcpyu.h \
tag.c tag.h \
util.c util.h \
detect.c detect.h \
signature.c signature.h \
mempool.c mempool.h \
sf_sdlist.c sf_sdlist.h \
fpcreate.c fpcreate.h \
fpdetect.c fpdetect.h \
pcrm.c pcrm.h \
bounds.h \
byte_extract.c \
byte_extract.h \
timersub.h \
spo_plugbase.h \
sfthreshold.c sfthreshold.h \
packet_time.c packet_time.h \
event_wrapper.c event_wrapper.h \
smalloc.h \
event_queue.c event_queue.h \
ipv6_port.h \
ppm.c ppm.h \
pcap_pkthdr32.h \
cpuclock.h \
sf_types.h \
log_text.c log_text.h \
detection_filter.c detection_filter.h \
detection_util.c detection_util.h \
rate_filter.c rate_filter.h \
obfuscation.c obfuscation.h \
rule_option_types.h \
sfdaq.c sfdaq.h

snort_LDADD = output-plugins/libspo.a \
detection-plugins/libspd.a            \
dynamic-plugins/libdynamic.a            \
preprocessors/libspp.a                \
parser/libparser.a \
target-based/libtarget_based.a \
preprocessors/HttpInspect/libhttp_inspect.a \
preprocessors/Stream5/libstream5.a \
sfutil/libsfutil.a

if BUILD_DYNAMIC_EXAMPLES
EXAMPLES_DIR = dynamic-examples
endif

SUBDIRS = sfutil win32 output-plugins detection-plugins dynamic-plugins preprocessors parser dynamic-preprocessors target-based $(EXAMPLES_DIR)

INCLUDES = @INCLUDES@
