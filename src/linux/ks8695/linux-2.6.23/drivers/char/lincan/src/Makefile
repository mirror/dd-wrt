SUPPORTED_CARDS = pcan_pci vscan
#pip pccan smartcan nsi cc_can104 \
	          pc_i03 pcm3680 aim104 m437 pcccan ssv \
		  bfadcan pikronisa kv_pcican msmcan virtual template \
		  unican unican_cl2 ems_cpcpci adlink7841 oscar \
		  esdpci200
#		  hms30c7202_can c_can c_can_irq tscan1
#		  pcan_dongle

O_OBJS	     += $(SUPPORTED_CARDS:%=%.o)
O_OBJS       += can_queue.o can_quekern.o devcommon.o main.o modparms.o \
		setup.o finish.o irq.o sysdep_lnx.o boardlist.o \
		sja1000p.o sja1000.o i82527.o  \
	        open.o proc.o close.o write.o read.o \
		ioctl.o ioctl_query.o ioctl_remote.o select.o fasync.o
list-multi := can.o
can-objs := $(O_OBJS)

can.o: $(can-objs)
	$(LD) -r -o $@ $(can-objs)

obj-$(CONFIG_LINCAN) 		+= can.o
#include $(TOPDIR)/Rules.make
