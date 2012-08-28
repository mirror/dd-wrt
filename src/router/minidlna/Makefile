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
SUBDIRS += ffmpeg-0.11.1
SUBDIRS += libogg-1.3.0
SUBDIRS += libexif-0.6.19
SUBDIRS += flac-1.2.1
SUBDIRS += libvorbis-1.3.3
SUBDIRS += jpeg-7
SUBDIRS += sqlite-3.6.22
SUBDIRS += libid3tag-0.15.0b
#/projects/hnd/tools/linux/hndtools-mipsel-linux-uclibc-4.2.3
TOOL_CHAIN_PATH=$(TOOLCHAIN)
#/home/michael/DLNA
MINI_DLNA_PATH=$(shell pwd)

CFLAGS += $(COPTS) -fPIC  -ffunction-sections -fdata-sections -Wl,--gc-sections

# [FIXME] added by Michael Jiang.
# The below variables  should be modified when you use different platform.
HOST=$(ARCH)-linux-linux
HOST_COMPILE=$(HOST)-
CROSS_COMPILE=$(ARCH)-linux-uclibc-
#HOST_COMPILE=mipsel-linux-linux26-
#CROSS=mipsel-linux-uclibc
# comments ended, by Michael Jiang.

all: zlib ffmpeg libogg libexif sqlite3 libvorbis flac libid3tag libjpeg
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

ffmpeg:
	if [ ! -f ./ffmpeg-0.11.1/config.mak ];then \
	cd ffmpeg-0.11.1 && (./configure --enable-shared --enable-gpl --target-os=linux  --arch=$(ARCH) --extra-cflags="$(COPTS)  -ffunction-sections -fdata-sections -Wl,--gc-sections " --cc="ccache $(ARCH)-linux-uclibc-gcc" --enable-cross-compile --disable-muxers --disable-encoders --disable-avfilter --disable-filters --enable-small --disable-postproc --disable-network \
	--enable-shared \
	--enable-static \
	--disable-debug \
	--disable-doc \
	--disable-dxva2 \
	--enable-pthreads \
	--disable-optimizations \
	--enable-small \
	--disable-stripping \
	--enable-zlib \
	--disable-ffmpeg \
	--disable-ffplay \
	--disable-ffprobe \
	--disable-ffserver \
	--disable-outdevs \
	--disable-avfilter \
	--disable-postproc \
	--disable-swscale \
	--disable-everything \
	--enable-decoder=aac \
	--enable-decoder=ac3 \
	--enable-decoder=flac \
	--enable-decoder=h264 \
	--enable-decoder=jpegls \
	--enable-decoder=mp3 \
	--enable-decoder=mpeg1video \
	--enable-decoder=mpeg2video \
	--enable-decoder=mpeg4 \
	--enable-decoder=mpegvideo \
	--enable-decoder=mwav1 \
	--enable-decoder=mwav2 \
	--enable-decoder=vc1 \
	--enable-decoder=vc1_crystalhd \
	--enable-decoder=png \
	--enable-demuxer=aac \
	--enable-demuxer=vc1 \
	--enable-demuxer=ac3 \
	--enable-demuxer=avi \
	--enable-demuxer=flac \
	--enable-demuxer=h264 \
	--enable-demuxer=matroska \
	--enable-demuxer=mov \
	--enable-demuxer=mp3 \
	--enable-demuxer=mpegvideo \
	--enable-protocol=file); \
	fi

libogg:
	if [ ! -f ./libogg-1.3.0/Makefile ];then \
	cd libogg-1.3.0 && (./configure --host=$(HOST)  CC="ccache $(CROSS_COMPILE)gcc"  CFLAGS="$(COPTS)  -ffunction-sections -fdata-sections -Wl,--gc-sections"); \
	fi

libexif:
	if [ ! -f ./libexif-0.6.19/Makefile ];then \
	cd libexif-0.6.19 && (./configure --host=$(HOST)  CC="ccache $(CROSS_COMPILE)gcc"  CFLAGS="$(COPTS)  -ffunction-sections -fdata-sections -Wl,--gc-sections"); \
	fi

libjpeg:
	if [ ! -f ./jpeg-7/Makefile ];then \
	cd jpeg-7 && (./configure --host=$(HOST)  CC="ccache $(CROSS_COMPILE)gcc"  CFLAGS="$(COPTS)  -ffunction-sections -fdata-sections -Wl,--gc-sections"); \
	fi

sqlite3:
	if [ ! -f ./sqlite-3.6.22/Makefile ];then \
	cd sqlite-3.6.22 && (./configure --host=$(HOST)  CC="ccache $(CROSS_COMPILE)gcc" CFLAGS="$(COPTS)  -ffunction-sections -fdata-sections -Wl,--gc-sections -DSQLITE_OMIT_TRIGGERS" \
				--enable-threadsafe --disable-dynamic-extensions --disable-dynamic-extensions ); \
	fi

libvorbis:
	cd libogg-1.3.0 && make TOOL_CHAIN_PATH=$(TOOL_CHAIN_PATH) \
				CROSS_COMPILE=$(CROSS_COMPILE) \
				HOST_COMPILE=$(HOST_COMPILE) \
				MINI_DLNA_PATH=$(MINI_DLNA_PATH)				
	if [ ! -f ./libvorbis-1.3.3/Makefile ];then \
	cd libvorbis-1.3.3 && (./configure --host=$(HOST)  CC=$(CROSS_COMPILE)gcc \
					CFLAGS="$(COPTS) -I$(MINI_DLNA_PATH)/libogg-1.3.0/include/  -ffunction-sections -fdata-sections -Wl,--gc-sections" \
					LDFLAGS=-L$(MINI_DLNA_PATH)/libogg-1.3.0/src/.libs/ \
					); \
	fi
