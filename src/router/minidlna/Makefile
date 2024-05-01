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
FFMPEG_VER=3.1

#SUBDIRS += zlib-1.2.7
SUBDIRS += ffmpeg-$(FFMPEG_VER)
SUBDIRS += libogg-1.3.5
SUBDIRS += libexif-0.6.24
SUBDIRS += flac-1.4.2
SUBDIRS += libvorbis-1.3.7
SUBDIRS += libjpeg-turbo-2.1.4
SUBDIRS += sqlite-3.6.22
SUBDIRS += libid3tag-0.15.1b
#/projects/hnd/tools/linux/hndtools-mipsel-linux-uclibc-4.2.3
TOOL_CHAIN_PATH=$(TOOLCHAIN)
#/home/michael/DLNA
MINI_DLNA_PATH=$(shell pwd)

CFLAGS += $(COPTS) -ffunction-sections -fdata-sections -Wl,--gc-sections

CFLAGS += $(MIPS16_OPT) $(THUMB) -fPIC
COPTS += $(MIPS16_OPT) $(THUMB) -fPIC

# [FIXME] added by Michael Jiang.
# The below variables  should be modified when you use different platform.
HOST=$(ARCH)-linux
HOST_COMPILE=$(HOST)-
CROSS_COMPILE=$(ARCH)-linux-uclibc-
#HOST_COMPILE=mipsel-linux-linux26-
#CROSS=mipsel-linux-uclibc
# comments ended, by Michael Jiang.

FFMPEG_FILTER_CONFIG= $(foreach c, $(2), --$(1)="$(c)")

FFMPEG_DECODERS:=aac ac3 atrac3 h264 jpegls mp3 mpeg1video mpeg2video mpeg4 mpeg4aac mpegvideo png wmav1 wmav2 svq3
FFMPEG_CONFIGURE_DECODERS:=$(call FFMPEG_FILTER_CONFIG,enable-decoder,$(FFMPEG_DECODERS))

FFMPEG_PARSERS:=aac ac3 h264 mpeg4video mpegaudio mpegvideo
FFMPEG_CONFIGURE_PARSERS:=$(call FFMPEG_FILTER_CONFIG,enable-parser,$(FFMPEG_PARSERS))

FFMPEG_PROTOCOLS:=file
FFMPEG_CONFIGURE_PROTOCOLS:=$(call FFMPEG_FILTER_CONFIG,enable-protocol,$(FFMPEG_PROTOCOLS))

FFMPEG_DISABLED_DEMUXERS:=amr apc ape ass bethsoftvid bfi c93 daud dnxhd dsicin dxa ffm gsm gxf idcin iff image2 image2pipe ingenient ipmovie lmlm4 mm mmf msnwc_tcp mtv mxf nsv nut oma pva rawvideo rl2 roq rpl segafilm shorten siff smacker sol str thp tiertexseq tta txd vmd voc wc3 wsaud wsvqa xa yuv4mpegpipe
FFMPEG_CONFIGURE_DEMUXERS:=$(call FFMPEG_FILTER_CONFIG,disable-demuxer,$(FFMPEG_DISABLED_DEMUXERS))


SIMD=1
ifeq ($(ARCH),armeb)
SIMD=0
endif


all: ffmpeg libogg libexif sqlite3 libvorbis flac libid3tag libjpeg
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

ffmpeg:
	if [ ! -f ./ffmpeg-$(FFMPEG_VER)/config.mak ]; then \
	cd ffmpeg-$(FFMPEG_VER) && (symver_asm_label=no symver_gnu_asm=no symver=no CC="$(CC)" \
		./configure --enable-cross-compile --arch=$(ARCH) --target_os=linux \
		--cross-prefix=$(CROSS_COMPILE) --disable-shared --enable-static --enable-gpl --disable-doc \
		--enable-pthreads --enable-small --disable-encoders --disable-filters \
		--disable-muxers --disable-devices --disable-ffmpeg --disable-ffplay \
		--disable-ffserver --disable-ffprobe --disable-avdevice --disable-swscale \
		--disable-hwaccels --disable-network --disable-bsfs \
		--enable-demuxers $(FFMPEG_CONFIGURE_DEMUXERS) \
		--disable-decoders $(FFMPEG_CONFIGURE_DECODERS) \
		--disable-parsers $(FFMPEG_CONFIGURE_PARSERS) \
		--disable-protocols $(FFMPEG_CONFIGURE_PROTOCOLS) \
		--disable-asm --disable-altivec \
		--disable-postproc --disable-avfilter \
		--extra-cflags="$(COPTS) $(MIPS16_OPT) $(THUMB) -ffunction-sections -fdata-sections  -D_GNU_SOURCE -I$(TOP)/zlib $(LTO)" \
		--extra-ldflags="-ffunction-sections -fdata-sections -Wl,--gc-sections  $(LDLTO)" \
		--extra-libs="-L$(TOP)/zlib -lz" \
		--enable-zlib --disable-debug --prefix=''); \
	fi

