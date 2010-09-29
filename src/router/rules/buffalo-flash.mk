buffalo_flash: 
	if test -e "buffalo_flash/Makefile"; then make -C buffalo_flash; fi
	@true

buffalo_flash-clean:
	if test -e "buffalo_flash/Makefile"; then make -C buffalo_flash clean; fi
	@true

buffalo_flash-install:
	if test -e "buffalo_flash/Makefile"; then make -C buffalo_flash install; fi
	@true
