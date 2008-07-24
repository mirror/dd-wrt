#
# Makefile for the linux kernel.
#

# Object file lists.

obj-y			:= arch.o irq.o mm.o time.o sl3516_device.o
obj-m			:=
obj-n			:=


obj-$(CONFIG_PCI) += pci.o
obj-$(CONFIG_SL2312_LPC) += lpc.o
obj-$(CONFIG_SL2312_USB) += sl2312-otg.o # sl2312-otg-1.o
obj-$(CONFIG_GEMINI_XOR_ACCE) += xor.o
obj-$(CONFIG_GEMINI_IPI)      += gemini_ipi.o
