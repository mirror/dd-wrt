
bin_SCRIPTS = mbim-network

mbim-network: mbim-network.in
	$(AM_V_GEN) sed -e s,@VERSION\@,$(VERSION), $< > $@.tmp && mv $@.tmp $@
	@chmod a+x $@

EXTRA_DIST = mbim-network.in

CLEANFILES = mbim-network
