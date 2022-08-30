
NDPI_SRC := ../../src
NDPI_PRO := ${NDPI_SRC}/lib/protocols

ccflags-y += -I${src}/${NDPI_SRC}/include -I${src}/${NDPI_SRC}/lib -I${src}/../libre -I${src}/${NDPI_SRC}/lib/third_party/include 
ccflags-y += -DHAVE_CONFIG_H -DNDPI_LIB_COMPILATION -DOPENDPI_NETFILTER_MODULE -DNDPI_DETECTION_SUPPORT_IPV6 -g 
ccflags-y += -Wno-declaration-after-statement
#ccflags-y += -Wshadow-local
# Needed for pahole
#ccflags-y += -femit-struct-debug-detailed=any

ifndef $(KERNEL_DIR)
#KERNEL_DIR := /home/seg/DEV/x86_64/src/linux/universal/linux-4.9
KERNEL_DIR := ${LINUXDIR}
endif

ifeq ($(shell grep -c userid ${LINUXDIR}/include/linux/skbuff.h),1)
ccflags-y += -DUSE_HACK_USERID=1
endif

NDPI_PROTO_SRC := $(shell cd $(src) ; echo $(NDPI_PRO)/*.c)

obj-m := xt_ndpi.o
xt_ndpi-y := main.o ndpi_strcol.o ndpi_proc_parsers.o ndpi_proc_generic.o \
		ndpi_proc_info.o  ndpi_proc_flow.o ndpi_proc_hostdef.o \
		ndpi_proc_ipdef.o ../libre/regexp.o \
		${NDPI_SRC}/lib/third_party/src/ndpi_md5.o \
		${NDPI_SRC}/lib/third_party/src/ndpi_sha1.o \
		${NDPI_SRC}/lib/third_party/src/ahocorasick.o \
		${NDPI_SRC}/lib/third_party/src/libcache.o \
		${NDPI_SRC}/lib/third_party/src/ndpi_patricia.o \
		${NDPI_SRC}/lib/third_party/src/gcrypt_light.o \
		${NDPI_SRC}/lib/third_party/src/btlib.o \
		${NDPI_SRC}/lib/ndpi_main.o \
		${NDPI_SRC}/lib/ndpi_utils.o \
		${NDPI_SRC}/lib/ndpi_serializer.o \
		${NDPI_SRC}/lib/ndpi_geoip.o \
		${NDPI_SRC}/lib/protocols.o \

all:	modules

test::
	@echo $(xt_ndpi-y)

modules:
	$(info $$xt_ndpi-y is [${xt_ndpi-y}] ${ccflags-y}) \
	make  -C ${KERNEL_DIR} M=$$PWD $@ $(MFLAGS);

modules_install:
	install -D xt_ndpi.ko $(INSTALLDIR)/lib/modules/$(KERNELRELEASE)/xt_ndpi.ko

clean:
	make -C ${KERNEL_DIR} M=$$PWD $@;
	rm -rf modules.order
distclean:
	find ../../src/lib/ ../libre/ -type f \( -name \*.o -o -name \*.cmd \) | xargs -r rm
