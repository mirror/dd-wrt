
CFLAGS := $(COPTS) -Wall -Wstrict-prototypes

CFLAGS += -D_GNU_SOURCE

SRCS    := stabridge.c

OBJS    := $(SRCS:.c=.o)
OBJECTS := $(OBJS)
TARGET  := stabridge

$(TARGET): $(OBJS)

all: $(TARGET)

clean:
	-rm -f $(TARGET) $(OBJS)

strip: $(TARGET)
	$(STRIP) $(TARGET)

install: all
	install -d $(INSTALLDIR)/usr/sbin
	install -D rstats $(INSTALLDIR)/usr/sbin/stabridge

TMPSRC := /tmp/stabridge

gpl:
	-rm -f stabridge.tar.gz
	rm -rf ${TMPSRC}
	mkdir -p ${TMPSRC}
	cp *.c *.h README COPYING Makefile ${TMPSRC}
	tar -C /tmp -zcvf stabridge.tar.gz stabridge
