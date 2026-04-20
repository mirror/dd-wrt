# Makefile for PPE RULE module

ccflags-y := -I$(obj)/../exports -I$(obj)/..
ccflags-y += -DPPE_DRV_BUILD_ID="$(BUILD_ID)"
ccflags-y += -Wall -Werror

ifeq ($(PPE_RULE_IPQ53XX), y)
ccflags-y += -DNSS_PPE_RULE_IPQ53XX
endif

ifeq ($(PPE_LOWMEM_PROFILE_16M), y)
ccflags-y += -DNSS_PPE_LOWMEM_PROFILE_16M
endif

ifeq ($(PPE_LOWMEM_PROFILE_256M), y)
ccflags-y += -DNSS_PPE_LOWMEM_PROFILE_256M
endif

obj-m += qca-nss-ppe-rule.o
qca-nss-ppe-rule-objs := ppe_rule.o

ifeq ($(PPE_RFS_ENABLED), y)
qca-nss-ppe-rule-objs += ppe_rfs/ppe_rfs.o
qca-nss-ppe-rule-objs += ppe_rfs/ppe_rfs_stats.o
ccflags-y += -DPPE_RFS_DEBUG_LEVEL=3
endif

ifndef PPE_LOWMEM_PROFILE_16M
ifeq ($(PPE_ACL_ENABLED), y)
qca-nss-ppe-rule-objs += ppe_acl/ppe_acl.o
qca-nss-ppe-rule-objs += ppe_acl/ppe_acl_dump.o
qca-nss-ppe-rule-objs += ppe_acl/ppe_acl_stats.o
ccflags-y += -DPPE_ACL_DEBUG_LEVEL=3
endif

ifeq ($(PPE_POLICER_ENABLED), y)
qca-nss-ppe-rule-objs += ppe_policer/ppe_policer.o
qca-nss-ppe-rule-objs += ppe_policer/ppe_policer_stats.o
ccflags-y += -DPPE_POLICER_DEBUG_LEVEL=3
endif
endif

ifndef PPE_LOWMEM_PROFILE_16M
ifndef PPE_LOWMEM_PROFILE_256M
ifeq ($(PPE_MIRROR_ENABLED), y)
qca-nss-ppe-rule-objs += ppe_mirror/ppe_mirror.o
qca-nss-ppe-rule-objs += ppe_mirror/ppe_mirror_stats.o
ccflags-y += -DPPE_MIRROR_DEBUG_LEVEL=3
endif

ifeq ($(PPE_PRIORITY_ENABLED), y)
qca-nss-ppe-rule-objs += ppe_priority/ppe_priority.o
qca-nss-ppe-rule-objs += ppe_priority/ppe_priority_stats.o
ccflags-y += -DPPE_PRIORITY_DEBUG_LEVEL=3
endif
endif
endif
