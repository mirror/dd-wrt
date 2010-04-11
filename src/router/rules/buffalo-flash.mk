buffalo_flash-checkout:
	rm -rf $(TOP)/buffalo_flash
	svn co svn://svn.dd-wrt.com/private/buffalo_flash $(TOP)/buffalo_flash

buffalo_flash-update:
	svn update $(TOP)/buffalo_flash


buffalo_flash: 
	if test -e "buffalo_flash/Makefile"; then make -C buffalo_flash; fi
	@true

buffalo_flash-clean:
	if test -e "buffalo_flash/Makefile"; then make -C buffalo_flash clean; fi
	@true

buffalo_flash-install:
	if test -e "buffalo_flash/Makefile"; then make -C buffalo_flash install; fi
	@true
