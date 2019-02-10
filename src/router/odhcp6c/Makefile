###
###	Makefile for disktype
###

OBJS   = dhcpv6.o md5.o odhcp6c.o ra.o script.o
TARGET = odhcp6c

CFLAGS   = $(COPTS)  -ffunction-sections -fdata-sections -Wl,--gc-sections -DNEED_PRINTF -std=c99 -DEXT_CER_ID -DEXT_PREFIX_CLASS -D_BSD_SOURCE -D_GNU_SOURCE
LDFLAGS = -ffunction-sections -fdata-sections -Wl,--gc-sections

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) -o $(TARGET) $(OBJS)

$(OBJS): %.o: %.c
	$(CC) $(CFLAGS) -c $<


clean:
	$(RM) *.o *~ *% $(TARGET)
	
install:
	install -D $(TARGET) $(INSTALLDIR)/usr/sbin/$(TARGET)
	install -D odhcp6c-example-script.sh $(INSTALLDIR)/usr/sbin/odhcp6c-update
