services-clean:
	make -C services clean

services: nvram shared
	make -C services

services-install:
	make -C services install

