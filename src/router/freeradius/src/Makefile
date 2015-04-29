#
# Makefile
#
# Version:	$Id: b04e7b8cca65e4f37c96c79661a8a876a8aa339f $
#

include ../Make.inc

SUBDIRS		= include lib modules main
WHAT_TO_MAKE	= all

.PHONY: all clean install reconfig
all: freeradius-devel
	@$(MAKE) $(MFLAGS) WHAT_TO_MAKE=$@ common

clean:
	@$(MAKE) $(MFLAGS) WHAT_TO_MAKE=$@ common
	@rm -f include/*~ *~

install:
	@$(MAKE) $(MFLAGS) WHAT_TO_MAKE=$@ common

reconfig:
	@$(MAKE) $(MFLAGS) WHAT_TO_MAKE=$@ common

.PHONY: common $(SUBDIRS)

#
#  Ensure correct build order
lib: include

modules: lib

main: modules

common: $(SUBDIRS)

$(SUBDIRS): 
	@echo "Making $(WHAT_TO_MAKE) in $@..."
	$(MAKE) $(MFLAGS) -C $@ $(WHAT_TO_MAKE)

freeradius-devel:
	ln -sf include freeradius-devel
