SBINPATH = /usr/sbin
ETCPATH = /etc
INITPATH = /etc/rc.d/init.d
ifeq ($(CONFIG_MSSID),y)
CFLAGS = $(COPTS) $(MIPS16_OPT) $(LTO) -Wall -I$(SRCBASE)/include.v24 -I. -I$(TOP)/shared
else
CFLAGS = $(COPTS) $(MIPS16_OPT) $(LTO) -Wall -I$(SRCBASE)/include.v23 -I. -I$(TOP)/shared
endif
CFLAGS += -fPIC
LIBS = -lcrypt -lnvram
LFLAGS = $(LDLTO) $(COPTS) -L$(TOP)/nvram

OBJS = bpalogin.o protocol.o transaction.o gen_md5.o unixmain.o libddns3.a

bpalogin: $(OBJS)
	$(CC) -o $@ $(OBJS) $(LIBS) $(LFLAGS)

.c.o:
	$(CC) $(CFLAGS) -c $<

clean:
	rm -fr *.o *.a
	rm -fr bpalogin
	@make -C ddns3-client clean

pristine: clean
	rm -f bpalogin

$(OBJS): bpalogin.h

libddns3.a:
	@make -C ddns3-client libddns3.a
	ln -sf ddns3-client/libddns3.a

install: bpalogin bpalogin.init
	mkdir -p $(SBINPATH)
	mkdir -p $(ETCPATH)
	mkdir -p $(INITPATH)
	install -m 755 bpalogin $(SBINPATH)
	install -m 600 bpalogin.conf $(ETCPATH)
	install -m 755 bpalogin.init $(INITPATH)/bpalogin
