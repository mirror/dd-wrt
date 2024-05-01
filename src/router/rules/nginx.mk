ifeq ($(ARCH),i386)
NGINX_FLAGS += -DNGX_HAVE_LITTLE_ENDIAN -DNGX_PTR_SIZE=4
NGINX_CONF = ngx_auto_config.h
endif
ifeq ($(ARCH),x86_64)
NGINX_FLAGS += -DNGX_HAVE_LITTLE_ENDIAN -DNGX_PTR_SIZE=8
NGINX_CONF = ngx_auto_config_64.h
endif
ifeq ($(ARCH),aarch64)
NGINX_FLAGS += -DNGX_HAVE_LITTLE_ENDIAN -DNGX_PTR_SIZE=8
NGINX_CONF = ngx_auto_config_64.h
endif
ifeq ($(ARCH),mipsel)
NGINX_FLAGS += -DNGX_HAVE_LITTLE_ENDIAN -DNGX_PTR_SIZE=4 -DNGX_HAVE_LEVEL1_DCACHE_LINESIZE -DDCACHE_LINESIZE=32
NGINX_CONF = ngx_auto_config.h
endif
ifeq ($(ARCH),mips)
NGINX_FLAGS += -DNGX_PTR_SIZE=4 -DNGX_HAVE_LEVEL1_DCACHE_LINESIZE -DDCACHE_LINESIZE=32
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
NGINX_FLAGS += -DNGX_PTR_SIZE=8 -DNGX_HAVE_LEVEL1_DCACHE_LINESIZE -DDCACHE_LINESIZE=128
NGINX_CONF = ngx_auto_config_64.h
endif
ifeq ($(ARCH),armeb)
NGINX_FLAGS += -DNGX_PTR_SIZE=4 -DNGX_HAVE_LEVEL1_DCACHE_LINESIZE -DDCACHE_LINESIZE=32
NGINX_CONF = ngx_auto_config.h
endif


nginx-configure:
	cd nginx && CC=gcc;CFLAGS=-O2 && ./configure --with-http_v2_module --with-threads --with-http_ssl_module --with-http_mp4_module --add-module=../nginx-rtmp-module
#	cp nginx/Makefile.use nginx/objs/Makefile

nginx-clean:
	find $(TOP)/nginx/objs -name *.o -delete
#	$(MAKE) -C nginx clean 

nginx: openssl
	cp nginx/Makefile.use nginx/objs/Makefile
	cp nginx/$(NGINX_CONF) nginx/objs/ngx_auto_config.h
	$(MAKE) -C nginx CFLAGS="$(NGINX_FLAGS) -DNEED_PRINTF $(LTO) -D_GNU_SOURCE $(COPTS) $(MIPS16_OPT) -ffunction-sections -fdata-sections -Wl,--gc-sections -I../nginx-rtmp-module -I$(TOP)/zlib  -I$(TOP)/pcre -I$(TOP)/openssl/include"

nginx-install:
	$(MAKE) -C nginx install DESTDIR=$(INSTALLDIR)/nginx
