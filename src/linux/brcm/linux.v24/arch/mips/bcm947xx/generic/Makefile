#
# Makefile for the BCM947xx specific kernel interface routines
# under Linux.
#

.S.s:
	$(CPP) $(AFLAGS) $< -o $*.s
.S.o:
	$(CC) $(AFLAGS) -c $< -o $*.o

O_TARGET        := brcm.o

obj-y	:= int-handler.o irq.o

include $(TOPDIR)/Rules.make
