softether-configure:
	@true

softether:
	$(MAKE) -C softether clean
	$(MAKE) -C softether CC=gcc FLAGS="-O2 -DCPU_64" src/bin/BuiltHamcoreFiles/unix/hamcore.se2
	$(MAKE) -C softether clean
	$(MAKE) -C softether FLAGS="$(MIPS16_OPT) -I$(TOP)/quagga -L$(TOP)/quagga/readline -I$(TOP)/openssl/include -L$(TOP)/openssl -I$(TOP)/zlib -L$(TOP)/zlib -I$(TOP)/ncurses/include -L$(TOP)/ncurses/lib"

softether-clean:
	$(MAKE) -C softether clean

softether-install:
	$(MAKE) -C softether install
