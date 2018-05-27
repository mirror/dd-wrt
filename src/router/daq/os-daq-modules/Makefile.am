AUTOMAKE_OPTIONS = foreign

ACLOCAL_AMFLAGS = -I m4

bin_SCRIPTS = daq-modules-config

pkglib_LTLIBRARIES =

lib_LTLIBRARIES = libdaq_static_modules.la
libdaq_static_modules_la_SOURCES = \
daq_static_modules.c \
daq_static_modules.h
libdaq_static_modules_la_CFLAGS =
libdaq_static_modules_la_LDFLAGS = -static -avoid-version
libdaq_static_modules_la_LIBADD =

if BUILD_AFPACKET_MODULE
if BUILD_SHARED_MODULES
    pkglib_LTLIBRARIES += daq_afpacket.la
    daq_afpacket_la_SOURCES = daq_afpacket.c
    daq_afpacket_la_CFLAGS = -DBUILDING_SO
    daq_afpacket_la_LDFLAGS = -module -export-dynamic -avoid-version -shared @XCCFLAGS@
    daq_afpacket_la_LIBADD = $(top_builddir)/sfbpf/libsfbpf.la
endif
    libdaq_static_modules_la_SOURCES += daq_afpacket.c
    libdaq_static_modules_la_CFLAGS += -DBUILD_AFPACKET_MODULE
endif

if BUILD_PCAP_MODULE
if BUILD_SHARED_MODULES
    pkglib_LTLIBRARIES += daq_pcap.la
    daq_pcap_la_SOURCES = daq_pcap.c
    daq_pcap_la_CFLAGS = -DBUILDING_SO
    daq_pcap_la_LDFLAGS = -module -export-dynamic -avoid-version -shared @XCCFLAGS@
    daq_pcap_la_LIBADD = -lpcap
endif
    libdaq_static_modules_la_SOURCES += daq_pcap.c
    libdaq_static_modules_la_CFLAGS += -DBUILD_PCAP_MODULE
endif

if BUILD_DUMP_MODULE
if BUILD_SHARED_MODULES
    pkglib_LTLIBRARIES += daq_dump.la
    daq_dump_la_SOURCES = daq_dump.c
    daq_dump_la_CFLAGS = -DBUILDING_SO
    daq_dump_la_LDFLAGS = -module -export-dynamic -avoid-version -shared @XCCFLAGS@
    daq_dump_la_LIBADD = -lpcap
endif
    libdaq_static_modules_la_SOURCES += daq_dump.c
    libdaq_static_modules_la_CFLAGS += -DBUILD_DUMP_MODULE
endif

if BUILD_IPFW_MODULE
if BUILD_SHARED_MODULES
    pkglib_LTLIBRARIES += daq_ipfw.la
    daq_ipfw_la_SOURCES = daq_ipfw.c
    daq_ipfw_la_CFLAGS = -DBUILDING_SO
    daq_ipfw_la_LDFLAGS = -module -export-dynamic -avoid-version -shared @XCCFLAGS@
    daq_ipfw_la_LIBADD = $(top_builddir)/sfbpf/libsfbpf.la
endif
    libdaq_static_modules_la_SOURCES += daq_ipfw.c
    libdaq_static_modules_la_CFLAGS += -DBUILD_IPFW_MODULE
endif

if BUILD_IPQ_MODULE
if BUILD_SHARED_MODULES
    pkglib_LTLIBRARIES += daq_ipq.la
    daq_ipq_la_SOURCES = daq_ipq.c
    daq_ipq_la_CFLAGS = -DBUILDING_SO
    daq_ipq_la_LDFLAGS = -module -export-dynamic -avoid-version -shared @DNET_LDFLAGS@ @XCCFLAGS@
    daq_ipq_la_LIBADD = -lipq @DNET_LDFLAGS@ $(top_builddir)/sfbpf/libsfbpf.la
endif
    libdaq_static_modules_la_SOURCES += daq_ipq.c
    libdaq_static_modules_la_CFLAGS += -DBUILD_IPQ_MODULE
endif

if BUILD_NFQ_MODULE
if BUILD_SHARED_MODULES
    pkglib_LTLIBRARIES += daq_nfq.la
    daq_nfq_la_SOURCES = daq_nfq.c
    daq_nfq_la_CFLAGS = -DBUILDING_SO
    daq_nfq_la_LDFLAGS = -module -export-dynamic -avoid-version -shared @DNET_LDFLAGS@ @XCCFLAGS@
    daq_nfq_la_LIBADD = -lnfnetlink -lnetfilter_queue @DNET_LDFLAGS@ $(top_builddir)/sfbpf/libsfbpf.la
endif
    libdaq_static_modules_la_SOURCES += daq_nfq.c
    libdaq_static_modules_la_CFLAGS += -DBUILD_NFQ_MODULE
endif

if BUILD_NETMAP_MODULE
if BUILD_SHARED_MODULES
    pkglib_LTLIBRARIES += daq_netmap.la
    daq_netmap_la_SOURCES = daq_netmap.c
    daq_netmap_la_CFLAGS = -DBUILDING_SO
    daq_netmap_la_LDFLAGS = -module -export-dynamic -avoid-version -shared @XCCFLAGS@
    daq_netmap_la_LIBADD = $(top_builddir)/sfbpf/libsfbpf.la
endif
    libdaq_static_modules_la_SOURCES += daq_netmap.c
    libdaq_static_modules_la_CFLAGS += -DBUILD_NETMAP_MODULE
endif

AM_CPPFLAGS = -I$(top_srcdir)/api -I$(top_srcdir)/sfbpf -I$(top_builddir)/sfbpf

EXTRA_DIST = daq-modules-config.in

