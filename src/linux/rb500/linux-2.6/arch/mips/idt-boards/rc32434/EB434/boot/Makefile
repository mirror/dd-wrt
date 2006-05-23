###############################################################################
#
#  BRIEF MODULE DESCRIPTION
#     Makefile create a compressed zImage or Rommable rImage
#
#  Copyright 2004 IDT Inc. (rischelp@idt.com)
#
#  This program is free software; you can redistribute  it and/or modify it
#  under  the terms of  the GNU General  Public License as published by the
#  Free Software Foundation;  either version 2 of the  License, or (at your
#  option) any later version.
#
#  THIS  SOFTWARE  IS PROVIDED   ``AS  IS'' AND   ANY  EXPRESS OR IMPLIED
#  WARRANTIES,   INCLUDING, BUT NOT  LIMITED  TO, THE IMPLIED WARRANTIES OF
#   MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN
#   NO  EVENT  SHALL   THE AUTHOR  BE    LIABLE FOR ANY   DIRECT, INDIRECT,
#   INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
#   NOT LIMITED   TO, PROCUREMENT OF  SUBSTITUTE GOODS  OR SERVICES; LOSS OF
#   USE, DATA,  OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
#   ANY THEORY OF LIABILITY, WHETHER IN  CONTRACT, STRICT LIABILITY, OR TORT
#   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
#   THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
#   You should have received a copy of the  GNU General Public License along
#   with this program; if not, write  to the Free Software Foundation, Inc.,
#   675 Mass Ave, Cambridge, MA 02139, USA.
# 
# 
###############################################################################
#  May 2004 rkt
# 
#  Initial Release
# 
# 
###############################################################################

###############################################################################
# The following is taken from IDT/Sim Makefile
#############################################################################
TARGET=434
TARGETDIR=S434

#
# following refers to memory type in use in eval board and if more than one
# then the order is implied.  These are values for the switch MEMCFG.
#
SRAM_ONLY=1
SDRAM_ONLY=2
SRAM_N_SDRAM=3
SDRAM_N_SRAM=4

# following refers to size of the DRAM space.
# These are values for the switch DRAMSZ.

MB32=1
MB64=2
MB128=3
MB32SO=4

MACH= -DEB434 -DS434 -DCPU_R32434 -DMIPSEL -DINET -DMEMCFG=$(SDRAM_ONLY) -DDRAMSZ=$(MB32) -DIDTSIM -DMHZ=$(MHZ) -DNVRAM_RTC -DUSE_SPI
COMMSWITCHES = $(MACH)
#***************** END IDT/Sim Makefile ##################################### 
ZDEBUG=1
export ZDEBUG

# working space for gunzip:
FREE_RAM      := 0x80C00000
END_RAM       := 0x80E00000

KERNELCONFIG  := $(TOPDIR)/.config
include $(KERNELCONFIG)

SIZE = $(CROSS_COMPILE)size

O_FORMAT = $(shell $(OBJDUMP) -i | head -2 | grep elf32)

SYSTEM	      := $(TOPDIR)/vmlinux
ZBSS          := 0x800A0000

ZIMSTART      := $(CONFIG_IDT_ZIMAGE_ADDR)
RIMSTART      := 0x9FC00000

LOADADDR      := 0x$(shell $(NM) $(SYSTEM) | grep "A _text" |cut -f1 -d' ')
KERNEL_ENTRY  := $(shell $(OBJDUMP) -f $(SYSTEM) | sed -n -e 's/^start address //p')

####################################################################################
ZIMFLAGS        = s/IMSTART/$(ZIMSTART)/;s/BSS_START/$(ZBSS)/
RIMFLAGS        = s/IMSTART/$(RIMSTART)/;s/BSS_START/$(ZBSS)/
CFLAGS	:= -fno-pic -nostdinc -G 0 -mno-abicalls -fno-pic -pipe -I$(TOPDIR)/include
AFLAGS	:= -D__ASSEMBLY__ $(CFLAGS)

