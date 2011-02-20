IFX_DSL_MAX_DEVICE=1
IFX_DSL_LINES_PER_DEVICE=1
IFX_DSL_CHANNELS_PER_LINE=1


dsl_cpe_control-configure:
	cd dsl_cpe_control && ./configure --target=$(ARCH)-linux --host=$(ARCH)-linux --prefix=/usr CFLAGS="$(COPTS) -DNEED_PRINTF" CPPFLAGS="$(COPTS) -DNEED_PRINTF" CXXFLAGS="$(COPTS) -DNEED_PRINTF" \
	--with-max-device="$(IFX_DSL_MAX_DEVICE)" \
	--with-lines-per-device="$(IFX_DSL_LINES_PER_DEVICE)" \
	--with-channels-per-line="$(IFX_DSL_CHANNELS_PER_LINE)" \
	--enable-danube \
	--enable-driver-include="-I$(TOP)/dsl_cpe_control/include" \
	--enable-debug-prints \
	--enable-add-appl-cflags="-DMAX_CLI_PIPES=2" \
	--enable-cmv-scripts \
	--enable-debug-tool-interface \
	--enable-adsl-led \
	--enable-dsl-ceoc \
	--enable-script-notification \
	--enable-dsl-pm \
	--enable-dsl-pm-total \
	--enable-dsl-pm-history \
	--enable-dsl-pm-showtime \
	--enable-dsl-pm-channel-counters \
	--enable-dsl-pm-datapath-counters \
	--enable-dsl-pm-line-counters \
	--enable-dsl-pm-channel-thresholds \
	--enable-dsl-pm-datapath-thresholds \
	--enable-dsl-pm-line-thresholds \
	--enable-dsl-pm-optional-parameters \
	--enable-cli-support \
	--enable-soap-support


dsl_cpe_control:
	make -C dsl_cpe_control

dsl_cpe_control-install:
	install -D dsl_cpe_control/src/dsl_cpe_control $(INSTALLDIR)/dsl_cpe_control/usr/sbin/dsl_cpe_control


ifneq ($(CONFIG_AR9),y)
ifneq ($(CONFIG_ANNEXA),y)
ifneq ($(CONFIG_ANNEXB),y)
	install -D dsl_cpe_control/annex_a_danube.bin $(INSTALLDIR)/dsl_cpe_control/usr/lib/firmware/annex_a.bin
	install -D dsl_cpe_control/annex_b_danube.bin $(INSTALLDIR)/dsl_cpe_control/usr/lib/firmware/annex_b.bin
endif
endif
ifeq ($(CONFIG_ANNEXA),y)
	install -D dsl_cpe_control/annex_a_danube.bin $(INSTALLDIR)/dsl_cpe_control/usr/lib/firmware/annex_a.bin
endif
ifeq ($(CONFIG_ANNEXB),y)
	install -D dsl_cpe_control/annex_b_danube.bin $(INSTALLDIR)/dsl_cpe_control/usr/lib/firmware/annex_b.bin
endif
else
ifneq ($(CONFIG_ANNEXA),y)
ifneq ($(CONFIG_ANNEXB),y)
	install -D dsl_cpe_control/annex_a_ar9.bin $(INSTALLDIR)/dsl_cpe_control/usr/lib/firmware/annex_a.bin
	install -D dsl_cpe_control/annex_b_ar9.bin $(INSTALLDIR)/dsl_cpe_control/usr/lib/firmware/annex_b.bin
endif
endif
ifeq ($(CONFIG_ANNEXA),y)
	install -D dsl_cpe_control/annex_a_ar9.bin $(INSTALLDIR)/dsl_cpe_control/usr/lib/firmware/annex_a.bin
endif
ifeq ($(CONFIG_ANNEXB),y)
	install -D dsl_cpe_control/annex_b_ar9.bin $(INSTALLDIR)/dsl_cpe_control/usr/lib/firmware/annex_b.bin
endif


endif
#	make  -C squid install DESTDIR=$(INSTALLDIR)/squid	

