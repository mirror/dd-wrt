#
# Makefile for the linux kernel.
#

obj-y					:= core.o lm.o
obj-$(CONFIG_MACH_GW2388)		+= laguna-setup.o
obj-$(CONFIG_SMP)			+= platsmp.o headsmp.o
obj-$(CONFIG_HOTPLUG_CPU)		+= hotplug.o
obj-$(CONFIG_LOCAL_TIMERS)		+= localtimer.o
obj-$(CONFIG_PCIEPORTBUS)		+= pcie.o
obj-$(CONFIG_CNS3XXX_RAID)			+= rdma.o
obj-$(CONFIG_CNS3XXX_DMAC)		+= dmac.o
obj-$(CONFIG_CNS3XXX_PM_API)	+= pm.o
#CFLAGS_pm.o = -O0
