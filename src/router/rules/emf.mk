emf:
	$(MAKE) -C emf/emfconf CROSS=$(CROSS_COMPILE)
	$(MAKE) -C emf/igsconf CROSS=$(CROSS_COMPILE)
	$(MAKE) -C emf/emf
	$(MAKE) -C emf/igs

emf-clean:
	$(MAKE) -C emf/emfconf clean
	$(MAKE) -C emf/igsconf clean
	$(MAKE) -C emf/emf clean
	$(MAKE) -C emf/igs clean

emf-install:
	$(MAKE) -C emf/igsconf CROSS=$(CROSS_COMPILE) INSTALLDIR=$(INSTALLDIR)/emf install
	$(MAKE) -C emf/emfconf CROSS=$(CROSS_COMPILE) INSTALLDIR=$(INSTALLDIR)/emf install
	$(MAKE) -C emf/igs CROSS=$(CROSS_COMPILE) INSTALL_MOD_PATH=$(INSTALLDIR)/emf install
	$(MAKE) -C emf/emf CROSS=$(CROSS_COMPILE) INSTALL_MOD_PATH=$(INSTALLDIR)/emf install
