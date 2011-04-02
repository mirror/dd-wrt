rt3062:
	make -C rt3062 CHIPSET=3062

rt3062-install:
	install -D rt3062/os/linux/rt3062ap.ko $(INSTALLDIR)/rt3062/lib/rt3062/rt3062ap.ko


rt2860:
	make -C rt3062 CHIPSET=2860

rt2860-install:
	install -D rt3062/os/linux/rt2860ap.ko $(INSTALLDIR)/rt3062/lib/rt3062/rt2860ap.ko
	

