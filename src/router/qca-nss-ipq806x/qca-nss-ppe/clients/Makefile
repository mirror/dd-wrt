# Makefile for the PPE driver

ccflags-y := -I$(obj) -I$(obj)/.. -I$(obj)/../..

obj-$(bridge-mgr)+= bridge/
obj-$(vlan-mgr)+= vlan/
obj-$(pppoe-mgr)+= pppoe/
obj-$(lag-mgr)+= lag/
obj-$(tunipip6-mgr)+= tunipip6/
obj-$(gretap-mgr)+= gretap/
obj-$(vxlanmgr)+= vxlanmgr/
obj-$(mapt-mgr)+= mapt/
obj-$(capwap-mgr)+= capwapmgr/
obj-$(dtls-mgr)+= dtlsmgr/
obj-$(l2tp-mgr)+= l2tp/
obj-$(pvxlan-mgr)+= pvxlanmgr/

obj ?= .
