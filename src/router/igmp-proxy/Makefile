# **************************************************************
# Makefile for building the IGMP Proxy module.
#	Build Targets 
#		all 	- Builds the IGMP Proxy executable. IGMPProxy
#		clean 	- Cleans all the object files. 
# **************************************************************

# Compiler. 
#CC = mipsel-linux-uclibc-gcc
CFLAGS = $(COPTS)

# Linker. 
#LD = mipsel-linux-uclibc-ld

all: igmp-proxy

# List of all object modules that make up the IGMPProxy.
OBJECTS = igmp_proxy.o listlib.o

igmp-proxy: $(OBJECTS)
	if test -e "brcm/Makefile"; then make -C brcm; else $(CC) $(CFLAGS) $(OBJECTS) -o igmprt2; fi

igmp_proxy.o: igmp_proxy.c
	$(CC) $(CFLAGS) -c igmp_proxy.c

listlib.o: listlib.c
	$(CC) $(CFLAGS) -c listlib.c

clean:
	if test -e "brcm/Makefile"; then make -C brcm clean; else rm -rf $(OBJECTS); fi
	

install:
	if test -e "brcm/Makefile"; then install -D brcm/igmprt-$(ARCH) $(INSTALLDIR)/usr/sbin/igmprt; else install -D igmprt2 $(INSTALLDIR)/usr/sbin/igmprt; fi
