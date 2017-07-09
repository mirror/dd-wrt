obj-m += fast-classifier.o

ifeq ($(CONFIG_IPV6),y)
SFE_SUPPORT_IPV6=y
endif
ifeq ($(CONFIG_IPV6),m)
SFE_SUPPORT_IPV6=y
endif

ccflags-$(SFE_SUPPORT_IPV6) += -DSFE_SUPPORT_IPV6

ccflags-y += -I$(obj)/../shortcut-fe

obj ?= .
