#CC=gcc
WFLAGS=-Wall
#CFLAGS=$(COPTS)
INCLUDES=-Iinclude

LIBNAME=libnl-tiny.so
#LIBNAME=libnl-tiny.a

all: $(LIBNAME)

%.o: %.c
	$(CC) $(WFLAGS) -c -o $@ $(INCLUDES) $(CFLAGS) $<

LIBNL_OBJ=nl.o handlers.o msg.o attr.o cache.o cache_mngt.o object.o socket.o error.o
GENL_OBJ=genl.o genl_family.o genl_ctrl.o genl_mngt.o

$(LIBNAME): $(LIBNL_OBJ) $(GENL_OBJ)
	#$(AR) rcu $@ $^
	$(CC) -shared -o $@ $^

clean:
	-rm -f $(LIBNAME) *.o *.a
	@true
