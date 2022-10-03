# copy libxt_WGOBFS.so to:
#     alpinelinux /usr/lib/xtables/
#     debian      /usr/lib/x86_64-linux-gnu/xtables
#     centos7     /usr/lib64/xtables
#

IDIR    =../include
CFLAGS  = -O2 -Wall -I$(IDIR)
CFLAGS += -fpic
LDFLAGS = -shared

TARGET = libxt_WGOBFS.so
OBJECTS = libxt_WGOBFS.o siphash.o


all: $(TARGET)

clean:
	rm -f $(OBJECTS) $(TARGET)

$(TARGET): $(OBJECTS)
	gcc $(CFLAGS) $(LDFLAGS) -o $@ $^
