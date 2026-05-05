###################################################
# Makefile for the NSS EIP IPSEC client
###################################################

EIP_IPSEC_MOD_NAME=qca-nss-eip-ipsec

ccflags-y += -Wall -Werror
ccflags-y += -DNSS_EIP_BUILD_ID=\"'Build_ID - $(shell date +'%m/%d/%y, %H:%M:%S') SoC=$(SoC)'\"

obj-m += $(EIP_IPSEC_MOD_NAME)-$(SoC).o

$(EIP_IPSEC_MOD_NAME)-$(SoC)-objs += eip_ipsec.o
$(EIP_IPSEC_MOD_NAME)-$(SoC)-objs += eip_ipsec_dev.o
$(EIP_IPSEC_MOD_NAME)-$(SoC)-objs += eip_ipsec_sa.o
$(EIP_IPSEC_MOD_NAME)-$(SoC)-objs += eip_ipsec_proto.o
$(EIP_IPSEC_MOD_NAME)-$(SoC)-objs += eip_ipsec_xfrm.o
$(EIP_IPSEC_MOD_NAME)-$(SoC)-objs += eip_ipsec_flow.o

obj ?= .
EIP_IPSEC_INCLUDE = -I$(obj)/../../driver/exports

ccflags-y += $(EIP_IPSEC_INCLUDE)
ccflags-y += -Wall -Werror
ccflags-y += -DEIP_IPSEC_VP_SUPP
