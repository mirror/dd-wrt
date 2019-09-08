all:
	$(MAKE) -C ipt
	$(MAKE) -C src

modules_install:
	$(MAKE) -C src modules_install
install:
	$(MAKE) -C ipt install
clean:
	$(MAKE) -C src clean
	$(MAKE) -C ipt clean
distclean:
	$(MAKE) clean
	$(MAKE) -C src distclean