libogg:
	if [ ! -f ./libogg-1.3.5/Makefile ];then \
	cd libogg-1.3.5 && aclocal && libtoolize && ./autogen.sh && (./configure --host=$(HOST) --disable-shared  CC="ccache $(CROSS_COMPILE)gcc"  CFLAGS="$(COPTS) $(MIPS16_OPT) $(THUMB) $(LTO)  -ffunction-sections -fdata-sections -Wl,--gc-sections" AR_FLAGS="cru $(LTOPLUGIN)" RANLIB="$(ARCH)-linux-ranlib $(LTOPLUGIN)"); \
	fi

libexif:
	if [ ! -f ./libexif-0.6.24/Makefile ];then \
	cd libexif-0.6.24 && (./configure --host=$(HOST) --disable-shared  CC="ccache $(CROSS_COMPILE)gcc"  CFLAGS="$(COPTS) $(MIPS16_OPT) $(THUMB) $(LTO)  -ffunction-sections -fdata-sections -Wl,--gc-sections" AR_FLAGS="cru $(LTOPLUGIN)" RANLIB="$(ARCH)-linux-ranlib $(LTOPLUGIN)"); \
	fi

libjpeg:
	if [ ! -f ./libjpeg-turbo-2.1.4/CMakeCache.txt ];then \
	echo "$(ARCH)-linux-uclibc-gcc-ar \$$@" > libjpeg-turbo-2.1.4/$(ARCH)-linux-uclibc-gcc-ar; \
	chmod 777 libjpeg-turbo-2.1.4/$(ARCH)-linux-uclibc-gcc-ar; \
	echo "$(ARCH)-linux-uclibc-gcc-ranlib \$$@" > libjpeg-turbo-2.1.4/$(ARCH)-linux-uclibc-gcc-ranlib; \
	chmod 777 libjpeg-turbo-2.1.4/$(ARCH)-linux-uclibc-gcc-ranlib; \
	(cd libjpeg-turbo-2.1.4; \
		CFLAGS="$(COPTS) $(MIPS16_OPT) $(THUMB) $(LTO) -ffunction-sections -fdata-sections -Wl,--gc-sections" \
		LDFLAGS="$(COPTS) $(MIPS16_OPT) $(THUMB) $(LTO) -ffunction-sections -fdata-sections -Wl,--gc-sections" \
		CXXFLAGS="$(COPTS) $(MIPS16_OPT) $(THUMB) $(LTO) -ffunction-sections -fdata-sections -Wl,--gc-sections" \
		AR_FLAGS="cru $(LTOPLUGIN)" \
		RANLIB="$(ARCH)-linux-gcc-ranlib" \
		AR="$(ARCH)-linux-gcc-ar" \
		cmake \
			--debug-output \
			-DWITH_JPEG8=1 \
			-DWITH_SIMD=$(SIMD) \
			-DENABLE_SHARED=0 \
			-DENABLE_STATIC=1 \
			-DCMAKE_SYSTEM_NAME=Linux \
			-DCMAKE_SYSTEM_VERSION=1 \
			-DCMAKE_SYSTEM_PROCESSOR=$(ARCH) \
			-DCMAKE_BUILD_TYPE=Release \
			-DCMAKE_C_FLAGS_RELEASE="-DNDEBUG" \
			-DCMAKE_CXX_FLAGS_RELEASE="-DNDEBUG" \
			-DCMAKE_C_COMPILER=$(CROSS_COMPILE)gcc \
			-DCMAKE_AR="$(ARCH)-linux-uclibc-gcc-ar" \
			-DCMAKE_RANLIB="$(ARCH)-linux-uclibc-gcc-ranlib" \
			-DCMAKE_C_COMPILER_RANLIB="$(ARCH)-linux-uclibc-gcc-ranlib" \
			-DCMAKE_CXX_COMPILER=$(CROSS_COMPILE)g++ \
			-DCMAKE_EXE_LINKER_FLAGS="$(COPTS) $(MIPS16_OPT) $(THUMB) $(LTO) -ffunction-sections -fdata-sections -Wl,--gc-sections" \
			-DCMAKE_MODULE_LINKER_FLAGS="$(COPTS) $(MIPS16_OPT) $(THUMB) $(LTO) -ffunction-sections -fdata-sections -Wl,--gc-sections" \
			-DCMAKE_SHARED_LINKER_FLAGS="$(COPTS) $(MIPS16_OPT) $(THUMB) $(LTO) -ffunction-sections -fdata-sections -Wl,--gc-sections" \
			-DCMAKE_FIND_ROOT_PATH=$(TOP)/minidlna/libjpeg-turbo-2.1.4 \
			-DCMAKE_FIND_ROOT_PATH_MODE_PROGRAM=BOTH \
			-DCMAKE_FIND_ROOT_PATH_MODE_LIBRARY=ONLY \
			-DCMAKE_FIND_ROOT_PATH_MODE_INCLUDE=ONLY \
			-DDL_LIBRARY=$(TOP)/minidlna/libjpeg-turbo-2.1.4 \
			-DCMAKE_PREFIX_PATH=$(TOP)/minidlna/libjpeg-turbo-2.1.4 \
			-DCMAKE_STRIP=: \
			-DCMAKE_INSTALL_PREFIX=/usr \
			. \
	) \
	fi

