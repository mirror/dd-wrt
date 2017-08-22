CC = mips-linux-gcc
CFLAGS += -static -Os -pipe -mips32r2 -mtune=34kc -msoft-float  -fno-caller-saves  -mno-branch-likely  -minterlink-mips16 -mips16  -ffunction-sections -fdata-sections -Wl,--gc-sections -DNEED_PRINTF

obj = mtd.o

mtd:
	gcc -o bin2h bin2h.c
	./bin2h -i ../mips-uclibc/aligned.uimage -o image.h -n image
	$(CC) $(CFLAGS) -o mtd mtd.c
	sstrip mtd
clean:
	rm -f mtd
