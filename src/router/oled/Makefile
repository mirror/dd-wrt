ifneq ($(wildcard $(SRCBASE)/cy_conf.mak),)
  include $(SRCBASE)/cy_conf.mak
endif

include $(TOP)/.config

OBJS := oled_print.o oled.o gpio.o
CFLAGS := $(COPTS)
  
LDFLAGS := -L../libnet/lib

all: oled-print

oled-print: $(OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)

clean:
	rm -f *.o *~ oled-print

install: all
	install -d $(INSTALLDIR)/oled/usr/sbin
	install oled-print $(INSTALLDIR)/oled/usr/sbin