####################################################################################
OBJECTS= $(obj)/piggy.o $(obj)/head.o $(obj)/misc.o
ifneq ($(ZDEBUG),0)
OBJECTS += $(obj)/uart16550.o
endif

$(obj)/zImage.lds: $(obj)/image.lds.in $(KERNELCONFIG)
	@sed "$(ZIMFLAGS)" < $< > $@

$(obj)/rImage.lds: $(obj)/image.lds.in $(KERNELCONFIG)
	@sed "$(RIMFLAGS)" < $< > $@

$(obj)/piggy.o: $(SYSTEM) $(obj)/Makefile
	$(OBJCOPY) -S -O binary -R .note -R .comment $(SYSTEM) $(SYSTEM).bin
	gzip -f -9 < $(SYSTEM).bin > $(SYSTEM).gz
	echo "O_FORMAT:  " $(O_FORMAT); 
	$(LD) -r -b binary --oformat $(O_FORMAT) -o $(obj)/piggy.o $(SYSTEM).gz
	rm -f $(SYSTEM).bin $(SYSTEM).gz

$(obj)/head.o: $(obj)/head.S $(SYSTEM) $(obj)/Makefile
	$(CC) $(AFLAGS) -DKERNEL_ENTRY=$(KERNEL_ENTRY) -c $(obj)/head.S -o $(obj)/head.o

$(obj)/misc.o: $(obj)/misc.c $(obj)/Makefile
	$(CC) $(CFLAGS) -DLOADADDR=$(LOADADDR) -DFREE_RAM=$(FREE_RAM) -DEND_RAM=$(END_RAM) \
		-c $< -DZDEBUG=$(ZDEBUG) -o $(obj)/misc.o

$(obj)/uart16550.o: $(obj)/uart16550.c $(KERNELCONFIG)
	$(CC) $(CFLAGS) -c $< -o $(obj)/uart16550.o

$(obj)/csu_idt.o: $(obj)/csu_idt.S Makefile $(SYSTEM)
	$(CC) $(AFLAGS) $(COMMSWITCHES) -c $< -o $(obj)/csu_idt.o

zImage: $(obj)/zImage.lds $(SYSTEM) $(OBJECTS)
	$(LD) -T$(obj)/zImage.lds -o $(TOPDIR)/zImage $(OBJECTS)
	$(OBJCOPY) -S -O binary -R .note -R .comment $(TOPDIR)/zImage $(TOPDIR)/zImage.bin
	$(OBJCOPY) -I binary -S -O srec --srec-forceS3 --srec-len=32 --change-start=0x00000000 \
		 $(TOPDIR)/zImage.bin $(TOPDIR)/zImage.prm
	$(SIZE) $(TOPDIR)/zImage |awk -F" " '{ print $$4 "\t" $$5 }' > $(TOPDIR)/zImage.size
	rm -f *.o

rImage: $(obj)/rImage.lds $(SYSTEM) $(OBJECTS) $(obj)/csu_idt.o
	@rm -f $(TOPDIR)/*.prm
	$(LD) -T$(obj)/rImage.lds -o $(TOPDIR)/rImage $(obj)/csu_idt.o $(OBJECTS) 
	$(OBJCOPY) -S -O binary -R .note -R .comment $(TOPDIR)/rImage $(TOPDIR)/rImage.bin
	$(OBJCOPY) -I binary -S -O srec --srec-forceS3 --srec-len=32 --change-start=0x00000000 \
		 $(TOPDIR)/rImage.bin $(TOPDIR)/rImage.prm
	$(SIZE) $(TOPDIR)/rImage |awk -F" " '{ print $$4 "\t" $$5 }' > $(TOPDIR)/rImage.size
	rm -f *.o
clean:
	rm -f *.o $(TOPDIR)/zImage* $(TOPDIR)/rImage* $(TOPDIR)/*.prm $(TOPDIR)/rImage.size $(TOPDIR)/zImage.size
