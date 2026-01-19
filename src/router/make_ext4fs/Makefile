CC ?= gcc
CFLAGS += -Iinclude -Ilibsparse/include

ifeq ($(STATIC),1)
  ZLIB := -Wl,-Bstatic -lz -Wl,-Bdynamic
else
  ZLIB := -lz
endif

OBJ := \
    allocate.o \
	canned_fs_config.o \
    contents.o \
    crc16.o \
    ext4fixup.o \
    ext4_sb.o \
    ext4_utils.o \
    extent.o \
    indirect.o \
	make_ext4fs_main.o \
    make_ext4fs.o \
    sha1.o \
    uuid.o \
    wipe.o

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $^

make_ext4fs: $(OBJ) libsparse/libsparse.a
	$(CC) $(LDFLAGS) -o $@ $^ $(ZLIB)

libsparse/libsparse.a:
	$(MAKE) -C libsparse/ libsparse.a

clean:
	$(MAKE) -C libsparse/ clean
	rm -f $(OBJ) make_ext4fs
