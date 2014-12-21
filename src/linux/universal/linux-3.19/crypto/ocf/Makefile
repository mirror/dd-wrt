
OCF_OBJS = crypto.o criov.o

ifdef CONFIG_OCF_RANDOMHARVEST
	OCF_OBJS += random.o
endif

ifdef CONFIG_OCF_FIPS
	OCF_OBJS += rndtest.o
endif

EXTRA_CFLAGS += -I$(obj)/.

obj-$(CONFIG_OCF_OCF)         += ocf.o
obj-$(CONFIG_OCF_CRYPTODEV)   += cryptodev.o
obj-$(CONFIG_OCF_CRYPTOSOFT)  += cryptosoft.o
obj-$(CONFIG_OCF_BENCH)       += ocf-bench.o

obj-$(CONFIG_OCF_SAFE)    += safe/
obj-$(CONFIG_OCF_HIFN)    += hifn/
obj-$(CONFIG_OCF_IXP4XX)  += ixp4xx/
obj-$(CONFIG_OCF_TALITOS) += talitos/
obj-$(CONFIG_OCF_PASEMI)  += pasemi/
obj-$(CONFIG_OCF_EP80579) += ep80579/
obj-$(CONFIG_OCF_CRYPTOCTEON) += cryptocteon/
obj-$(CONFIG_OCF_KIRKWOOD) += kirkwood/
obj-$(CONFIG_OCF_OCFNULL) += ocfnull/
obj-$(CONFIG_OCF_C7108) += c7108/
obj-$(CONFIG_OCF_UBSEC_SSB) += ubsec_ssb/

ocf-objs := $(OCF_OBJS)

