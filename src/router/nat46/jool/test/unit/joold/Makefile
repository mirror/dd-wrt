# It appears the -C's during the makes below prevent this include from happening
# when it's supposed to. Therefore, I can't just do "include ../common.mk".
# I need the absolute path of the file.
# The easiest way I found to get to the "current" directory is the mouthful
# below.
# It still has at least one major problem: if the path contains whitespace,
# `lastword $(MAKEFILE_LIST)` goes apeshit. This is the one and only reason why
# the unit tests need to be run in a space-free directory.
include $(shell dirname $(realpath $(lastword $(MAKEFILE_LIST))))/../common.mk


UNIT = joold

obj-m += $(UNIT).o

$(UNIT)-objs += $(MIN_REQS)
$(UNIT)-objs += ../../../src/common/config.o
$(UNIT)-objs += ../../../src/mod/common/db/bib/entry.o
$(UNIT)-objs += ../../../src/mod/common/nl/attribute.o
$(UNIT)-objs += joold_test.o


all:
	make -C ${KERNEL_DIR} M=$$PWD;
modules:
	make -C ${KERNEL_DIR} M=$$PWD $@;
clean:
	make -C ${KERNEL_DIR} M=$$PWD $@;
test:
	sudo dmesg -C
	-sudo insmod $(UNIT).ko && sudo rmmod $(UNIT)
	sudo dmesg -tc
