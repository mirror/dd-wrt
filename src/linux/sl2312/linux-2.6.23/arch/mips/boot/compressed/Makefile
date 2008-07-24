#
# linux/arch/mips/boot/compressed/Makefile
#
# create a compressed vmlinuz image from the original vmlinux
#
# Note: SYSTEM, ZTEXTADDR, ZBSSADDR and ZRELADDR are exported
# from arch/mips/boot/Makefile. Their meanings are as follows:
#
# ZTEXTADDR
#	Start address of decompressor.  You normally call the kernel
#	at this address to start it booting.  This doesn't have
#	to be located in RAM, it can be in flash or other read-only or
#	read-write addressable medium. It should be an address in kseg1.
#
# ZBSSADDR
#	Start address of zero-initialised work area for the decompressor.
#	This must be pointing at RAM.  The decompressor will zero initialise
#	this for you. It should be an address in kseg1 too. The default for
#	this is to place it just after the image (so when running the code
#	from ROM you will need to set this to something useful).
#
# ZRELADDR
#	This is the address where the decompressed kernel will be written,
#	and eventually executed. It will be in kseg0.
#

HEAD		 = head.o
OBJS		 = misc.o 
MEMCPY 		 = $(TOPDIR)/arch/mips/lib/memcpy.o $(TOPDIR)/arch/mips/lib/memset.o
# CFLAGS	 = $(CPPFLAGS) -O2 -DSTDC_HEADERS $(CFLAGS_BOOT)
ZLDFLAGS	 = -G 0 -static -X -T $(obj)/ld.script

#
# Architecture dependencies
#
ifeq ($(CONFIG_ATHEROS),y)
OBJS		+= head-ar531x.o
UNCOMPRESS_H	= uncompress-ar531x.h
endif

ifeq ($(CONFIG_ATHEROS),y)
ifneq ($(CONFIG_BOOTLOADER),y)
ZTEXTADDR	= 0x80400000
else
ZTEXTADDR	= 0x9fc00000	# Link at cached address
ZBSSADDR	= 0x80700000	# Workspace for decompression @ 7MB
endif
ZRELADDR	= 0x80002000
endif



SEDFLAGS	= s/@@TEXTADDR@@/$(ZTEXTADDR)/;s/@@LOADADDR@@/$(ZRELADDR)/;

ifneq ($(ZBSSADDR),)
SEDFLAGS	+= s/@@BSSADDR@@/$(ZBSSADDR)/
else
SEDFLAGS	+= s/@@BSSADDR@@/ALIGN(4)/
endif

LIBGCC		:= $(shell $(CC) $(CFLAGS) --print-libgcc-file-name)

all:		vmlinuz

vmlinuz:	$(addprefix $(obj)/, $(OBJS)) $(HEAD) $(MEMCPY) $(obj)/payload.o $(obj)/ld.script
		$(LD) $(ZLDFLAGS) $(HEAD) $(addprefix $(obj)/, $(OBJS)) $(MEMCPY) $(obj)/payload.o $(LIBGCC) -o $(obj)/vmlinuz

$(HEAD): 	$(obj)/$(HEAD:.o=.S) $(TOPDIR)/include/linux/autoconf.h
		$(CC) $(AFLAGS) -Iinclude -c $(obj)/$(HEAD:.o=.S) -o $(obj)/$(HEAD) 

$(obj)/head-ar531x.o: $(obj)/head-ar531x.S $(TOPDIR)/include/linux/autoconf.h
		$(CC) $(AFLAGS) -Iinclude -c $(obj)/head-ar531x.S -o $(obj)/head-ar531x.o

$(obj)/payload.o:	$(SYSTEM) $(obj)/__payload.o
		$(OBJCOPY) -O binary -R .note -R .comment -S $(SYSTEM) $(obj)/payload
		gzip $(GZFLAGS) < $(obj)/payload > $(obj)/payload.gz
		$(LD) -r -o $@ $(obj)/__payload.o -b binary $(obj)/payload.gz
		rm -f $(obj)/payload $(obj)/payload.gz

$(obj)/ld.script:	$(obj)/ld.script.in $(obj)/Makefile $(TOPDIR)/arch/$(ARCH)/boot/Makefile $(TOPDIR)/.config
		sed "$(SEDFLAGS)" < $(obj)/ld.script.in > $@

clean:;		rm -f $(obj)/vmlinux $(obj)/core $(obj)/payload* $(obj)/ld.script *.o

.PHONY:	clean

uncompress.h:	$(UNCOMPRESS_H)
		ln -sf $(UNCOMPRESS_H) uncompress.h

misc.o:		misc.c uncompress.h $(TOPDIR)/lib/inflate.c

%.o: %.S
	$(CC) $(AFLAGS) $(EXTRA_AFLAGS) $(AFLAGS_$@) -c -o $@ $<
