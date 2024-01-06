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

EXTRA_CFLAGS += -DCONFIG_SMB_INSECURE_SERVER=1 -g -I$(TOP)/smbd/smbd -DCONFIG_SMB_SERVER_KERBEROS5=1 -Wno-unused-function

obj-$(CONFIG_SMB_SERVER) += ksmbd.o


clean-files	+= spnego_negtokeninit-asn1.c spnego_negtokeninit-asn1.h
clean-files	+= spnego_negtokentarg-asn1.c spnego_negtokentarg-asn1.h

ksmbd-y :=	unicode.o auth.o vfs.o vfs_cache.o connection.o crypto_ctx.o \
		server.o misc.o oplock.o ksmbd_work.o smbacl.o ndr.o\
		mgmt/ksmbd_ida.o mgmt/user_config.o mgmt/share_config.o \
		mgmt/tree_connect.o mgmt/user_session.o smb_common.o \
		transport_tcp.o transport_ipc.o buffer_pool.o

ksmbd-y +=	smb2pdu.o smb2ops.o smb2misc.o ksmbd_spnego_negtokeninit.asn1.o \
		ksmbd_spnego_negtokentarg.asn1.o asn1.o compat.o

$(obj)/asn1.o: $(obj)/ksmbd_spnego_negtokeninit.asn1.h $(obj)/ksmbd_spnego_negtokentarg.asn1.h

$(obj)/ksmbd_spnego_negtokeninit.asn1.o: $(obj)/ksmbd_spnego_negtokeninit.asn1.c $(obj)/ksmbd_spnego_negtokeninit.asn1.h
$(obj)/ksmbd_spnego_negtokentarg.asn1.o: $(obj)/ksmbd_spnego_negtokentarg.asn1.c $(obj)/ksmbd_spnego_negtokentarg.asn1.h

ksmbd-$(CONFIG_SMB_INSECURE_SERVER) += smb1pdu.o smb1ops.o smb1misc.o netmisc.o
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
