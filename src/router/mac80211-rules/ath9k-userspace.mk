iw-clean:
	make -C iw clean NL2FOUND=Y NL1FOUND= NLLIBNAME="libnl-tiny" CFLAGS="$(COPTS) $(MIPS16_OPT)  -ffunction-sections -fdata-sections -Wl,--gc-sections $(IW_CFLAGS)" LDFLAGS="$(COPT) $(IW_LDFLAGS)  -ffunction-sections -fdata-sections -Wl,--gc-sections" LIBS="-lm -lnl-tiny"

iw: libnltiny
	make -C iw NL2FOUND=Y NL1FOUND= NLLIBNAME="libnl-tiny" CFLAGS="$(COPTS) $(LTO) $(MIPS16_OPT)  -ffunction-sections -fdata-sections -Wl,--gc-sections $(IW_CFLAGS) -DNEED_PRINTF" LDFLAGS="$(IW_LDFLAGS) $(LDLTO)  -ffunction-sections -fdata-sections -Wl,--gc-sections" LIBS="-lm -lnl-tiny"

iw-install:
ifneq ($(CONFIG_NOWIFI),y)
	install -D iw/iw $(INSTALLDIR)/iw/usr/sbin/iw
else
	@true
endif

crda: libnltiny
	cd crda ; ./db2fw.py regulatory.db $(REGTXT)
	cd crda ; ./db2bin.py regulatory.bin $(REGTXT)
ifneq ($(CONFIG_CFG80211_INTERNAL_REGDB),y)
	make -C crda NL2FOUND=Y NL1FOUND= NLLIBS="-L$(TOP)/libnl-tiny -lm -lnl-tiny" NLLIBNAME="libnl-tiny" IW_CFLAGS="$(COPTS)  -ffunction-sections -fdata-sections -Wl,--gc-sections $(MIPS16_OPT) $(IW_CFLAGS)  -DNEED_PRINTF" IW_LDFLAGS="$(IW_LDFLAGS)" REG_BIN="$(TOP)/crda/regulatory.bin" LIBS="-lm -lnl-tiny"
else
	@true
endif 

crda-install:
ifneq ($(CONFIG_CFG80211_INTERNAL_REGDB),y)
ifneq ($(CONFIG_NOWIFI),y)
	install -D crda/crda $(INSTALLDIR)/crda/sbin/crda
	install -D $(REGPATH)/$(REGBIN) $(INSTALLDIR)/crda/lib/crda/regulatory.bin
else
	@true
endif
else
# die regulatory-db brauchen wir für den userspace !!!! Ja...
	install -D crda/crda.sh $(INSTALLDIR)/crda/sbin/crda
	install -D $(REGPATH)/$(REGBIN) $(INSTALLDIR)/crda/lib/crda/regulatory.bin
	@true
endif

crda-clean:
	make -C crda NL2FOUND=Y NL1FOUND= NLLIBNAME="libnl-tiny" IW_CFLAGS="$(COPTS) $(MIPS16_OPT)  -ffunction-sections -fdata-sections -Wl,--gc-sections $(IW_CFLAGS)" IW_LDFLAGS="$(IW_LDFLAGS)  -ffunction-sections -fdata-sections -Wl,--gc-sections" REG_BIN="$(TOP)/crda/regulatory.bin" LIBS="-lm -lnl-tiny" clean

autochannel-clean:
	make -C autochannel clean NL2FOUND=Y NL1FOUND= NLLIBNAME="libnl-tiny" CFLAGS="$(COPTS) $(MIPS16_OPT) -ffunction-sections -fdata-sections -Wl,--gc-sections $(IW_CFLAGS)" LDFLAGS="$(IW_LDFLAGS)  -ffunction-sections -fdata-sections -Wl,--gc-sections" LIBS="-lm -lnl-tiny"

autochannel: libnltiny
	make -C autochannel NL2FOUND=Y NL1FOUND= NLLIBNAME="libnl-tiny" CFLAGS="$(COPTS) $(MIPS16_OPT) -ffunction-sections -fdata-sections -Wl,--gc-sections $(IW_CFLAGS)  -DNEED_PRINTF" LDFLAGS="$(IW_LDFLAGS)  -ffunction-sections -fdata-sections -Wl,--gc-sections" LIBS="-lm -lnl-tiny"

autochannel-install:
ifneq ($(CONFIG_NOWIFI),y)
	install -D autochannel/autochannel $(INSTALLDIR)/autochannel/usr/sbin/autochannel
else
	@true
endif
