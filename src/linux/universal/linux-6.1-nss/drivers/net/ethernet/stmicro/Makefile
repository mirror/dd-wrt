# SPDX-License-Identifier: GPL-2.0-only
#
# Makefile for the STMicroelectronics device drivers.
#


ifeq ($(CONFIG_ARCH_QCOM),y)
obj-$(CONFIG_STMMAC_ETH) += stmmac_qcom/
else
obj-$(CONFIG_STMMAC_ETH) += stmmac/
endif
