
#############################################
# EXAMPLE MAKEFILE TO CREATE A OLSRD PLUGIN #
#############################################

#Alter this file to fit your needs

CC ?= gcc
STRIP ?= strip
PLUGINFLAGS =  -fPIC -DOLSR_PLUGIN -I../../src

ifeq ($(OS), linux)
NAME ?= olsrd_power.so.0.3
LIBDIR ?= $(INSTALL_PREFIX)/usr/lib
# -fPIC creates position independent code
LIBS ?= -lc -lm
CFLAGS ?= -Wall -g -O2 -Wmissing-prototypes -Wstrict-prototypes
LDFLAGS ?= -fPIC
INSTALL_LIB = install -D -m 755 $(NAME) $(LIBDIR)/$(NAME);\
	/sbin/ldconfig -n $(LIBDIR)
all:	plugin
install:install_olsrd
else
ifeq ($(OS), fbsd)
NAME ?= olsrd_power.so.0.3
LIBDIR ?= $(INSTALL_PREFIX)/usr/lib
# -fPIC creates position independent code
LIBS ?= -lc -lm
CFLAGS ?= -Wall -g -O2 -Wmissing-prototypes -Wstrict-prototypes
LDFLAGS ?= -fPIC
INSTALL_LIB = install -m 755 $(NAME) $(LIBDIR)/$(NAME);\
	/sbin/ldconfig
all:	plugin
install:install_olsrd
else
ifeq ($(OS), win32)
NAME ?= olsrd_power.dll
CFLAGS ?= -O2 -g -DWIN32 -mno-cygwin -I../../src/win32 -Wall -Wmissing-prototypes -Wstrict-prototypes $(PLUGINFLAGS)
LDFLAGS ?= -mno-cygwin -shared -fPIC
LIBS ?= -lws2_32
INSTALL_LIB = 	cp $(NAME) ../..
else
all: 	help
install:help
endif
endif
endif

CFLAGS +=  $(PLUGINFLAGS)

#Sourcefiles
#add yours here
SRCS=	src/olsrd_plugin.c src/olsrd_power.c

#Objectfiles
#add yours here
OBJS=	src/olsrd_plugin.o src/olsrd_power.o

#Headerfiles
#add yours here
HDRS=	src/olsrd_plugin.h src/olsrd_power.h src/olsr_plugin_io.h

# -Wl passes options to the linker(-soname,....)
# Use -lc to link it against C library
# not 100% sure if this is neccesary

plugin: $(OBJS)
	$(CC) $(LDFLAGS) -shared -Wl,-soname,$(NAME) \
	-Wl,--version-script=version-script.txt \
	-o $(NAME) $(OBJS) $(LIBS)


install_olsrd:
	$(STRIP) $(NAME)
	$(INSTALL_LIB)

clean:
	rm -f $(OBJS) $(NAME)


help:
	@echo
	@echo '******* olsr.org powerplugin *******'
	@echo ' You must provide a valid target OS '
	@echo ' by setting the OS variable! Valid  '
	@echo ' target OSes are:                   '
	@echo ' ---------------------------------  '
	@echo ' linux - GNU/Linux                  '
	@echo ' win32 - MS Windows                 '
	@echo ' fbsd  - FreeBSD                    '
	@echo ' ---------------------------------  '
	@echo ' Example - build for windows:       '
	@echo ' make OS=win32                      '
	@echo '************************************'
	@echo

src/olsrd_plugin.o: $(HDRS)

src/olsrd_power.o: $(HDRS)

