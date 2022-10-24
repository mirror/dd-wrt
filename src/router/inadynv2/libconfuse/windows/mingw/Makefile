CC=i686-pc-mingw32-gcc
AR=i686-pc-mingw32-ar
SRCDIR=../../src
VPATH=$(SRCDIR)
INCLUDE=-I. -I$(SRCDIR)
LFLAGS=-Pcfg_yy
CFLAGS=-Wall -DHAVE_CONFIG_H $(INCLUDE)
ARFLAGS=rc

all: libconfuse.a

libconfuse.a: confuse.o lexer.o
	$(AR) $(ARFLAGS) $@ $^

clean:
	rm -f *.o *.a
