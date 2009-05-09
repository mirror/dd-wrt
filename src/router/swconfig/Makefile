CFLAGS = $(COPTS) -Os -Iinclude -I. -DNEED_PRINTF

all: swconfig

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $^

LIBNL_OBJ=nl.o handlers.o msg.o attr.o cache.o cache_mngt.o object.o socket.o error.o
GENL_OBJ=genl.o genl_family.o genl_ctrl.o genl_mngt.o

swconfig: $(LIBNL_OBJ) $(GENL_OBJ) cli.o swlib.o
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBS)

clean:
	rm -f swconfig *.o

install: 
	install -D swconfig $(INSTALLDIR)/usr/sbin