CC := $(CC) -Wall -c $(COPTS) -DNEED_PRINTF -D_GNU_SOURCE $(MIPS16_OPT) $(LTO) -ffunction-sections -fdata-sections -Wl,--gc-sections

all:			libbridge.a

clean:
			rm -f *~
			rm -f *.o
			rm -f libbridge.a

libbridge.a:		libbridge_devif.o libbridge_if.o libbridge_init.o libbridge_misc.o
			$(AR) rcs $(LTOPLUGIN) libbridge.a libbridge_devif.o libbridge_if.o libbridge_init.o libbridge_misc.o
			$(RANLIB) $(LTOPLUGIN) libbridge.a


libbridge_devif.o:	libbridge_devif.c libbridge.h libbridge_private.h
			$(CC) -o libbridge_devif.o libbridge_devif.c

libbridge_if.o:		libbridge_if.c libbridge.h libbridge_private.h
			$(CC) -o libbridge_if.o libbridge_if.c

libbridge_init.o:	libbridge_init.c libbridge.h libbridge_private.h
			$(CC) -o libbridge_init.o libbridge_init.c

libbridge_misc.o:	libbridge_misc.c libbridge.h libbridge_private.h
			$(CC) -o libbridge_misc.o libbridge_misc.c
