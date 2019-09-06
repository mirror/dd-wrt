#
# Makefile for the kernel DVB device drivers.
#

dvb-net-$(CPTCFG_DVB_NET) := dvb_net.o

dvb-core-objs := dvbdev.o dmxdev.o dvb_demux.o		 	\
		 dvb_ca_en50221.o dvb_frontend.o 		\
		 $(dvb-net-y) dvb_ringbuffer.o dvb_math.o

obj-$(CPTCFG_DVB_CORE) += dvb-core.o
