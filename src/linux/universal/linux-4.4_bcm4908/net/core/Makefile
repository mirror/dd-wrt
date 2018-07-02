#
# Makefile for the Linux networking core.
#

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
ifdef CONFIG_BCM_KF_NBUFF
EXTRA_CFLAGS	+= -I$(srctree)/include/asm-generic -I$(srctree)/arch/arm64/brcm/include/bcm963xx -I$(srctree)/arch/arm64/brcm/include/pmc
#EXTRA_CFLAGS	+= -I $(TOPDIR)/include/asm-generic -I$(INC_BRCMDRIVER_PUB_PATH)/$(BRCM_BOARD) -I$(INC_BRCMSHARED_PUB_PATH)/$(BRCM_BOARD) -I$(INC_BRCMSHARED_PUB_PATH)/../flash
endif # CONFIG_BCM_KF_MTD_BCM963XX
ifdef CONFIG_BCM_KF_BLOG
obj-$(CONFIG_BCM_KF_BLOG) += blog.o blog_rule.o 
obj-$(CONFIG_BCM_KF_VLANCTL_BIND) += vlanctl_bind.o
endif # BCM_KF
ifdef CONFIG_BCM_KF_NBUFF
obj-y += nbuff.o iqos.o gbpm.o flwstif.o
endif # BCM_KF

ifdef CONFIG_BCM_KF_DPI
obj-y += devinfo.o dpistats.o urlinfo.o
endif # BCM_KF

