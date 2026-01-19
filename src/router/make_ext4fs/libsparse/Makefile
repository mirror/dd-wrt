CC ?= gcc
AR ?= ar

CFLAGS += -Iinclude

OBJ := \
		backed_block.o \
		output_file.o \
		sparse.o \
		sparse_crc32.o \
		sparse_err.o \
		sparse_read.o

%.o: %.c
		$(CC) $(CFLAGS) -c -o $@ $^

libsparse.a: $(OBJ)
		$(AR) rcs $@ $^

clean:
		rm -f $(OBJ) libsparse.a
