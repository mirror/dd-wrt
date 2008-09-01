include $(TOP)/.config

ifneq ($(wildcard $(SRCBASE)/cy_conf.mak),)
  include $(SRCBASE)/cy_conf.mak
endif

all:

install:
#	rm -rf $(INSTALLDIR)/www
	install -d $(INSTALLDIR)/www
	install -m 644 $(wildcard *.asp) $(wildcard *.svg) $(wildcard *.css) $(wildcard *.jpg) $(wildcard *.gif) $(wildcard *.js) $(wildcard *.htm) $(wildcard *.ico) $(wildcard *.html) $(INSTALLDIR)/www

ifeq ($(CONFIG_DIST),"micro")
	rm -f $(INSTALLDIR)/www/images/favicon.ico
	rm -f $(INSTALLDIR)/www/images/logo.png
ifneq ($(CONFIG_COMPRESSEDJS),y)
	cp common_micro.js $(INSTALLDIR)/www/common.js
	rm -f $(INSTALLDIR)/www/common_micro.js
else
	cp common_micro.compressed.js $(INSTALLDIR)/www/common.js
	rm -f $(INSTALLDIR)/www/common_micro.compressed.js
endif
	rm -f $(INSTALLDIR)/www/Log_incoming.asp
	rm -f $(INSTALLDIR)/www/Log_outgoing.asp
endif
ifeq ($(CONFIG_DIST),"micro-special")
	rm -f $(INSTALLDIR)/www/images/favicon.ico
	rm -f $(INSTALLDIR)/www/images/logo.png
ifneq ($(CONFIG_COMPRESSEDJS),y)
	cp common_micro.js $(INSTALLDIR)/www/common.js
	rm -f $(INSTALLDIR)/www/common_micro.js
else
	cp common_micro.compressed.js $(INSTALLDIR)/www/common.js
	rm -f $(INSTALLDIR)/www/common_micro.compressed.js
endif
	rm -f $(INSTALLDIR)/www/Log_incoming.asp
	rm -f $(INSTALLDIR)/www/Log_outgoing.asp
endif
	cp -a images $(INSTALLDIR)/www
	cp -a style $(INSTALLDIR)/www

ifneq ($(CONFIG_DIST),"micro")
ifneq ($(CONFIG_DIST),"micro-special")
ifeq ($(CONFIG_DEFAULT_COUNTRYCODE),JP)
	cp -a help_jp $(INSTALLDIR)/www
	mv $(INSTALLDIR)/www/help_jp $(INSTALLDIR)/www/help 
else
ifneq ($(CONFIG_EXTHELP),y)
	cp -a help $(INSTALLDIR)/www
endif
endif
	rm -f $(INSTALLDIR)/www/common_micro.js
	mkdir -p $(INSTALLDIR)/www/js
endif
endif
ifeq ($(CONFIG_TW6600),y)
	cp -a help $(INSTALLDIR)/www
	rm -f $(INSTALLDIR)/www/common_micro.js
	mkdir -p $(INSTALLDIR)/www/js
endif


	cp -a lang_pack $(INSTALLDIR)/www

