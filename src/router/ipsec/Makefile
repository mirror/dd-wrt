IPSEC_TUNNEL=ipsec_tunnel-0.9
KEYNOTE=keynote
ISAKMPD=isakmpd

include $(TOP)/.config

FEATURES=
ifeq ($(CONFIG_IPSEC_ISAKMPD_X509),y)
	FEATURES+=x509
endif
ifeq ($(CONFIG_IPSEC_KEYNOTE),y)
	FEATURES+=policy
endif

obj-$(CONFIG_IPSEC_IPSECADM) += ipsecadm
obj-$(CONFIG_IPSEC_KEYNOTE) += $(KEYNOTE)
obj-$(CONFIG_IPSEC_ISAKMPD) += $(ISAKMPD)

obj-clean := $(foreach obj,$(obj-y) $(obj-n),$(obj)-clean)
obj-install := $(foreach obj,$(obj-y),$(obj)-install)
	

all: $(obj-y)

install: $(obj-install)

clean: $(obj-clean)

ipsecadm:
	$(MAKE) -C $(IPSEC_TUNNEL)/util/ipsecadm

ipsecadm-clean:
	$(MAKE) -C $(IPSEC_TUNNEL)/util/ipsecadm clean

ipsecadm-install:
	chmod 0644 $(IPSEC_TUNNEL)/util/ipsecadm/ipsecadm
	install -D $(IPSEC_TUNNEL)/util/ipsecadm/ipsecadm \
	           $(INSTALLDIR)/usr/sbin/ipsecadm
	$(STRIP) $(INSTALLDIR)/usr/sbin/ipsecadm

isakmpd:
	$(MAKE) -C $(ISAKMPD) FEATURES+="$(FEATURES)"

isakmpd-install:
	chmod 0644 $(ISAKMPD)/isakmpd
	install -D $(ISAKMPD)/isakmpd $(INSTALLDIR)/usr/sbin/isakmpd
	$(STRIP) $(INSTALLDIR)/usr/sbin/isakmpd

keynote-install:
	# do nothing

#
# Generic rules
#

%:
	[ ! -d $* ] || $(MAKE) -C $*

%-clean:
	[ ! -d $* ] || $(MAKE) -C $* clean

%-install:
	[ ! -d $* ] || $(MAKE) -C $* install INSTALLDIR=$(INSTALLDIR)/$*

$(obj-y) $(obj-n) $(obj-clean) $(obj-install): dummy

.PHONY: all clean distclean mrproper install package
.PHONY: dummy

