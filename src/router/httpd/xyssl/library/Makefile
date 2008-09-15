
# Compilation flags:
#
#    SELF_TEST          enable all checkup routines
#    HAVE_RDTSC         CPU supports rdtsc (x86 only)
#    HAVE_SSE2          CPU supports sse2  (x86 only)
#    HAVE_LONGLONG      compiler supports long long
#    _BSD_EXTENSION     needed to compile on Plan 9
#
# Add -fPIC to CFLAGS to compile as a shared library

DEFINES	= -DHAVE_RDTSC -DHAVE_LONGLONG -DHAVE_RDTSC #-DSELF_TEST -DHAVE_RDTSC #-DHAVE_SSE2
CFLAGS	= $(DEFINES) -I../include/xyssl $(COPTS)

OBJS=aes.o       arc4.o      base64.o    bignum.o    \
     certs.o     des.o       dhm.o       havege.o    \
     md2.o       md4.o       md5.o       net.o       \
     rsa.o       sha1.o      sha2.o      ssl_cli.o   \
     ssl_srv.o   ssl_tls.o   timing.o    x509_read.o
#OBJS=aes.o  bignum.o    \
      des.o       dhm.o       havege.o    \
      sha2.o         \
     timing.o

all: static shared 

static: libxyssl.a

libxyssl.a: $(OBJS)
	$(AR) r $@ $(OBJS)

shared: libxyssl.so

libxyssl.so: $(OBJS) 
	$(LD) -shared -o $@ $^

.c.o:
	$(CC) $(CFLAGS) -c $<

clean:
	@rm -f *.o libxyssl.*