ifneq ($(CONFIG_LANGUAGE),y)
	rm -f $(INSTALLDIR)/www/lang_pack/*.js
	rm -f $(INSTALLDIR)/www/images/dd-wrt.gif	
	rm -f $(INSTALLDIR)/www/images/logo.png
#	rm -f $(INSTALLDIR)/www/images/88*
	rm -f $(INSTALLDIR)/www/images/valid-xhtml10.png	
	cp lang_pack/english.js $(INSTALLDIR)/www/lang_pack/
endif
	$(TOP)/tools/jsformat $(INSTALLDIR)/www/lang_pack/*.js
	ln -s ../tmp/www $(INSTALLDIR)/www/user

ifeq ($(CONFIG_WIKAR),y)
	cp -a wikar $(INSTALLDIR)/www/style
endif

ifeq ($(CONFIG_SPUTNIK_APD),y)
	cp sputnik/sputnik.png $(INSTALLDIR)/www/images
endif
ifeq ($(CONFIG_WIFIDOG),y)
	cp wifidog/*.png $(INSTALLDIR)/www/images
endif

ifeq ($(CONFIG_3COM),y)
	
	mkdir $(INSTALLDIR)/www/style/3com
	cp 3com/style.css $(INSTALLDIR)/www/style/3com
	cp 3com/images/* $(INSTALLDIR)/www/images
endif

ifeq ($(CONFIG_BUFFALO),y)
	mkdir $(INSTALLDIR)/www/style/buffalo
	cp buffalo/style.css $(INSTALLDIR)/www/style/buffalo
#	cp buffalo/style_.css $(INSTALLDIR)/www/style/buffalo
#	cp elegant/style_ie.css $(INSTALLDIR)/www/style/buffalo
#	cp buffalo/common.css $(INSTALLDIR)/www/style
	cp buffalo/images/* $(INSTALLDIR)/www/images
	cp buffalo/help/* $(INSTALLDIR)/www/help

#	mkdir $(INSTALLDIR)/www/style/buffalo
#	cp buffalo/style.css $(INSTALLDIR)/www/style/buffalo
#	cp buffalo/common_buffalo.css $(INSTALLDIR)/www/style
#	cp buffalo/images/* $(INSTALLDIR)/www/images
endif

ifeq ($(CONFIG_RED),y)
	mkdir $(INSTALLDIR)/www/style/red
#	cp red/style.css $(INSTALLDIR)/www/style/red
#	cp red/style_ie.css $(INSTALLDIR)/www/style/red
	touch $(INSTALLDIR)/www/style/red/style.css
	cp common_style/common.css $(INSTALLDIR)/www/style
	cp red/logo.png $(INSTALLDIR)/www/style
endif

ifeq ($(CONFIG_FONSTYLE),y)
	mkdir $(INSTALLDIR)/www/style/fon
	cp fon/style.css $(INSTALLDIR)/www/style/fon
	cp fon/style_ie.css $(INSTALLDIR)/www/style/fon
	cp fon/commonfon.css $(INSTALLDIR)/www/style
	cp fon/fon.jpg $(INSTALLDIR)/www/style
endif

ifeq ($(CONFIG_BLUE),y)
	mkdir $(INSTALLDIR)/www/style/blue
#	cp blue/style.css $(INSTALLDIR)/www/style/blue
#	cp blue/style_ie.css $(INSTALLDIR)/www/style/blue
	touch $(INSTALLDIR)/www/style/blue/style.css
	cp common_style/common.css $(INSTALLDIR)/www/style
	cp blue/logo.png $(INSTALLDIR)/www/style
endif

ifeq ($(CONFIG_ELEGANT),y)
	mkdir $(INSTALLDIR)/www/style/elegant
#	cp elegant/style.css $(INSTALLDIR)/www/style/elegant
#	cp elegant/style_ie.css $(INSTALLDIR)/www/style/elegant
	touch $(INSTALLDIR)/www/style/elegant/style.css
	cp common_style/common.css $(INSTALLDIR)/www/style
	cp elegant/logo.png $(INSTALLDIR)/www/style
endif

ifeq ($(CONFIG_CYAN),y)
	mkdir $(INSTALLDIR)/www/style/cyan
#	cp cyan/style.css $(INSTALLDIR)/www/style/cyan
#	cp cyan/style_ie.css $(INSTALLDIR)/www/style/cyan
	touch $(INSTALLDIR)/www/style/cyan/style.css
	cp common_style/common.css $(INSTALLDIR)/www/style
	cp cyan/logo.png $(INSTALLDIR)/www/style
endif

ifeq ($(CONFIG_CESAR),y)
	mkdir $(INSTALLDIR)/www/style/cesar
	cp cesar/style.css $(INSTALLDIR)/www/style/cesar
	cp cesar/style_ie.css $(INSTALLDIR)/www/style/cesar
	cp cesar/common.css $(INSTALLDIR)/www/style
	cp cesar/logo.png $(INSTALLDIR)/www/style
endif

ifeq ($(CONFIG_ALEDR),y)
	mkdir $(INSTALLDIR)/www/style/cyan
	cp aledr/style.css $(INSTALLDIR)/www/style/cyan
	cp aledr/style_ie.css $(INSTALLDIR)/www/style/cyan
	cp aledr/common.css $(INSTALLDIR)/www/style
	cp aledr/logo.jpg $(INSTALLDIR)/www/style
endif

ifeq ($(CONFIG_YELLOW),y)
	mkdir $(INSTALLDIR)/www/style/yellow
#	cp yellow/style.css $(INSTALLDIR)/www/style/yellow
#	cp yellow/style_ie.css $(INSTALLDIR)/www/style/yellow
	touch $(INSTALLDIR)/www/style/yellow/style.css
	cp common_style/common.css $(INSTALLDIR)/www/style
	cp yellow/logo.png $(INSTALLDIR)/www/style
endif

ifeq ($(CONFIG_GREEN),y)
	mkdir $(INSTALLDIR)/www/style/green
#	cp green/style.css $(INSTALLDIR)/www/style/green
#	cp green/style_ie.css $(INSTALLDIR)/www/style/green
	touch $(INSTALLDIR)/www/style/green/style.css
	cp common_style/common.css $(INSTALLDIR)/www/style
	cp green/logo.png $(INSTALLDIR)/www/style
endif

ifeq ($(CONFIG_ORANGE),y)
	mkdir $(INSTALLDIR)/www/style/orange
#	cp orange/style.css $(INSTALLDIR)/www/style/orange
#	cp orange/style_ie.css $(INSTALLDIR)/www/style/orange
	touch $(INSTALLDIR)/www/style/orange/style.css
	cp common_style/common.css $(INSTALLDIR)/www/style
	cp orange/logo.png $(INSTALLDIR)/www/style
endif

ifeq ($(CONFIG_34TELECOM),y)
	$(MAKE) -C $(SRCBASE)/router/kromo/34telecom/ install
	mkdir $(INSTALLDIR)/www/style/cyan
	cp cyan/style.css $(INSTALLDIR)/www/style/cyan
	cp cyan/style_ie.css $(INSTALLDIR)/www/style/cyan
	cp 34telecom/common.css $(INSTALLDIR)/www/style
#	cp cyan/logo.png $(INSTALLDIR)/www/style
	cp -f 34telecom/logo.png $(INSTALLDIR)/www/style/
else

ifeq ($(CONFIG_PWC),y)
	cp ../windows_js/javascripts/prototype.js $(INSTALLDIR)/www/js
	cp ../windows_js/javascripts/effects.js $(INSTALLDIR)/www/js
	cp ../windows_js/javascripts/window.js $(INSTALLDIR)/www/js
	cp ../windows_js/javascripts/window_effects.js $(INSTALLDIR)/www/js
	mkdir -p $(INSTALLDIR)/www/style/pwc
	mkdir -p $(INSTALLDIR)/www/style/pwc/default
	cp ../windows_js/themes/default/* $(INSTALLDIR)/www/style/pwc/default
	cp ../windows_js/themes/default.css $(INSTALLDIR)/www/style/pwc
	cp -r common_style/pwc $(INSTALLDIR)/www/style
	cp common_style/common.css $(INSTALLDIR)/www/style
endif

ifeq ($(CONFIG_SKYTEL),y)
	cp -a skytel $(INSTALLDIR)/www/style
endif

ifeq ($(CONFIG_NEWMEDIA),y)
	cp -f nmn/nmn_logo.png $(INSTALLDIR)/www/style/logo.png
endif

ifeq ($(CONFIG_SPUTNIK),y)
	cp -f sputnik/logo.png $(INSTALLDIR)/www/style
endif

ifeq ($(CONFIG_TELCOM),y)
	cp -f telcom/logo.png $(INSTALLDIR)/www/style
endif

ifeq ($(CONFIG_GLAUCO),y)
	cp -f glauco/logo.png $(INSTALLDIR)/www/style
	cp -f glauco/logo-help.png $(INSTALLDIR)/www/help/logo-help.png
	rm -rf $(INSTALLDIR)/www/help
	rm $(INSTALLDIR)/www/About.htm
endif

ifeq ($(CONFIG_GGEW),y)
	cp -f ggew/logo.png $(INSTALLDIR)/www/style/logo.png
endif
ifeq ($(CONFIG_DDLAN),y)
	cp -f ddlan/logo.png $(INSTALLDIR)/www/style/logo.png
ifeq ($(CONFIG_NS5),y)
	cp -f ddlan/logo_ns5.png $(INSTALLDIR)/www/style/logo.png
endif
ifeq ($(CONFIG_NS2),y)
	cp -f ddlan/logo_ns2.png $(INSTALLDIR)/www/style/logo.png
endif
	cp -f ddlan/common.css $(INSTALLDIR)/www/style
endif

ifeq ($(CONFIG_MAKSAT_CORAL_ELEGANT),y)
	mkdir $(INSTALLDIR)/www/style/maksat_coral_elegant
	cp maksat/style-coral.css $(INSTALLDIR)/www/style/maksat_coral_elegant/style.css
	cp maksat/common-coral.css $(INSTALLDIR)/www/style
endif
ifeq ($(CONFIG_TRIMAX),y)
	cp -f trimax/logo.png $(INSTALLDIR)/www/style/logo.png
endif
ifeq ($(CONFIG_MAKSAT),y)
	cp -f maksat/SuperChannel.asp $(INSTALLDIR)/www
	cp -f maksat/logo.png $(INSTALLDIR)/www/style/logo.png
	cp -f maksat/logo.png $(INSTALLDIR)/www/images/logo.png
	cp -f maksat/maksat_logo_help.png $(INSTALLDIR)/www/help/logo-help.png
	cp -f maksat/favicon.ico $(INSTALLDIR)/www/images/favicon.ico
	cp -f maksat/About.htm $(INSTALLDIR)/www/About.htm
	cp -f maksat/help/HManagement.asp $(INSTALLDIR)/www/help/HManagement.asp
	cp -f maksat/help/HServices.asp $(INSTALLDIR)/www/help/HServices.asp
	cp -f maksat/help/HUpgrade.asp $(INSTALLDIR)/www/help/HUpgrade.asp
	cp -f maksat/help/HStatus.asp $(INSTALLDIR)/www/help/HStatus.asp
	cp -f maksat/help/index.asp $(INSTALLDIR)/www/help/index.asp
	cp -f maksat/common.js $(INSTALLDIR)/www/common.js
	cp -f maksat/common.css $(INSTALLDIR)/www/style/common.css
	cp maksat/images/* $(INSTALLDIR)/www/images
	mkdir $(INSTALLDIR)/www/style/maksat
	cp maksat/maksat/* $(INSTALLDIR)/www/style/maksat
endif
ifneq ($(CONFIG_WAVESAT),y)
	rm -f $(INSTALLDIR)/www/WiMAX.asp 
endif

ifeq ($(CONFIG_MAKSAT_CORAL),y)
	cp -f maksat/logo-coral.png $(INSTALLDIR)/www/style/logo-coral.png
	cp -f maksat/logo-coral.png $(INSTALLDIR)/www/images/logo-coral.png
	cp maksat_coral/images/* $(INSTALLDIR)/www/images
	cp -f maksat/logo-coral.png $(INSTALLDIR)/www/images/logo-coral.png
	mkdir $(INSTALLDIR)/www/style/maksat_coral
	cp maksat_coral/maksat_coral/* $(INSTALLDIR)/www/style/maksat_coral
endif


ifeq ($(CONFIG_KODATA),y)
	cp -f kodata/logo.png $(INSTALLDIR)/www/style/logo.png
endif

ifeq ($(CONFIG_THOM),y)
	cp -f thom/logo.png $(INSTALLDIR)/www/style/logo.png
endif

#ifneq ($(CONFIG_MADWIFI),y)
#	cp Wireless_WDS.v23 $(INSTALLDIR)/www/Wireless_WDS.asp
#endif
ifneq ($(CONFIG_MSSID),y)
	cp WEP.v23 $(INSTALLDIR)/www/WEP.asp
	cp WL_WPATable.v23 $(INSTALLDIR)/www/WL_WPATable.asp
#	cp WPA.v23 $(INSTALLDIR)/www/WPA.asp
	cp WPA_Preshared.v23 $(INSTALLDIR)/www/WPA_Preshared.asp
	cp WPA_Radius.v23 $(INSTALLDIR)/www/WPA_Radius.asp	
	cp Wireless_Basic.v23 $(INSTALLDIR)/www/Wireless_Basic.asp
	cp Wireless_Advanced.v23 $(INSTALLDIR)/www/Wireless_Advanced.asp
endif


ifeq ($(CONFIG_SKYTRON),y)
	cp -a skytron $(INSTALLDIR)/www/style
endif

ifeq ($(CONFIG_POWERNOC),y)
	cp -a powernoc $(INSTALLDIR)/www/style
endif

ifeq ($(CONFIG_DLS),y)
	cp -a dls $(INSTALLDIR)/www/style
endif

ifeq ($(CONFIG_FREEBIRD),y)
	cp -a freebird $(INSTALLDIR)/www/style
endif

ifeq ($(CONFIG_WTS),y)
	cp -a wts $(INSTALLDIR)/www/style
endif

ifeq ($(CONFIG_OMNI),y)
	cp -a omni $(INSTALLDIR)/www/style
endif

ifeq ($(CONFIG_KROMO),y)
	cp -a kromo $(INSTALLDIR)/www/style
endif

ifeq ($(CONFIG_BRAINSLAYER),y)
	cp -a brainslayer $(INSTALLDIR)/www/style
endif

ifeq ($(CONFIG_XIRIAN),y)
	cp -a xirian $(INSTALLDIR)/www/style
endif

ifneq ($(AOL_SUPPORT),1)
	rm -f $(INSTALLDIR)/www/aol.asp
	rm -f $(INSTALLDIR)/www/aol_error.asp
endif

ifneq ($(HSIAB_SUPPORT),1)
	rm -f $(INSTALLDIR)/www/HotSpot_*
	rm -f $(INSTALLDIR)/www/Config.asp
	rm -f $(INSTALLDIR)/www/New_device.asp
	rm -f $(INSTALLDIR)/www/Old_device.asp
	rm -f $(INSTALLDIR)/www/Register*
endif

ifneq ($(HEARTBEAT_SUPPORT),1)
	rm -f $(INSTALLDIR)/www/index_heartbeat.asp
endif

ifneq ($(MPPPOE_SUPPORT),1)
	rm -f $(INSTALLDIR)/www/index_mpppoe.asp
	rm -f $(INSTALLDIR)/www/Status_mpppoe.asp
endif

ifneq ($(CONFIG_VLANTAGGING),y)
	rm -f $(INSTALLDIR)/www/Networking.asp
	rm -f $(INSTALLDIR)/www/Networking.live.asp
	rm -f $(INSTALLDIR)/www/help/HNetworking.asp
endif

ifneq ($(BACKUP_RESTORE_SUPPORT),1)
	rm -f $(INSTALLDIR)/www/Backup_Restore.asp
endif

ifneq ($(CONFIG_UPNP),y)
	rm -f $(INSTALLDIR)/www/UPnP.asp
	rm -f $(INSTALLDIR)/www/UPnP.live.asp
	rm -f $(INSTALLDIR)/www/help/HUPnP.asp
endif
ifneq ($(CONFIG_SUPERCHANNEL),y)
	rm -f $(INSTALLDIR)/www/SuperChannel.asp
endif
ifneq ($(CONFIG_WOL),y)
	rm -f $(INSTALLDIR)/www/Wol.asp
	rm -f $(INSTALLDIR)/www/help/HWol.asp
endif

ifneq ($(CONFIG_NOCAT),y)
	rm -f $(INSTALLDIR)/www/splash.html
	rm -f $(INSTALLDIR)/www/status.html
endif
ifneq ($(CONFIG_MSSID),y)
	cp Status_Wireless.asp.v23 $(INSTALLDIR)/www/Status_Wireless.asp
	cp Status_Wireless.live.asp.v23 $(INSTALLDIR)/www/Status_Wireless.live.asp
endif	
ifneq ($(CONFIG_WIVIZ),y)
	rm -f $(INSTALLDIR)/www/Wiviz_Survey.asp
	rm -f $(INSTALLDIR)/www/Wiviz.live.asp
endif

ifneq ($(CONFIG_OPENVPN),y)
	rm -f $(INSTALLDIR)/www/Status_OpenVPN.asp
endif

ifneq ($(CONFIG_PPPOESERVER),y)
	rm -f $(INSTALLDIR)/www/PPPoE_Server.asp
	rm -f $(INSTALLDIR)/www/help/HPPPoE_Server.asp
endif
ifneq ($(CONFIG_MILKFISH),y)
	rm -f $(INSTALLDIR)/www/Milkfish.asp
	rm -f $(INSTALLDIR)/www/Milkfish_aliases.asp
	rm -f $(INSTALLDIR)/www/Milkfish_database.asp
	rm -f $(INSTALLDIR)/www/Milkfish_dynsip.asp
	rm -f $(INSTALLDIR)/www/Milkfish_messaging.asp
	rm -f $(INSTALLDIR)/www/Milkfish_phonebook.asp
	rm -f $(INSTALLDIR)/www/Milkfish_siptrace.asp
	rm -f $(INSTALLDIR)/www/Milkfish_status.asp
	rm -f $(INSTALLDIR)/www/help/HMilkfish.asp
	rm -f $(INSTALLDIR)/www/images/sipwerk.png
endif
ifneq ($(CONFIG_EOP_TUNNEL),y)
	rm -f $(INSTALLDIR)/www/help/HEoIP.asp
endif
ifneq ($(CONFIG_SPUTNIK_APD),y)
	rm -f $(INSTALLDIR)/www/Status_SputnikAPD.asp
	rm -f $(INSTALLDIR)/www/Status_SputnikAPD.live.asp
endif
ifneq ($(CONFIG_CTORRENT),y)
	rm -f $(INSTALLDIR)/www/P2P.asp
endif
#	All: These pages are not used
	rm -f $(INSTALLDIR)/www/About_test.htm
	rm -f $(INSTALLDIR)/www/Logout.asp
	rm -f $(INSTALLDIR)/www/SysInfo.htm
	rm -f $(INSTALLDIR)/www/graph_cpu.svg	
	
ifeq ($(CONFIG_RAMON),y)
	$(MAKE) -C $(SRCBASE)/router/kromo/newmedia/ install
else

ifeq ($(CONFIG_SKYTRON),y)
	$(MAKE) -C $(SRCBASE)/router/kromo/skytron/ install
else
ifeq ($(CONFIG_SKYTEL),y)
	$(MAKE) -C $(SRCBASE)/router/kromo/skytel/ install
else
	$(MAKE) -C $(SRCBASE)/router/kromo/common/ install
endif

endif
endif
endif

ifeq ($(CONFIG_OMNI),y)
	$(MAKE) -C $(SRCBASE)/router/kromo/omni-wifi/ install
endif

#ifeq ($(CONFIG_DDLAN),y)
#	$(MAKE) -C $(SRCBASE)/router/kromo/ddlan/ install
#endif



	../../../../tools/strip $(INSTALLDIR)/www/*.asp
	../../../../tools/strip $(INSTALLDIR)/www/*.htm
	../../../../tools/strip $(INSTALLDIR)/www/help/*.asp
	
ifeq ($(CONFIG_SER),y)	
	$(MAKE) -C $(SRCBASE)/router/kromo/sipath/ install	
endif
	
	../../../../tools/write4 $(INSTALLDIR)/www/*.asp
	../../../../tools/write4 $(INSTALLDIR)/www/*.htm
	../../../../tools/write4 $(INSTALLDIR)/www/help/*.asp
	cd $(INSTALLDIR)/www  &&  find . -iname "CVS" | xargs rm -rf
	cd $(INSTALLDIR)/www  &&  find . -iname ".svn" | xargs rm -rf

	cd $(INSTALLDIR)/www && $(SRCBASE)/../tools/webcomp . $(SRCBASE)/../tools/filelist *.asp *.svg *.htm *.css *.js help/*.asp help/*.png help/*.css images/*.gif images/*.png images/*.jpg images/*.ico images/*/*.png images/*/*.gif js/* lang_pack/* style/* style/*/* style/*/*/* style/*/*/*/* style/*/*/*/*/* > $(SRCBASE)/router/httpd/html.c 
ifeq ($(CONFIG_PWC),y)
	rm -rf $(INSTALLDIR)/www/style/pwc/ddwrt
	rm -rf $(INSTALLDIR)/www/style/pwc/default
endif
	cd $(INSTALLDIR)/www && rm -f *.asp *.svg *.htm *.css *.js help/*.asp help/*.png help/*.css images/*.gif images/*.png images/*.jpg images/*.ico style/*.css style/*.png style/*/* images/*/*.png images/*/*.gif js/* lang_pack/*

#	cd $(INSTALLDIR)/www  &&  find . -name "*lzma" | xargs rm -f
#../../../../tools/webcomp . filelist $(INSTALLDIR)/www/*.asp $(INSTALLDIR)/www/*.htm  > webrom.c
	@true

webcomp:

clean:
