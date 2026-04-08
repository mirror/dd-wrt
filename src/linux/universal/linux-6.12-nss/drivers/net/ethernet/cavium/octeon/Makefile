#
# Makefile for the Cavium network device drivers.
#

ccflags-y := -DCVMX_BUILD_FOR_LINUX_KERNEL -DCVMX_ENABLE_CSR_ADDRESS_CHECKING=0 -DOCTEON_MODEL=OCTEON_CN70XX -DCVMX_ENABLE_PARAMETER_CHECKING=0
obj-$(CONFIG_OCTEON_BGX_PORT)		+= octeon-bgx-port.o
obj-$(CONFIG_OCTEON_BGX_NEXUS)		+= octeon-bgx-nexus.o
obj-$(CONFIG_OCTEON_COMMON_NEXUS)	+= octeon-common-nexus.o
obj-$(CONFIG_OCTEON_SRIO_NEXUS)		+= octeon-srio-nexus.o
obj-$(CONFIG_OCTEON3_ETHERNET)		+= octeon3-ethernet.o
obj-$(CONFIG_OCTEON_MGMT_ETHERNET)	+= octeon_mgmt.o
obj-$(CONFIG_OCTEON_POW_ONLY_ETHERNET)	+= octeon-pow-ethernet.o
obj-$(CONFIG_OCTEON_ETHERNET)		+= octeon-ethernet.o
obj-$(CONFIG_OCTEON_ETHERNET_MEM)	+= ethernet-mem.o
obj-$(CONFIG_OCTEON_ETHERNET_COMMON)	+= octeon_common.o

octeon-ethernet-objs := ethernet.o
octeon-ethernet-objs += ethernet-mdio.o
octeon-ethernet-objs += ethernet-rgmii.o
octeon-ethernet-objs += ethernet-rx.o
octeon-ethernet-objs += ethernet-sgmii.o
octeon-ethernet-objs += ethernet-spi.o
octeon-ethernet-objs += ethernet-tx.o
ifdef CONFIG_RAPIDIO
octeon-ethernet-objs += ethernet-srio.o
endif
