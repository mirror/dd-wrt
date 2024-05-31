export MAKE := make -j $(shell getconf _NPROCESSORS_ONLN)
export LTO := -flto -fwhole-program -flto-partition=none 
export LTOAUTO := -flto=auto -fno-fat-lto-objects
export LDLTOAUTO := -fuse-ld=bfd -flto=auto -fuse-linker-plugin
export LTOMIN := -flto
export LDLTO := -flto=$(shell getconf _NPROCESSORS_ONLN) -fuse-linker-plugin
export LTOPLUGIN := --plugin=$(shell $(CROSS_COMPILE)gcc --print-file-name=liblto_plugin.so)
export GCCAR := ${shell which $(ARCH)-linux-gcc-ar}
export GCCRANLIB := ${shell which $(ARCH)-linux-gcc-ranlib}


#COPTS+= -D_LARGEFILE64_SOURCE -D_FILE_OFFSET_BITS=64 -fno-unwind-tables -fno-asynchronous-unwind-tables -funsigned-char
COPTS+=  -D_LARGEFILE64_SOURCE -D_FILE_OFFSET_BITS=64 -fno-unwind-tables -fno-asynchronous-unwind-tables -DNDEBUG
ifeq ($(AGGRESSIVE_SIZE),y)
COPTS+= -falign-jumps=1 -falign-labels=1 -falign-loops=1 -falign-functions=1 -fno-guess-branch-probability -funsigned-char -finline-limit=0 -fno-builtin-printf
endif

ifeq ($(ARCH),arm)
MUSL_LD:=ld-musl-armhf.so.1
KERNEL_HEADER_ARCH:=arm
endif
ifeq ($(ARCH),aarch64)
MUSL_LD:=ld-musl-aarch64.so.1
KERNEL_HEADER_ARCH:=arm64
endif
ifeq ($(ARCHITECTURE),northstar)
MUSL_LD:=ld-musl-arm.so.1
KERNEL_HEADER_ARCH:=arm
endif
ifeq ($(ARCHITECTURE),openrisc)
MUSL_LD:=ld-musl-arm.so.1
KERNEL_HEADER_ARCH:=arm
endif
ifeq ($(ARCH),armeb)
MUSL_LD:=ld-musl-armeb.so.1
KERNEL_HEADER_ARCH:=arm
endif
ifeq ($(ARCH),mips)
MUSL_LD:=ld-musl-mips-sf.so.1
KERNEL_HEADER_ARCH:=mips
endif
ifeq ($(ARCH),mipsel)
MUSL_LD:=ld-musl-mipsel-sf.so.1
KERNEL_HEADER_ARCH:=mips
endif
ifeq ($(ARCH),mips64)
MUSL_LD:=ld-musl-mips64-sf.so.1
KERNEL_HEADER_ARCH:=mips
endif
ifeq ($(ARCH),i386)
MUSL_LD:=ld-musl-i386.so.1
KERNEL_HEADER_ARCH:=i386
endif
ifeq ($(ARCH),x86_64)
MUSL_LD:=ld-musl-x86_64.so.1
KERNEL_HEADER_ARCH:=x86_64
endif
ifeq ($(ARCH),powerpc)
MUSL_LD:=ld-musl-powerpc-sf.so.1
KERNEL_HEADER_ARCH:=powerpc
endif


install_headers:
# important step, required for new kernels
	-mkdir -p $(TOP)/kernel_headers/$(KERNELRELEASE)
	$(MAKE) -C $(LINUXDIR) headers_install ARCH=$(KERNEL_HEADER_ARCH) INSTALL_HDR_PATH=$(TOP)/kernel_headers/$(KERNELRELEASE)

	
