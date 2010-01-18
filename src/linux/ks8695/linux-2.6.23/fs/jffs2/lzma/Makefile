CFLAGS=-Wall -O2 -g
CC=gcc
LDFLAGS=
SOURCES=LzmaDec.c LzmaEnc.c LzFind.c
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=liblzma.a

all: $(SOURCES) $(EXECUTABLE)

#$(EXECUTABLE): $(OBJECTS) 
#	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

liblzma.a: $(OBJECTS)
	ar rcs liblzma.a $(OBJECTS)
	ranlib liblzma.a
		
.c.o: 
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f *.o *.a lzma
