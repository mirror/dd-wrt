rtl818x_pci-objs	:= dev.o rtl8225.o sa2400.o max2820.o grf5101.o rtl8225se.o

obj-$(CPTCFG_RTL8180)	+= rtl818x_pci.o

ccflags-y += -I$(backport_srctree)/drivers/net/wireless/realtek/rtl818x
