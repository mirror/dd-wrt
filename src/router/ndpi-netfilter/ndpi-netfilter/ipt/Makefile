NDPI_SRC := ../../src
NDPI_PRO := ${NDPI_SRC}/lib/protocols
XTBL := $(shell pkg-config --variable=xtlibdir xtables)

CFLAGS = -fPIC -I../.. -I${NDPI_SRC}/include -I${NDPI_SRC}/lib -I../src -I../libre -DOPENDPI_NETFILTER_MODULE -DXTABLES_INTERNAL -O2 -Wall

all:  libxt_ndpi.so
install:  libxt_ndpi.so
ifndef XTBL
$(error No pkg-config --variable=xtlibdir xtables)
endif
	if [ ! -d $(DESTDIR)$(XTBL) ]; then \
	echo "$(DESTDIR)$(XTBL): directory nonexistent. Creating..."; \
	mkdir -p $(DESTDIR)$(XTBL); \
	fi
	install -v libxt_ndpi.so ${DESTDIR}${XTBL}; \
	ln -fs libxt_ndpi.so ${DESTDIR}${XTBL}/libxt_NDPI.so

lib%.so: lib%.o
	$(CC) -shared -o $@ $^;
lib%.o: lib%.c ../src/xt_ndpi.h ${NDPI_SRC}/include/ndpi_config.h ../libre/regexp.h ../libre/regexp.c
	$(CC) ${CFLAGS} -D_INIT=lib$*_init -c -o $@ $<;
clean:
	rm -rf libxt_ndpi.so

