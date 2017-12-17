#
# Makefile for msm-bus driver specific files
#
obj-y += msm_bus_bimc.o msm_bus_noc.o msm_bus_core.o msm_bus_client_api.o \
	 msm_bus_id.o
obj-$(CONFIG_OF) += msm_bus_of.o

obj-y += msm_bus_fabric_adhoc.o msm_bus_arb_adhoc.o msm_bus_rules.o
obj-$(CONFIG_OF) += msm_bus_of_adhoc.o
obj-$(CONFIG_CORESIGHT) +=  msm_buspm_coresight_adhoc.o

obj-$(CONFIG_DEBUG_FS) += msm_bus_dbg.o
