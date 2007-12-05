# **************************************************************
# Makefile for building the IGMP Proxy module.
#	Build Targets 
#		all 	- Builds the IGMP Proxy executable. IGMPProxy
#		clean 	- Cleans all the object files. 
# **************************************************************

# Compiler. 
#CC = mipsel-linux-uclibc-gcc
CFLAGS = $(COPTS)

CFLAGS += -DIGMP_SEND_THE_SECOND_QUERY_PACKET=1 -DNEED_PRINTF

CFLAGS += -DIGMP_BLOCK_SOME_SPEC_ADDRS=1 -I$(TOP)/../include.v24 -I$(TOP)/shared -DHND_FIX -fomit-frame-pointer -fpic -ffunction-sections -fdata-sections -Wl,--gc-sections

LDFLAGS = -L$(TOP)/netconf -lnetconf -ffunction-sections -fdata-sections -Wl,--gc-sections

all: igmp-proxy

# List of all object modules that make up the IGMPProxy.
OBJECTS = igmprt.o proxy.o igmpv2.o util.o filter.o

igmp-proxy: $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) -o igmprt -L$(TOP)/netconf -L$(TOP)/nvram -lnetconf -lnvram

igmp_proxy.o: igmp_proxy.c
	$(CC) $(CFLAGS) -c igmp_proxy.c

listlib.o: listlib.c
	$(CC) $(CFLAGS) -c listlib.c

clean:
	rm -rf $(OBJECTS)
	

install:
	install -D igmprt $(INSTALLDIR)/usr/sbin/igmprt
