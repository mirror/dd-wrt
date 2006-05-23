#
# Makefile for the RB500 board specific parts of the kernel
#

# .S.s:
# 	$(CPP) $(CFLAGS) $< -o $*.s
# .S.o:
# 	$(CC) $(CFLAGS) -c $< -o $*.o

# O_TARGET := rb500.o
# export-objs = prom.o rb.o
obj-y	 += irq.o time.o setup.o rbIRQ.o serial_gdb.o serial.o prom.o rb.o

EXTRA_AFLAGS := $(CFLAGS)
