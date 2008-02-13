# This is quite ugly but at least it works
ifeq ($(OS),linux)
SUBDIRS := $(shell find -maxdepth 2 -name Makefile -not -path ./Makefile -printf "%h\n")
else
ifeq ($(OS),win32)
SUBDIRS := bmf dot_draw httpinfo mini pgraph secure txtinfo
else
SUBDIRS := bmf dot_draw dyn_gw dyn_gw_plain httpinfo mini nameservice pgraph secure txtinfo
endif
endif

.PHONY: $(SUBDIRS)

all: $(SUBDIRS)
clean: $(SUBDIRS)
install: $(SUBDIRS)

$(SUBDIRS):
	$(MAKE) OS=$(OS) -C $@ $(MAKECMDGOALS)
