#########################################################################
#									#
#           Template Makefile for Assembly Language Programs            #
#									#
#########################################################################

# Author:   John Zaitseff <J.Zaitseff@unsw.edu.au>, Modified by
#           Andrew Greenberg (andrew@uad.com)
# Date:     18th August, 2003
# Version:  1.0

# Please note that every line that is indented (or, in the language of
# Makefiles, the command part of the rules) is done so with TAB
# characters, NOT with spaces.  This is important and a subtle trap for
# the unwary!

ALL = gps4020_download
all: $(ALL)

GPS4020_DOWNLOAD_FILES = gps4020_start.o gps4020_download.o tty.o
gps4020_download: $(GPS4020_DOWNLOAD_FILES)
	$(CC) $(LDFLAGS) -Tarm.ld -o $@ $(GPS4020_DOWNLOAD_FILES)
	$(OC) $(OCFLAGS) $@ $@.bin
	$(SIZE) $@

clean:
	rm -f $(ALL) *.o *~

# The following variables and implicit rules are required for the GNU
# Assembler for ARM.  You probably do not need to modify anything here.

AS        = arm-elf-as
SIZE      = arm-elf-size
LD        = arm-elf-ld
CC        = arm-elf-gcc
OC        = arm-elf-objcopy

# ADG: we don't need debugging info: removed "ASFLAGS   = --gdwarf2"
ASFLAGS   =
LDFLAGS   = -nostdlib -Wl,-static -Wl,-Map,$@.map
OCFLAGS   = --output-target binary
LOADLIBES =
LDLIBS    =
CFLAGS    = -O2 -msoft-float 

.SUFFIXES:

# Assemble ARM assembly language source (.s) to an object file (.o) using as.
# eg: "arm-elf-as -marm7tdmi gps4020_download.s -o gps4020_download.o"

%.o: %.s
	$(AS) -marm7tdmi $(ASFLAGS) $< -o $@

%.o: %.S
	$(CC) -c -marm7tdmi $(ASFLAGS) $< -o $@

# Link object file (.0) into an ARM executable (.elf), using ld.
# eg: "arm-elf-ld -o gps4020_download.o gps4020_download.elf"

%.elf: %.o
	$(LD) $(LDFLAGS) $< $(LOADLIBES) $(LDLIBS) -o $@

# "Copy" ELF file (.elf) into a binary file for execution using objcopy.
# eg: "arm-elf-objcopy --output-target binary gps4020_download.elf gps4020_download.bin"

%.bin: %.elf
	$(OC) $(OCFLAGS) $< $@

# Compile "C" code
%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

# Miscellaneous rules

.PHONY:	all clean
.DEFAULT:
.SUFFIXES:
