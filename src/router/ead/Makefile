#CC       = gcc
CPPFLAGS = -I. -Itinysrp
CFLAGS   = $(COPTS) -Wall -I$(SRCBASE)/include.v24 -I$(TOP)/libpcap_noring  -ffunction-sections -fdata-sections -Wl,--gc-sections
LDFLAGS	 =

LIBS	 = tinysrp/libtinysrp.a
LIBS_EAD = $(LIBS) -L$(TOP)/libpcap_noring $(TOP)/libpcap_noring/libpcap.a -L$(TOP)/nvram -lnvram
CONFIGURE_ARGS = --host=$(ARCH)-linux --target=$(ARCH)-linux 

all: ead ead-client

obj = ead-crypt.o

tinysrp/Makefile:
	cd tinysrp; ./configure $(CONFIGURE_ARGS)

tinysrp/libtinysrp.a: tinysrp/Makefile
	-$(MAKE) -C tinysrp CFLAGS="$(CFLAGS)"

%.o: %.c $(wildcard *.h) tinysrp/libtinysrp.a
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

ead.o: filter.c
ead-crypt.o: aes.c sha1.c

ead: ead.o $(obj) tinysrp/libtinysrp.a
	$(CC) -o $@ $< $(obj) $(LDFLAGS) $(LIBS_EAD)

ead-client: ead-client.o $(obj)
	$(CC) -o $@ $< $(obj) $(LDFLAGS) $(LIBS) -L$(TOP)/libutils -L$(TOP)/nvram  -lutils -lnvram

clean:
	rm -f *.o ead ead-client
	if [ -f tinysrp/Makefile ]; then $(MAKE) -C tinysrp distclean; fi


install: 
	@true
#	install -d $(INSTALLDIR)/usr/sbin
#	install ead $(INSTALLDIR)/usr/sbin
#	install ead-client $(INSTALLDIR)/usr/sbin