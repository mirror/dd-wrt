#AR=ar
#CC=gcc
CF=-Wall $(COPTS)
CL=-lcrypt
#ST=strip

all:		ddns3

ddns3:		ddns3.c libddns3.a
	$(CC) $(CF) $(CL) ddns3.c libddns3.a -o ddns3
	$(ST) ddns3

libddns3.a:	sockio.o crypto.o auth.o md5.o ctx.o
	$(AR) -csr libddns3.a *.o

ctx.o:		ctx.c ctx.h
	$(CC) $(CF) -c ctx.c -o ctx.o

sockio.o:	sockio.c sockio.h
	$(CC) $(CF) -c sockio.c -o sockio.o

auth.o: 	auth.c auth.h
	$(CC) $(CF) -c auth.c -o auth.o

crypto.o:	crypto.c crypto.h
	$(CC) $(CF) -c crypto.c -o crypto.o

md5.o:		md5.c md5.h
	$(CC) $(CF) -c md5.c -o md5.o


clean:
	rm -f *.o core *.a ddns3
