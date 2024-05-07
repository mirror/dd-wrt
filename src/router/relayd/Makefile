#CFLAGS = -Os -Wall -Werror -pedantic --std=gnu99C
CFLAGS+= -I$(TOP)/kernel_headers/$(KERNELRELEASE)/include
CFLAGS += $(COPTS) $(MIPS16_OPT) $(LTO) -D_GNU_SOURCE -ffunction-sections -fdata-sections -Wl,--gc-sections  -D_GNU_SOURCE
CPPFLAGS = $(CFLAGS) -I. -I$(LINUXDIR)/include
LDFLAGS =

all: relayd 

relayd: uloop.o main.o route.o dhcp.o
	$(CC) -o $@ $^ $(LDFLAGS)

relayd.h: list.h
uloop.c: uloop.h
main.c: uloop.h relayd.h
route.c: relayd.h

%.o: %.c
	$(CC) -c $(CPPFLAGS) $(CFLAGS) -o $@ $^


clean:
	rm -f relayd *.o
