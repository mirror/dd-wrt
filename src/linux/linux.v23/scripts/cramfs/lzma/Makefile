ALL_OBJECTS = \
	7z.o           DeflateDecoder.o  InByte.o       LZMADecoder.o   MemoryInOutStreams.o  WindowOut.o \
	7zlzma.o       DeflateEncoder.o  LSBFDecoder.o  LZMAEncoder.o   OutByte.o             cmdline.o \
	AriBitCoder.o  HuffmanEncoder.o  LSBFEncoder.o  LenCoder.o   getopt.o \
	CRC.o          IInOutStreams.o   LZMA.o         LiteralCoder.o  WindowIn.o            getopt1.o 

CFLAGS=-g -Wall
CXXFLAGS=-g -Wall
OBJECTS = \
	7z.o MemoryInOutStreams.o IInOutStreams.o 7zlzma.o LZMA.o LZMAEncoder.o AriBitCoder.o \
	OutByte.o WindowIn.o LiteralCoder.o LenCoder.o

lzma: $(OBJECTS)
	g++ -g $(OBJECTS) -o lzma

