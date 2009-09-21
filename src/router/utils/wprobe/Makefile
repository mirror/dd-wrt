
CPPFLAGS += -Iinclude
LDFLAGS =

USE_LIBNL_MICRO=0

ifeq ($(USE_LIBNL_MICRO),1)
LIBNL_PREFIX = /usr/local
LIBNL = $(LIBNL_PREFIX)/lib/libnl-micro.a
CPPFLAGS += -I$(LIBNL_PREFIX)/include/libnl-micro
EXTRA_CFLAGS += -DNO_LOCAL_ACCESS
else
LIBNL = -lnl
endif

LIBM = -lm
LIBS = $(LIBM)
LIBNL_OBJ=nl.o handlers.o msg.o attr.o cache.o cache_mngt.o object.o socket.o error.o
GENL_OBJ=genl.o genl_family.o genl_ctrl.o genl_mngt.o



all: libwprobe.a wprobe-util

libwprobe.a: wprobe-lib.o $(LIBNL_OBJ) $(GENL_OBJ)
	rm -f $@
	$(AR) rcu $@ $^
#	$(RANLIB) $@

%.o: %.c
	$(CC) $(WFLAGS) -c -o $@ $(CPPFLAGS) $(CFLAGS) $(EXTRA_CFLAGS) $<

wprobe-util: wprobe-util.o wprobe-lib.o $(LIBNL_OBJ) $(GENL_OBJ)
	$(CC) -o $@ $^ $(LDFLAGS) $(LIBS)

clean:
	rm -f *.o *.a wprobe-util
