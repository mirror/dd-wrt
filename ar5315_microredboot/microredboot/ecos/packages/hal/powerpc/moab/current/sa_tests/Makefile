#
# Makefile for simple tests
#

CC = powerpc-eabi-gcc
CFLAGS = -mcpu=405 -msoft-float -O2 -g
LDFLAGS = -msoft-float -mcpu=405 -g -nostdlib -Wl,-static -Wl,-Map,$@.map

TESTS = test_sdram_ocm.elf test_sdram_dram.elf test_flash.elf

.S.o:
	$(CC) $(CFLAGS) -c -o $@ $*.S

all: ${TESTS}

TEST_SDRAM_OBJS = start.o tty.o sdram_test.o
test_sdram_ocm.elf: ${TEST_SDRAM_OBJS}
	$(CC) $(LDFLAGS) -Tppc_ocm.ld -o $@ ${TEST_SDRAM_OBJS}
test_sdram_dram.elf: ${TEST_SDRAM_OBJS}
	$(CC) $(LDFLAGS) -Tppc_dram.ld -o $@ ${TEST_SDRAM_OBJS}

TEST_FLASH_OBJS = start.o tty.o flash_test.o
test_flash.elf: ${TEST_FLASH_OBJS}
	$(CC) $(LDFLAGS) -Tppc_ocm.ld -o $@ ${TEST_FLASH_OBJS}

clean:
	rm -f *.o *~ ${TESTS} *.map *.elf
