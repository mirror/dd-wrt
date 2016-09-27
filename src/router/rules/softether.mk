softether-configure:
	@true

softether: zlib quagga ncurses
	-rm -f softether/bin/vpnserver/hamcore.se2
	-rm -f softether/bin/vpnclient/hamcore.se2
	-rm -f softether/bin/vpnbridge/hamcore.se2
	-rm -f softether/bin/vpncmd/hamcore.se2
	-rm -f softether/tmp/hamcorebuilder
	-rm -f softether/src/bin/BuiltHamcoreFiles/unix/hamcore.se2
	$(MAKE) -C softether clean
	$(MAKE) -C softether CC="ccache gcc" FLAGS="-O2 -DCPU_64" src/bin/BuiltHamcoreFiles/unix/hamcore.se2
	$(MAKE) -C softether clean
	$(MAKE) -C softether FLAGS="$(COPTS) $(MIPS16_OPT) -I$(TOP)/quagga -L$(TOP)/quagga/readline -I$(TOP)/openssl/include -L$(TOP)/openssl -I$(TOP)/zlib -L$(TOP)/zlib -I$(TOP)/ncurses/include -L$(TOP)/ncurses/lib  -ffunction-sections -fdata-sections -Wl,--gc-sections"

softether-clean:
	$(MAKE) -C softether clean

softether-install:
	$(MAKE) -C softether install
