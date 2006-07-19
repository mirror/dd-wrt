# **************************************************************
# Makefile for building the IGMP Proxy module.
#	Build Targets 
#		all 	- Builds the IGMP Proxy executable. IGMPProxy
#		clean 	- Cleans all the object files. 
# **************************************************************

# Compiler. 
#CC = mipsel-linux-uclibc-gcc
CFLAGS = -Os -mips32 -mtune=mips32 -pipe

# Linker. 
#LD = mipsel-linux-uclibc-ld

all: igmp-proxy

# List of all object modules that make up the IGMPProxy.
OBJECTS = igmp_proxy.o listlib.o

igmp-proxy: $(OBJECTS)
		$(CC) $(CFLAGS) $(OBJECTS) -o igmprt2

igmp_proxy.o: igmp_proxy.c
	$(CC) $(CFLAGS) -c igmp_proxy.c

listlib.o: listlib.c
	$(CC) $(CFLAGS) -c listlib.c

clean:
	rm -rf $(OBJECTS)
