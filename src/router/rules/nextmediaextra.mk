nextmediaextra-configure:
	@true
nextmediaextra:
	@true
nextmediaextra-install:
ifeq ($(CONFIG_MUSL),y)
	-nextmediaextra/install.sh $(TOP) $(INSTALLDIR) $(PLATFORM)-musl 
else
	-nextmediaextra/install.sh $(TOP) $(INSTALLDIR) $(PLATFORM) 
endif
