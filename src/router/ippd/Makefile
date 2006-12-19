CFLAGS=$(COPTS) -Wall -fomit-frame-pointer
LDFLAGS=$(COPTS) -Wl,-allow-shlib-undefined

OBJS=main.o conn.o printers.o http.o ipp.o array.o

all: ippd

dynamic: ippd install

install:
	install -m 755 ippd $(INSTALL_DIR)/bin
	$(STRIP) $(INSTALL_DIR)/bin/ippd

main.o: main.c conn.h array.h

ipp.o: ipp.c ipp.h array.h

ippd: $(OBJS)
	$(CC) $(LDFLAGS) -o ippd $(OBJS)

clean:
	-rm -f ippd $(OBJS)
