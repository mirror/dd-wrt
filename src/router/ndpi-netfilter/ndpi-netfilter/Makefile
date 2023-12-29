PROTO_CFG_FILES = $(wildcard ../src/lib/ndpi_network_list_*.yaml)


all:
#	gcc -DNDPI_STATIC= -I ../src/include -I ../src/lib/third_party/include -o ../src/lib/ndpi_network_list_compile ../src/lib/ndpi_network_list_compile.c
	#../src/lib/ndpi_network_list_compile -o ../src/lib/ndpi_network_list.c.inc $(PROTO_CFG_FILES)
#	$(MAKE) -C ipt
	+$(MAKE) -C src

modules_install:
	$(MAKE) -C src modules_install
install:
	$(MAKE) -C ipt install
clean:
	$(MAKE) -C src clean
#	$(MAKE) -C ipt clean

distclean:
	$(MAKE) clean
	$(MAKE) -C src distclean
