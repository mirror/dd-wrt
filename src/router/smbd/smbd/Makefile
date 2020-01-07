#ifneq ($(KERNELRELEASE),)
# For kernel build

# CONFIG_SMB_SERVER_SMBDIRECT is supported in the kernel above 4.12 version.
SMBDIRECT_SUPPORTED = $(shell [ $(VERSION) -gt 4 -o \( $(VERSION) -eq 4 -a \
		      $(PATCHLEVEL) -gt 12 \) ] && echo y)

ifeq "$(CONFIG_SMB_SERVER_SMBDIRECT)" "y"
ifneq "$(call SMBDIRECT_SUPPORTED)" "y"
$(error CONFIG_SMB_SERVER_SMBDIRECT is supported in the kernel above 4.12 version)
endif
endif

EXTRA_CFLAGS += -DCONFIG_SMB_INSECURE_SERVER=1
obj-$(CONFIG_SMB_SERVER) += smbd.o

smbd-y :=	unicode.o auth.o vfs.o vfs_cache.o \
		misc.o oplock.o netmisc.o \
		mgmt/smbd_ida.o mgmt/user_config.o mgmt/share_config.o \
		mgmt/tree_connect.o mgmt/user_session.o smb_common.o \
		buffer_pool.o transport_tcp.o transport_ipc.o server.o \
		connection.o crypto_ctx.o smbd_work.o

smbd-y +=	smb2pdu.o smb2ops.o smb2misc.o asn1.o
smbd-$(CONFIG_SMB_INSECURE_SERVER) += smb1pdu.o smb1ops.o smb1misc.o
smbd-$(CONFIG_SMB_SERVER_SMBDIRECT) += transport_rdma.o
#else
# For external module build
EXTRA_FLAGS += -I$(PWD) 
KDIR	?= $(LINUXDIR)
MDIR	?= /lib/modules/$(shell uname -r)
PWD	:= $(shell pwd)
PWD	:= $(shell pwd)

export CONFIG_SMB_SERVER := m
export CONFIG_SMB_INSECURE_SERVER := y
all:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean

install: smbd.ko
	rm -f $(INSTALLDIR)/smbd/lib/modules/$(KERNELRELEASE)/smbd.ko
	install -m644 -b -D smbd.ko $(INSTALLDIR)/smbd/lib/modules/$(KERNELRELEASE)/smbd.ko

uninstall:
	rm -f $(INSTALLDIR)/smbd/lib/modules/$(KERNELRELEASE)/smbd.ko
#endif

.PHONY : all clean install uninstall
