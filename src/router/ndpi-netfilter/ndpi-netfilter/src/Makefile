
NDPI_SRC := ../../src
NDPI_PRO := ${NDPI_SRC}/lib/protocols

ccflags-y += -I${src}/${NDPI_SRC}/include -I${src}/${NDPI_SRC}/lib -I${src}/../libre -I${src}/${NDPI_SRC}/lib/third_party/include 
ccflags-y += -DHAVE_CONFIG_H -DNDPI_LIB_COMPILATION -DOPENDPI_NETFILTER_MODULE


KERNEL_DIR := ${LINUXDIR}

ifeq ($(shell grep -c userid ${LINUXDIR}/include/linux/skbuff.h),1)
ccflags-y += -DUSE_HACK_USERID=1
endif

NDPI_PROTO_SRC := $(shell cd $(src) ; echo $(NDPI_PRO)/*.c)

obj-m := xt_ndpi.o
xt_ndpi-y := main.o ndpi_strcol.o ndpi_proc_parsers.o ndpi_proc_generic.o \
		ndpi_proc_info.o  ndpi_proc_flow.o ndpi_proc_hostdef.o \
		ndpi_proc_ipdef.o ../libre/regexp.o \
		${NDPI_SRC}/lib/third_party/src/node.o \
		${NDPI_SRC}/lib/third_party/src/ahocorasick.o \
		${NDPI_SRC}/lib/third_party/src/libcache.o \
		${NDPI_SRC}/lib/third_party/src/ndpi_patricia.o \
		${NDPI_SRC}/lib/third_party/src/ndpi_md5.o \
		${NDPI_SRC}/lib/third_party/src/ht_hash.o \
		${NDPI_SRC}/lib/third_party/src/btlib.o \
		${NDPI_SRC}/lib/ndpi_main.o \
		$(patsubst %.c,%.o, $(NDPI_PROTO_SRC))

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
	rm -f ${xt_ndpi-y}
	rm -rf modules.order
distclean:
	find ../../src/lib/ -type f \( -name \*.o -o -name \*.cmd \) | xargs -r rm
