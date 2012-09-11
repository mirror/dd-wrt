obj-m = lantiq_mei.o lantiq_atm.o
lantiq_atm-objs := ifxmips_atm_core.o

ifeq ($(BUILD_VARIANT),danube)
  CFLAGS_MODULE+=-DCONFIG_DANUBE
  EXTRA_CFLAGS+=-DCONFIG_DANUBE
  lantiq_atm-objs += ifxmips_atm_danube.o
endif
ifeq ($(BUILD_VARIANT),ase)
  CFLAGS_MODULE+=-DCONFIG_AMAZON_SE
  EXTRA_CFLAGS+=-DCONFIG_AMAZON_SE
  lantiq_atm-objs += ifxmips_atm_amazon_se.o
endif
ifeq ($(BUILD_VARIANT),ar9)
  CFLAGS_MODULE+=-DCONFIG_AR9
  EXTRA_CFLAGS+=-DCONFIG_AR9
  lantiq_atm-objs += ifxmips_atm_ar9.o
endif
ifeq ($(BUILD_VARIANT),vr9)
  CFLAGS_MODULE+=-DCONFIG_VR9
  EXTRA_CFLAGS+=-DCONFIG_VR9
  lantiq_atm-objs += ifxmips_atm_vr9.o
endif

EXTRA_CFLAGS += -DHAVE_CONFIG_H -DLINUX -D__LINUX__ -D__KERNEL__ -DEXPORT_SYMTAB -Idrivers/net/ethernet/ifxatm/ifxos -Idrivers/net/ethernet/ifxatm/include -Idrivers/net/ethernet/ifxatm
ifeq ($(CONFIG_SOC_VR9),y)
EXTRA_CFLAGS += -DCONFIG_VR9
CFLAGS_MODULE+=-DCONFIG_VR9
lantiq_atm-objs += ifxmips_atm_vr9.o
endif
ifeq ($(CONFIG_AR9),y)
EXTRA_CFLAGS += -DCONFIG_AR9
CFLAGS_MODULE+=-DCONFIG_AR9
lantiq_atm-objs += ifxmips_atm_ar9.o
else 
ifeq ($(CONFIG_SOC_XWAY),y)
CFLAGS_MODULE+=-DCONFIG_DANUBE
EXTRA_CFLAGS += -DCONFIG_DANUBE
lantiq_atm-objs += ifxmips_atm_danube.o
endif
endif