realclean: $(obj-clean)
	rm -f .config.old .config.cmd
	#umount $(TARGETDIR)
	rm -rf $(INSTALLDIR)
	rm -rf $(TARGETDIR)
	rm -f $(TARGETDIR)/*
	-rm -f $(ARCH)-uclibc/*

	
clean: rc-clean httpd-clean services-clean radauth-clean shared-clean libutils-clean nvram-clean madwifi-clean madwifi_mimo-clean busybox-clean dnsmasq-clean iptables-clean pppd-clean iproute2-clean
	rm -f .config.old .config.cmd
	#umount $(TARGETDIR)
	rm -rf httpd/ej_temp
	rm -rf $(INSTALLDIR)
	rm -rf $(TARGETDIR)
	rm -f $(TARGETDIR)/*
	-rm -f $(ARCH)-uclibc/*

clean_target:
	rm -rf $(TARGETDIR)
	rm -rf $(INSTALLDIR)

distclean mrproper: $(obj-distclean) clean_target
	rm -rf $(INSTALLDIR)
	-$(MAKE) -C $(LINUXDIR) distclean
	-$(MAKE) -C $(LINUXDIR)/arch/mips/bcm947xx/compressed clean
	$(MAKE) -C config clean
	rm -f .config $(LINUXDIR)/.config
	rm -f .config.old .config.cmd

optimize-lib:
ifneq ($(CONFIG_MUSL),y)
	cp ${shell $(ARCH)-linux-gcc -print-file-name=libc.so.0} $(ARCH)-uclibc/target/lib/libc.so.0 
else
	cp ${shell $(ARCH)-linux-gcc -print-file-name=libc.so} $(ARCH)-uclibc/target/lib/libc.so 
endif
ifneq ($(CONFIG_MUSL),y)
	cp ${shell $(ARCH)-linux-gcc -print-file-name=ld-uClibc.so.0} $(ARCH)-uclibc/target/lib/ld-uClibc.so.0 
else
	cd $(ARCH)-uclibc/target/lib && ln -sf libc.so $(MUSL_LD)
endif
	cp ${shell $(ARCH)-linux-gcc -print-file-name=libgcc_s.so.1} $(ARCH)-uclibc/target/lib/libgcc_s.so.1 
ifeq ($(CONFIG_LIBDL),y)
	-cp ${shell $(ARCH)-linux-gcc -print-file-name=libdl.so.0} $(ARCH)-uclibc/target/lib/libdl.so.0 
endif
ifeq ($(CONFIG_LIBRT),y)
	-cp ${shell $(ARCH)-linux-gcc -print-file-name=librt.so.0} $(ARCH)-uclibc/target/lib/librt.so.0 
endif
ifeq ($(CONFIG_LIBNSL),y)
	-cp ${shell $(ARCH)-linux-gcc -print-file-name=libnsl.so.0} $(ARCH)-uclibc/target/lib/libnsl.so.0 
endif
ifeq ($(CONFIG_LIBUTIL),y)
	-cp ${shell $(ARCH)-linux-gcc -print-file-name=libutil.so.0} $(ARCH)-uclibc/target/lib/libutil.so.0 
endif
ifeq ($(CONFIG_LIBCPP),y)
	-cp ${shell $(ARCH)-linux-gcc -print-file-name=libstdc++.so.6} $(ARCH)-uclibc/target/lib/libstdc++.so.6 
endif
ifeq ($(CONFIG_LIBCRYPT),y)
	-cp ${shell $(ARCH)-linux-gcc -print-file-name=libcrypt.so.0} $(ARCH)-uclibc/target/lib/libcrypt.so.0 
endif
ifeq ($(CONFIG_LIBM),y)
	-cp ${shell $(ARCH)-linux-gcc -print-file-name=libm.so.0} $(ARCH)-uclibc/target/lib/libm.so.0 
endif
ifeq ($(CONFIG_LIBRESOLV),y)
	-cp ${shell $(ARCH)-linux-gcc -print-file-name=libresolv.so.0} $(ARCH)-uclibc/target/lib/libresolv.so.0 
endif
ifeq ($(CONFIG_LIBPTHREAD),y)
	-cp ${shell $(ARCH)-linux-gcc -print-file-name=libpthread.so.0} $(ARCH)-uclibc/target/lib/libpthread.so.0 
endif
ifeq ($(CONFIG_SQUID),y)
	-cp ${shell $(ARCH)-linux-gcc -print-file-name=libatomic.so.1} $(ARCH)-uclibc/target/lib/libatomic.so.1 
endif
ifeq ($(CONFIG_OPENSSL),y)
	-cp ${shell $(ARCH)-linux-gcc -print-file-name=libatomic.so.1} $(ARCH)-uclibc/target/lib/libatomic.so.1 
endif
ifeq ($(CONFIG_FREERADIUS),y)
	-cp ${shell $(ARCH)-linux-gcc -print-file-name=libatomic.so.1} $(ARCH)-uclibc/target/lib/libatomic.so.1 
endif
ifeq ($(CONFIG_IPERF),y)
	-cp ${shell $(ARCH)-linux-gcc -print-file-name=libatomic.so.1} $(ARCH)-uclibc/target/lib/libatomic.so.1 
endif
	
ifeq ($(CONFIG_RELINK),y)
ifneq ($(CONFIG_MUSL),y)
	relink-lib.sh \
		"$(ARCH)-linux-" \
		"${shell $(ARCH)-linux-gcc -print-file-name=libc_so.a}" \
		"${shell $(ARCH)-linux-gcc -print-file-name=libc_so.a}" \
		"$(ARCH)-uclibc/target/lib/libc.so.0" \
		-Wl,-init,__uClibc_init -Wl,-soname=libc.so.0 \
		${shell $(ARCH)-linux-gcc -print-file-name=libgcc_s.so.1}
else
	relink-lib.sh \
		"$(ARCH)-linux-" \
		"${shell $(ARCH)-linux-gcc -print-file-name=libc.a}" \
		"${shell $(ARCH)-linux-gcc -print-file-name=libc.a}" \
		"$(ARCH)-uclibc/target/lib/libc.so" \
		-Wl,-init,__uClibc_init -Wl,-soname=libc.so \
		${shell $(ARCH)-linux-gcc -print-file-name=libgcc_s.so.1}
endif

	-relink-lib.sh \
		"$(ARCH)-linux-" \
		"${shell $(ARCH)-linux-gcc -print-file-name=libcrypt.so}" \
		"${shell $(ARCH)-linux-gcc -print-file-name=libcrypt_pic.a}" \
		"$(ARCH)-uclibc/target/lib/libcrypt.so.0" \
		${shell $(ARCH)-linux-gcc -print-file-name=libgcc_s.so.1} \
		-Wl,-soname=libcrypt.so.0

	-relink-lib.sh \
		"$(ARCH)-linux-" \
		"${shell $(ARCH)-linux-gcc -print-file-name=libm.so}" \
		"${shell $(ARCH)-linux-gcc -print-file-name=libm_pic.a}" \
		"$(ARCH)-uclibc/target/lib/libm.so.0" \
		${shell $(ARCH)-linux-gcc -print-file-name=libgcc_s.so.1} \
		-Wl,-soname=libm.so.0 

	-relink-lib.sh \
		"$(ARCH)-linux-" \
		"${shell $(ARCH)-linux-gcc -print-file-name=libpthread.so.0}" \
		"${shell $(ARCH)-linux-gcc -print-file-name=libpthread_so.a}" \
		"$(ARCH)-uclibc/target/lib/libpthread.so.0" \
		-Wl,-z,nodelete,-z,initfirst,-init=__pthread_initialize_minimal_internal \
		${shell $(ARCH)-linux-gcc -print-file-name=libgcc_s.so.1} \
		-Wl,-soname=libpthread.so.0
endif
	cp ${shell $(ARCH)-linux-gcc -print-file-name=libgcc_s.so.1} $(ARCH)-uclibc/target/lib/libgcc_s.so.1 
#	relink-lib.sh \
#		"$(ARCH)-linux-" \
#		"${shell $(ARCH)-linux-gcc -print-file-name=libgcc_s.so.1}" \
#		"${shell $(ARCH)-linux-gcc -print-file-name=libgcc_pic.a}" \
#		"$(ARCH)-uclibc/target/lib/libgcc_s.so.1" \
#		-Wl,--version-script=${shell $(ARCH)-linux-gcc -print-file-name=libgcc.map} -Wl,-soname=libgcc_s.so.1



ifneq ($(CONFIG_NOOPT),y)
	rm -rf /tmp/$(ARCHITECTURE)/mklibs-out
	rm -f /tmp/$(ARCHITECTURE)/mklibs-progs
	-mkdir -p /tmp/$(ARCHITECTURE)/
	find $(TARGETDIR) -type f -perm /100 -exec \
		file -r -N -F '' {} + | \
		awk ' /executable.*dynamically/ { print $$1 }' > /tmp/$(ARCHITECTURE)/mklibs-progs

	find $(TARGETDIR) -type f -name \*.so\* -exec \
		file -r -N -F '' {} + | \
		awk ' /shared object/ { print $$1 }' >> /tmp/$(ARCHITECTURE)/mklibs-progs

	mkdir -p /tmp/$(ARCHITECTURE)/mklibs-out
ifneq ($(CONFIG_MUSL),y)
	mklibs.py -D \
		-d /tmp/$(ARCHITECTURE)/mklibs-out \
		--sysroot $(TARGETDIR) \
		-L /lib \
		-L /usr/lib \
		--ldlib /lib/ld-uClibc.so.0 \
		--target $(ARCH)-linux-uclibc \
		`cat /tmp/$(ARCHITECTURE)/mklibs-progs` 2>&1
	cp /tmp/$(ARCHITECTURE)/mklibs-out/* $(TARGETDIR)/lib
else
	cp mklibs/* ${shell $(ARCH)-linux-gcc -print-file-name=include}/../../../../../bin
	rm -f /tmp/$(ARCHITECTURE)/lib/*
	cp musl/lib/*.so $(TARGETDIR)/lib
	-./mklibs/mklibs.py -D \
		-d /tmp/$(ARCHITECTURE)/mklibs-out \
		--sysroot $(TARGETDIR) \
		-L /lib \
		-L /usr/lib \
		-L /usr/lib/plexmediaserver/lib \
		--ldlib /lib/$(MUSL_LD) \
		--target $(ARCH)-linux-uclibc \
		`cat /tmp/$(ARCHITECTURE)/mklibs-progs` 2>&1

	-cp /tmp/$(ARCHITECTURE)/lib/* $(TARGETDIR)/lib
endif
endif
	../../tools/optimize_lib.sh libutils/ libutils.so libutils.a libutils_min.so $(TARGETDIR) $(TARGETDIR)/usr/lib/libutils.so  $(TOP)/libutils/libutils_ld
	../../tools/optimize_lib.sh libutils/ libwireless.so libwireless.a libwireless_min.so $(TARGETDIR) $(TARGETDIR)/usr/lib/libwireless.so $(TOP)/libutils/libwireless_ld
	../../tools/optimize_lib.sh libutils/ libshutils.so libshutils.a libshutils_min.so $(TARGETDIR) $(TARGETDIR)/usr/lib/libshutils.so $(TOP)/libutils/libshutils_ld
	../../tools/optimize_lib.sh nvram/ libnvram.so libnvram.a libnvram_min.so $(TARGETDIR) $(TARGETDIR)/lib/libnvram.so $(TOP)/nvram/libnvram_ld
#	rm -f $(TARGETDIR)/lib/*.a
#	rm -f $(TARGETDIR)/lib/*.map
#	cp lib.$(ARCH)/libresolv.so.0 $(TARGETDIR)/lib
#	cp lib.$(ARCH)/libgcc_s.so.1 $(TARGETDIR)/lib

kernel-relink-prep:
	rm -rf $(LINUXDIR)/include/ksym
	rm -f $(LINUXDIR)/include/generated/autoksyms.h
	rm -f $(LINUXDIR)/whitelist.h
	touch $(LINUXDIR)/whitelist.h

kernel-relink:
	
	rm -rf $(TARGETDIR)/lib/modules
	$(MAKE) -C $(LINUXDIR) modules_install DEPMOD=/bin/true INSTALL_MOD_PATH=$(TARGETDIR)
	rm -f $(TARGETDIR)/lib/modules/$(KERNELRELEASE)/build
	rm -f $(TARGETDIR)/lib/modules/$(KERNELRELEASE)/source

	-$(MAKE) -f Makefile.$(MAKEEXT) ath9k
	-$(MAKE) -f Makefile.$(MAKEEXT) ath9k-install
	-$(MAKE) -f Makefile.$(MAKEEXT) libutils
	-$(MAKE) -f Makefile.$(MAKEEXT) madwifi
	-$(MAKE) -f Makefile.$(MAKEEXT) madwifi-install
	-$(MAKE) -f Makefile.$(MAKEEXT) batman-adv
	-$(MAKE) -f Makefile.$(MAKEEXT) batman-adv-install
	-$(MAKE) -f Makefile.$(MAKEEXT) ndpi-netfilter
	-$(MAKE) -f Makefile.$(MAKEEXT) ndpi-netfilter-install
ifeq ($(CONFIG_NTFS3G),y)
	-$(MAKE) -f Makefile.$(MAKEEXT) ntfs3
	-$(MAKE) -f Makefile.$(MAKEEXT) ntfs3-install
endif
ifeq ($(CONFIG_IPV6),y)
	-$(MAKE) -f Makefile.$(MAKEEXT) nat46
	-$(MAKE) -f Makefile.$(MAKEEXT) nat46-install
endif
ifeq ($(CONFIG_ZFS),y)
	-$(MAKE) -f Makefile.$(MAKEEXT) zfs
	-$(MAKE) -f Makefile.$(MAKEEXT) zfs-install
endif
ifeq ($(CONFIG_QCA_NSS),y)
	-$(MAKE) -f Makefile.$(MAKEEXT) qca-nss
	-$(MAKE) -f Makefile.$(MAKEEXT) qca-nss-install
endif
ifeq ($(CONFIG_SMBD),y)
	-$(MAKE) -f Makefile.$(MAKEEXT) smbd
	-$(MAKE) -f Makefile.$(MAKEEXT) smbd-install
endif
ifeq ($(CONFIG_WIREGUARD),y)
	-$(MAKE) -f Makefile.$(MAKEEXT) wireguard
	-$(MAKE) -f Makefile.$(MAKEEXT) wireguard-install
endif
ifeq ($(CONFIG_OPENVPN),y)
	-$(MAKE) -f Makefile.$(MAKEEXT) openvpn-dco
	-$(MAKE) -f Makefile.$(MAKEEXT) openvpn-dco-install
endif
ifeq ($(CONFIG_I2C_GPIO_CUSTOM),y)
	-$(MAKE) -f Makefile.$(MAKEEXT) i2c-custom-gpio
	-$(MAKE) -f Makefile.$(MAKEEXT) i2c-custom-gpio-install
endif
ifeq ($(CONFIG_CAKE),y)
	-$(MAKE) -f Makefile.$(MAKEEXT) cake
	-$(MAKE) -f Makefile.$(MAKEEXT) cake-install
	-$(MAKE) -f Makefile.$(MAKEEXT) fq_codel_fast
	-$(MAKE) -f Makefile.$(MAKEEXT) fq_codel_fast-install
endif

	mkdir -p $(ARCH)-uclibc/target/usr/sbin/
	mkdir -p $(ARCH)-uclibc/target/lib/crda

#	cp $(TOP)/qtnhost/host/arm/qdpc-host.ko $(ARCH)-uclibc/target/lib/modules/$(KERNELRELEASE)/

	
	find $(ARCH)-uclibc/install $(ARCH)-uclibc/target  -name \*.ko | \
		xargs $(ARCH)-linux-nm | \
		awk '$$1 == "U" { print $$2 } ' | \
		sort -u > $(LINUXDIR)/mod_symtab.txt
	$(ARCH)-linux-nm -n $(LINUXDIR)/vmlinux.o | awk '/^[0-9a-f]+ [rR] __ksymtab_/ {print substr($$$$3,11)}' > $(LINUXDIR)/kernel_symtab.txt
#	grep -f $(LINUXDIR)/mod_symtab.txt $(LINUXDIR)/kernel_symtab.txt -F > $(LINUXDIR)/sym_include.txt
	cat $(LINUXDIR)/mod_symtab.txt > $(LINUXDIR)/sym_include.txt
	grep -vf $(LINUXDIR)/mod_symtab.txt $(LINUXDIR)/kernel_symtab.txt -F > $(LINUXDIR)/sym_exclude.txt
	( \
		cat $(LINUXDIR)/mod_symtab.txt | \
			awk '{print "#undef __KSYM_" $$$$1 " " }'; \
		cat $(LINUXDIR)/mod_symtab.txt | \
			awk '{print "#define __KSYM_" $$$$1 " 1" }'; \
		echo; \
	) > $(LINUXDIR)/whitelist.h
	( \
		echo '#define SYMTAB_KEEP \'; \
		cat $(LINUXDIR)/sym_include.txt | \
			awk '{print "KEEP(*(___ksymtab+" $$$$1 ")) \\" }'; \
		echo; \
		echo '#define SYMTAB_KEEP_GPL \'; \
		cat $(LINUXDIR)/sym_include.txt | \
			awk '{print "KEEP(*(___ksymtab_gpl+" $$$$1 ")) \\" }'; \
		echo; \
		echo '#define SYMTAB_DISCARD \'; \
		cat $(LINUXDIR)/sym_exclude.txt | \
			awk '{print "*(___ksymtab+" $$$$1 ") \\" }'; \
		echo; \
		echo '#define SYMTAB_DISCARD_GPL \'; \
		cat $(LINUXDIR)/sym_exclude.txt | \
			awk '{print "*(___ksymtab_gpl+" $$$$1 ") \\" }'; \
		echo; \
	) > $(LINUXDIR)/symtab.h
	#rm -f $(LINUXDIR)/vmlinux
	touch $(LINUXDIR)/include/generated/autoksyms.h
	make -j 4 -C $(LINUXDIR) zImage dtbs MAKE=make EXTRA_LDSFLAGS="-I$(LINUXDIR) -include symtab.h" CROSS_COMPILE="ccache $(ARCH)-openwrt-linux-"
