#all:
#	cd atm; $(MAKE) $(MFLAGS) all
#	cp ./atm/lib/libatm.a ./br2684ctl
#	cd br2684ctl; $(MAKE) $(MFLAGS) 
libatm:
	cd atm; $(MAKE) $(MFLAGS) all
	cp ./atm/lib/libatm.a ./br2684ctl

install:
	mkdir -p $(INSTALLDIR)/usr/sbin
	cd br2684ctl; $(MAKE) $(MFLAGS) dynamic
	cd pvc2684ctl; $(MAKE) $(MFLAGS) dynamic
	install -m 755 br2684ctl/br2684ctl $(INSTALLDIR)/usr/sbin
	$(STRIP) $(INSTALLDIR)/usr/sbin/br2684ctl
	cd pvc2684ctl; $(MAKE) $(MFLAGS) install

dynamic: libatm install


static: libatm 
	cd br2684ctl; $(MAKE) $(MFLAGS) static

all: libatm 
	cd br2684ctl; $(MAKE) $(MFLAGS) static


clean:
	rm -f `find . -name '*.[oas]' -print`
	rm -f `find . -name 'core' -print`
	rm -f `find . -name '*~' -print`
	cd atm; $(MAKE) clean
	cd br2684ctl; $(MAKE) clean
	cd pvc2684ctl; $(MAKE) $(MFLAGS) clean

dist-clean:	clean
	rm -f Makefile `find . -name Makefile -print`
