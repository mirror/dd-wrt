CFLAGS = $(COPTS) -Wall $(EXTRACFLAGS) -DNEED_PRINTF $(LTO)

all: tftpc tftpd

tftpc: tftpc.c
	$(CC) $(CFLAGS) $(LDLTO) -o tftpc tftpc.c

tftpd: tftpd.c
	$(CC) $(CFLAGS) $(LDLTO) -o tftpd tftpd.c

clean:
	rm -f tftpd tftpc

strip:
	strip tftpd tftpc

