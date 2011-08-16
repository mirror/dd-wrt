AUTOMAKE_OPTIONS = foreign

ACLOCAL_AMFLAGS = -I m4

bin_SCRIPTS = daq-modules-config

pkglib_LTLIBRARIES = 

STATIC_LIBS =

lib_LTLIBRARIES = libdaq_static_modules.la
libdaq_static_modules_la_SOURCES = \
daq_static_modules.c \
daq_static_modules.h
libdaq_static_modules_la_CFLAGS = 
libdaq_static_modules_la_LDFLAGS = -static -avoid-version
libdaq_static_modules_la_LIBADD =

if BUILD_AFPACKET_MODULE
    pkglib_LTLIBRARIES += daq_afpacket.la
    daq_afpacket_la_SOURCES = daq_afpacket.c
    daq_afpacket_la_CFLAGS = -DBUILDING_SO
    daq_afpacket_la_LDFLAGS = -module -export-dynamic -avoid-version -shared
    daq_afpacket_la_LIBADD = $(top_srcdir)/sfbpf/libsfbpf.la
    libdaq_static_modules_la_SOURCES += daq_afpacket.c
    libdaq_static_modules_la_CFLAGS += -DBUILD_AFPACKET_MODULE
    libdaq_static_modules_la_LIBADD += $(top_srcdir)/sfbpf/libsfbpf.la
    STATIC_LIBS += -lsfbpf
endif

if BUILD_PCAP_MODULE
    pkglib_LTLIBRARIES += daq_pcap.la
    daq_pcap_la_SOURCES = daq_pcap.c
    daq_pcap_la_CFLAGS = -DBUILDING_SO
    daq_pcap_la_LDFLAGS = -module -export-dynamic -avoid-version -shared
    daq_pcap_la_LIBADD = -lpcap
    libdaq_static_modules_la_SOURCES += daq_pcap.c
    libdaq_static_modules_la_CFLAGS += -DBUILD_PCAP_MODULE
    libdaq_static_modules_la_LIBADD += -lpcap
    STATIC_LIBS += -lpcap
endif

if BUILD_DUMP_MODULE
    pkglib_LTLIBRARIES += daq_dump.la
    daq_dump_la_SOURCES = daq_dump.c
    daq_dump_la_CFLAGS = -DBUILDING_SO
    daq_dump_la_LDFLAGS = -module -export-dynamic -avoid-version -shared
    daq_dump_la_LIBADD = -lpcap
    libdaq_static_modules_la_SOURCES += daq_dump.c
    libdaq_static_modules_la_CFLAGS += -DBUILD_DUMP_MODULE
    libdaq_static_modules_la_LIBADD += -lpcap
    STATIC_LIBS += -lpcap
endif

if BUILD_IPFW_MODULE
    pkglib_LTLIBRARIES += daq_ipfw.la
    daq_ipfw_la_SOURCES = daq_ipfw.c
    daq_ipfw_la_CFLAGS = -DBUILDING_SO
    daq_ipfw_la_LDFLAGS = -module -export-dynamic -avoid-version -shared
    daq_ipfw_la_LIBADD = $(top_srcdir)/sfbpf/libsfbpf.la
    libdaq_static_modules_la_SOURCES += daq_ipfw.c
    libdaq_static_modules_la_CFLAGS += -DBUILD_IPFW_MODULE
    libdaq_static_modules_la_LIBADD += $(top_srcdir)/sfbpf/libsfbpf.la
    STATIC_LIBS += -lsfbpf
endif

if BUILD_IPQ_MODULE
    pkglib_LTLIBRARIES += daq_ipq.la
    daq_ipq_la_SOURCES = daq_ipq.c
    daq_ipq_la_CFLAGS = -DBUILDING_SO
    daq_ipq_la_LDFLAGS = -module -export-dynamic -avoid-version -shared
    daq_ipq_la_LIBADD = -lipq -ldnet $(top_srcdir)/sfbpf/libsfbpf.la
    libdaq_static_modules_la_SOURCES += daq_ipq.c
    libdaq_static_modules_la_CFLAGS += -DBUILD_IPQ_MODULE
    libdaq_static_modules_la_LIBADD += -lipq -ldnet $(top_srcdir)/sfbpf/libsfbpf.la
    STATIC_LIBS += -lipq -ldnet -lsfbpf
endif

if BUILD_NFQ_MODULE
    pkglib_LTLIBRARIES += daq_nfq.la
    daq_nfq_la_SOURCES = daq_nfq.c
    daq_nfq_la_CFLAGS = -DBUILDING_SO
    daq_nfq_la_LDFLAGS = -module -export-dynamic -avoid-version -shared
    daq_nfq_la_LIBADD = -lnfnetlink -lnetfilter_queue -ldnet $(top_srcdir)/sfbpf/libsfbpf.la
    libdaq_static_modules_la_SOURCES += daq_nfq.c
    libdaq_static_modules_la_CFLAGS += -DBUILD_NFQ_MODULE
    libdaq_static_modules_la_LIBADD += -lnfnetlink -lnetfilter_queue -ldnet $(top_srcdir)/sfbpf/libsfbpf.la
    STATIC_LIBS += -lnfnetlink -lnetfilter_queue -ldnet -lsfbpf
endif

INCLUDES = -I$(top_srcdir)/api -I$(top_srcdir)/sfbpf

CLEANFILES = daq-modules-config

EXTRA_DIST = daq-modules-config.in

daq-modules-config:
	sed -e 's|@LIBDIR@|@libdir@|g' -e 's|@STATIC_LIBS@|"$(STATIC_LIBS)"|g' daq-modules-config.in > daq-modules-config
