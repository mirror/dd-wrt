all:
	$(MAKE) -C apfsck
	$(MAKE) -C mkapfs
	$(MAKE) -C apfs-snap
	$(MAKE) -C apfs-label


clean:
	$(MAKE) -C apfsck clean
	$(MAKE) -C mkapfs clean
	$(MAKE) -C apfs-snap clean
	$(MAKE) -C apfs-label clean
	$(MAKE) -C lib clean


install:
	$(MAKE) -C apfsck install
	$(MAKE) -C mkapfs install
	$(MAKE) -C apfs-snap install
	$(MAKE) -C apfs-label install
