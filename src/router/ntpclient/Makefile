
#CC = mipsel-linux-uclibc-gcc
CFLAGS += $(COPTS) $(MIPS16_OPT) -D_GNU_SOURCE -ffunction-sections -fdata-sections -Wl,--gc-sections  -Wall

all: ntpclient

ntpclient: mini-ntpclient.c
	$(CC) $(CFLAGS) mini-ntpclient.c -o ntpclient

clean:
	rm -f ntpclient *.o
