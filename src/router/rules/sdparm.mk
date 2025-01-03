sdparm-configure:
	cd sdparm && ./autogen.sh && ./configure --host=$(ARCH)-uclibc-linux \
	CFLAGS="$(COPTS) $(LTO) $(MIPS16_OPT) -I$(TOP)/kernel_headers/$(KERNELRELEASE)/include -D_GNU_SOURCE -DNEED_PRINTF -Drpl_malloc=malloc -ffunction-sections -fdata-sections -Wl,--gc-sections" \
	LDFLAGS="$(LDLTO) -ffunction-sections -fdata-sections -Wl,--gc-sections" \

sdparm:
	$(MAKE) -C sdparm

sdparm-install:
	install -D sdparm/src/sdparm $(INSTALLDIR)/sdparm/usr/sbin/sdparm
	
sdparm-clean:
	$(MAKE) -C sdparm clean