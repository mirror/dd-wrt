# RAM start + 1M
LOAD_TEXT_START := 0x80100000
# RAM start + 20M
RUN_TEXT_START  := 0x81400000
KERNEL_ENTRY := 0x80041000
BOARD_DEFINE := AR531X

CROSS_COMPILE = $(ARCH)-linux-

OBJCOPY:= $(CROSS_COMPILE)objcopy
CFLAGS := -fno-builtin -Os -G 0 -mno-abicalls -fno-pic -Wall -DKERNEL_ENTRY=${KERNEL_ENTRY} -DLOAD_TEXT_START=${LOAD_TEXT_START} -DRUN_TEXT_START=${RUN_TEXT_START} -D_LZMA_IN_CB -D${BOARD_DEFINE}

.S.s:
	$(CPP) $(CFLAGS) $< -o $*.s
.S.o:
	$(CC) $(CFLAGS) -c $< -o $*.o
.c.o:
	$(CC) $(CFLAGS) -c $< -o $*.o

CC =       $(CROSS_COMPILE)gcc
LD =       $(CROSS_COMPILE)ld
OBJDUMP =  $(CROSS_COMPILE)objdump

O_FORMAT = $(shell $(OBJDUMP) -i | head -2 | grep elf32)

# Drop some uninteresting sections in the kernel.
# This is only relevant for ELF kernels but doesn't hurt a.out
drop-sections   = .reginfo .mdebug .comment
strip-flags     = $(addprefix --remove-section=,$(drop-sections))

all : lzma.elf

stage2.lds: stage2.lds.in
	sed -e 's,@LOADADDR@,$(RUN_TEXT_START),g' -e 's,@ENTRY@,_start,g' $< >$@

kernel.o: vmlinux.lzma stage2.lds
	$(LD) -r -b binary --oformat $(O_FORMAT) -o $@ $<

ifeq ($(IMAGE_COPY),1)
lzma.o: decompress.o LzmaDecode.o kernel.o printf.o uart16550.o
	sed -e 's,@LOADADDR@,$(RUN_TEXT_START),g' -e 's,@ENTRY@,entry,g' lzma.lds.in >lzma-stage2.lds
	$(LD) -static --no-warn-mismatch -e entry -Tlzma-stage2.lds -o temp-$@ $^
	$(OBJCOPY) -O binary -R .reginfo -R .note -R .comment -R .mdebug -S temp-$@ lzma.tmp
	@echo "SECTIONS { .data : { code_start = .; *(.data) code_stop = .; }}" > lzma-data.lds
	$(LD) -no-warn-mismatch -T lzma-data.lds -r -o $@ -b binary lzma.tmp --oformat $(O_FORMAT)
	
lzma.elf: start.o lzma.o
	sed -e 's,@LOADADDR@,$(KERNEL_ENTRY),g' lzma-copy.lds.in >lzma-copy.lds
	$(LD) -s -Tlzma-copy.lds -o $@ $^
else
lzma.elf: start.o decompress.o LzmaDecode.o kernel.o
	$(LD) -s -Tstage2.lds -o $@.noreloc $^
	$(OBJCOPY) -j .text --adjust-vma=$$(($(LOAD_TEXT_START)-$(RUN_TEXT_START))) $@.noreloc $@
endif

clean:
	rm -f *.o lzma.elf *.tmp *.lds
