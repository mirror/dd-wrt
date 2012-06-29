NMEALIB_PATH = nmealib
LIBRARY_PATH = wireformat
JAVA_PATH = wireformat-java
include $(LIBRARY_PATH)/Makefile.inc
NMEALIB_NAME := nmea
LIBRARY_NAME := $(PROJECT)


include Makefile.inc

TOPDIR = ../..
include $(TOPDIR)/Makefile.inc


CFLAGS += -Werror -D_GNU_SOURCE


RESOURCESDIR = ./resources
NMEALIB_INC = $(NMEALIB_PATH)/include
NMEALIB_LIB = $(NMEALIB_PATH)/lib
LIBRARY_INC = $(LIBRARY_PATH)/include
LIBRARY_LIB = $(LIBRARY_PATH)/lib

CFLAGS += -I $(NMEALIB_INC) -I $(LIBRARY_INC)
LIBS += -L $(NMEALIB_LIB) -L $(LIBRARY_LIB) -l$(NMEALIB_NAME) -lm -l$(LIBRARY_NAME)

ifneq ($(OS),linux)

.PHONY: all default_target install clean

all: default_target

default_target install clean:
	@echo "*** $(PLUGIN_NAME) plugin only supported on Linux, sorry!"

else

GIT_SHA = $(shell git describe 2> /dev/null || echo "Unknown")
CFLAGS += -DGIT_SHA=\"$(GIT_SHA)\" -DPLUGIN_VER=\"$(PLUGIN_VER)\"

.PHONY: all default_target install uninstall clean doc doc-clean nmealib library java java-instal java-uninstall

all: default_target

default_target: nmealib library $(PLUGIN_FULLNAME)

# repeat from toplevel Makefile.inc, and adjust: we need the library includes
%.d: %.c
	@$(filter-out $(CCACHE),$(CC)) -M $(strip $(CPPFLAGS)) -I $(NMEALIB_INC) -I $(LIBRARY_INC) "$<" | sed -e '1s/\($(call quote,$(*F))\.o\)[ :]*/$(call quote,$(*D)/\1 $@: Makefile $(TOPDIR)$(if $(TOPDIR),/)Makefile.inc) /g' >"$@"

$(PLUGIN_FULLNAME): $(OBJS) version-script.txt
	@echo "[LD] $@"
	@$(CC) $(LDFLAGS) -o $(PLUGIN_FULLNAME) $(OBJS) $(LIBS)

install: all
	@$(MAKE) -C $(NMEALIB_PATH) DESTDIR=$(DESTDIR) install
	@$(MAKE) -C $(LIBRARY_PATH) DESTDIR=$(DESTDIR) install
	$(INSTALL_LIB)
	mkdir -p "$(ETCDIR)"
	cp "$(RESOURCESDIR)/olsrd.pud.position.conf" "$(ETCDIR)"
	$(STRIP) "$(LIBDIR)/$(PLUGIN_FULLNAME)"

uninstall:
	@$(MAKE) -C $(NMEALIB_PATH) DESTDIR=$(DESTDIR) uninstall
	@$(MAKE) -C $(LIBRARY_PATH) DESTDIR=$(DESTDIR) uninstall
	rm -f "$(LIBDIR)/lib$(PLUGIN_NAME).so" "$(LIBDIR)/$(PLUGIN_NAME)" "$(ETCDIR)/olsrd.pud.position.conf"
	$(UNINSTALL_LIB)
	rmdir -v -p --ignore-fail-on-non-empty "$(LIBDIR)" "$(ETCDIR)"

clean:
	@echo "[$@]"
	@rm -f $(OBJS) $(SRCS:%.c=%.d) "$(PLUGIN_FULLNAME)"
	@$(MAKE) -C doc clean
	@$(MAKE) -C $(NMEALIB_PATH) clean
	@$(MAKE) -C $(LIBRARY_PATH) clean
	@$(MAKE) -C $(JAVA_PATH) clean

doc:
	@$(MAKE) -C doc all
	@$(MAKE) -C $(NMEALIB_PATH) doc
	@$(MAKE) -C $(LIBRARY_PATH) doc
	@$(MAKE) -C $(JAVA_PATH) doc

doc-clean:
	@$(MAKE) -C doc clean
	@$(MAKE) -C $(NMEALIB_PATH) doc clean
	@$(MAKE) -C $(LIBRARY_PATH) doc clean
	@$(MAKE) -C $(JAVA_PATH) doc clean

nmealib:
	@$(MAKE) -C $(NMEALIB_PATH) all

library:
	@$(MAKE) -C $(LIBRARY_PATH) all

java:
	@$(MAKE) -C $(JAVA_PATH) all

java-install: java
	@$(MAKE) -C $(JAVA_PATH) DESTDIR=$(DESTDIR) install

java-uninstall:
	@$(MAKE) -C $(JAVA_PATH) DESTDIR=$(DESTDIR) uninstall

endif
