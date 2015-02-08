NMEALIB_PATH = nmealib
LIBRARY_PATH = wireformat
JAVA_PATH = wireformat-java
include $(LIBRARY_PATH)/Makefile.inc
NMEALIB_NAME := nmea
LIBRARY_NAME := $(PROJECT)


include Makefile.inc

TOPDIR = ../..
include $(TOPDIR)/Makefile.inc


PUD_NMEALIB_STATICALLY_LINKED ?=
CFLAGS += -D_GNU_SOURCE


RESOURCESDIR = ./resources
NMEALIB_INC = $(NMEALIB_PATH)/include
NMEALIB_LIB = $(NMEALIB_PATH)/lib
LIBRARY_INC = $(LIBRARY_PATH)/include
LIBRARY_LIB = $(LIBRARY_PATH)/lib
NMEALIB_LIB_STATIC = $(NMEALIB_LIB)/lib$(NMEALIB_NAME).a

CFLAGS += -I $(NMEALIB_INC) -I $(LIBRARY_INC)
ifeq ($(PUD_NMEALIB_STATICALLY_LINKED),)
LIBS += -L $(NMEALIB_LIB) -L $(LIBRARY_LIB) -l$(NMEALIB_NAME) -lm -l$(LIBRARY_NAME)
else
# libnmea is statically linked
LIBS += -L $(LIBRARY_LIB) -lm -l$(LIBRARY_NAME)
endif


SUPPORTED = 0
ifeq ($(OS),linux)
SUPPORTED = 1
endif
ifeq ($(OS),android)
SUPPORTED = 1
endif


ifeq ($(SUPPORTED),0)

.PHONY: all default_target install clean

all: default_target

default_target install clean:
	@echo "*** $(PLUGIN_NAME) plugin not supported on $(OS), sorry!"

else

GIT_SHA = $(shell git describe 2> /dev/null || echo "Unknown")
CFLAGS += -DGIT_SHA=\"$(GIT_SHA)\" -DPLUGIN_VER=\"$(PLUGIN_VER)\"

.PHONY: all default_target install uninstall clean doc doc-clean nmealib library java java-instal java-uninstall

all: default_target

ifeq ($(OS),android)
# On Android Google forgot to include regex engine code for Froyo version (but also there was
# no support in older versions for it) so we have here this missing code.
# http://groups.google.com/group/android-ndk/browse_thread/thread/5ea6f0650f0e3fc
OBJS +=     $(REGEX_LIB)
CFLAGS += -D__POSIX_VISIBLE
endif

default_target: nmealib library $(PLUGIN_FULLNAME)

# repeat from toplevel Makefile.inc, and adjust: we need the library includes
%.d: %.c
	$(MAKECMDPREFIX)$(filter-out $(CCACHE),$(CC)) -M $(strip $(CPPFLAGS)) -I $(NMEALIB_INC) -I $(LIBRARY_INC) "$<" | sed -e '1s/\($(call quote,$(*F))\.o\)[ :]*/$(call quote,$(*D)/\1 $@: Makefile $(TOPDIR)$(if $(TOPDIR),/)Makefile.inc) /g' >"$@"

$(PLUGIN_FULLNAME): $(OBJS) version-script.txt
ifeq ($(PUD_NMEALIB_STATICALLY_LINKED),)
ifeq ($(VERBOSE),0)
	@echo "[LD] $@ (nmealib dynamically linked)"
endif
	$(MAKECMDPREFIX)$(CC) $(LDFLAGS) -o $(PLUGIN_FULLNAME) $(OBJS) $(LIBS)
else
ifeq ($(VERBOSE),0)
	@echo "[LD] $@ (nmealib statically linked)"
endif
	$(MAKECMDPREFIX)$(CC) $(LDFLAGS) -o $(PLUGIN_FULLNAME) $(OBJS) $(NMEALIB_LIB_STATIC) $(LIBS)
endif

install: all
	$(MAKECMDPREFIX)$(MAKE) -C "$(NMEALIB_PATH)" DESTDIR="$(DESTDIR)" install
	$(MAKECMDPREFIX)$(MAKE) -C "$(LIBRARY_PATH)" DESTDIR="$(DESTDIR)" install
	$(INSTALL_LIB)
	mkdir -p "$(ETCDIR)"
	cp "$(RESOURCESDIR)/olsrd.pud.position.conf" "$(ETCDIR)"
	$(STRIP) "$(LIBDIR)/$(PLUGIN_FULLNAME)"

uninstall:
	$(MAKECMDPREFIX)$(MAKE) -C "$(NMEALIB_PATH)" DESTDIR="$(DESTDIR)" uninstall
	$(MAKECMDPREFIX)$(MAKE) -C "$(LIBRARY_PATH)" DESTDIR="$(DESTDIR)" uninstall
	rm -f "$(LIBDIR)/lib$(PLUGIN_NAME).so" "$(LIBDIR)/$(PLUGIN_NAME)" "$(ETCDIR)/olsrd.pud.position.conf"
	$(UNINSTALL_LIB)
	rmdir -v -p --ignore-fail-on-non-empty "$(LIBDIR)" "$(ETCDIR)"

clean:
ifeq ($(VERBOSE),0)
	@echo "[$@]"
endif
	$(MAKECMDPREFIX)rm -f $(OBJS) $(SRCS:%.c=%.d) "$(PLUGIN_FULLNAME)"
	$(MAKECMDPREFIX)$(MAKE) -C doc clean
	$(MAKECMDPREFIX)$(MAKE) -C "$(NMEALIB_PATH)" clean
	$(MAKECMDPREFIX)$(MAKE) -C "$(LIBRARY_PATH)" clean
	$(MAKECMDPREFIX)$(MAKE) -C "$(JAVA_PATH)" clean

doc:
	$(MAKECMDPREFIX)$(MAKE) -C doc all
	$(MAKECMDPREFIX)$(MAKE) -C "$(NMEALIB_PATH)" doc
	$(MAKECMDPREFIX)$(MAKE) -C "$(LIBRARY_PATH)" doc
	$(MAKECMDPREFIX)$(MAKE) -C "$(JAVA_PATH)" doc

doc-clean:
	$(MAKECMDPREFIX)$(MAKE) -C doc clean
	$(MAKECMDPREFIX)$(MAKE) -C "$(NMEALIB_PATH)" doc clean
	$(MAKECMDPREFIX)$(MAKE) -C "$(LIBRARY_PATH)" doc clean
	$(MAKECMDPREFIX)$(MAKE) -C "$(JAVA_PATH)" doc clean

nmealib:
	$(MAKECMDPREFIX)$(MAKE) -C "$(NMEALIB_PATH)" all

library:
	$(MAKECMDPREFIX)$(MAKE) -C "$(LIBRARY_PATH)" all

java:	library
	$(MAKECMDPREFIX)$(MAKE) -C "$(JAVA_PATH)" all

java-install: java
	$(MAKECMDPREFIX)$(MAKE) -C "$(JAVA_PATH)" DESTDIR="$(DESTDIR)" install

java-uninstall:
	$(MAKECMDPREFIX)$(MAKE) -C "$(JAVA_PATH)" DESTDIR="$(DESTDIR)" uninstall

endif
