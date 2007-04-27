rtpproxy:
	cd rtpproxy && ./configure --host=$(ARCH)-uclibc-linux
	$(MAKE) -C rtpproxy

rtpproxy-install:

	install -D rtpproxy/rtpproxy $(INSTALLDIR)/rtpproxy/usr/bin/rtpproxy
	
rtpproxy-clean:
	$(MAKE) -C rtpproxy clean