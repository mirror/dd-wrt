AUTOMAKE_OPTIONS=foreign no-dependencies

CORELIBS = @CORELIBS@
MORELIBS = @MORELIBS@ @DARWINLIBS@
MOREDEFS = @MOREDEFS@
INCLUDES = @INCS@ @DARWININCS@
DEFS = @DEFS@ $(MOREDEFS)
#CFLAGS = -O0 -fprofile-arcs -ftest-coverage
CFLAGS =  

CLEANFILES = 

sbin_PROGRAMS = lldpd
lldpd_SOURCES = \
	lldp_main.c lldp_debug.c lldp_neighbor.c \
	lldp_port.h \
	tlv/tlv.h tlv/tlv.c tlv/tlv_common.h tlv/tlv_common.c\
	tlv/oui/tlv_000ecf.h tlv/oui/tlv_000ecf.c \
	tlv/oui/tlv_00120f.h tlv/oui/tlv_00120f.c \
	tlv/oui/tlv_0012bb.h tlv/oui/tlv_0012bb.c \
	tlv/oui/tlv_0080c2.h tlv/oui/tlv_0080c2.c \
	tx_sm.c tx_sm.h rx_sm.c rx_sm.h \
	msap.c msap.h \
	framehandlers/bpf/lldp_bpf_framer.h framehandlers/bpf/lldp_bpf_framer.c \
	framehandlers/bpf/bpflib.c framehandlers/bpf/bpflib.h \
	framehandlers/linux/lldp_linux_framer.h framehandlers/linux/lldp_linux_framer.c 
lldpd_LDADD = $(CORELIBS) $(MORELIBS)
