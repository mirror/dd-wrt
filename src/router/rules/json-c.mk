json-c-configure: 
	cd json-c && ./autogen.sh && \
		ac_cv_func_malloc_0_nonnull=yes \
		ac_cv_func_realloc_0_nonnull=yes \
		./configure --host=$(ARCH)-linux \
		--prefix=$(TOP)/_staging \
		--libdir=$(TOP)/_staging/usr/lib \
		--includedir=$(TOP)/_staging/usr/include \
		CFLAGS="-fPIC -DNEED_PRINTF $(COPTS) $(MIPS16_OPT)" LDFLAGS="-lm"
	make -C json-c
	-mkdir -p $(TOP)/_staging
	make -C json-c install

json-c:
	make -C json-c
	-mkdir -p $(TOP)/_staging
	make -C json-c install

json-c-install:
	install -D json-c/.libs/libjson-c.so.2.0.2 $(INSTALLDIR)/json-c/usr/lib/libjson-c.so.2.0.2
	cd $(INSTALLDIR)/json-c/usr/lib ; ln -s libjson-c.so.2.0.2 libjson-c.so.2  ; true
	cd $(INSTALLDIR)/json-c/usr/lib ; ln -s libjson-c.so.2.0.2 libjson-c.so  ; true

json-c-clean: 
	make -C json-c clean
