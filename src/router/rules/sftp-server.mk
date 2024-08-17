sftp-server-clean:
	make -C sftp-server clean

sftp-server: nvram shared
	make -C sftp-server

sftp-server-install:
	make -C sftp-server install INSTALLDIR=$(INSTALLDIR)/sftp-server

