zlib-clean:
	make -C zlib clean

zlib:
	make -j 4 -C zlib

zlib-install:
	@true

