CC = gcc
CFLAGS = -W -Wall -O2 -g
CPPFLAGS = -I../../include
LDLIBS = -lz
PROGS = mkcramfs #cramfsck

USE_LZMA=y

ifdef USE_LZMA
MK_OBJS = MemoryInOutStreams.o IInOutStreams.o 7zlzma.o LZMA.o LZMAEncoder.o AriBitCoder.o OutByte.o WindowIn.o LiteralCoder.o LenCoder.o
FS_OBJS = 7zlzma.o IInOutStreams.o LZMADecoder.o
CFLAGS += -DLZMA_SUPPORT

vpath %.cc lzma
vpath %.o lzma
endif

all: $(PROGS)

mkcramfs: mkcramfs.o $(MK_OBJS)
	g++ mkcramfs.o $(CFLAGS) $(MK_OBJS) $(LDLIBS) -o mkcramfs

distclean clean:
	rm -f $(PROGS) *.o

.PHONY: all clean
