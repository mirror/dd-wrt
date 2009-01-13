
# To compile on SunOS: add "-lsocket -lnsl" to LDFLAGS
# To compile on MinGW: add "-lws2_32" to LDFLAGS

CFLAGS	= -Iinclude
OFLAGS	= $(COPTS) -ffunction-sections -fdata-sections -Wl,--gc-sections
LDFLAGS	= -Llibrary -lxyssl 

APPS =  wgets
.SILENT:

all: $(APPS)

library/libxyssl.a:
	make -C library

wgets: wgets.c library/libxyssl.a
	echo   "  CC    wgets.c"
	$(CC) $(CFLAGS) $(OFLAGS) wgets.c  $(LDFLAGS) -o $@

clean:
	rm -f $(APPS)
	make -C library clean

    