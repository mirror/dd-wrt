#
# Makefile for the linux kernel.
#

ccflags-$(CONFIG_ARCH_MULTIPLATFORM) := -I$(srctree)/$(src)/include \
        -I$(srctree)/arch/arm/plat-orion/include

KBUILD_CPPFLAGS += -Iarch/arm/mach-alpine/include/al_hal \
				   -Iarch/arm/mach-alpine/include/mach

obj-y					+= alpine_machine.o
obj-y					+= alpine_cpu_pm.o
obj-$(CONFIG_PCI)			+= pcie_of.o
obj-y					+= al_fabric.o
obj-y 					+= al_hal/
obj-$(CONFIG_SMP)			+= platsmp.o
obj-$(CONFIG_HOTPLUG_CPU)		+= sleep-alpine.o
obj-$(CONFIG_PCI_MSI)			+= msix.o

# only build sleep-alpine once
ifneq ($(CONFIG_HOTPLUG_CPU), y)
obj-$(CONFIG_CPU_IDLE_ALPINE)   += sleep-alpine.o
endif