sqlite3:
	if [ ! -f ./sqlite-3.6.22/Makefile ];then \
	cd sqlite-3.6.22 && libtoolize -ci --force; \
	aclocal; \
	automake --add-missing; \
	autoreconf -fi; \
	(./configure --host=$(HOST) --disable-shared  CC="ccache $(CROSS_COMPILE)gcc" CFLAGS="$(COPTS) $(MIPS16_OPT) $(THUMB) $(LTO) -DSQLITE_OMIT_DEPRECATED -DSQLITE_OMIT_FLOATING_POINT -ffunction-sections -fdata-sections -Wl,--gc-sections -DSQLITE_OMIT_TRIGGERS" AR_FLAGS="cru $(LTOPLUGIN)" RANLIB="$(ARCH)-linux-ranlib $(LTOPLUGIN)" \
				--enable-threadsafe --enable-dynamic-extensions=no ); \
	fi

libvorbis:
	cd libogg-1.3.5 && make TOOL_CHAIN_PATH=$(TOOL_CHAIN_PATH) \
				CROSS_COMPILE=$(CROSS_COMPILE) \
				HOST_COMPILE=$(HOST_COMPILE) \
				MINI_DLNA_PATH=$(MINI_DLNA_PATH)				
	if [ ! -f ./libvorbis-1.3.7/Makefile ];then \
	cd libvorbis-1.3.7 && (./configure --host=$(HOST) --disable-shared  CC="ccache $(CROSS_COMPILE)gcc" \
					CFLAGS="$(COPTS) $(MIPS16_OPT) $(THUMB) -I$(MINI_DLNA_PATH)/libogg-1.3.5/include/  -ffunction-sections -fdata-sections -Wl,--gc-sections" \
					LDFLAGS="-L$(MINI_DLNA_PATH)/libogg-1.3.5/src/.libs/ $(LDLTO)" \
					AR_FLAGS="cru $(LTOPLUGIN)" RANLIB="$(ARCH)-linux-ranlib $(LTOPLUGIN)" \
					); \
	fi
