ifeq ($(wildcard .config),)
$(error You must run configure before you can build)
endif

include .config

ifeq ($(wildcard $(KDIR)/include),)
$(error Cannot find kernel include directory. Rerun configure! $(KDIR)/include)
endif

SUBDIRS = kernel util

include Subdirs.mak
