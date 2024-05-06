# It appears the -C's during the makes below prevent this include from happening
# when it's supposed to.
# For that reason, I can't just do "include ../common.mk". I need the absolute
# path of the file.
# Unfortunately, while the (as always utterly useless) working directory is (as
# always) brain-dead easy to access, the easiest way I found to get to the
# "current" directory is the mouthful below.
# And yet, it still has at least one major problem: if the path contains
# whitespace, `lastword $(MAKEFILE_LIST)` goes apeshit.
# This is the one and only reason why the unit tests need to be run in a
# space-free directory.
include $(shell dirname $(realpath $(lastword $(MAKEFILE_LIST))))/../common.mk


UNIT = sessiondb

obj-m += $(UNIT).o

$(UNIT)-objs += $(MIN_REQS)
$(UNIT)-objs += ../../../src/mod/common/translation_state.o
$(UNIT)-objs += ../../../src/mod/common/wrapper-config.o
$(UNIT)-objs += ../../../src/mod/common/wrapper-global.o
$(UNIT)-objs += ../../../src/mod/common/db/global.o
$(UNIT)-objs += ../../../src/mod/common/db/rbtree.o
$(UNIT)-objs += ../../../src/mod/common/db/bib/db.o
$(UNIT)-objs += ../../../src/mod/common/db/bib/entry.o
$(UNIT)-objs += ../../../src/mod/common/nl/attribute.o
$(UNIT)-objs += ../impersonator/bib.o
$(UNIT)-objs += ../impersonator/icmp_wrapper.o
$(UNIT)-objs += ../impersonator/route.o
$(UNIT)-objs += ../impersonator/stats.o
$(UNIT)-objs += ../impersonator/xlator.o
$(UNIT)-objs += sessiondb_test.o


all:
	make -C ${KERNEL_DIR} M=$$PWD;
modules:
	make -C ${KERNEL_DIR} M=$$PWD $@;
clean:
	make -C ${KERNEL_DIR} M=$$PWD $@;
test:
	sudo dmesg -C
	-sudo insmod $(UNIT).ko && sudo rmmod $(UNIT)
	sudo dmesg -tc | less
