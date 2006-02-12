
CC ?= gcc
STRIP ?= strip
LIBDIR ?= $(INSTALL_PREFIX)/usr/lib
PLUGINFLAGS = -fPIC -DOLSR_PLUGIN -I../../src

ifeq ($(OS), linux)
NAME ?= olsrd_dyn_gw.so.0.3
MYFLAGS ?= -fPIC -g # Uncomment -g for debugging
LIBS ?= -lpthread -lc -lm
CFLAGS ?= -Wall -Wmissing-prototypes -Wstrict-prototypes -g -O2
INSTALL_LIB = install -D -m 755 $(NAME) $(LIBDIR)/$(NAME);\
	      /sbin/ldconfig -n $(LIBDIR)
all:	plugin
install:install_olsrd
else
ifeq ($(OS), fbsd)
NAME ?= olsrd_dyn_gw.so.0.3
MYFLAGS ?= -fPIC -g # Uncomment -g for debugging
LIBS ?= -lpthread -lc -lm
CFLAGS ?= -Wall -Wmissing-prototypes -Wstrict-prototypes -g -O2 
INSTALL_LIB = install -m 755 $(NAME) $(LIBDIR)/$(NAME);\
	      /sbin/ldconfig
all:	plugin
install:install_olsrd
else
ifeq ($(OS), win32)
NAME ?= olsrd_dyn_gw.dll
NAMEFLAGS ?= -o $(NAME)
CFLAGS ?= -g -O2 -DWIN32 -mno-cygwin -I../../src/win32 -Wall -Wmissing-prototypes -Wstrict-prototypes 
LDFLAGS ?= -mno-cygwin -shared
LIBS ?= -lws2_32
COMPATOBJ = ../../src/win32/compat.o
INSTALL_LIB = 	cp $(NAME) ../..
all:	plugin
install:install_olsrd
else
all: 	help
install:help
endif
endif
endif

CFLAGS += $(PLUGINFLAGS)

#Sourcefiles
SRCS=	src/olsrd_plugin.c src/olsrd_dyn_gw.c

#Objectfiles
OBJS=	src/olsrd_plugin.o src/olsrd_dyn_gw.o

#Headerfiles
HDRS=	src/olsrd_plugin.h src/olsrd_dyn_gw.h

plugin: $(OBJS)
	$(CC) $(LDFLAGS) $(MYFLAGS) -shared -Wl,-soname,$(NAME) \
	-Wl,--version-script=version-script.txt \
	-o $(NAME) $(OBJS) $(LIBS)


install_olsrd:
	$(STRIP) $(NAME)
	$(INSTALL_LIB)
clean:
	rm -f $(OBJS) $(NAME)

help:
	@echo
	@echo '**** olsr.org dynGW plugin Make ****'
	@echo ' You must provide a valid target OS '
	@echo ' by setting the OS variable! Valid  '
	@echo ' target OSes are:                   '
	@echo ' ---------------------------------  '
	@echo ' linux - GNU/Linux                  '
	@echo ' fbsd  - FreeBSD                    '
	@echo ' win32 - Windows                    '
	@echo ' ---------------------------------  '
	@echo ' Example - build for FreeBSD:       '
	@echo ' gmake OS=fbsd                      '
	@echo '************************************'
	@echo

src/olsrd_plugin.o: $(HDRS)

src/olsrd_dyn_gw.o: $(HDRS)
