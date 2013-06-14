nextmediaextra-configure:
	@true
nextmediaextra:
	@true
nextmediaextra-install:
	-nextmediaextra/install.sh $(TOP) $(INSTALLDIR)

nld-configure: 
	@true

nld: libubox
	$(MAKE) -C private/nld all

nld-install:
	install -D private/nld/nld $(INSTALLDIR)/nld/usr/sbin/nld
	install -D private/nld/scripts/nldadd.sh $(INSTALLDIR)/nld/usr/sbin/nldadd.sh
	install -D private/nld/scripts/nldstart.sh $(INSTALLDIR)/nld/usr/sbin/nldstart.sh
	install -D private/nld/scripts/nldhelper.sh $(INSTALLDIR)/nld/usr/sbin/nldhelper.sh
	install -D private/nld/scripts/nldstop.sh $(INSTALLDIR)/nld/usr/sbin/nldstop.sh
	install -D private/nld/scripts/netmask_to_cidr.sh $(INSTALLDIR)/nld/usr/sbin/netmask_to_cidr.sh
	install -D private/nld/show_caps $(INSTALLDIR)/nld/usr/sbin/show_caps
	#install -D private/nld/register_has_cap $(INSTALLDIR)/nld/usr/sbin/register_has_cap

nld-clean:
	$(MAKE) -C private/nld clean
	
