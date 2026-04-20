ccflags-y := -I$(obj)/../exports -I$(srctree)/net/bridge -DNSS_QDISC_DEBUG_LEVEL=2

# If NSS_QDISC_PPE_SUPPORT is removed, then remove/disable
# nss_ppe.o nss_ppe_mc.o from qca-nss-qdisc-objs list.
ifeq ($(SoC),$(filter $(SoC),ipq807x ipq807x_64 ipq60xx ipq60xx_64))
ccflags-y += -DNSS_QDISC_PPE_SUPPORT -DNSS_QDISC_BRIDGE_SUPPORT
endif
ccflags-y += -DNSS_CLIENT_BUILD_ID="$(BUILD_ID)"

ccflags-y += -Wall -Werror

obj-m += qca-nss-qdisc-$(SoC).o
qca-nss-qdisc-$(SoC)-objs := 	nss_bf.o \
			nss_blackhole.o \
			nss_codel.o \
			nss_fifo.o \
			nss_htb.o \
			nss_prio.o \
			nss_qdisc.o \
			nss_qdisc_htable.o \
			nss_qdisc_stats.o \
			nss_tbl.o \
			nss_wred.o \
			nss_wrr.o

ifeq ($(SoC),$(filter $(SoC),ipq807x ipq807x_64 ipq60xx ipq60xx_64))
qca-nss-qdisc-$(SoC)-objs += 	nss_ppe.o \
			nss_ppe_mc.o
endif

obj-$(igs)+= igs/
