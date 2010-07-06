
OLSRD_PLUGIN =	true
PLUGIN_NAME =	olsrd_mdns
PLUGIN_VER =	1.0.0

TOPDIR = ../..
include $(TOPDIR)/Makefile.inc

LIBS +=	$(OS_LIB_PTHREAD)

# Must be specified along with -lpthread on linux
CPPFLAGS += $(OS_CFLAG_PTHREAD)

ifneq ($(OS),linux)

default_target install clean:
	@echo "*** mdns Plugin only supported on Linux, sorry!"

else

default_target: $(PLUGIN_FULLNAME)

$(PLUGIN_FULLNAME): $(OBJS) version-script.txt
		$(CC) $(LDFLAGS) -o $(PLUGIN_FULLNAME) $(OBJS) $(LIBS)

install:	$(PLUGIN_FULLNAME)
		$(STRIP) $(PLUGIN_FULLNAME)
		$(INSTALL_LIB)

clean:
		rm -f $(OBJS) $(SRCS:%.c=%.d) $(PLUGIN_FULLNAME)

endif
