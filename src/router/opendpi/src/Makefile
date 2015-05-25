


OPENDPI_PATH2 := ${TOP}/opendpi/opendpi
OPENDPI_SRC := opendpi
OPENDPI_PRO := ${OPENDPI_SRC}/lib/protocols

ccflags-y += -I${src}/${OPENDPI_SRC}/include -I${src}/${OPENDPI_SRC}/lib -I${src}/${OPENDPI_SRC}/lib/third_party/include -DOPENDPI_NETFILTER_MODULE -I$(TOP)/opendpi -DCONFIG_NOPRINTK=1

EXTRA_CFLAGS := $(ccflags-y)
MODULES_DIR := $(INSTALLDIR)/opendpi/lib/opendpi
KERNEL_DIR := ${LINUXDIR}

obj-m := xt_opendpi.o
xt_opendpi-y := main.o 

all:
	if test -d ${OPENDPI_SRC}; then \
		cp ${OPENDPI_PATH2}/* ${OPENDPI_SRC} -R; \
	else \
		mkdir ${OPENDPI_SRC}; \
		cp ${OPENDPI_PATH2}/* ${OPENDPI_SRC} -R; \
	fi
	make -C ${KERNEL_DIR} CROSS_COMPILE="ccache $(CROSS_COMPILE)" M=$$PWD;
	rm -r ${OPENDPI_SRC}
modules:
	make -C ${KERNEL_DIR} M=$$PWD $@;

modules_install:
	install -D xt_opendpi.ko $(INSTALLDIR)/lib/modules/$(KERNELRELEASE)/xt_opendpi.ko

clean:
	make -C ${KERNEL_DIR} M=$$PWD $@;
	rm -rf modules.order ${OPENDPI_SRC}
