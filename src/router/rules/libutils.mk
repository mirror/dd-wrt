ifeq ($(CONFIG_SAMBA3),y)
	JANSSON=jansson
endif
ifeq ($(CONFIG_FTP),y)
	JANSSON=jansson
endif
ifeq ($(CONFIG_MINIDLNA),y)
	JANSSON=jansson
endif


libutils-clean:
	make -C libutils clean

libutils: nvram libnltiny  $(JANSSON)
	make -C libutils

libutils-install:
	make -C libutils install

