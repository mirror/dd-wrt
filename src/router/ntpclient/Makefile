
#CC = mipsel-linux-uclibc-gcc
CFLAGS += $(COPTS) -Wall

all: ntpclient

ntpclient: mini-ntpclient.c
	$(CC) $(CFLAGS) mini-ntpclient.c -o ntpclient

clean:
	rm -f ntpclient *.o
