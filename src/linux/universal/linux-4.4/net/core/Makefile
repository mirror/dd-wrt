ifeq ($(CONFIG_MIPS),y)
ifeq ($(CONFIG_CPU_MIPS32_R1),y)
EXTRA_CFLAGS+=-I$(srctree)/brcm/mipsel_r1/include -I$(srctree)/brcm/mipsel_r1/shared/bcmwifi/include -DBCMDRIVER -Dlinux
EXTRA_AFLAGS+=-I$(srctree)/brcm/mipsel_r1/include -I$(srctree)/brcm/mipsel_r1/shared/bcmwifi/include -DBCMDRIVER -Dlinux
EXTRA_CFLAGS+=-I$(srctree)/brcm/mipsel_r1/common/include
EXTRA_AFLAGS+=-I$(srctree)/brcm/mipsel_r1/common/include
else
EXTRA_CFLAGS+=-I$(srctree)/brcm/mipsel/include -I$(srctree)/brcm/mipsel/shared/bcmwifi/include -DBCMDRIVER -Dlinux
EXTRA_AFLAGS+=-I$(srctree)/brcm/mipsel/include -I$(srctree)/brcm/mipsel/shared/bcmwifi/include -DBCMDRIVER -Dlinux
EXTRA_CFLAGS+=-I$(srctree)/brcm/mipsel/common/include
EXTRA_AFLAGS+=-I$(srctree)/brcm/mipsel/common/include
endif
else
EXTRA_CFLAGS+=-I$(srctree)/brcm/arm/include -I$(srctree)/brcm/arm/shared/bcmwifi/include -DBCMDRIVER -Dlinux
EXTRA_AFLAGS+=-I$(srctree)/brcm/arm/include -I$(srctree)/brcm/arm/shared/bcmwifi/include -DBCMDRIVER -Dlinux
EXTRA_CFLAGS+=-I$(srctree)/brcm/arm/common/include
EXTRA_AFLAGS+=-I$(srctree)/brcm/arm/common/include
endif
#
# Makefile for the Linux networking core.
#
EXTRA_CFLAGS+=-g

obj-y := sock.o request_sock.o skbuff.o datagram.o stream.o scm.o \
	 gen_stats.o gen_estimator.o net_namespace.o secure_seq.o flow_dissector.o

obj-$(CONFIG_SYSCTL) += sysctl_net_core.o

obj-y		     += dev.o ethtool.o dev_addr_lists.o dst.o netevent.o \
			neighbour.o rtnetlink.o utils.o link_watch.o filter.o dev_ioctl.o tso.o

obj-$(CONFIG_SOCK_DIAG) += sock_diag.o
obj-$(CONFIG_XFRM) += flow.o
obj-y += net-sysfs.o
obj-$(CONFIG_PROC_FS) += net-procfs.o
obj-$(CONFIG_NET_PKTGEN) += pktgen.o
obj-$(CONFIG_NETPOLL) += netpoll.o
obj-$(CONFIG_FIB_RULES) += fib_rules.o
obj-$(CONFIG_TRACEPOINTS) += net-traces.o
obj-$(CONFIG_NET_DROP_MONITOR) += drop_monitor.o
obj-$(CONFIG_NETWORK_PHY_TIMESTAMPING) += timestamping.o
obj-$(CONFIG_NET_PTP_CLASSIFY) += ptp_classifier.o
obj-$(CONFIG_CGROUP_NET_PRIO) += netprio_cgroup.o
obj-$(CONFIG_CGROUP_NET_CLASSID) += netclassid_cgroup.o
obj-$(CONFIG_LWTUNNEL) += lwtunnel.o
obj-$(CONFIG_DST_CACHE) += dst_cache.o
