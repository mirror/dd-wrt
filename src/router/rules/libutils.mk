libutils-clean:
	make -C libutils clean

libutils: nvram libnltiny
	make -C libutils

libutils-install:
	make -C libutils install

