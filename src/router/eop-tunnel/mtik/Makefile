all: eoip

eoip: eoipcr.c libnetlink.o
	$(CC) $(COPTS) $(MIPS16_OPT) $(LTO) -DNEED_PRINTF -I$(TOP)/kernel_headers/$(KERNELRELEASE)/include -ffunction-sections -fdata-sections -Wl,--gc-sections -Wall -o eoip eoipcr.c libnetlink.o
 
libnetlink.o: libnetlink.c libnetlink.h
	$(CC) $(COPTS) $(MIPS16_OPT) $(LTO) -DNEED_PRINTF -I$(TOP)/kernel_headers/$(KERNELRELEASE)/include -ffunction-sections -fdata-sections -Wl,--gc-sections -Wall -c libnetlink.c

clean:
	rm -f eoip libnetlink.o
