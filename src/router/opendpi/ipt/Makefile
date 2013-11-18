OPENDPI_PATH2 := $(TOP)/opendpi/opendpi
OPENDPI_SRC := opendpi
OPENDPI_PRO := ${OPENDPI_SRC}/lib/protocols

CFLAGS = -fPIC -I. -I${OPENDPI_SRC}/include -I${OPENDPI_SRC}/lib -I../src -DOPENDPI_NETFILTER_MODULE -O2 -Wall

all:
	if test -d ${OPENDPI_SRC}; then \
		cp ${OPENDPI_PATH2}/* ${OPENDPI_SRC} -R; \
	else \
		mkdir ${OPENDPI_SRC}; \
		cp ${OPENDPI_PATH2}/* ${OPENDPI_SRC} -R; \
	fi
#	make libxt_opendpi.so
	rm -r ${OPENDPI_SRC}
lib%.so: lib%.o
	$(ARCH)-linux-gcc -shared -o $@ $^;
lib%.o: lib%.c
	$(ARCH)-linux-gcc ${CFLAGS} -D_INIT=lib$*_init -c -o $@ $<;
clean:
	rm -rf ${OPENDPI_SRC}
#	rm -rf libxt_opendpi.so ${OPENDPI_SRC}
