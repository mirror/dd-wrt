# Makefile for the PPE Qdisc client

ccflags-y += -I$(obj)/../../exports -I$(obj)/../exports

obj-m += qca-nss-ppe-qdisc.o
qca-nss-ppe-qdisc-objs += ppe_fifo.o \
			ppe_htb.o \
			ppe_prio.o \
			ppe_qdisc.o \
			ppe_qdisc_port.o \
			ppe_qdisc_res.o \
			ppe_qdisc_stats.o \
			ppe_red.o \
			ppe_tbl.o \
			ppe_wrr.o

ccflags-y += -DPPE_QDISC_DEBUG_LEVEL=4
ccflags-y += -Wall -Werror
