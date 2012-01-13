#CFLAGS = -Os -Wall -Werror -pedantic --std=gnu99
CFLAGS += $(COPTS)
CPPFLAGS = -I. -I$(LINUXDIR)/include
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
