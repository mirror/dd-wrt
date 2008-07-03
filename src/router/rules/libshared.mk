shared-clean:
	make -C shared clean

shared: wireless-tools nvram
	make -C shared

shared-install:
	make -C shared install

