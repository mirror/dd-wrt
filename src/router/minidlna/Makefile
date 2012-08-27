############################################################################################
#
#	Makefile For MiniDLNA
#
#	Author: Michael Jiang
#	Date: 2010.05.01
#	
#	Description:
#		1. I don't re-configure ffmpeg, jpeg-7, zlib & minidlna , because their Makefiles 
#		   have been modified by me. Others will be re-configured again if no Makefiles; 
#		2. It should be compiled in Linux2.6 platform only.
#		3. There are two variables in Makefile which you should take care about. one
#		   is $HOST_COMPILE and the other is $HOST. I can't refer to them from ap/config.in,
#		   so I make a hard code here. When you want to do a portation please take care
#		   them.   
#
#############################################################################################
SUBDIRS += zlib-1.2.3
SUBDIRS += ffmpeg-0.5.1
SUBDIRS += libogg-1.1.4
SUBDIRS += libexif-0.6.19
SUBDIRS += flac-1.2.1
SUBDIRS += libvorbis-1.2.3
SUBDIRS += jpeg-7
SUBDIRS += sqlite-3.6.22
SUBDIRS += libid3tag-0.15.0b
#/projects/hnd/tools/linux/hndtools-mipsel-linux-uclibc-4.2.3
TOOL_CHAIN_PATH=$(TOOLCHAIN)
#/home/michael/DLNA
MINI_DLNA_PATH=$(shell pwd)

CFLAGS += $(COPTS) -fPIC

# [FIXME] added by Michael Jiang.
# The below variables  should be modified when you use different platform.
HOST=$(ARCH)-linux-linux
HOST_COMPILE=$(HOST)-
CROSS_COMPILE=$(ARCH)-linux-uclibc-
#HOST_COMPILE=mipsel-linux-linux26-
#CROSS=mipsel-linux-uclibc
# comments ended, by Michael Jiang.

all: zlib libogg libexif sqlite3 libvorbis flac libid3tag libjpeg
	for i in $(SUBDIRS); do \
		(cd $$i && make TOOL_CHAIN_PATH=$(TOOL_CHAIN_PATH) \
				CROSS_COMPILE=$(CROSS_COMPILE) \
				HOST_COMPILE=$(HOST_COMPILE) \
				MINI_DLNA_PATH=$(MINI_DLNA_PATH)) \
	done
	make copy
	(cd minidlna && make MINI_DLNA_PATH=$(MINI_DLNA_PATH) \
			     CROSS_COMPILE=$(CROSS_COMPILE) \
			     HOST_COMPILE=$(HOST_COMPILE) \
			     MINI_DLNA_PATH=$(MINI_DLNA_PATH))
	cp ./minidlna/minidlna	./lib/minidlna
	#make install

zlib:
	# compile zlib first, bcz. libid3tag needs it for configuration.
	(cd zlib-1.2.3 && make TOOL_CHAIN_PATH=$(TOOL_CHAIN_PATH) \
	CROSS_COMPILE=$(CROSS_COMPILE) \
	HOST_COMPILE=$(HOST_COMPILE) \
	MINI_DLNA_PATH=$(MINI_DLNA_PATH))

libogg:
	if [ ! -f ./libogg-1.1.4/Makefile ];then \
	cd libogg-1.1.4 && (./configure --host=$(HOST)  CC="ccache $(CROSS_COMPILE)gcc"  CFLAGS=$(COPTS)); \
	fi

libexif:
	if [ ! -f ./libexif-0.6.19/Makefile ];then \
	cd libexif-0.6.19 && (./configure --host=$(HOST)  CC="ccache $(CROSS_COMPILE)gcc"  CFLAGS=$(COPTS)); \
	fi

libjpeg:
	if [ ! -f ./jpeg-7/Makefile ];then \
	cd jpeg-7 && (./configure --host=$(HOST)  CC="ccache $(CROSS_COMPILE)gcc"  CFLAGS=$(COPTS)); \
	fi

sqlite3:
	if [ ! -f ./sqlite-3.6.22/Makefile ];then \
	cd sqlite-3.6.22 && (./configure --host=$(HOST)  CC="ccache $(CROSS_COMPILE)gcc" CFLAGS=$(COPTS) \
				--enable-threadsafe ); \
	fi

libvorbis:
	cd libogg-1.1.4 && make TOOL_CHAIN_PATH=$(TOOL_CHAIN_PATH) \
				CROSS_COMPILE=$(CROSS_COMPILE) \
				HOST_COMPILE=$(HOST_COMPILE) \
				MINI_DLNA_PATH=$(MINI_DLNA_PATH)				
	if [ ! -f ./libvorbis-1.2.3/Makefile ];then \
	cd libvorbis-1.2.3 && (./configure --host=$(HOST)  CC=$(CROSS_COMPILE)gcc \
					CFLAGS=$(COPTS) -I$(MINI_DLNA_PATH)/libogg-1.1.4/include/ \
					LDFLAGS=-L$(MINI_DLNA_PATH)/libogg-1.1.4/src/.libs/ \
					); \
	fi
