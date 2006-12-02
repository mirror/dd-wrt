#
# linux/arch/i386/boot/compressed/Makefile
#
# create a compressed vmlinux image from the original vmlinux
# patched by Ming-Ching Tiew <mctiew@yahoo.com> for kernel 2.6
# requires program 'lzma' from LZMA SDK ( http://www.7-zip.org/ ) to work
#    $ mkdir lzma
#    $ cd lzma
#    $ tar tvjf ../lzma417.tar.bz2
#    $ cd SRC/7zip/Compress/LZMA_Alone
#    $ dos2unix makefile
#    $ make
#    $ su
#    # cp lzma /usr/bin
#
targets		:= vmlinux vmlinux.bin vmlinux.bin.lzma head.o lzma_misc.o piggy.o
EXTRA_AFLAGS	:= -traditional

LDFLAGS_vmlinux := -Ttext $(IMAGE_OFFSET) -e startup_32

$(obj)/vmlinux: $(obj)/head.o $(obj)/lzma_misc.o $(obj)/piggy.o FORCE
	$(call if_changed,ld)
	@:

$(obj)/vmlinux.bin: vmlinux FORCE
	$(call if_changed,objcopy)

$(obj)/vmlinux.bin.lzma: $(obj)/vmlinux.bin FORCE
	$(call if_changed,lzma)

LDFLAGS_piggy.o := -r --format binary --oformat elf32-i386 -T

$(obj)/piggy.o: $(obj)/vmlinux.scr $(obj)/vmlinux.bin.lzma FORCE
	$(call if_changed,ld)
