all:
	cd src && make
	cd slsh && make
	cd modules && make
#
clean:
	-cd src && make clean
	-cd slsh && make clean
	-cd modules && make clean
