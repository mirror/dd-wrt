ccflags-y := -I$(obj)/../exports -I$(srctree)/net/bridge -DNSS_QDISC_DEBUG_LEVEL=2

# If NSS_QDISC_PPE_SUPPORT is removed, then remove/disable
# nss_ppe.o nss_ppe_mc.o from qca-nss-qdisc-objs list.
ifeq ($(SoC),$(filter $(SoC),ipq807x ipq807x_64 ipq60xx ipq60xx_64))
ccflags-y += -DNSS_QDISC_PPE_SUPPORT -DNSS_QDISC_BRIDGE_SUPPORT
endif

ccflags-y += -Wall -Werror

obj-m += qca-nss-qdisc.o
qca-nss-qdisc-objs := 	nss_qdisc.o \
			nss_fifo.o \
			nss_codel.o \
			nss_tbl.o \
			nss_prio.o \
			nss_bf.o \
			nss_wrr.o \
			nss_htb.o \
			nss_blackhole.o \
			nss_wred.o

ifeq ($(SoC),$(filter $(SoC),ipq807x ipq807x_64 ipq60xx ipq60xx_64))
qca-nss-qdisc-objs += 	nss_ppe.o \
			nss_ppe_mc.o
endif

#obj-$(igs)+= igs/
