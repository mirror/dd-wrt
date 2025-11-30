CC = mips-linux-gcc
CFLAGS += -static -Os -pipe -mips32r2 -mtune=34kc -msoft-float  -fno-caller-saves  -mno-branch-likely  -minterlink-mips16 -mips16 -DNEED_PRINTF

obj = mtd.o

mtd:
	gcc -o bin2h bin2h.c
	./bin2h -i ../mips-uclibc/aligned.uimage -o image.h -n image
	./bin2h -i ../mips-uclibc/root.uimage -o kernel.h -n kernel
	./bin2h -i ../mips-uclibc/root.fs -o rootfs.h -n rootfs
	/opt/diet/bin/diet $(CC) $(CFLAGS) -o mtd mtd.c
	/opt/diet/bin/diet $(CC) $(CFLAGS) -DSPLIT -o mtd2 mtd.c
	sstrip mtd
	sstrip mtd2
clean:
	rm -f mtd
	rm -f mtd2
