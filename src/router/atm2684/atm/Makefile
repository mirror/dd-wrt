# "qgen" and "saal" _must_ appear before "sigd"
# "lib" must appear before anything else
# "maint" must appear after "qgen"

DIRS=lib arpd
#DIRS=lib test debug qgen saal sigd maint arpd ilmid man led lane \
#  mpoad switch # extra

all:
		for n in $(DIRS); do $(MAKE) -C $$n || exit; done

install:
		for n in $(DIRS); do \
		  $(MAKE) -C $$n instdirs || exit; \
		  $(MAKE) -C $$n install || exit; done

uninstall:
		for n in $(DIRS); do $(MAKE) -C $$n uninstall || exit; done

filenames:
		@for n in $(DIRS); do $(MAKE) -s -C $$n filenames || exit; done

depend:
		for n in $(DIRS); do $(MAKE) -C $$n depend || exit; done

clean:
		rm -f core
		for n in $(DIRS); do $(MAKE) -C $$n clean || exit; done

spotless:	clean
		for n in $(DIRS); do $(MAKE) -C $$n spotless || exit; done
