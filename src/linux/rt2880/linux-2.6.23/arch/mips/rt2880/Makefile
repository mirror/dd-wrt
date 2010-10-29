###############################################################################
#  Jan 2007 Bruce Chang
#
#  Initial Release
#
#
#
###############################################################################


.S.s:
	$(CPP) $(CFLAGS) $< -o $*.s
.S.o:
	$(CC) $(CFLAGS) -c $< -o $*.o

obj-y    := mipsIRQ.o reset.o init.o irq.o \
            memory.o printf.o cmdline.o setup.o time.o
obj-$(CONFIG_PCI) += pci.o
obj-$(CONFIG_DWC_OTG) += lm.o
obj-$(CONFIG_RALINK_TIMER_DFS) +=  rt_timer.o

ifdef CONFIG_RT3XXX_EHCI
obj-y +=  ehci_ohci.o
else ifdef CONFIG_RT3XXX_OHCI
obj-y +=  ehci_ohci.o
endif
