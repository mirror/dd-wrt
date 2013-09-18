
OLSRD_PLUGIN =	true
PLUGIN_NAME =	olsrd_p2pd
PLUGIN_VER =	0.1.0

TOPDIR = ../..
include $(TOPDIR)/Makefile.inc

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

default_target: $(PLUGIN_FULLNAME)

$(PLUGIN_FULLNAME): $(OBJS) version-script.txt
ifeq ($(VERBOSE),0)
		@echo "[LD] $@"
endif
		$(MAKECMDPREFIX)$(CC) $(LDFLAGS) -o $(PLUGIN_FULLNAME) $(OBJS) $(LIBS)

install:	$(PLUGIN_FULLNAME)
		$(STRIP) $(PLUGIN_FULLNAME)
		$(INSTALL_LIB)

uninstall:
		$(UNINSTALL_LIB)

clean:
		rm -f $(OBJS) $(SRCS:%.c=%.d) $(PLUGIN_FULLNAME)

endif
