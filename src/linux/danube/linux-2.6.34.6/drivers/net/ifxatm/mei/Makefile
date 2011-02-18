obj-m = lantiq_mei.o lantiq_atm.o

lantiq_atm-objs := ifxmips_atm_core.o 
ifeq ($(CONFIG_AR9),y)
lantiq_atm-objs += ifxmips_atm_ar9.o
else
lantiq_atm-objs += ifxmips_atm_danube.o
endif
EXTRA_CFLAGS += -DHAVE_CONFIG_H -DLINUX -D__LINUX__ -D__KERNEL__ -DEXPORT_SYMTAB -Idrivers/net/ifxatm/ifxos -Idrivers/net/ifxatm/include -Idrivers/net/ifxatm
