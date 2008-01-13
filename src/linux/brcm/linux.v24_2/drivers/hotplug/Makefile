#
# Makefile for the Linux kernel pci hotplug controller drivers.
#

O_TARGET	:= vmlinux-obj.o

list-multi	:= cpqphp.o pci_hotplug.o ibmphp.o acpiphp.o shpchp.o pciehp.o 

export-objs	:= pci_hotplug_core.o pci_hotplug_util.o

obj-$(CONFIG_HOTPLUG_PCI)		+= pci_hotplug.o
obj-$(CONFIG_HOTPLUG_PCI_COMPAQ)	+= cpqphp.o
obj-$(CONFIG_HOTPLUG_PCI_IBM)		+= ibmphp.o
obj-$(CONFIG_HOTPLUG_PCI_ACPI)		+= acpiphp.o
obj-$(CONFIG_HOTPLUG_PCI_SHPC)		+= shpchp.o
obj-$(CONFIG_HOTPLUG_PCI_PCIE)		+= pciehp.o

pci_hotplug-objs	:=	pci_hotplug_core.o	\
				pci_hotplug_util.o

cpqphp-objs		:=	cpqphp_core.o	\
				cpqphp_ctrl.o	\
				cpqphp_pci.o

ibmphp-objs		:=	ibmphp_core.o	\
				ibmphp_ebda.o	\
				ibmphp_pci.o	\
				ibmphp_res.o	\
				ibmphp_hpc.o

acpiphp_objs		:=	acpiphp_core.o	\
				acpiphp_glue.o	\
				acpiphp_pci.o	\
				acpiphp_res.o

pciehp-objs		:=	pciehp_core.o	\
				pciehp_ctrl.o	\
				pciehp_hpc.o	\
				pciehp_pci.o
ifdef CONFIG_ACPI_INTERPRETER
  pciehp-objs += pciehprm_acpi.o
else
  pciehp-objs += pciehprm_nonacpi.o
  EXTRA_CFLAGS  += -D_LINUX -I$(TOPDIR)/drivers/acpi 
endif

shpchp-objs		:=	shpchp_core.o	\
				shpchp_ctrl.o	\
				shpchp_hpc.o	\
				shpchp_pci.o
ifdef CONFIG_ACPI_INTERPRETER
  shpchp-objs += shpchprm_acpi.o
  EXTRA_CFLAGS  += -D_LINUX -I$(TOPDIR)/drivers/acpi 
else
  ifdef CONFIG_HOTPLUG_PCI_SHPC_PHPRM_LEGACY
    shpchp-objs += shpchprm_legacy.o
  else
    shpchp-objs += shpchprm_nonacpi.o
  endif
endif

ifeq ($(CONFIG_HOTPLUG_PCI_COMPAQ_NVRAM),y)
	cpqphp-objs += cpqphp_nvram.o
endif

ifeq ($(CONFIG_PROC_FS),y)
	cpqphp-objs += cpqphp_proc.o
endif

include $(TOPDIR)/Rules.make

pci_hotplug.o: $(pci_hotplug-objs)
	$(LD) -r -o $@ $(pci_hotplug-objs)

cpqphp.o: $(cpqphp-objs)
	$(LD) -r -o $@ $(cpqphp-objs)

ibmphp.o: $(ibmphp-objs)
	$(LD) -r -o $@ $(ibmphp-objs)

acpiphp.o: $(acpiphp_objs)
	$(LD) -r -o $@ $(acpiphp_objs)

pciehp.o: $(pciehp-objs)
	$(LD) -r -o $@ $(pciehp-objs)

shpchp.o: $(shpchp-objs)
	$(LD) -r -o $@ $(shpchp-objs)

