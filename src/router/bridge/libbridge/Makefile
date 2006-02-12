CC := $(CC) -Wall -c -g $(COPTS) 

all:			libbridge.a

clean:
			rcsclean *
			rm -f *~
			rm -f *.o
			rm -f libbridge.a

libbridge.a:		libbridge_compat.o libbridge_devif.o libbridge_if.o libbridge_init.o libbridge_misc.o
			$(AR) rcs libbridge.a libbridge_compat.o libbridge_devif.o libbridge_if.o libbridge_init.o libbridge_misc.o
			$(RANLIB) libbridge.a

libbridge_compat.o:	libbridge_compat.c if_index.c
			$(CC) -o libbridge_compat.o libbridge_compat.c

libbridge_devif.o:	libbridge_devif.c libbridge.h libbridge_private.h
			$(CC) -o libbridge_devif.o libbridge_devif.c

libbridge_if.o:		libbridge_if.c libbridge.h libbridge_private.h
			$(CC) -o libbridge_if.o libbridge_if.c

libbridge_init.o:	libbridge_init.c libbridge.h libbridge_private.h
			$(CC) -o libbridge_init.o libbridge_init.c

libbridge_misc.o:	libbridge_misc.c libbridge.h libbridge_private.h
			$(CC) -o libbridge_misc.o libbridge_misc.c
