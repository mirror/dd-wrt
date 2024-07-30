#CC=gcc
WFLAGS=-Wall
#CFLAGS=$(COPTS)
INCLUDES=-Iinclude

LIBNAME=libnl-tiny.so
#LIBNAME=libnl-tiny.a

all: $(LIBNAME)

%.o: %.c
	$(CC) $(WFLAGS) -c -o $@ $(INCLUDES) $(CFLAGS) $(MIPS16_OPT) -D_GNU_SOURCE -DNDEBUG -Wall -Werror -Wextra -Wno-unused-parameter $<

LIBNL_OBJ=nl.o handlers.o msg.o attr.o cache.o cache_mngt.o object.o socket.o error.o
GENL_OBJ=genl.o genl_family.o genl_ctrl.o genl_mngt.o unl.o

$(LIBNAME): $(LIBNL_OBJ) $(GENL_OBJ)
	#$(AR) rcu $@ $^
	$(CC) $(CFLAGS) -shared -o $@ $^

clean:
	-rm -f $(LIBNAME) *.o *.a
	@true
