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
FFMPEG_VER=2.0

SUBDIRS += zlib-1.2.7
SUBDIRS += ffmpeg-$(FFMPEG_VER)
SUBDIRS += libogg-1.3.0
SUBDIRS += libexif-0.6.19
SUBDIRS += flac-1.2.1
SUBDIRS += libvorbis-1.3.3
SUBDIRS += jpeg-8
SUBDIRS += sqlite-3.6.22
SUBDIRS += libid3tag-0.15.0b
#/projects/hnd/tools/linux/hndtools-mipsel-linux-uclibc-4.2.3
TOOL_CHAIN_PATH=$(TOOLCHAIN)
#/home/michael/DLNA
MINI_DLNA_PATH=$(shell pwd)

CFLAGS += $(COPTS) -ffunction-sections -fdata-sections -Wl,--gc-sections

CFLAGS += $(MIPS16_OPT) 
COPTS += $(MIPS16_OPT) 

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
	(cd zlib-1.2.7 && make TOOL_CHAIN_PATH=$(TOOL_CHAIN_PATH) \
	CROSS_COMPILE=$(CROSS_COMPILE) \
	HOST_COMPILE=$(HOST_COMPILE) \
	MINI_DLNA_PATH=$(MINI_DLNA_PATH))

ffmpeg:
	if [ ! -f ./ffmpeg-$(FFMPEG_VER)/config.mak ];then \
	cd ffmpeg-$(FFMPEG_VER) && (./configure --enable-shared --enable-gpl --target-os=linux  --arch=$(ARCH) --extra-cflags="$(COPTS)  -ffunction-sections -fdata-sections -Wl,--gc-sections " --cc="ccache $(ARCH)-linux-uclibc-gcc" --enable-cross-compile --disable-muxers --disable-encoders --disable-avfilter --disable-filters --enable-small --disable-postproc --disable-network \
	--enable-shared \
	--disable-mipsfpu \
	--disable-mipsdspr1 \
	--disable-mipsdspr2 \
	--enable-static \
	--disable-debug \
	--disable-doc \
	--disable-dxva2 \
	--enable-pthreads \
	--enable-small \
	--disable-stripping \
	--enable-zlib \
	--disable-ffmpeg \
	--disable-asm \
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
	--enable-decoder=flv \
	--enable-decoder=h264 \
	--enable-decoder=h264_crystalhd \
	--enable-decoder=h264_vda \
	--enable-decoder=h264_vdpau \
	--enable-decoder=jpegls \
	--enable-decoder=mp3 \
	--enable-decoder=mpeg1video \
	--enable-decoder=mpeg2video \
	--enable-decoder=msmpeg4_crystalhd \
	--enable-decoder=msmpeg4v1 \
	--enable-decoder=msmpeg4v2 \
	--enable-decoder=msmpeg4v3 \
	--enable-decoder=mpeg4 \
	--enable-decoder=mpeg4_crystalhd \
	--enable-decoder=mpeg4_vdpau \
	--enable-decoder=mpegvideo \
	--enable-decoder=mwav1 \
	--enable-decoder=mwav2 \
	--enable-decoder=vc1 \
	--enable-decoder=vc1_crystalhd \
	--enable-decoder=vc1_vdpau \
	--enable-decoder=png \
	--enable-decoder=wmv1 \
	--enable-decoder=wmv2 \
	--enable-decoder=wmv3 \
	--enable-decoder=wmv3_crystalhd \
	--enable-decoder=wmv3_vdpau \
	--enable-decoder=truehd \
	--enable-demuxer=aac \
	--enable-demuxer=vc1 \
	--enable-demuxer=vc1t \
	--enable-demuxer=ac3 \
	--enable-demuxer=avi \
	--enable-demuxer=avisynth \
	--enable-demuxer=dts \
	--enable-demuxer=dtshd \
	--enable-demuxer=flac \
	--enable-demuxer=h264 \
	--enable-demuxer=matroska \
	--enable-demuxer=mov \
	--enable-demuxer=mp3 \
	--enable-demuxer=m4v \
	--enable-demuxer=mpegts \
	--enable-demuxer=mpegps \
	--enable-demuxer=ogg \
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
	if [ ! -f ./jpeg-8/Makefile ];then \
	cd jpeg-8 && (./configure --host=$(HOST)  CC="ccache $(CROSS_COMPILE)gcc"  CFLAGS="$(COPTS)  -ffunction-sections -fdata-sections -Wl,--gc-sections"); \
	fi

sqlite3:
	if [ ! -f ./sqlite-3.6.22/Makefile ];then \
	cd sqlite-3.6.22 && (./configure --host=$(HOST)  CC="ccache $(CROSS_COMPILE)gcc" CFLAGS="$(COPTS) -fPIC  -ffunction-sections -fdata-sections -Wl,--gc-sections -DSQLITE_OMIT_TRIGGERS" \
				--enable-threadsafe --enable-dynamic-extensions=no ); \
	fi

libvorbis:
	cd libogg-1.3.0 && make TOOL_CHAIN_PATH=$(TOOL_CHAIN_PATH) \
				CROSS_COMPILE=$(CROSS_COMPILE) \
				HOST_COMPILE=$(HOST_COMPILE) \
				MINI_DLNA_PATH=$(MINI_DLNA_PATH)				
	if [ ! -f ./libvorbis-1.3.3/Makefile ];then \
	cd libvorbis-1.3.3 && (./configure --host=$(HOST)  CC="ccache $(CROSS_COMPILE)gcc" \
					CFLAGS="$(COPTS) -I$(MINI_DLNA_PATH)/libogg-1.3.0/include/  -ffunction-sections -fdata-sections -Wl,--gc-sections" \
					LDFLAGS=-L$(MINI_DLNA_PATH)/libogg-1.3.0/src/.libs/ \
					); \
	fi
flac:
	if [ ! -f ./flac-1.2.1/Makefile ];then \
	cd flac-1.2.1 && (./configure --host=$(HOST) --disable-altivec  CC="ccache $(CROSS_COMPILE)gcc" \
				CFLAGS="$(COPTS) -I$(MINI_DLNA_PATH)/libogg-1.3.0/include/  -ffunction-sections -fdata-sections -Wl,--gc-sections" \
				LDFLAGS=-L$(MINI_DLNA_PATH)/libogg-1.3.0/src/.libs/ \
				); \
	fi

libid3tag:
	if [ ! -f ./libid3tag-0.15.0b/Makefile ];then \
	cd libid3tag-0.15.0b && (./configure --host=$(HOST)  CC="ccache $(CROSS_COMPILE)gcc" \
				CPPFLAGS="$(COPTS) -I$(MINI_DLNA_PATH)/zlib-1.2.7/  -ffunction-sections -fdata-sections -Wl,--gc-sections" \
				LDFLAGS=-L$(MINI_DLNA_PATH)/zlib-1.2.7/ \
				); \
	fi
clean:
	-for i in ${SUBDIRS}; do \
		(cd $$i; make clean); \
	done
	(cd minidlna && make clean MINI_DLNA_PATH=$(MINI_DLNA_PATH) \
			     CROSS_COMPILE=$(CROSS_COMPILE) \
			     HOST_COMPILE=$(HOST_COMPILE) \
			     MINI_DLNA_PATH=$(MINI_DLNA_PATH))
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
	cp ./zlib-1.2.7/libz.a				./lib/libz.a && \
	cp ./libvorbis-1.3.3/lib/.libs/libvorbis.a	./lib/libvorbis.a && \
	cp ./flac-1.2.1/src/libFLAC/.libs/libFLAC.a		./lib/libFLAC.a && \
	cp ./libexif-0.6.19/libexif/.libs/libexif.a 		./lib/libexif.a  && \
	cp ./libogg-1.3.0/src/.libs/libogg.a		./lib/libogg.a && \
	cp ./ffmpeg-$(FFMPEG_VER)/libavutil/libavutil.a		./lib/libavutil.a && \
	cp ./ffmpeg-$(FFMPEG_VER)/libavdevice/libavdevice.a		./lib/libavdevice.a && \
	cp ./ffmpeg-$(FFMPEG_VER)/libavformat/libavformat.a		./lib/libavformat.a && \
	cp ./ffmpeg-$(FFMPEG_VER)/libavcodec/libavcodec.a		./lib/libavcodec.a && \
	cp ./sqlite-3.6.22/.libs/libsqlite3.a 		./lib/libsqlite3.a && \
	cp ./jpeg-8/.libs/libjpeg.a			./lib/libjpeg.a && \
	cp ./libid3tag-0.15.0b/.libs/libid3tag.a 	./lib/libid3tag.a

distclean:
	-cd ./libogg-1.3.0/	&& make distclean 
	-cd ./libexif-0.6.19/	&& make distclean
	-cd ./sqlite-3.6.22/	&& make distclean
	-cd ./libvorbis-1.3.3/	&& make distclean
	-cd ./libid3tag-0.15.0b/	&& make distclean
	-rm -f ./libid3tag-0.15.0b/Makefile
	-cd ./flac-1.2.1/	&& make distclean
	-cd ./zlib-1.2.7/	&& make clean
	-cd ./ffmpeg-$(FFMPEG_VER)/	&& make distclean
	-cd ./jpeg-8/		&& make distclean
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


