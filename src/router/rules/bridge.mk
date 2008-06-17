bridge:
	@true
#	$(MAKE) -C bridge brctl/brctl

bridge-clean:
	@true
#	@echo "Cleaning bridge"
##	@if [ -e bridge/libbridge/libbridge.a ]; then\
#		rm bridge/brctl/*.o bridge/libbridge/*.a bridge/libbridge/*.o;\
#	else echo "Nothing to clean";\
#	fi;

bridge-install:
	@true
#	install -D bridge/brctl/brctl $(INSTALLDIR)/bridge/usr/sbin/brctl
#	$(STRIP) $(INSTALLDIR)/bridge/usr/sbin/brctl