flac:
	if [ ! -f ./flac-1.4.2/Makefile ];then \
	cd flac-1.4.2 && ./autogen.sh && (./configure --disable-shared --host=$(HOST) --disable-altivec  CC="ccache $(CROSS_COMPILE)gcc" \
				CFLAGS="$(COPTS) $(MIPS16_OPT) $(THUMB) -I$(MINI_DLNA_PATH)/libogg-1.3.5/include/  -ffunction-sections -fdata-sections -Wl,--gc-sections $(LTO)" \
				CPPFLAGS="$(COPTS) $(MIPS16_OPT) $(THUMB) -I$(MINI_DLNA_PATH)/libogg-1.3.5/include/  -ffunction-sections -fdata-sections -Wl,--gc-sections $(LTO)" \
				CXXFLAGS="$(COPTS) $(MIPS16_OPT) $(THUMB) -I$(MINI_DLNA_PATH)/libogg-1.3.5/include/  -ffunction-sections -fdata-sections -Wl,--gc-sections $(LTO)" \
				LDFLAGS="-L$(MINI_DLNA_PATH)/libogg-1.3.5/src/.libs/ $(LDLTO)" \
				AR_FLAGS="cru $(LTOPLUGIN)" RANLIB="$(ARCH)-linux-ranlib $(LTOPLUGIN)" \
				); \
	fi

libid3tag:
	if [ ! -f ./libid3tag-0.15.1b/Makefile ];then \
	cd libid3tag-0.15.1b && libtoolize -ci --force; \
	aclocal; \
	automake --add-missing; \
	autoreconf -fi; \
	(./configure --disable-shared --disable-dependency-tracking --host=$(HOST)  CC="ccache $(CROSS_COMPILE)gcc" \
				CPPFLAGS="$(COPTS) $(MIPS16_OPT) $(THUMB)  -I$(TOP)/zlib/  -ffunction-sections -fdata-sections -Wl,--gc-sections $(LTO)" \
				LDFLAGS="-L$(TOP)/zlib/ $(LDLTO)" \
				AR_FLAGS="cru $(LTOPLUGIN)" RANLIB="$(ARCH)-linux-ranlib $(LTOPLUGIN)" \
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
#	-rm ./lib/libavdevice.a
	-rm ./lib/libavformat.a
	-rm ./lib/libavcodec.a
	-rm ./lib/libsqlite3.a
	-rm ./lib/libjpeg.a
	-rm ./lib/libid3tag.a
	-rm ./lib/minidlna
copy: 
	cp ./libvorbis-1.3.7/lib/.libs/libvorbis.a	./lib/libvorbis.a && \
	cp ./flac-1.4.2/src/libFLAC/.libs/libFLAC-static.a		./lib/libFLAC.a && \
	cp ./libexif-0.6.24/libexif/.libs/libexif.a 		./lib/libexif.a  && \
	cp ./libogg-1.3.5/src/.libs/libogg.a		./lib/libogg.a && \
	cp ./ffmpeg-$(FFMPEG_VER)/libavutil/libavutil.a		./lib/libavutil.a && \
#	cp ./ffmpeg-$(FFMPEG_VER)/libavdevice/libavdevice.a		./lib/libavdevice.a && \
	cp ./ffmpeg-$(FFMPEG_VER)/libavformat/libavformat.a		./lib/libavformat.a && \
	cp ./ffmpeg-$(FFMPEG_VER)/libavcodec/libavcodec.a		./lib/libavcodec.a && \
	cp ./sqlite-3.6.22/.libs/libsqlite3.a 		./lib/libsqlite3.a && \
	cp ./libjpeg-turbo-2.1.4/libturbojpeg.a			./lib/libjpeg.a && \
	cp ./libid3tag-0.15.1b/.libs/libid3tag.a 	./lib/libid3tag.a

distclean:
	-cd ./libogg-1.3.5/	&& make distclean 
	-cd ./libexif-0.6.24/	&& make distclean
	-cd ./sqlite-3.6.22/	&& make distclean
	-cd ./libvorbis-1.3.7/	&& make distclean
	-cd ./libid3tag-0.15.1b/	&& make distclean
	-rm -f ./libid3tag-0.15.1b/Makefile
	-cd ./flac-1.4.2/	&& make distclean
	-rm -f ./flac-1.4.2/Makefile
	-cd ./ffmpeg-$(FFMPEG_VER)/	&& make distclean
	-cd ./libjpeg-turbo-2.1.4/	&& make clean && rm -f CMakeCache.txt
	-cd ./minidlna/		&& make clean
	-rm ./lib/*.so.*
	-rm ./lib/minidlna
	-rm ./ffmpeg-$(FFMPEG_VER)/config.mak

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


