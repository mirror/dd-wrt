ifeq ($(ARCH),i386)
NGINX_FLAGS += -DNGX_HAVE_LITTLE_ENDIAN -DNGX_PTR_SIZE=4
NGINX_CONF = ngx_auto_config.h
endif
ifeq ($(ARCH),x86_64)
NGINX_FLAGS += -DNGX_HAVE_LITTLE_ENDIAN -DNGX_PTR_SIZE=8
NGINX_CONF = ngx_auto_config_64.h
endif
ifeq ($(ARCH),mipsel)
NGINX_FLAGS += -DNGX_HAVE_LITTLE_ENDIAN -DNGX_PTR_SIZE=4
NGINX_CONF = ngx_auto_config.h
endif
ifeq ($(ARCH),mips)
NGINX_FLAGS += -DNGX_PTR_SIZE=4
NGINX_CONF = ngx_auto_config.h
endif
ifeq ($(ARCH),arm)
NGINX_FLAGS += -DNGX_HAVE_LITTLE_ENDIAN -DNGX_PTR_SIZE=4
NGINX_CONF = ngx_auto_config.h
endif
ifeq ($(ARCH),powerpc)
NGINX_FLAGS += -DNGX_PTR_SIZE=4
NGINX_CONF = ngx_auto_config.h
endif
ifeq ($(ARCH),mips64)
NGINX_FLAGS += -DNGX_PTR_SIZE=8
NGINX_CONF = ngx_auto_config_64.h
endif
ifeq ($(ARCH),armeb)
NGINX_FLAGS += -DNGX_PTR_SIZE=4
NGINX_CONF = ngx_auto_config.h
endif


nginx-configure:
	cd nginx && CC=gcc;CFLAGS=-O2 && ./configure --with-http_ssl_module --prefix=/usr --add-module=../nginx-rtmp-module
	cp nginx/objs/Makefile.use nginx/objs/Makefile

nginx-clean:
	$(MAKE) -C nginx clean 

nginx: openssl
	cp nginx/Makefile.use nginx/objs/Makefile
	cp nginx/$(NGINX_CONF) nginx/objs/$(NGINX_CONF)
	$(MAKE) -C nginx CFLAGS="$(NGINX_FLAGS) -D_GNU_SOURCE $(COPTS) $(MIPS16_OPT) -ffunction-sections -fdata-sections -Wl,--gc-sections -I../nginx-rtmp-module -I$(TOP)/pcre -I$(TOP)/openssl/include"
	#CFLAGS="$(COPTS) $(MIPS16_OPT) $(LTO) -ffunction-sections -fdata-sections -Wl,--gc-sections -I$(TOP)/openssl/include" LDFLAGS="$(COPTS) $(MIPS16_OPT) $(LDLTO) -ffunction-sections -fdata-sections -Wl,--gc-sections  -L$(TOP)/openssl"

nginx-install:
#	install -D nginx/src/nginx $(INSTALLDIR)/nginx/usr/sbin/nginx

