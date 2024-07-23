include $(TOP)/.config

CFLAGS	+= $(COPTS) $(LTO) $(MIPS16_OPT) -DNEED_PRINTF

OBJS := getconf.o

all: getconf

clean:
	rm -f *.o getconf

install: all
	install -d $(INSTALLDIR)/usr/sbin
	install -D getconf $(INSTALLDIR)/usr/bin/getconf

getconf: $(OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)
