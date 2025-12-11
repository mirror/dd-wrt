CFLAGS	+= -g3 -Wall -Wno-unknown-warning-option -Wno-array-bounds -Wno-format-truncation -Werror -Wno-error=deprecated-declarations --std=gnu99 -DUBUS_SUPPORT -DNEED_PRINTF
CFLAGS	+= $(COPTS) -I$(TOP)/libubox -I$(TOP)/ubus -I$(TOP)/_staging/usr/include -I$(TOP)/libbpf/src -I$(TOP)/libnl-tiny/include -I$(TOP)/kernel_headers/$(KERNELRELEASE)/include

LDFLAGS += -L$(TOP)/libbpf/src -lbpf -L$(TOP)/libnl-tiny -lnl-tiny -L$(TOP)/elfutils/libelf -lelf -L$(TOP)/zlib -lz -L$(TOP)/libubox -lubox -L$(TOP)/ubus -lubus

OBJS := main.o nl.o device.o fdb.o bpf.o flow.o ubus.o

all: bridger

clean:
	rm -f *.o bridger

install: all
	install -d $(INSTALLDIR)/usr/sbin
	install -D bridger $(INSTALLDIR)/usr/sbin/bridger

bridger: $(OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)
