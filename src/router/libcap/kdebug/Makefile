topdir=$(shell pwd)/..
include ../Make.Rules

test:
	./test-kernel.sh

all:
	@echo cd to kdebug to test a kernel build

install:

clean:
	$(LOCALCLEAN)
	rm -f fs.conf initramfs.img
