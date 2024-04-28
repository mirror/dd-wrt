TOOL_MOD_NAME=qca-nss-crypto-tool

obj-m	+= $(TOOL_MOD_NAME).o
$(TOOL_MOD_NAME)-objs = nss_crypto_bench.o

obj ?= .
#ccflags-y += -DCONFIG_NSS_CRYPTO_TOOL_DBG
ccflags-y += -DNSS_CRYPTO_BUILD_ID=\"'Build_ID - $(shell date +'%m/%d/%y, %H:%M:%S')'\"
ccflags-y += -I$(obj)/../include -I$(obj)/../src -I$(obj)/
ccflags-y += -Werror