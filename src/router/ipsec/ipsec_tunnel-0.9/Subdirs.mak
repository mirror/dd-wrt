.PHONY: all
all:
	@for d in $(SUBDIRS); do $(MAKE) -C $$d $@ || exit $$?; done

.PHONY: install
install:
	@for d in $(SUBDIRS); do $(MAKE) -C $$d $@ || exit $$?; done

.PHONY: clean
clean:
	@for d in $(SUBDIRS); do $(MAKE) -C $$d $@ || exit $$?; done

.PHONY: depend
depend:
	@for d in $(SUBDIRS); do $(MAKE) -C $$d $@ || exit $$?; done
