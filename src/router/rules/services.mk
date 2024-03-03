services-clean:
	make -C services clean

services: nvram shared
	make -j 4 -C services

services-install:
	make -C services install

