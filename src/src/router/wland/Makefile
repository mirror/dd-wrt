
include $(TOP)/.config

ifneq ($(wildcard $(SRCBASE)/cy_conf.mak),)
  include $(SRCBASE)/cy_conf.mak
endif


CFLAGS	= -I../rc -I$(TOP)/shared -I$(SRCBASE)/include -Wall -I$(SRCBASE)/
CFLAGS  += -I$(TOP)/iptables/include -I$(TOP)/iptables/include/libipq -I$(TOP)/libnet/include -DL_ENDIAN -DLIBNET_LIL_ENDIAN -DLIB1X_LIL_ENDIAN
CFLAGS	+= -s -Os 
LDFLAGS	+= -L$(TOP)/nvram -L$(INSTALLDIR)/nvram/usr/lib -lnvram -L$(TOP)/shared -L$(INSTALLDIR)/shared/usr/lib -lshared -L$(SRCBASE)/router/libnet/lib -lnet -L$(TOP)/matrixssl/src -lmatrixssl  -lcrypt 

OBJS := ../rc/wland.o



vpath %.c $(TOP)/shared $(SRCBASE)/rts/src

all: wland

clean:
	rm -f *.o wland

install: all
	install -d $(INSTALLDIR)/sbin
	install wland $(INSTALLDIR)/sbin
	$(STRIP) $(INSTALLDIR)/sbin/wland

wland: $(OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)

$(OBJS): $(CY_DEPS)
