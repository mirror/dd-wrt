
#############################################
#          OLSR.ORG SECURE PLUGIN           #
#############################################


CC ?= gcc
STRIP ?= strip
PLUGINFLAGS = -fPIC -DOLSR_PLUGIN -I../../src

ifdef USE_OPENSSL
PLUGINFLAGS += -DUSE_OPENSSL
endif

ifdef DEBUG
PLUGINFLAGS += -DDEBUG
endif

ifeq ($(OS), linux)
NAME ?= olsrd_secure.so.0.5
LIBDIR ?= $(INSTALL_PREFIX)/usr/lib
LIBS ?= -lc -lm
ifdef USE_OPENSSL
LIBS += -lssl -lcrypto
endif
LDFLAGS += -fPIC
CFLAGS ?= -Wall -Wmissing-prototypes -Wstrict-prototypes -g -O2
INSTALL_LIB = install -D -m 755 $(NAME) $(LIBDIR)/$(NAME);\
	/sbin/ldconfig -n $(LIBDIR)

all:	plugin
install:install_olsrd
else
ifeq ($(OS), fbsd)
NAME ?= olsrd_secure.so.0.5
LIBDIR ?= $(INSTALL_PREFIX)/usr/lib
LIBS ?= -lc -lm
ifdef USE_OPENSSL
LIBS += -lssl -lcrypto
endif
LDFLAGS += -fPIC
CFLAGS ?= -Wall -Wmissing-prototypes -Wstrict-prototypes -g -O2
INSTALL_LIB = install -m 755 $(NAME) $(LIBDIR)/$(NAME);\
	/sbin/ldconfig

all:	plugin
install:install_olsrd
else
ifeq ($(OS), win32)
NAME ?= olsrd_secure.dll
CFLAGS ?= -O2 -g -DWIN32 -mno-cygwin -I../../src/win32 -Wall -Wmissing-prototypes -Wstrict-prototypes
LDFLAGS ?= -fPIC -mno-cygwin -shared
LIBS ?= -lws2_32
ifdef USE_OPENSSL
LIBS += -lssl -lcrypto
endif
INSTALL_LIB = 	cp $(NAME) ../..
all:    plugin
install:install_olsrd
else
all: 	help
install:help
endif
endif
endif

CFLAGS +=  $(PLUGINFLAGS)

#Sourcefiles
SRCS=	src/olsrd_plugin.c src/olsrd_secure.c

#Objectfiles
OBJS=	src/olsrd_plugin.o src/olsrd_secure.o

#Headerfiles
HDRS=	src/olsrd_plugin.h src/olsrd_secure.h

ifndef USE_OPENSSL
#Sourcefiles
SRCS+=	src/md5.c

#Objectfiles
OBJS+=	src/md5.o

#Headerfiles
HDRS+=	src/md5.h

endif

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
	@echo '**** olsr.org secure plugin Make ***'
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

src/olsrd_secure.o: $(HDRS)
