fdisk-clean:
	make -C util-linux/fdisk clean

fdisk:
	make -C util-linux/fdisk

fdisk-install:
	make -C util-linux/fdisk install INSTALLDIR="mkdir -p" SBINDIR=$(INSTALLDIR)/fdisk/sbin INSTALLBIN="install -D"


