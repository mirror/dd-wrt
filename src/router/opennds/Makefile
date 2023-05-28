
CC?=gcc
CFLAGS?=-O2 -g -Wall
CFLAGS+=-Isrc
#CFLAGS+=-Wall -Wwrite-strings -pedantic -std=gnu99
LDFLAGS+=-pthread
LDLIBS=-lmicrohttpd

STRIP=yes

NDS_OBJS=src/auth.o src/client_list.o src/commandline.o src/conf.o \
	src/debug.o src/fw_iptables.o src/main.o src/http_microhttpd.o src/http_microhttpd_utils.o \
	src/ndsctl_thread.o src/safe.o src/util.o

.PHONY: all clean install

all: opennds ndsctl

%.o : %.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

opennds: $(NDS_OBJS) $(LIBHTTPD_OBJS)
	$(CC) $(LDFLAGS) -o opennds $+ $(LDLIBS)

ndsctl: src/ndsctl.o
	$(CC) $(LDFLAGS) -o ndsctl $+ $(LDLIBS)

clean:
	rm -f opennds ndsctl src/*.o
	rm -rf dist

install:
#ifeq(yes,$(STRIP))
	strip opennds
	strip ndsctl
#endif
	mkdir -p $(DESTDIR)/usr/bin/
	cp ndsctl $(DESTDIR)/usr/bin/
	cp opennds $(DESTDIR)/usr/bin/
	mkdir -p $(DESTDIR)/etc/opennds/htdocs/images
	cp resources/opennds.conf $(DESTDIR)/etc/opennds/
	cp linux_openwrt/opennds/files/etc/config/opennds $(DESTDIR)/etc/opennds/opennds.uci
	cp resources/splash.css $(DESTDIR)/etc/opennds/htdocs/
	cp resources/splash.jpg $(DESTDIR)/etc/opennds/htdocs/images/
	mkdir -p $(DESTDIR)/etc/systemd/system
	cp resources/opennds.service $(DESTDIR)/etc/systemd/system/
	mkdir -p $(DESTDIR)/usr/lib/opennds
	cp forward_authentication_service/binauth/binauth_log.sh $(DESTDIR)/usr/lib/opennds/
	sed -i 's/#!\/bin\/sh/#!\/bin\/bash/' $(DESTDIR)/usr/lib/opennds/binauth_log.sh
	cp forward_authentication_service/libs/libopennds.sh $(DESTDIR)/usr/lib/opennds/
	sed -i '0,/#!\/bin\/sh/{s/#!\/bin\/sh/#!\/bin\/bash/}' $(DESTDIR)/usr/lib/opennds/libopennds.sh
	cp forward_authentication_service/PreAuth/theme_click-to-continue-basic.sh $(DESTDIR)/usr/lib/opennds/
	sed -i 's/#!\/bin\/sh/#!\/bin\/bash/' $(DESTDIR)/usr/lib/opennds/theme_click-to-continue-basic.sh
	cp forward_authentication_service/PreAuth/theme_click-to-continue-custom-placeholders.sh $(DESTDIR)/usr/lib/opennds/
	sed -i 's/#!\/bin\/sh/#!\/bin\/bash/' $(DESTDIR)/usr/lib/opennds/theme_click-to-continue-custom-placeholders.sh
	cp forward_authentication_service/PreAuth/theme_user-email-login-basic.sh $(DESTDIR)/usr/lib/opennds/
	sed -i 's/#!\/bin\/sh/#!\/bin\/bash/' $(DESTDIR)/usr/lib/opennds/theme_user-email-login-basic.sh
	cp forward_authentication_service/PreAuth/theme_user-email-login-custom-placeholders.sh $(DESTDIR)/usr/lib/opennds/
	sed -i 's/#!\/bin\/sh/#!\/bin\/bash/' $(DESTDIR)/usr/lib/opennds/theme_user-email-login-custom-placeholders.sh
	cp forward_authentication_service/libs/get_client_interface.sh $(DESTDIR)/usr/lib/opennds/
	sed -i 's/#!\/bin\/sh/#!\/bin\/bash/' $(DESTDIR)/usr/lib/opennds/get_client_interface.sh
	cp forward_authentication_service/libs/client_params.sh $(DESTDIR)/usr/lib/opennds/
	sed -i 's/#!\/bin\/sh/#!\/bin\/bash/' $(DESTDIR)/usr/lib/opennds/client_params.sh
	cp forward_authentication_service/libs/unescape.sh $(DESTDIR)/usr/lib/opennds/
	sed -i 's/#!\/bin\/sh/#!\/bin\/bash/' $(DESTDIR)/usr/lib/opennds/unescape.sh
	cp forward_authentication_service/libs/authmon.sh $(DESTDIR)/usr/lib/opennds/
	sed -i 's/#!\/bin\/sh/#!\/bin\/bash/' $(DESTDIR)/usr/lib/opennds/authmon.sh
	cp forward_authentication_service/libs/dnsconfig.sh $(DESTDIR)/usr/lib/opennds/
	sed -i 's/#!\/bin\/sh/#!\/bin\/bash/' $(DESTDIR)/usr/lib/opennds/dnsconfig.sh
	cp forward_authentication_service/libs/download_resources.sh $(DESTDIR)/usr/lib/opennds/
	sed -i 's/#!\/bin\/sh/#!\/bin\/bash/' $(DESTDIR)/usr/lib/opennds/download_resources.sh
	cp forward_authentication_service/libs/post-request.php $(DESTDIR)/usr/lib/opennds/
	cp forward_authentication_service/fas-aes/fas-aes.php $(DESTDIR)/etc/opennds/
	cp forward_authentication_service/fas-hid/fas-hid.php $(DESTDIR)/etc/opennds/
	cp forward_authentication_service/fas-aes/fas-aes-https.php $(DESTDIR)/etc/opennds/


