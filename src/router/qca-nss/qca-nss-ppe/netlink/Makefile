ccflags-y := -Wall -Werror
ccflags-y += -I$(obj)/include -I$(obj)/../exports -I$(obj)/..
ccflags-y += -DNSS_PPENL_DEBUG_LEVEL=4
ccflags-y += -DNSS_CLIENT_BUILD_ID="$(BUILD_ID)"
ccflags-y += -DCONFIG_NSS_PPENL_ACL=1
ccflags-y += -DCONFIG_NSS_PPENL_POLICER=1

qca-nss-ppe-netlink-objs := nss_ppenl.o
qca-nss-ppe-netlink-objs += nss_ppenl_acl.o
qca-nss-ppe-netlink-objs += nss_ppenl_policer.o

obj-m += qca-nss-ppe-netlink.o
