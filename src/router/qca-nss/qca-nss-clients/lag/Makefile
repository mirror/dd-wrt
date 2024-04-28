# Makefile for the lagmgr client

ifeq ($(SoC),$(filter $(SoC),ipq807x ipq807x_64 ipq60xx ipq60xx_64))
ccflags-y += -DNSS_LAG_PPE_SUPPORT
endif

ccflags-y := -I$(obj) -I$(obj)/..
ccflags-y += -DNSS_CLIENT_BUILD_ID="$(BUILD_ID)"
ccflags-y += -DNSS_LAG_MGR_DEBUG_LEVEL=0
ccflags-y += -Werror

obj-m += qca-nss-lag-mgr.o
qca-nss-lag-mgr-objs := nss_lag.o

obj ?= .
