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


UNIT = page

obj-m += $(UNIT).o

$(UNIT)-objs += $(MIN_REQS)
$(UNIT)-objs += ../impersonator/icmp_wrapper.o
$(UNIT)-objs += ../impersonator/nat64.o
$(UNIT)-objs += ../impersonator/nf_hook.o
$(UNIT)-objs += ../impersonator/route.o
$(UNIT)-objs += ../impersonator/stats.o
$(UNIT)-objs += ../impersonator/send_packet.o
$(UNIT)-objs += ../framework/skb_generator.o
$(UNIT)-objs += ../framework/types.o

$(UNIT)-objs += ../../../src/mod/common/address_xlat.o
$(UNIT)-objs += ../../../src/mod/common/ipv6_hdr_iterator.o
$(UNIT)-objs += ../../../src/mod/common/packet.o
$(UNIT)-objs += ../../../src/mod/common/rfc6052.o
$(UNIT)-objs += ../../../src/mod/common/rtrie.o
$(UNIT)-objs += ../../../src/mod/common/skbuff.o
$(UNIT)-objs += ../../../src/mod/common/trace.o
$(UNIT)-objs += ../../../src/mod/common/translation_state.o
$(UNIT)-objs += ../../../src/mod/common/wrapper-config.o
$(UNIT)-objs += ../../../src/mod/common/wrapper-global.o
$(UNIT)-objs += ../../../src/mod/common/xlator.o
$(UNIT)-objs += ../../../src/mod/common/db/denylist4.o
$(UNIT)-objs += ../../../src/mod/common/db/eam.o
$(UNIT)-objs += ../../../src/mod/common/db/global.o
$(UNIT)-objs += ../../../src/mod/common/db/rfc6791v4.o
$(UNIT)-objs += ../../../src/mod/common/db/rfc6791v6.o
$(UNIT)-objs += ../../../src/mod/common/nl/attribute.o

$(UNIT)-objs += ../../../src/mod/common/steps/compute_outgoing_tuple_siit.o
$(UNIT)-objs += ../../../src/mod/common/steps/handling_hairpinning_siit.o
$(UNIT)-objs += ../../../src/mod/common/rfc7915/4to6.o
$(UNIT)-objs += ../../../src/mod/common/rfc7915/6to4.o
$(UNIT)-objs += ../../../src/mod/common/rfc7915/common.o
$(UNIT)-objs += ../../../src/mod/common/rfc7915/core.o
$(UNIT)-objs += ../../../src/mod/common/core.o

$(UNIT)-objs += impersonator.o
$(UNIT)-objs += page_test.o

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
