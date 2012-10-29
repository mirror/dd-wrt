CC = gcc
CXX = g++
#OPT = -fast

OPT = -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE -funroll-loops -mtune=opteron -march=opteron -mfpmath=sse -m3dnow -msse2 -Ofast -D_LZMA_PARAMS -D_7ZIP_ST
#OPT += -O3 -no-prec-div -ipo -msse2 -xSSE2 -finline-functions
INCLUDEDIR = .

CFLAGS := -I$(INCLUDEDIR)  $(OPT) -D_LZMA_PARAMS -pthread
CXXFLAGS := -I$(INCLUDEDIR) $(OPT) -D_LZMA_PARAMS -pthread
CPPFLAGS := -I$(INCLUDEDIR)  $(OPT) -D_LZMA_PARAMS -pthread

OBJS = \
  lzmalib/Alloc.o \
  lzmalib/LzFind.o \
  lzmalib/LzmaDec.o \
  lzmalib/LzmaEnc.o \
  lzmalib/LzmaLib.o \
  ZLib.o 

all: mksquashfs-lzma unsquashfs-lzma

mksquashfs: mksquashfs.o read_fs.o sort.o 
	$(CC) $(OPT)  mksquashfs.o read_fs.o sort.o -lz -o $@

mksquashfs-lzma: mksquashfs.o read_fs.o sort.o $(OBJS)
	$(CXX) $(OPT) mksquashfs.o read_fs.o sort.o $(OBJS) -lpthread -o $@

mksquashfs.o: mksquashfs.c squashfs_fs.h mksquashfs.h global.h sort.h

read_fs.o: read_fs.c squashfs_fs.h read_fs.h global.h

sort.o: sort.c squashfs_fs.h global.h sort.h

unsquashfs: unsquashfs.o
	$(CC) $(OPT) unsquashfs.o -lz -o $@

unsquashfs-lzma: unsquashfs.o
	$(CC) $(OPT) unsquashfs.o $(OBJS) -lpthread -o $@ 

unsquashfs.o: unsquashfs.c squashfs_fs.h read_fs.h global.h

clean:
	rm -f *.o 
	rm -f lzmalib/*.o 
	rm -f mksquashfs-lzma
	rm -f unsquashfs-lzma
