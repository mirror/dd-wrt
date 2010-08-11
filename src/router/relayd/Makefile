#CFLAGS = -Os -Wall -Werror -pedantic --std=gnu99
CFLAGS += $(COPTS)
CPPFLAGS = -I.
LDFLAGS =

all: relayd 

relayd: uloop.o main.o
	$(CC) -o $@ $^ $(LDFLAGS)

uloop.c: uloop.h
main.c: uloop.h

%.o: %.c
	$(CC) -c $(CPPFLAGS) $(CFLAGS) -o $@ $^


clean:
	rm -f relayd *.o
