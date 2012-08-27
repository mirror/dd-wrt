minidlna-configure:
	cd minidlna && make clean
	cd minidlna && make distclean
	cd minidlna && make

minidlna:
	cd minidlna && make

minidlna-clean:
	cd minidlna && make clean

minidlna-install:
	cd minidlna && make install TARGETDIR=$(ARCH)-uclibc/install/minidlna