flac:
	if [ ! -f ./flac-1.2.1/Makefile ];then \
	cd flac-1.2.1 && (./configure --host=$(HOST)  CC=$(CROSS_COMPILE)gcc \
				CFLAGS=$(COPTS) -I$(MINI_DLNA_PATH)/libogg-1.1.4/include/ \
				LDFLAGS=-L$(MINI_DLNA_PATH)/libogg-1.1.4/src/.libs/ \
				); \
	fi

libid3tag:
	if [ ! -f ./libid3tag-0.15.0b/Makefile ];then \
	cd libid3tag-0.15.0b && (./configure --host=$(HOST)  CC=$(CROSS_COMPILE)gcc \
				CPPFLAGS=$(COPTS) -I$(MINI_DLNA_PATH)/zlib-1.2.3/ \
				LDFLAGS=-L$(MINI_DLNA_PATH)/zlib-1.2.3/ \
				); \
	fi
clean:
	for i in ${SUBDIRS}; do \
		(cd $$i; make clean); \
	done
	rm ./lib/libz.so.1
	rm ./lib/libvorbis.so.0
	rm ./lib/libFLAC.so.8
	rm ./lib/libogg.so.0
	rm ./lib/libexif.so.12
	rm ./lib/libavutil.so.49
	rm ./lib/libavdevice.so.52
	rm ./lib/libavformat.so.52
	rm ./lib/libavcodec.so.52
	rm ./lib/libsqlite3.so.0
	rm ./lib/libjpeg.so.7
	rm ./lib/libid3tag.so.0
	rm ./lib/minidlna
copy: 
	cp ./zlib-1.2.3/libz.so.1.2.3				./lib/libz.so.1 && \
	cp ./libvorbis-1.2.3/lib/.libs/libvorbis.so.0.4.3	./lib/libvorbis.so.0 && \
	cp ./flac-1.2.1/src/libFLAC/.libs/libFLAC.so.8.2.0	./lib/libFLAC.so.8 && \
	cp ./libexif-0.6.19/libexif/.libs/libexif.so.12.3.1 	./lib/libexif.so.12  && \
	cp ./libogg-1.1.4/src/.libs/libogg.so.0.6.0		./lib/libogg.so.0 && \
	cp ./ffmpeg-0.5.1/libavutil/libavutil.so.49		./lib/libavutil.so.49 && \
	cp ./ffmpeg-0.5.1/libavdevice/libavdevice.so.52		./lib/libavdevice.so.52 && \
	cp ./ffmpeg-0.5.1/libavformat/libavformat.so.52		./lib/libavformat.so.52 && \
	cp ./ffmpeg-0.5.1/libavcodec/libavcodec.so.52		./lib/libavcodec.so.52 && \
	cp ./sqlite-3.6.22/.libs/libsqlite3.so.0.8.6 		./lib/libsqlite3.so.0 && \
	cp ./jpeg-7/.libs/libjpeg.so.7.0.0			./lib/libjpeg.so.7 && \
	cp ./libid3tag-0.15.0b/.libs/libid3tag.so.0.2.0 	./lib/libid3tag.so.0

distclean:
	cd ./libogg-1.1.4/	&& make distclean 
	cd ./libexif-0.6.19/	&& make distclean
	cd ./sqlite-3.6.22/	&& make distclean
	cd ./libvorbis-1.2.3/	&& make distclean
	cd ./libid3tag-0.15.0b/	&& make distclean
	cd ./flac-1.2.1/	&& make distclean
	cd ./zlib-1.2.3/	&& make clean
	cd ./ffmpeg-0.5.1/	&& make clean
	cd ./jpeg-7/		&& make distclean
	cd ./minidlna/		&& make clean
	rm ./lib/*.so.*
	rm ./lib/minidlna

install:
	$(CROSS_COMPILE)strip ./lib/*
	install -m 755 minidlna/minidlna.conf $(TARGETDIR)/usr/
	install -m 755 lib/libz.so.1  $(TARGETDIR)/usr/lib
	install -m 755 lib/libvorbis.so.0 $(TARGETDIR)/usr/lib
	install -m 755 lib/libFLAC.so.8 $(TARGETDIR)/usr/lib
	install -m 755 lib/libexif.so.12 $(TARGETDIR)/usr/lib
	install -m 755 lib/libogg.so.0 $(TARGETDIR)/usr/lib
	install -m 755 lib/libavutil.so.49 $(TARGETDIR)/usr/lib
	install -m 755 lib/libavdevice.so.52 $(TARGETDIR)/usr/lib
	install -m 755 lib/libavformat.so.52 $(TARGETDIR)/usr/lib
	install -m 755 lib/libavcodec.so.52 $(TARGETDIR)/usr/lib
	install -m 755 lib/libsqlite3.so.0 $(TARGETDIR)/usr/lib
	install -m 755 lib/libjpeg.so.7 $(TARGETDIR)/usr/lib
	install -m 755 lib/libid3tag.so.0 $(TARGETDIR)/usr/lib
	install -m 755 lib/minidlna $(TARGETDIR)/usr/sbin/


