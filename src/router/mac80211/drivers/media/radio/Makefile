#
# Makefile for the kernel character device drivers.
#

obj-$(CPTCFG_RADIO_ISA) += radio-isa.o
obj-$(CPTCFG_RADIO_AZTECH) += radio-aztech.o
obj-$(CPTCFG_RADIO_RTRACK2) += radio-rtrack2.o
obj-$(CPTCFG_RADIO_SF16FMI) += radio-sf16fmi.o
obj-$(CPTCFG_RADIO_SF16FMR2) += radio-sf16fmr2.o
obj-$(CPTCFG_RADIO_CADET) += radio-cadet.o
obj-$(CPTCFG_RADIO_TYPHOON) += radio-typhoon.o
obj-$(CPTCFG_RADIO_TERRATEC) += radio-terratec.o
obj-$(CPTCFG_RADIO_MAXIRADIO) += radio-maxiradio.o
obj-$(CPTCFG_RADIO_SHARK) += radio-shark.o
obj-$(CPTCFG_RADIO_SHARK2) += shark2.o
obj-$(CPTCFG_RADIO_RTRACK) += radio-aimslab.o
obj-$(CPTCFG_RADIO_ZOLTRIX) += radio-zoltrix.o
obj-$(CPTCFG_RADIO_GEMTEK) += radio-gemtek.o
obj-$(CPTCFG_RADIO_TRUST) += radio-trust.o
obj-$(CPTCFG_RADIO_SI476X) += radio-si476x.o
obj-$(CPTCFG_RADIO_MIROPCM20) += radio-miropcm20.o
obj-$(CPTCFG_USB_DSBR) += dsbr100.o
obj-$(CPTCFG_RADIO_SI470X) += si470x/
obj-$(CPTCFG_RADIO_SI4713) += si4713/
obj-$(CPTCFG_USB_MR800) += radio-mr800.o
obj-$(CPTCFG_USB_KEENE) += radio-keene.o
obj-$(CPTCFG_USB_MA901) += radio-ma901.o
obj-$(CPTCFG_RADIO_TEA5764) += radio-tea5764.o
obj-$(CPTCFG_RADIO_SAA7706H) += saa7706h.o
obj-$(CPTCFG_RADIO_TEF6862) += tef6862.o
obj-$(CPTCFG_RADIO_TIMBERDALE) += radio-timb.o
obj-$(CPTCFG_RADIO_WL1273) += radio-wl1273.o
obj-$(CPTCFG_RADIO_WL128X) += wl128x/
obj-$(CPTCFG_RADIO_TEA575X) += tea575x.o
obj-$(CPTCFG_USB_RAREMONO) += radio-raremono.o

shark2-objs := radio-shark2.o radio-tea5777.o

ccflags-y += -Isound
