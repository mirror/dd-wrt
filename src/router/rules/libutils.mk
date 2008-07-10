libutils-clean:
	make -C libutils clean

libutils: nvram
	make -C libutils

libutils-install:
	make -C libutils install

