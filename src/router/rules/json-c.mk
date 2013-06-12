json-c-configure: 
	cd json-c && \
		ac_cv_func_malloc_0_nonnull=yes \
		ac_cv_func_realloc_0_nonnull=yes \
		./configure --host=$(ARCH)-linux \
		--prefix=$(TOP)/_staging \
		--libdir=$(TOP)/_staging/usr/lib \
		--includedir=$(TOP)/_staging/usr/include \
		CFLAGS="-fPIC -DNEED_PRINTF $(COPTS) $(MIPS16_OPT)" 

json-c:
	make -C json-c
	-mkdir -p $(TOP)/_staging
	make -C json-c install
	make -C json-c install-libjsonincludeHEADERS

json-c-install:
	install -D json-c/.libs/libjson.so.0.0.1 $(INSTALLDIR)/json-c/usr/lib/libjson.so.0.0.1
	cd $(INSTALLDIR)/json-c/usr/lib ; ln -s libjson.so.0.0.1 libjson.so.0  ; true
	cd $(INSTALLDIR)/json-c/usr/lib ; ln -s libjson.so.0.0.1 libjson.so  ; true

json-c-clean: 
	make -C json-c clean
