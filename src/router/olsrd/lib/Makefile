SUBDIRS = dot_draw dyn_gw httpinfo mini nameservice powerinfo secure tas

.PHONY: $(SUBDIRS)

all: $(SUBDIRS)
clean: $(SUBDIRS)
install: $(SUBDIRS)

$(SUBDIRS):
	$(MAKE) -C $@ $(MAKECMDGOALS)
