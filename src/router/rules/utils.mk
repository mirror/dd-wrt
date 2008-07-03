utils-clean:
	make -C utils clean

utils: nvram shared
	make -C utils

utils-install:
	make -C utils install