flac:
	if [ ! -f ./flac-1.2.1/Makefile ];then \
	cd flac-1.2.1 && (./configure --host=$(HOST)  CC=$(CROSS_COMPILE)gcc \
				CFLAGS="$(COPTS) -I$(MINI_DLNA_PATH)/libogg-1.3.0/include/  -ffunction-sections -fdata-sections -Wl,--gc-sections" \
				LDFLAGS=-L$(MINI_DLNA_PATH)/libogg-1.3.0/src/.libs/ \
				); \
	fi

libid3tag:
	if [ ! -f ./libid3tag-0.15.0b/Makefile ];then \
	cd libid3tag-0.15.0b && (./configure --host=$(HOST)  CC=$(CROSS_COMPILE)gcc \
				CPPFLAGS="$(COPTS) -I$(MINI_DLNA_PATH)/zlib-1.2.3/  -ffunction-sections -fdata-sections -Wl,--gc-sections" \
				LDFLAGS=-L$(MINI_DLNA_PATH)/zlib-1.2.3/ \
				); \
	fi
clean:
	-for i in ${SUBDIRS}; do \
		(cd $$i; make clean); \
	done
	-rm ./lib/libz.a
	-rm ./lib/libvorbis.a
	-rm ./lib/libFLAC.a
	-rm ./lib/libogg.a
	-rm ./lib/libexif.a
	-rm ./lib/libavutil.a
	-rm ./lib/libavdevice.a
	-rm ./lib/libavformat.a
	-rm ./lib/libavcodec.a
	-rm ./lib/libsqlite3.a
	-rm ./lib/libjpeg.a
	-rm ./lib/libid3tag.a
	-rm ./lib/minidlna
copy: 
	cp ./zlib-1.2.3/libz.a				./lib/libz.a && \
	cp ./libvorbis-1.3.3/lib/.libs/libvorbis.a	./lib/libvorbis.a && \
	cp ./flac-1.2.1/src/libFLAC/.libs/libFLAC.a		./lib/libFLAC.a && \
	cp ./libexif-0.6.19/libexif/.libs/libexif.a 		./lib/libexif.a  && \
	cp ./libogg-1.3.0/src/.libs/libogg.a		./lib/libogg.a && \
	cp ./ffmpeg-0.11.1/libavutil/libavutil.a		./lib/libavutil.a && \
	cp ./ffmpeg-0.11.1/libavdevice/libavdevice.a		./lib/libavdevice.a && \
	cp ./ffmpeg-0.11.1/libavformat/libavformat.a		./lib/libavformat.a && \
	cp ./ffmpeg-0.11.1/libavcodec/libavcodec.a		./lib/libavcodec.a && \
	cp ./sqlite-3.6.22/.libs/libsqlite3.a 		./lib/libsqlite3.a && \
	cp ./jpeg-7/.libs/libjpeg.a			./lib/libjpeg.a && \
	cp ./libid3tag-0.15.0b/.libs/libid3tag.a 	./lib/libid3tag.a

distclean:
	-cd ./libogg-1.3.0/	&& make distclean 
	-cd ./libexif-0.6.19/	&& make distclean
	-cd ./sqlite-3.6.22/	&& make distclean
	-cd ./libvorbis-1.3.3/	&& make distclean
	-cd ./libid3tag-0.15.0b/	&& make distclean
	-cd ./flac-1.2.1/	&& make distclean
	-cd ./zlib-1.2.3/	&& make clean
	-cd ./ffmpeg-0.11.1/	&& make distclean
	-cd ./jpeg-7/		&& make distclean
	-cd ./minidlna/		&& make clean
	-rm ./lib/*.so.*
	-rm ./lib/minidlna

install:
	mkdir -p $(TARGETDIR)/usr/lib
	mkdir -p $(TARGETDIR)/etc
	mkdir -p $(TARGETDIR)/usr/sbin
	install -m 755 minidlna/minidlna.conf $(TARGETDIR)/etc/
#	install -m 755 lib/libz.so.1  $(TARGETDIR)/usr/lib
#	install -m 755 lib/libvorbis.so.0 $(TARGETDIR)/usr/lib
#	install -m 755 lib/libFLAC.so.8 $(TARGETDIR)/usr/lib
#	install -m 755 lib/libexif.so.12 $(TARGETDIR)/usr/lib
#	install -m 755 lib/libogg.so.0 $(TARGETDIR)/usr/lib
#	install -m 755 lib/libavutil.so.51 $(TARGETDIR)/usr/lib
#	install -m 755 lib/libavdevice.so.54 $(TARGETDIR)/usr/lib
#	install -m 755 lib/libavformat.so.54 $(TARGETDIR)/usr/lib
#	install -m 755 lib/libavcodec.so.54 $(TARGETDIR)/usr/lib
#	install -m 755 lib/libsqlite3.so.0 $(TARGETDIR)/usr/lib
#	install -m 755 lib/libjpeg.so.7 $(TARGETDIR)/usr/lib
#	install -m 755 lib/libid3tag.a $(TARGETDIR)/usr/lib
	install -m 755 lib/minidlna $(TARGETDIR)/usr/sbin/


