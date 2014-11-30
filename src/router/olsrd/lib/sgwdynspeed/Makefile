include Makefile.inc

TOPDIR = ../..
include $(TOPDIR)/Makefile.inc


RESOURCESDIR = ./resources

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

.PHONY: all default_target install uninstall clean doc doc-clean java java-instal java-uninstall

all: default_target

default_target: $(PLUGIN_FULLNAME)

$(PLUGIN_FULLNAME): $(OBJS) version-script.txt
ifeq ($(VERBOSE),0)
	@echo "[LD] $@"
endif
	$(MAKECMDPREFIX)$(CC) $(LDFLAGS) -o $(PLUGIN_FULLNAME) $(OBJS) $(LIBS)

install: all
	$(INSTALL_LIB)
	mkdir -p "$(ETCDIR)"
	cp "$(RESOURCESDIR)/olsrd.sgw.speed.conf" "$(ETCDIR)"
	$(STRIP) "$(LIBDIR)/$(PLUGIN_FULLNAME)"

uninstall:
	rm -f "$(LIBDIR)/lib$(PLUGIN_NAME).so" "$(LIBDIR)/$(PLUGIN_NAME)" "$(ETCDIR)/olsrd.sgw.speed.conf"
	$(UNINSTALL_LIB)
	rmdir -v -p --ignore-fail-on-non-empty "$(LIBDIR)" "$(ETCDIR)"

clean:
ifeq ($(VERBOSE),0)
	@echo "[$@]"
endif
	$(MAKECMDPREFIX)rm -f $(OBJS) $(SRCS:%.c=%.d) "$(PLUGIN_FULLNAME)"
	$(MAKECMDPREFIX)$(MAKE) -C doc clean

doc:
	$(MAKECMDPREFIX)$(MAKE) -C doc all

doc-clean:
	$(MAKECMDPREFIX)$(MAKE) -C doc clean

endif
