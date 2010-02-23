ROOTDIR=../..
LINUXDIR=../../src/kernels/mips-linux-2.4.25/
CFLAGS += -DUSE_HOSTCC
SRC = crc32.c mkfwimage.c
EXEC = $(subst .c, , $(SRC))

all:$(EXEC)

$(EXEC):
	$(CC) $(CFLAGS) $(LDFLAGS) -o mkfwimage $(SRC) $(LDLIBS)

clean:
	rm -f *.o $(EXEC) mkfwimage


