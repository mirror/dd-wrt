NDPI_PATH2 := $(TOP)/ndpi/ndpi/src
NDPI_SRC := ndpi_cpy
NDPI_PRO := ${NDPI_SRC}/lib/protocols

ccflags-y += -I${src}/${NDPI_SRC}/include -I${src}/${NDPI_SRC}/lib -DOPENDPI_NETFILTER_MODULE -DNDPI_IPTABLES_EXT

MODULES_DIR := $(INSTALLDIR)/opendpi/lib/opendpi
KERNEL_DIR := ${LINUXDIR}

obj-m := xt_ndpi.o
xt_ndpi-y := main.o

all:
	if test -d ${NDPI_SRC}; then \
		cp ${NDPI_PATH2}/* ${NDPI_SRC} -R; \
	else \
		mkdir ${NDPI_SRC}; \
		cp ${NDPI_PATH2}/* ${NDPI_SRC} -R; \
	fi
	cp ${NDPI_SRC}/../../nDPI-patch/src/* ${NDPI_SRC}/ -R;
	cp ${NDPI_SRC}/lib/third_party/src/*.c ${NDPI_SRC}/lib -R;
	cp ${NDPI_SRC}/lib/third_party/include/*.h ${NDPI_SRC}/lib -R;
	cp ${NDPI_SRC}/lib/third_party/include/*.h ${NDPI_SRC}/include -R;
	sed -i "s/^\s*void ndpi_free_flow/\/\/void ndpi_free_flow/" ${NDPI_SRC}/include/ndpi_api.h;
	make -C ${KERNEL_DIR} M=$$PWD;
	rm -r ${NDPI_SRC}
modules:
	make -C ${KERNEL_DIR} M=$$PWD $@;
modules_install:
	install -D xt_opendpi.ko $(INSTALLDIR)/lib/modules/$(KERNELRELEASE)/xt_opendpi.ko
clean:
	make -C ${KERNEL_DIR} M=$$PWD $@;
	rm -rf modules.order ${NDPI_SRC}
