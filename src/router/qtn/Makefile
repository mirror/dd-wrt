include $(TOP)/.config


all: 
	make -C libqcsapi_client
	make -C tftp

clean: 
	make -C libqcsapi_client clean
	make -C tftp clean

install:
	install -D tftp/tftpd $(INSTALLDIR)/usr/sbin/tftpd
	mkdir -p $(INSTALLDIR)/etc/qtn
	mkdir -p $(INSTALLDIR)/usr/lib
	cp libqcsapi_client/libqcsapi_client.so.1 $(INSTALLDIR)/usr/lib
	cp images/* $(INSTALLDIR)/etc/qtn
	

