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
#-DCONFIG_SMB_SERVER_DEBUGGING=1

obj-$(CONFIG_SMB_SERVER) += ksmbd.o

ksmbd-y :=	unicode.o auth.o vfs.o vfs_cache.o \
		misc.o oplock.o netmisc.o \
		mgmt/ksmbd_ida.o mgmt/user_config.o mgmt/share_config.o \
		mgmt/tree_connect.o mgmt/user_session.o smb_common.o \
		buffer_pool.o transport_tcp.o transport_ipc.o server.o \
		connection.o crypto_ctx.o ksmbd_work.o

ksmbd-y +=	smb2pdu.o smb2ops.o smb2misc.o asn1.o
ksmbd-$(CONFIG_SMB_INSECURE_SERVER) += smb1pdu.o smb1ops.o smb1misc.o
ksmbd-$(CONFIG_SMB_SERVER_SMBDIRECT) += transport_rdma.o
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

install: ksmbd.ko
	rm -f $(INSTALLDIR)/smbd/lib/modules/$(KERNELRELEASE)/ksmbd.ko
	install -m644 -b -D ksmbd.ko $(INSTALLDIR)/smbd/lib/modules/$(KERNELRELEASE)/ksmbd.ko

uninstall:
	rm -f $(INSTALLDIR)/smbd/lib/modules/$(KERNELRELEASE)/ksmbd.ko
#endif

.PHONY : all clean install uninstall
