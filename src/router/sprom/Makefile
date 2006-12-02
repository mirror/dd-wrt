VERSION = 001

PREFIX = /usr/local
CFLAGS = -std=c99 $(COPTS) -fomit-frame-pointer -Wall -D_BSD_SOURCE
LDFLAGS =


OBJECTS = bcm43xx_sprom.o utils.o

CFLAGS += -DVERSION_=$(VERSION)

all: bcm43xx_sprom

bcm43xx_sprom: $(OBJECTS)
	$(CC) $(CFLAGS) -o bcm43xx-sprom $(OBJECTS) $(LDFLAGS)

install: all
	-install -o 0 -g 0 -m 755 bcm43xx-sprom $(INSTALLDIR)/bin/

clean:
	-rm -f *~ *.o *.orig *.rej bcm43xx-sprom

# dependencies
bcm43xx_sprom.o: bcm43xx_sprom.h utils.h
utils.o: utils.h bcm43xx_sprom.h
