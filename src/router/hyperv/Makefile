# Makefile for Hyper-V tools

CC = $(ARCH)-linux-gcc
PTHREAD_LIBS = -lpthread
WARNINGS = -Wall -Wextra
CFLAGS = $(WARNINGS) $(PTHREAD_LIBS) $(COPTS)

CFLAGS += -D__EXPORTED_HEADERS__ -I$(LINUXDIR)/include/uapi -I$(LINUXDIR)/include -ffunction-sections -fdata-sections -Wl,--gc-sections

all: hv_kvp_daemon hv_vss_daemon hv_fcopy_daemon
%: %.c
	$(CC) $(CFLAGS) -o $@ $^

clean:
	$(RM) hv_kvp_daemon hv_vss_daemon hv_fcopy_daemon

install:
	mkdir -p $(INSTALLDIR)/usr/sbin
	cp -f hv_kvp_daemon hv_vss_daemon hv_fcopy_daemon $(INSTALLDIR)/usr/sbin
	cp -f lsvmbus *.sh $(INSTALLDIR)/usr/sbin
	chmod 777 $(INSTALLDIR)/usr/sbin/lsvmbus