ccflags-y := -I$(obj) -I$(obj)/.. -I$(obj)/../exports -DNSS_DEBUG_LEVEL=0 #-DPROFILE_DEBUG
ccflags-y += -Werror

obj-m += qca-nss-profile-drv.o
qca-nss-profile-drv-objs := profile.o

ccflags-y += $(NSS_CCFLAGS)
