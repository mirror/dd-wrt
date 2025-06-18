include $(TOP)/.config

ifneq ($(wildcard $(SRCBASE)/cy_conf.mak),)
  include $(SRCBASE)/cy_conf.mak
endif

all:

installwebfiles:
#	rm -rf $(INSTALLDIR)/www
	install -d $(INSTALLDIR)/www
	install -m 644 $(wildcard *.asp) $(wildcard *.svg) $(wildcard *.css) $(wildcard *.jpg) $(wildcard *.gif) $(wildcard *.js) $(wildcard *.htm) $(wildcard *.ico) $(wildcard *.html) $(INSTALLDIR)/www
	cp common.js $(INSTALLDIR)/www/common.js
	cp -a images $(INSTALLDIR)/www
	cp -a style $(INSTALLDIR)/www
	cp common_style/buttons.css $(INSTALLDIR)/www/style
#	cp common_style/buttons_dark.css $(INSTALLDIR)/www/style
	
ifeq ($(CONFIG_DIST),"micro")
	rm -f $(INSTALLDIR)/www/favicon.ico
#	rm -f $(INSTALLDIR)/www/images/dd-logo.png
	rm -f $(INSTALLDIR)/www/images/valid-xhtml10.png
ifneq ($(CONFIG_COMPRESSEDJS),y)
	cp common_micro.js $(INSTALLDIR)/www/common.js
else
	cp common_micro.compressed.js $(INSTALLDIR)/www/common.js
endif
	rm -f $(INSTALLDIR)/www/Log_incoming.asp
	rm -f $(INSTALLDIR)/www/Log_outgoing.asp
	rm -f $(INSTALLDIR)/www/RuleTable.asp
endif

#ifneq ($(CONFIG_AOSS),y)
#	rm -f $(INSTALLDIR)/www/AOSS.asp
#endif

ifneq ($(CONFIG_SPOTPASS),y)
	rm -f $(INSTALLDIR)/www/Nintendo.asp
endif
ifneq ($(CONFIG_IPV6),y)
	rm -f $(INSTALLDIR)/www/IPV6.asp
endif
ifneq ($(CONFIG_WPA3),y)
	rm -f $(INSTALLDIR)/www/Roaming.asp
endif

ifeq ($(CONFIG_DIST),"micro-special")
	rm -f $(INSTALLDIR)/www/favicon.ico
#	rm -f $(INSTALLDIR)/www/images/dd-logo.png
	rm -f $(INSTALLDIR)/www/images/valid-xhtml10.png
ifneq ($(CONFIG_COMPRESSEDJS),y)
	cp common_micro.js $(INSTALLDIR)/www/common.js
else
	cp common_micro.compressed.js $(INSTALLDIR)/www/common.js
endif
	rm -f $(INSTALLDIR)/www/Log_incoming.asp
	rm -f $(INSTALLDIR)/www/Log_outgoing.asp
endif

	rm -f $(INSTALLDIR)/www/common_micro.js
	rm -f $(INSTALLDIR)/www/common_micro.compressed.js

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
	mkdir -p $(INSTALLDIR)/www/js
endif
endif

ifeq ($(CONFIG_TW6600),y)
	cp -a help $(INSTALLDIR)/www
	mkdir -p $(INSTALLDIR)/www/js
endif


	mkdir -p $(INSTALLDIR)/www/lang_pack
	cp lang_pack/*.js $(INSTALLDIR)/www/lang_pack	
	rm -f $(INSTALLDIR)/www/lang_pack/arabic.js	
	
ifneq ($(CONFIG_LANGUAGE),y)
	rm -f $(INSTALLDIR)/www/lang_pack/*.js
	cp lang_pack/english.js $(INSTALLDIR)/www/lang_pack/
endif
	$(TOP)/tools/jsformat $(INSTALLDIR)/www/lang_pack/*.js
	ln -s ../tmp/www $(INSTALLDIR)/www/user


ifeq ($(CONFIG_WIKAR),y)
	cp -a wikar $(INSTALLDIR)/www/style
	cp common_style/common_legacy.css $(INSTALLDIR)/www/style
endif


#ifeq ($(CONFIG_SPUTNIK_APD),y)
#	cp sputnik/sputnik.png $(INSTALLDIR)/www/images
#endif
ifeq ($(CONFIG_WIFIDOG),y)
	cp wifidog/*.png $(INSTALLDIR)/www/images
endif
ifeq ($(CONFIG_HOTSPOT),y)
	cp hotspotsystem/hotspotsystem.png $(INSTALLDIR)/www/images
endif

ifeq ($(CONFIG_3COM),y)
	
	mkdir $(INSTALLDIR)/www/style/3com
	cp 3com/style.css $(INSTALLDIR)/www/style/3com
	cp 3com/images/* $(INSTALLDIR)/www/images
endif

ifeq ($(CONFIG_BUFFALO),y)
ifneq ($(CONFIG_SPOTPASS),y)
	
	mkdir $(INSTALLDIR)/www/style/buffalo
	cp buffalo/style.css $(INSTALLDIR)/www/style/buffalo
	cp buffalo/favicon.ico $(INSTALLDIR)/www/images
	cp buffalo/images/* $(INSTALLDIR)/www/images
	cp buffalo/help/* $(INSTALLDIR)/www/help
endif
endif
ifeq ($(CONFIG_IAS),y)
	mkdir $(INSTALLDIR)/www/intatstart
	mkdir $(INSTALLDIR)/www/intatstart/css
	mkdir $(INSTALLDIR)/www/intatstart/js
	mkdir $(INSTALLDIR)/www/intatstart/config
	mkdir $(INSTALLDIR)/www/intatstart/lang_pack
	cp -r ../../private/buffalo/intatstart/www/css/*.css $(INSTALLDIR)/www/intatstart/css
	cp -r ../../private/buffalo/intatstart/www/css/images $(INSTALLDIR)/www/intatstart/css
	cp -r ../../private/buffalo/intatstart/www/js/*.js $(INSTALLDIR)/www/intatstart/js
	cp -r ../../private/buffalo/intatstart/www/config/*.webconfig $(INSTALLDIR)/www/intatstart/config
	cp -r ../../private/buffalo/intatstart/www/config/*.webconfig $(TOP)/httpd/ej_temp
	cp -r ../../private/buffalo/intatstart/www/lang_pack/*.js $(INSTALLDIR)/www/intatstart/lang_pack
	cp -r ../../private/buffalo/intatstart/kromo/dd-wrt/* $(INSTALLDIR)/www/
ifeq ($(CONFIG_IAS_NOPLNPWD),y)
	install $(TOP)/private/buffalo/intatstart/config/admincard_noplnpwd.webdisplay $(INSTALLDIR)/www/intatstart/config/admincard.webdisplay
else
	install $(TOP)/private/buffalo/intatstart/config/admincard.webdisplay $(INSTALLDIR)/www/intatstart/config/admincard.webdisplay
endif
endif
ifeq ($(CONFIG_SPOTPASS),y)
	mkdir $(INSTALLDIR)/www/style/nintendo
	cp nintendo/style.css $(INSTALLDIR)/www/style/nintendo
	cp buffalo/favicon.ico $(INSTALLDIR)/www/images
	cp nintendo/images/* $(INSTALLDIR)/www/images
endif

ifeq ($(CONFIG_FREECWMP),y)
	mkdir -p $(INSTALLDIR)/jffs/freecwmp
	mkdir -p $(INSTALLDIR)/etc/config
	-cp ../../private/freecwmp-startscript/zzz_freecwmp.startup $(INSTALLDIR)/etc/config
	-cp ../../private/freecwmp-startscript/zzz_freecwmp.firewall $(INSTALLDIR)/etc/config
	-cp ../../private/freecwmp/Status_CWMP.asp $(INSTALLDIR)/www
	ln -s ../tmp/freecwmp/bin $(INSTALLDIR)/jffs/bin
	ln -s ../tmp/freecwmp/etc $(INSTALLDIR)/jffs/etc
	ln -s ../tmp/freecwmp/lib $(INSTALLDIR)/jffs/lib
	ln -s ../tmp/freecwmp/sbin $(INSTALLDIR)/jffs/sbin
	ln -s ../tmp/freecwmp/usr $(INSTALLDIR)/jffs/usr
	-cp -r ../../private/freecwmp/* $(INSTALLDIR)/jffs/freecwmp
	ln -sf /jffs/freecwmp/usr/sbin/freecwmpd-set-dhcp-option.sh $(INSTALLDIR)/etc/udhcpc_vendorspecific
	-rm -Rf $(INSTALLDIR)/jffs/freecwmp/config
	-rm -Rf $(INSTALLDIR)/jffs/freecwmp/httpd
	-rm -Rf $(INSTALLDIR)/jffs/freecwmp/Status_CWMP.asp
	cp -r ../../private/freecwmp/config/freecwmp.webconfig $(INSTALLDIR)/etc/config
	cp -r ../../private/freecwmp/config/freecwmp.webconfig $(TOP)/httpd/ej_temp
	cp -r ../../private/freecwmp/config/freecwmp.nvramconfig $(INSTALLDIR)/etc/config
	cp -r ../../private/freecwmp/lang_pack/freecwmp-english.js $(INSTALLDIR)/www/lang_pack
	mkdir -p $(INSTALLDIR)/usr/lib
	ln -sf /jffs/usr/lib/valgrind $(INSTALLDIR)/usr/lib/
endif

ifeq ($(CONFIG_UNFY),y)
	mkdir $(INSTALLDIR)/www/style/buffalo
	cp buffalo/style.css $(INSTALLDIR)/www/style/buffalo
	cp buffalo/favicon.ico $(INSTALLDIR)/www/images
	cp buffalo/images/* $(INSTALLDIR)/www/images
	cp buffalo/help/* $(INSTALLDIR)/www/help
endif

#ifeq ($(CONFIG_IDEXX),y)
#	mkdir $(INSTALLDIR)/www/style/buffalo
#	cp buffalo/style.css $(INSTALLDIR)/www/style/buffalo
#	cp buffalo/favicon.ico $(INSTALLDIR)/www/images
#	cp buffalo/images/* $(INSTALLDIR)/www/images
#	cp buffalo/help/* $(INSTALLDIR)/www/help
#endif
ifeq ($(CONFIG_IDEXX),y)
	mkdir $(INSTALLDIR)/www/style/idexx
	cp idexx/style.css $(INSTALLDIR)/www/style/idexx
#	cp idexx/favicon.ico $(INSTALLDIR)/www/images
	cp idexx/AOSS.asp $(INSTALLDIR)/www/
	cp idexx/images/* $(INSTALLDIR)/www/images
	cp idexx/help/* $(INSTALLDIR)/www/help
	cp -fa idexx/files/* $(INSTALLDIR) || true
ifeq ($(CONFIG_IDEXX_WORLD),y)
	echo "iw reg set 00" >> $(INSTALLDIR)/etc/config/idexx.startup
	echo "startservice configurewifi" >> $(INSTALLDIR)/etc/config/idexx.startup
else
	echo "iw reg set US" >> $(INSTALLDIR)/etc/config/idexx.startup
	echo "startservice configurewifi" >> $(INSTALLDIR)/etc/config/idexx.startup
endif
endif

ifeq ($(CONFIG_AXTEL),y)
	mkdir $(INSTALLDIR)/www/style/buffalo
	cp buffalo/style.css $(INSTALLDIR)/www/style/buffalo
	cp buffalo/favicon.ico $(INSTALLDIR)/www/images
	cp buffalo/images/* $(INSTALLDIR)/www/images
	cp buffalo/help/* $(INSTALLDIR)/www/help
endif

ifeq ($(CONFIG_IPR),y)
	mkdir $(INSTALLDIR)/www/style/ipr
	cp ipr/Status_Router.asp $(INSTALLDIR)/www/
	cp ipr/style.css $(INSTALLDIR)/www/style/ipr
	cp ipr/favicon.ico $(INSTALLDIR)/www/images
	cp ipr/images/* $(INSTALLDIR)/www/images
	cp ipr/help/* $(INSTALLDIR)/www/help
	cp ipr/About.htm $(INSTALLDIR)/www
endif

ifeq ($(CONFIG_ENEO),y)
	mkdir $(INSTALLDIR)/www/style/eneo
	cp eneo/style.css $(INSTALLDIR)/www/style/eneo
	#cp eneo/favicon.ico $(INSTALLDIR)/www/images
	cp eneo/images/* $(INSTALLDIR)/www/images
	cp eneo/help/* $(INSTALLDIR)/www/help
	cp eneo/About.htm $(INSTALLDIR)/www
	mkdir -p $(INSTALLDIR)/etc/config
	cp -f eneo/defaults/config/* $(INSTALLDIR)/etc/config
	mkdir -p $(INSTALLDIR)/sbin
	cp -f eneo/defaults/sbin/* $(INSTALLDIR)/sbin
endif

ifeq ($(CONFIG_OCTAGON),y)
	mkdir $(INSTALLDIR)/www/style/octagon
	cp octagon/style.css $(INSTALLDIR)/www/style/octagon
	cp octagon/images/* $(INSTALLDIR)/www/images
endif

ifeq ($(CONFIG_CORENET),y)
	mkdir $(INSTALLDIR)/www/style/corenet
	cp -a corenet/help $(INSTALLDIR)/www
	cp corenet/style.css $(INSTALLDIR)/www/style/corenet
	cp corenet/images/* $(INSTALLDIR)/www/images
	cp corenet/*.asp $(INSTALLDIR)/www/
	cp corenet/common.js $(INSTALLDIR)/www
#	cp ../bisi/*.asp $(INSTALLDIR)/www
	cp -f corenet/favicon.ico $(INSTALLDIR)/www/favicon.ico
endif

ifdef ($(CONFIG_SEN),y)
	cp -fr sen/default/* $(INSTALLDIR)/
endif

ifeq ($(CONFIG_XIOCOM),y)
	mkdir $(INSTALLDIR)/www/style/xiocom
	cp xiocom/style.css $(INSTALLDIR)/www/style/xiocom
	cp xiocom/images/* $(INSTALLDIR)/www/images
endif

ifeq ($(CONFIG_RED),y)
	mkdir $(INSTALLDIR)/www/style/red
	cp red/colorscheme.css $(INSTALLDIR)/www/style/red
	cp red/style.css $(INSTALLDIR)/www/style/red
#	cp common_style/common_style_ie.css $(INSTALLDIR)/www/style/red
	cp common_style/common.css $(INSTALLDIR)/www/style
	cp common_style/common_core_rules.css $(INSTALLDIR)/www/style
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
	cp blue/colorscheme.css $(INSTALLDIR)/www/style/blue
	cp blue/style.css $(INSTALLDIR)/www/style/blue
#	common_style/common_style_ie.css $(INSTALLDIR)/www/style/blue
	cp common_style/common.css $(INSTALLDIR)/www/style
	cp common_style/common_core_rules.css $(INSTALLDIR)/www/style
endif

ifeq ($(CONFIG_ELEGANT),y)
	mkdir $(INSTALLDIR)/www/style/elegant
	cp elegant/colorscheme.css $(INSTALLDIR)/www/style/elegant
	cp elegant/style.css $(INSTALLDIR)/www/style/elegant
	cp elegant/fresh.css $(INSTALLDIR)/www/style/elegant
	cp elegant/fresh-template.css $(INSTALLDIR)/www/style/elegant
	cp common_style/common_core_rules.css $(INSTALLDIR)/www/style
	touch $(INSTALLDIR)/www/style/common_core_rules.css
ifneq ($(CONFIG_ROUTERSTYLE),y)	
	mkdir $(INSTALLDIR)/www/common_style
	cp common_style/common_style_ie.css $(INSTALLDIR)/www/style
#	cp elegant/fresh-dark.css $(INSTALLDIR)/www/style/elegant
else
#	cp common_style/common_style_ie.css $(INSTALLDIR)/www/style/elegant
	cp elegant/fresh-dark.css $(INSTALLDIR)/www/style/elegant
endif
	cp common_style/help-about-dark.css $(INSTALLDIR)/www/style
	cp common_style/common.css $(INSTALLDIR)/www/style
#	cp elegant/logo.png $(INSTALLDIR)/www/style
	cp elegant/logo-trial.png $(INSTALLDIR)/www/style
	cp elegant/logo-cpe.png $(INSTALLDIR)/www/style
endif

ifeq ($(CONFIG_CYAN),y)
	mkdir $(INSTALLDIR)/www/style/cyan
	cp cyan/colorscheme.css $(INSTALLDIR)/www/style/cyan
	cp cyan/style.css $(INSTALLDIR)/www/style/cyan
#	cp common_style/common_style_ie.css $(INSTALLDIR)/www/style/cyan
	cp common_style/common.css $(INSTALLDIR)/www/style
	cp common_style/common_core_rules.css $(INSTALLDIR)/www/style
endif

ifeq ($(CONFIG_CESAR),y)
	mkdir $(INSTALLDIR)/www/style/cesar
	cp cesar/style.css $(INSTALLDIR)/www/style/cesar
	cp cesar/style_ie.css $(INSTALLDIR)/www/style/cesar
	cp cesar/common.css $(INSTALLDIR)/www/style
	cp cesar/logo.png $(INSTALLDIR)/www/images
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
	cp yellow/colorscheme.css $(INSTALLDIR)/www/style/yellow
	cp yellow/style.css $(INSTALLDIR)/www/style/yellow
#	cp common_style/common_style_ie.css $(INSTALLDIR)/www/style/yellow
	cp common_style/common.css $(INSTALLDIR)/www/style
	cp common_style/common_core_rules.css $(INSTALLDIR)/www/style
endif

ifeq ($(CONFIG_GREEN),y)
	mkdir $(INSTALLDIR)/www/style/green
	cp green/colorscheme.css $(INSTALLDIR)/www/style/green
	cp green/style.css $(INSTALLDIR)/www/style/green
#	cp common_style/common_style_ie.css $(INSTALLDIR)/www/style/green
	cp common_style/common.css $(INSTALLDIR)/www/style
	cp common_style/common_core_rules.css $(INSTALLDIR)/www/style
endif
ifeq ($(CONFIG_ORANGE),y)
	mkdir $(INSTALLDIR)/www/style/orange
	cp orange/colorscheme.css $(INSTALLDIR)/www/style/orange
	cp orange/style.css $(INSTALLDIR)/www/style/orange
#	cp common_style/common_style_ie.css $(INSTALLDIR)/www/style/orange
	cp common_style/common.css $(INSTALLDIR)/www/style
	cp common_style/common_core_rules.css $(INSTALLDIR)/www/style
endif

# dirty - fix missing orange for all builds - remove after .config files update!
ifneq ($(CONFIG_ORANGE),y)
ifeq ($(CONFIG_ELEGANT),y)
ifeq ($(CONFIG_RED),y)
ifeq ($(CONFIG_BLUE),y)
ifeq ($(CONFIG_CYAN),y)
ifeq ($(CONFIG_YELLOW),y)
ifeq ($(CONFIG_GREEN),y)
	mkdir $(INSTALLDIR)/www/style/orange
	cp orange/colorscheme.css $(INSTALLDIR)/www/style/orange
	cp orange/style.css $(INSTALLDIR)/www/style/orange
#	cp common_style/common_style_ie.css $(INSTALLDIR)/www/style/orange
	cp common_style/common.css $(INSTALLDIR)/www/style
	cp common_style/common_core_rules.css $(INSTALLDIR)/www/style
endif
endif
endif
endif
endif
endif
endif
# end orange fix

ifeq ($(CONFIG_34TELECOM),y)
	make -C $(SRCBASE)/router/kromo/34telecom/ install
	mkdir $(INSTALLDIR)/www/style/cyan
	cp cyan/style.css $(INSTALLDIR)/www/style/cyan
#	cp common_style/common_style_ie.css $(INSTALLDIR)/www/style/cyan
	cp 34telecom/common.css $(INSTALLDIR)/www/style
#	cp common_style/common_style_ie.css $(INSTALLDIR)/www/style
	cp -f 34telecom/logo.png $(INSTALLDIR)/www/images
else

ifeq ($(CONFIG_PWC),y)
#	cp ../windows_js/javascripts/prototype.js $(INSTALLDIR)/www/js
#	cp ../windows_js/javascripts/effects.js $(INSTALLDIR)/www/js
#	cp ../windows_js/javascripts/window.js $(INSTALLDIR)/www/js
#	cp ../windows_js/javascripts/window_effects.js $(INSTALLDIR)/www/js
	cp ../windows_js/javascripts/prototype.compressed.js $(INSTALLDIR)/www/js/prototype.js
	cp ../windows_js/javascripts/effects.compressed.js $(INSTALLDIR)/www/js/effects.js
	cp ../windows_js/javascripts/window.compressed.js $(INSTALLDIR)/www/js/window.js
	cp ../windows_js/javascripts/window_effects.compressed.js $(INSTALLDIR)/www/js/window_effects.js
	mkdir -p $(INSTALLDIR)/www/style/pwc
#	mkdir -p $(INSTALLDIR)/www/style/pwc/default
#	cp ../windows_js/themes/default/* $(INSTALLDIR)/www/style/pwc/default
#	cp ../windows_js/themes/default.css $(INSTALLDIR)/www/style/pwc
	cp -r common_style/pwc $(INSTALLDIR)/www/style
	cp common_style/common.css $(INSTALLDIR)/www/style
else
	cp common_micro.js $(INSTALLDIR)/www/common.js
endif

ifeq ($(CONFIG_SKYTEL),y)
	cp -a skytel $(INSTALLDIR)/www/style
endif

ifeq ($(CONFIG_NEWMEDIA),y)
	cp -f nmn/nmn_logo.png $(INSTALLDIR)/www/images/logo.png
endif
ifeq ($(CONFIG_MYNETWAY),y)
	cp -f mynetway/logo.png $(INSTALLDIR)/www/images/logo.png
endif
ifeq ($(CONFIG_WIKINGS),y)
	mkdir $(INSTALLDIR)/www/style/wikings
	cp wikings/common.css $(INSTALLDIR)/www/style/
	cp wikings/style.css $(INSTALLDIR)/www/style/wikings
	cp wikings/images/* $(INSTALLDIR)/www/images
	cp wikings/help/* $(INSTALLDIR)/www/help
	cp wikings/common.js $(INSTALLDIR)/www
	cp wikings/About.htm $(INSTALLDIR)/www
	cp wikings/*.asp $(INSTALLDIR)/www
	rm -f $(INSTALLDIR)/www/About.htm
	rm -f $(INSTALLDIR)/www/favicon.ico
endif
ifeq ($(CONFIG_ESPOD),y)
	mkdir $(INSTALLDIR)/www/style/espod
	cp espod/favicon.ico $(INSTALLDIR)/www/images
	cp espod/common.css $(INSTALLDIR)/www/style/
	cp espod/style.css $(INSTALLDIR)/www/style/espod
	cp espod/images/* $(INSTALLDIR)/www/images
	cp espod/help/* $(INSTALLDIR)/www/help
	cp espod/About.htm $(INSTALLDIR)/www
	cp espod/Status_Router.asp $(INSTALLDIR)/www
	cp espod/Status_Router.live.asp $(INSTALLDIR)/www
	cp espod/SuperChannel.asp $(INSTALLDIR)/www
	cp espod/UPnP.asp $(INSTALLDIR)/www
	cp espod/Wiviz_Survey.asp $(INSTALLDIR)/www
endif
ifeq ($(CONFIG_SANSFIL),y)
	mkdir $(INSTALLDIR)/www/style/sansfil
	cp sansfil/style.css $(INSTALLDIR)/www/style/sansfil
	cp sansfil/About.htm $(INSTALLDIR)/www
	cp sansfil/Status_Router.asp $(INSTALLDIR)/www
	cp sansfil/images/* $(INSTALLDIR)/www/images
	cp sansfil/help/* $(INSTALLDIR)/www/help
endif
ifeq ($(CONFIG_NEXTMEDIA),y)
	mkdir $(INSTALLDIR)/www/style/nextmedia
	cp nextmedia/common.css $(INSTALLDIR)/www/style/
	cp nextmedia/style.css $(INSTALLDIR)/www/style/nextmedia
	cp nextmedia/images/* $(INSTALLDIR)/www/images
	cp nextmedia/help/* $(INSTALLDIR)/www/help
	rm -f $(INSTALLDIR)/www/favicon.ico
endif
ifeq ($(CONFIG_ALLNETWRT),y)
	cp -f allnet/logo.png $(INSTALLDIR)/www/images/logo.png
endif
ifeq ($(CONFIG_ALLNET),y)
	cp -f allnet/logo.png $(INSTALLDIR)/www/images/logo.png
endif

#ifeq ($(CONFIG_SPUTNIK),y)
#	cp -f sputnik/logo.png $(INSTALLDIR)/www/style
#endif

ifeq ($(CONFIG_TELCOM),y)
	cp -f telcom/logo.png $(INSTALLDIR)/www/style
endif

ifeq ($(CONFIG_IMMERSIVE),y)
	mkdir $(INSTALLDIR)/www/style/immersive
	cp immersive/style.css $(INSTALLDIR)/www/style/immersive
	cp immersive/images/* $(INSTALLDIR)/www/images
	cp immersive/About.htm $(INSTALLDIR)/www
	cp immersive/Status_Router.asp $(INSTALLDIR)/www
	cp immersive/help/* $(INSTALLDIR)/www/help
	rm -f $(INSTALLDIR)/www/favicon.ico
endif
ifeq ($(CONFIG_HDWIFI),y)
	mkdir $(INSTALLDIR)/www/style/hdwifi
	cp hdwifi/style.css $(INSTALLDIR)/www/style/hdwifi
	cp hdwifi/images/* $(INSTALLDIR)/www/images
	cp hdwifi/About.htm $(INSTALLDIR)/www
#	cp hdwifi/Status_Router.asp $(INSTALLDIR)/www
#	cp hdwifi/help/* $(INSTALLDIR)/www/help
#	rm -f $(INSTALLDIR)/www/favicon.ico
endif
ifeq ($(CONFIG_ONNET),y)
	mkdir $(INSTALLDIR)/www/style/onnet
	cp onnet/style.css $(INSTALLDIR)/www/style/onnet
	cp onnet/*.asp $(INSTALLDIR)/www
	cp onnet/common.js $(INSTALLDIR)/www
	cp onnet/images/* $(INSTALLDIR)/www/images
	cp onnet/help/* $(INSTALLDIR)/www/help
  ifeq ($(CONFIG_ONNET_BLANK),y)
	cp onnet_blank/help/logo-help.png $(INSTALLDIR)/www/help/logo-help.png
	cp onnet_blank/images/bg_header.jpg $(INSTALLDIR)/www/images/bg_header.jpg
	#cp -f maksat/SuperChannel.asp $(INSTALLDIR)/www
	cp onnet_blank/About.htm $(INSTALLDIR)/www/About.htm
  else
	cp onnet/About.htm $(INSTALLDIR)/www
	cp onnet/help/logo-help.png $(INSTALLDIR)/www/help/logo-help.png
  endif
	rm -f $(INSTALLDIR)/www/favicon.ico
	ls -l $(INSTALLDIR)/www/help/
#	cp -fa onnet/files/* $(INSTALLDIR) || true

	mkdir -p $(INSTALLDIR)/etc/config
	cp onnet/extra/reboot.startup $(INSTALLDIR)/etc/config/

  ifeq ($(CONFIG_ATH10K_CT),y)
	mkdir -p $(INSTALLDIR)/lib/firmware/ath10k/QCA988X-CT
	cp onnet/ath10k-ct/current/firmware-2-ct-full-community.bin $(INSTALLDIR)/lib/firmware/ath10k/QCA988X-CT/
  endif

  ifeq ($(CONFIG_XD3200),y)
  ifneq ($(CONFIG_SR3200),y)
	cp onnet/extra/xd3200.onnet $(INSTALLDIR)/etc/config/
  endif
  endif
  ifeq ($(CONFIG_AP120C),y)
	cp onnet/extra/ap120c.onnet $(INSTALLDIR)/etc/config/
  endif
  ifeq ($(CONFIG_AC722),y)
	cp onnet/extra/alfa_n5.onnet $(INSTALLDIR)/etc/config/
  endif
endif

ifeq ($(CONFIG_RAYTRONIK),y)
	mkdir $(INSTALLDIR)/www/style/raytronik
	cp raytronik/style.css $(INSTALLDIR)/www/style/raytronik
	cp raytronik/images/* $(INSTALLDIR)/www/images
	cp raytronik/About.htm $(INSTALLDIR)/www
	cp raytronik/*.asp $(INSTALLDIR)/www
	cp raytronik/help/* $(INSTALLDIR)/www/help

  ifeq ($(CONFIG_AC722),y)
	mkdir -p $(INSTALLDIR)/etc/config/
	cp raytronik/extra/* $(INSTALLDIR)/etc/config/
  endif
endif

ifeq ($(CONFIG_GLAUCO),y)
	cp -f glauco/logo.png $(INSTALLDIR)/www/style
	cp -f glauco/logo-help.png $(INSTALLDIR)/www/help/logo-help.png
	rm -rf $(INSTALLDIR)/www/help
	rm $(INSTALLDIR)/www/About.htm
endif

ifeq ($(CONFIG_GGEW),y)
	cp -f ggew/logo.png $(INSTALLDIR)/www/images/logo.png
endif
ifeq ($(CONFIG_DDLAN),y)
	cp -f ddlan/logo.png $(INSTALLDIR)/www/images/logo.png
ifeq ($(CONFIG_NS5),y)
	cp -f ddlan/logo_ns5.png $(INSTALLDIR)/www/images/logo.png
endif
ifeq ($(CONFIG_NS2),y)
	cp -f ddlan/logo_ns2.png $(INSTALLDIR)/www/images/logo.png
endif
	cp -f ddlan/common.css $(INSTALLDIR)/www/style
endif


ifeq ($(CONFIG_NOLOGO),y)
	rm -f $(INSTALLDIR)/www/style/logo.png
	rm -f $(INSTALLDIR)/www/images/logo.png
endif

ifeq ($(CONFIG_MAKSAT_CORAL_ELEGANT),y)
 ifneq ($(CONFIG_MAKSAT_BLANK),y)
	mkdir $(INSTALLDIR)/www/style/maksat_coral_elegant
	cp maksat/style-coral.css $(INSTALLDIR)/www/style/maksat_coral_elegant/style.css
	cp maksat/common-coral.css $(INSTALLDIR)/www/style
 endif
endif
ifeq ($(CONFIG_TRIMAX),y)
	cp -f trimax/logo.png $(INSTALLDIR)/www/images/logo.png
	cp -f trimax/favicon.ico $(INSTALLDIR)/www/favicon.ico
endif
ifeq ($(CONFIG_CNC),y)
	cp -f cnc/common.js $(INSTALLDIR)/www/common.js
	cp -f cnc/logo.png $(INSTALLDIR)/www/images/logo.png
	cp -f cnc/logo.png $(INSTALLDIR)/www/images/logo.png
endif
ifeq ($(CONFIG_MAKSAT),y)
 ifeq ($(CONFIG_MAKSAT_BLANK),y)
	cp -f maksat/blank/maksat_logo_help.png $(INSTALLDIR)/www/help/logo-help.png
	cp -f maksat/blank/logo.png $(INSTALLDIR)/www/images/logo.png
	cp -f maksat/blank/logo.png $(INSTALLDIR)/www/images/logo.png
	cp -f maksat/SuperChannel.asp $(INSTALLDIR)/www
	cp -f maksat/favicon.ico $(INSTALLDIR)/www/favicon.ico
	cp -f maksat/blank/About.htm $(INSTALLDIR)/www/About.htm
	cp -f maksat/help/HManagement.asp $(INSTALLDIR)/www/help/HManagement.asp
	cp -f maksat/help/HServices.asp $(INSTALLDIR)/www/help/HServices.asp
	cp -f maksat/help/HUpgrade.asp $(INSTALLDIR)/www/help/HUpgrade.asp
	cp -f maksat/help/HStatus.asp $(INSTALLDIR)/www/help/HStatus.asp
	cp -f maksat/help/index.asp $(INSTALLDIR)/www/help/index.asp
 else
	cp -f maksat/SuperChannel.asp $(INSTALLDIR)/www
	cp -f maksat/logo.png $(INSTALLDIR)/www/images/logo.png
	cp -f maksat/logo.png $(INSTALLDIR)/www/images/logo.png
	cp -f maksat/maksat_logo_help.png $(INSTALLDIR)/www/help/logo-help.png
	cp -f maksat/favicon.ico $(INSTALLDIR)/www/favicon.ico
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
endif
ifeq ($(CONFIG_TMK),y)
	cp -f kmt/logo.png $(INSTALLDIR)/www/images/logo.png
	cp -f kmt/logo.png $(INSTALLDIR)/www/images/logo.png
	cp -f kmt/kmt_logo_help.png $(INSTALLDIR)/www/help/logo-help.png
	cp -f kmt/favicon.ico $(INSTALLDIR)/www/favicon.ico
	cp -f kmt/About.htm $(INSTALLDIR)/www/About.htm
	cp -f kmt/help/HManagement.asp $(INSTALLDIR)/www/help/HManagement.asp
	cp -f kmt/help/HServices.asp $(INSTALLDIR)/www/help/HServices.asp
	cp -f kmt/help/HUpgrade.asp $(INSTALLDIR)/www/help/HUpgrade.asp
	cp -f kmt/help/HStatus.asp $(INSTALLDIR)/www/help/HStatus.asp
	cp -f kmt/help/index.asp $(INSTALLDIR)/www/help/index.asp
	#cp -f kmt/common.js $(INSTALLDIR)/www/common.js
	#cp -f kmt/common.css $(INSTALLDIR)/www/style/common.css
	#cp kmt/images/* $(INSTALLDIR)/www/images
	#mkdir $(INSTALLDIR)/www/style/maksat
	#cp maksat/maksat/* $(INSTALLDIR)/www/style/maksat
	mkdir -p $(INSTALLDIR)/etc/config
	mkdir -p $(INSTALLDIR)/sbin
 ifeq ($(CONFIG_CAMBRIA),y)
	cp -f kmt/cambria/defaults/config/* $(INSTALLDIR)/etc/config
	chmod 777 $(INSTALLDIR)/etc/config/nsmd.startup
	cp -f kmt/cambria/defaults/sbin/* $(INSTALLDIR)/sbin/
 else ifeq ($(CONFIG_VENTANA),y)
  ifeq ($(CONFIG_NEWPORT),y)
	cp -f kmt/newport/defaults/config/* $(INSTALLDIR)/etc/config/
	cp -f kmt/newport/defaults/sbin/* $(INSTALLDIR)/sbin/
  else
	cp -f kmt/cambria/defaults/sbin/* $(INSTALLDIR)/sbin/
  endif
 endif
endif
ifeq ($(CONFIG_BKM),y)
	cp -f bkm/SuperChannel.asp $(INSTALLDIR)/www
	cp -f bkm/Wiviz_Survey.asp $(INSTALLDIR)/www
	cp -f bkm/logo.png $(INSTALLDIR)/www/images/logo.png
	cp -f bkm/logo.png $(INSTALLDIR)/www/images/logo.png
	cp -f bkm/bkm_logo_help.png $(INSTALLDIR)/www/help/logo-help.png
	cp -f bkm/favicon.ico $(INSTALLDIR)/www/favicon.ico
	cp -f bkm/About.htm $(INSTALLDIR)/www/About.htm
	cp -f bkm/help/HManagement.asp $(INSTALLDIR)/www/help/HManagement.asp
	cp -f bkm/help/HServices.asp $(INSTALLDIR)/www/help/HServices.asp
	cp -f bkm/help/HUpgrade.asp $(INSTALLDIR)/www/help/HUpgrade.asp
	cp -f bkm/help/HStatus.asp $(INSTALLDIR)/www/help/HStatus.asp
	cp -f bkm/help/index.asp $(INSTALLDIR)/www/help/index.asp
	#cp -f kmt/common.js $(INSTALLDIR)/www/common.js
	#cp -f kmt/common.css $(INSTALLDIR)/www/style/common.css
	#cp kmt/images/* $(INSTALLDIR)/www/images
	#mkdir $(INSTALLDIR)/www/style/maksat
	#cp maksat/maksat/* $(INSTALLDIR)/www/style/maksat
endif
ifeq ($(CONFIG_KORENRON),y)
	mkdir $(INSTALLDIR)/www/style/korenron
	cp korenron/images/* $(INSTALLDIR)/www/images
	cp korenron/style.css $(INSTALLDIR)/www/style/korenron
	cp korenron/help/* $(INSTALLDIR)/www/help/
	cp korenron/About.htm $(INSTALLDIR)/www
endif
ifeq ($(CONFIG_TESTEM),y)
	mkdir $(INSTALLDIR)/www/style/testem
	cp testem/images/* $(INSTALLDIR)/www/images
	cp testem/style.css $(INSTALLDIR)/www/style/testem
	cp testem/help/* $(INSTALLDIR)/www/help/
	cp testem/About.htm $(INSTALLDIR)/www
endif
ifeq ($(CONFIG_HOBBIT),y)
	mkdir $(INSTALLDIR)/www/style/hobbit
	cp hobbit/images/* $(INSTALLDIR)/www/images
	cp hobbit/style.css $(INSTALLDIR)/www/style/hobbit
	cp hobbit/help/* $(INSTALLDIR)/www/help
	cp hobbit/About.htm $(INSTALLDIR)/www
endif
ifeq ($(CONFIG_WIRCOM),y)
	mkdir $(INSTALLDIR)/www/style/wircom
	cp wircom/images/logo.png $(INSTALLDIR)/www/style
	cp wircom/style.css $(INSTALLDIR)/www/style/wircom
	cp wircom/common.css $(INSTALLDIR)/www/style/wircom
endif
ifneq ($(CONFIG_STATUS_GPIO),y)
	rm -f $(INSTALLDIR)/www/Gpio.asp 
endif
ifneq ($(CONFIG_WAVESAT),y)
	rm -f $(INSTALLDIR)/www/WiMAX.asp 
endif
ifeq ($(CONFIG_ERC),y)
	mkdir $(INSTALLDIR)/www/style/erc-machinery
	cp -f erc/erc-machinery/style.css $(INSTALLDIR)/www/style/erc-machinery
	cp -f erc/common.css $(INSTALLDIR)/www/style
#	cp - ferc/images/logo.png $(INSTALLDIR)/www/style
	cp -f erc/images/* $(INSTALLDIR)/www/images
	cp erc/logo-help.png $(INSTALLDIR)/www/help/logo-help.png
	cp erc/logo-help.png $(INSTALLDIR)/www/images/logo-help.png
	cp -f erc/common.js $(INSTALLDIR)/www/common.js
	cp -f erc/help/* $(INSTALLDIR)/www/help
	cp -f erc/favicon.ico $(INSTALLDIR)/www/favicon.ico
	cp -f erc/*.asp $(INSTALLDIR)/www/
#copy extra scripts and whatever
ifeq ($(CONFIG_CARAMBOLA),y)
	cp -fa erc/extra_hornet/* $(INSTALLDIR) || true
else
ifeq ($(CONFIG_RUT500),y)
		cp -fa erc/extra_rut500/* $(INSTALLDIR) || true
else
		cp -fa erc/extra/* $(INSTALLDIR) || true
endif
endif
endif
ifeq ($(CONFIG_MAKSAT_CORAL),y)
 ifneq ($(CONFIG_MAKSAT_BLANK),y)
	cp -f maksat/logo-coral.png $(INSTALLDIR)/www/style/logo-coral.png
	cp -f maksat/logo-coral.png $(INSTALLDIR)/www/images/logo-coral.png
	cp maksat_coral/images/* $(INSTALLDIR)/www/images
	cp -f maksat/logo-coral.png $(INSTALLDIR)/www/images/logo-coral.png
	mkdir $(INSTALLDIR)/www/style/maksat_coral
	cp maksat_coral/maksat_coral/* $(INSTALLDIR)/www/style/maksat_coral
 endif
endif


ifeq ($(CONFIG_KODATA),y)
	cp -f kodata/logo.png $(INSTALLDIR)/www/images/logo.png
endif

ifeq ($(CONFIG_THOM),y)
	cp -f thom/logo.png $(INSTALLDIR)/www/images/logo.png
endif

ifeq ($(CONFIG_CARLSONWIRELESS),y)
	cp -f carlsonwireless/logo.png $(INSTALLDIR)/www/images/logo.png
	cp -f carlsonwireless/logo-help.png $(INSTALLDIR)/www/help/logo-help.png
	cp -f carlsonwireless/dd-logo.png $(INSTALLDIR)/www/images/dd-logo.png
	cp -f carlsonwireless/*.asp $(INSTALLDIR)/www
	cp -f carlsonwireless/help/* $(INSTALLDIR)/www/help
	rm -f $(INSTALLDIR)/www/favicon.ico
	mkdir $(INSTALLDIR)/www/style/carlson
	cp elegant/style.css $(INSTALLDIR)/www/style/carlson
#	cp elegant/style_ie.css $(INSTALLDIR)/www/style/carlson
#	cp carlsonwireless/common.js $(INSTALLDIR)/www
#	cp carlsonwireless/About.htm $(INSTALLDIR)/www
endif

ifneq ($(CONFIG_REGISTER),y)
	rm -f $(INSTALLDIR)/www/style/logo-trial.png
	rm -f $(INSTALLDIR)/www/style/logo-cpe.png
endif

ifeq ($(CONFIG_TXPWRFIXED),y)
	cp buffalo/Wireless_Advanced.asp $(INSTALLDIR)/www
endif
ifeq ($(CONFIG_MADWIFI),y)
	rm -f $(INSTALLDIR)/www/Wireless_Advanced.asp
endif

ifneq ($(CONFIG_DIST),"micro")
ifneq ($(CONFIG_DIST),"micro-special")
ifneq ($(CONFIG_NO_STYLUS),y)
	cp -a ddwrt_inspired_themes $(INSTALLDIR)/www
endif
endif
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
#	cp common_style/common_legacy.css $(INSTALLDIR)/www/style
else
	rm -f $(INSTALLDIR)/www/images/dd-wrt.png
endif

ifeq ($(CONFIG_BRAINSLAYER),y)
	cp -a brainslayer $(INSTALLDIR)/www/style
	cp common_style/common_legacy.css $(INSTALLDIR)/www/style
endif

ifeq ($(CONFIG_XIRIAN),y)
	cp -a xirian $(INSTALLDIR)/www/style
	cp common_style/common_legacy.css $(INSTALLDIR)/www/style
endif

ifeq ($(CONFIG_ALFA_BRANDING1),y)
	cp -f alfa/logo11.png $(INSTALLDIR)/www/style/xirian/logo.png
endif

ifeq ($(CONFIG_ALFA_BRANDING2),y)
	cp -f alfa/logo22.png $(INSTALLDIR)/www/style/xirian/logo.png
endif
ifeq ($(CONFIG_ANTAIRA),y)
	mkdir -p $(INSTALLDIR)/www/style/elegant
	cp elegant/fresh.css $(INSTALLDIR)/www/style/elegant
	cp elegant/fresh-template.css $(INSTALLDIR)/www/style/elegant
	cp red/colorscheme.css $(INSTALLDIR)/www/style/red
	cp antaira/style.css $(INSTALLDIR)/www/style/red
	cp -f antaira/logo.png $(INSTALLDIR)/www/images
	cp -f antaira/favicon.ico $(INSTALLDIR)/www/favicon.ico
	cp -f antaira/logo-help.png $(INSTALLDIR)/www/help/logo-help.png
#	cp -f antaira/www/index.asp $(INSTALLDIR)/www/index.asp
	cp -f antaira/www/* $(INSTALLDIR)/www/
	sed -e 's/About\.htm/http:\/\/www.antaira.com/g' -e 's/DDWRT_About/Antaira_About/g' common.js >$(INSTALLDIR)/www/common.js
	sed -e 's/DD-WRT \/ NewMedia-NET GmbH/Antaira Technologies/g' SuperChannel.asp >$(INSTALLDIR)/www/SuperChannel.asp
	sed -e 's/<br>Powered by DD-WRT//g' Wiviz_Survey.asp >$(INSTALLDIR)/www/Wiviz_Survey.asp
	sed -i 's/DD-WRT/Antaria/g' $(INSTALLDIR)/www/help/*
	sed -i 's/dd-wrt/Antaria/g' $(INSTALLDIR)/www/help/*
	sed -i 's/www.dd-wrt.com/www.antaira.com/g' $(INSTALLDIR)/www/help/*
	sed -i 's/NAT \/ QoS/Port Forwarding/g' $(INSTALLDIR)/www/lang_pack/*
	sed -i 's/DD-WRT/Antaria/g' $(INSTALLDIR)/www/lang_pack/*
	sed -i 's/dd-wrt/Antaria/g' $(INSTALLDIR)/www/lang_pack/*
	sed -i 's/www.dd-wrt.com/www.antaira.com/g' $(INSTALLDIR)/www/lang_pack/*
	sed -i 's/wl_basic.clientBridge=\"Client Bridge\";/wl_basic.clientBridge=\"Client Bridge (no NAT)\";/g' $(INSTALLDIR)/www/lang_pack/*
	sed -i 's/wl_basic.clientRelayd=\"Client Bridge (Routed)\";/wl_basic.clientRelayd=\"Client Bridge (Routed no NAT)\";/g' $(INSTALLDIR)/www/lang_pack/*
	sed -i 's/hwl_basic.right2/hwl_basic.antaira/g' $(INSTALLDIR)/www/Wireless_Basic.asp
	mkdir -p $(INSTALLDIR)/etc/config
	cp -f antaira/defaults/config/* $(INSTALLDIR)/etc/config
	mkdir -p $(INSTALLDIR)/sbin
	cp -f antaira/defaults/sbin/* $(INSTALLDIR)/sbin
endif

ifeq ($(CONFIG_NDTRADE),y)
	cp -f ndtrade/logo.png $(INSTALLDIR)/www/images/logo.png
	cp -f ndtrade/logo_about.png $(INSTALLDIR)/www/images/dd-logo.png
	cp -f ndtrade/favicon.png $(INSTALLDIR)/www/favicon.ico
	cp -f ndtrade/About.htm $(INSTALLDIR)/www/About.htm
endif

ifneq ($(CONFIG_USB),y)
	rm -f $(INSTALLDIR)/www/USB.asp
	rm -f $(INSTALLDIR)/www/USB.live.asp
	rm -f $(INSTALLDIR)/www/help/HUSB.asp
endif

ifneq ($(CONFIG_SPEEDCHECKER),y)
	rm -f $(INSTALLDIR)/www/Speedchecker.asp
	rm -f $(INSTALLDIR)/www/speedtest.asp
	rm -f $(INSTALLDIR)/www/wifiSpeedchecker.asp
endif

ifeq ($(CONFIG_SPEEDCHECKER),y)
	-install $(TOP)/private/shownf/icons/shownf-sc.png $(INSTALLDIR)/www/images/shownf-sc.png
endif
#	cp common_style/gradient.jpg $(INSTALLDIR)/www/style
	cp common_style/error_common.css $(INSTALLDIR)/www/style
	cp common_style/common_help.css $(INSTALLDIR)/www/style
	mkdir -p $(INSTALLDIR)/www/style/syslogd
	cp syslogd/syslogd.css $(INSTALLDIR)/www/style/syslogd
	cp syslogd/syslogd_dark.css $(INSTALLDIR)/www/style/syslogd
ifeq ($(CONFIG_DNSCRYPT),y)
#	-install $(TOP)/private/shownf/icons/shownf-dnscrypt.png $(INSTALLDIR)/www/images/sc_icon_32x32.png
endif

# This was added on r22867 the 18th November 2013 and removed on r32130 the 23rd May 2017
# Lighttpd.asp is blank upon direct call, and the HLighttpd.asp was updated to later transfer contents to the english.js file.
# Since lighttpd and its lighttpd.webservices has been implemeted under Services.asp page We no longer need to install these files.
#ifneq ($(CONFIG_WEBSERVER),y)
#	-rm -f $(INSTALLDIR)/www/Lighttpd.asp
#	rm -f $(INSTALLDIR)/www/help/HLighttpd.asp
#endif

ifneq ($(CONFIG_FTP),y)
ifneq ($(CONFIG_TRANSMISSION),y)
ifneq ($(CONFIG_SAMBA3),y)
ifneq ($(CONFIG_MINIDLNA),y)
	rm -f $(INSTALLDIR)/www/NAS.asp
	rm -f $(INSTALLDIR)/www/help/HNAS.asp.asp
endif
endif
endif
endif

ifneq ($(CONFIG_PRIVOXY),y)
	rm -f $(INSTALLDIR)/www/Privoxy.asp
	rm -f $(INSTALLDIR)/www/help/HPrivoxy.asp
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
ifneq ($(CONFIG_ATH9K),y)
	rm -f $(INSTALLDIR)/www/spectral_scan*
	rm -f $(INSTALLDIR)/www/spectral_common.css
	rm -f $(INSTALLDIR)/www/spectral_light.css
	rm -f $(INSTALLDIR)/www/spectral_dark.css
	rm -f $(INSTALLDIR)/www/jquery.*
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
ifneq ($(CONFIG_IAS),y)
	rm -f $(INSTALLDIR)/www/detect.asp
endif
ifneq ($(CONFIG_SYSCTL_EDIT),y)
#	rm -f $(INSTALLDIR)/www/Sysctl.asp
endif
ifneq ($(CONFIG_SUPERCHANNEL),y)
	rm -f $(INSTALLDIR)/www/SuperChannel.asp
endif
ifneq ($(CONFIG_FREERADIUS),y)
	rm -f $(INSTALLDIR)/www/FreeRadius.asp
	rm -f $(INSTALLDIR)/www/FreeRadius.live.asp
endif
ifneq ($(CONFIG_POKER),y)
	rm -f $(INSTALLDIR)/www/Poker.asp
	rm -f $(INSTALLDIR)/www/Poker.live.asp
	rm -f $(INSTALLDIR)/www/PokerEdit.asp
	rm -f $(INSTALLDIR)/www/PokerEdit.live.asp
endif
ifneq ($(CONFIG_WOL),y)
	rm -f $(INSTALLDIR)/www/Wol.asp
	rm -f $(INSTALLDIR)/www/help/HWol.asp
endif
ifneq ($(CONFIG_PPTPD),y)
	rm -f $(INSTALLDIR)/www/PPTP.asp
	rm -f $(INSTALLDIR)/www/help/HPPTP.asp
endif
ifneq ($(CONFIG_L2TP),y)
	rm -f $(INSTALLDIR)/www/index_l2tp.asp
endif
ifneq ($(CONFIG_SYSLOG),y)
	rm -f $(INSTALLDIR)/www/Syslog.asp
endif
ifneq ($(CONFIG_PPPOE),y)
	rm -f $(INSTALLDIR)/www/index_pppoe_dual.asp
endif
ifneq ($(CONFIG_PPPOATM),y)
	rm -f $(INSTALLDIR)/www/index_pppoa.asp
	rm -f $(INSTALLDIR)/www/index_pppoa_atm.asp
endif
ifneq ($(CONFIG_DANUBE),y)
	rm -f $(INSTALLDIR)/www/index_pppoa_atm.asp
	rm -f $(INSTALLDIR)/www/index_pppoe_atm.asp
	rm -f $(INSTALLDIR)/www/index_bridge.asp
endif
ifneq ($(CONFIG_AOSS),y)
ifneq ($(CONFIG_WPS),y)
	rm -f $(INSTALLDIR)/www/AOSS.asp
endif
endif
ifneq ($(CONFIG_BUFFALO),y)
	rm -f $(INSTALLDIR)/www/SetupAssistant.asp
	rm -f $(INSTALLDIR)/www/sas_*.asp
	rm -Rf $(INSTALLDIR)/www/intatstart
endif
ifneq ($(CONFIG_3G),y)
	rm -f $(INSTALLDIR)/www/index_3g.asp
endif
ifneq ($(CONFIG_ATH9K),y)
	rm -f $(INSTALLDIR)/www/Channel_Survey.asp
endif

#ifneq ($(CONFIG_NOCAT),y)
	rm -f $(INSTALLDIR)/www/images/splash.jpg
	rm -f $(INSTALLDIR)/www/splash.css
	rm -f $(INSTALLDIR)/www/splash.html
	rm -f $(INSTALLDIR)/www/status.html
#endif
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
#ifneq ($(CONFIG_MILKFISH),y)
#	rm -f $(INSTALLDIR)/www/Milkfish.asp
#	rm -f $(INSTALLDIR)/www/Milkfish_aliases.asp
#	rm -f $(INSTALLDIR)/www/Milkfish_database.asp
#	rm -f $(INSTALLDIR)/www/Milkfish_dynsip.asp
#	rm -f $(INSTALLDIR)/www/Milkfish_messaging.asp
#	rm -f $(INSTALLDIR)/www/Milkfish_phonebook.asp
#	rm -f $(INSTALLDIR)/www/Milkfish_siptrace.asp
#	rm -f $(INSTALLDIR)/www/Milkfish_status.asp
#	rm -f $(INSTALLDIR)/www/help/HMilkfish.asp
#	rm -f $(INSTALLDIR)/www/images/sipwerk.png
#endif
ifneq ($(CONFIG_EOP_TUNNEL),y)
	rm -f $(INSTALLDIR)/www/help/HEoIP.asp
endif
ifneq ($(CONFIG_3G),y)
	rm -f $(INSTALLDIR)/www/index_3g.asp
endif
#ifneq ($(CONFIG_SPUTNIK_APD),y)
#	rm -f $(INSTALLDIR)/www/Status_SputnikAPD.asp
#	rm -f $(INSTALLDIR)/www/Status_SputnikAPD.live.asp
#endif
ifneq ($(CONFIG_CTORRENT),y)
	rm -f $(INSTALLDIR)/www/P2P.asp
endif
#	All: These pages are not used
	rm -f $(INSTALLDIR)/www/About_test.htm
	rm -f $(INSTALLDIR)/www/SysInfo.htm
	rm -f $(INSTALLDIR)/www/graph_cpu.svg	
	rm -f $(INSTALLDIR)/www/AnchorFree.asp
	rm -f $(INSTALLDIR)/www/AnchorFree.live.asp
	rm -f $(INSTALLDIR)/www/help/HAnchorFree.asp
	
ifeq ($(CONFIG_RAMON),y)
	make -C $(SRCBASE)/router/kromo/newmedia/ install
else

ifeq ($(CONFIG_SKYTRON),y)
	make -C $(SRCBASE)/router/kromo/skytron/ install
else
ifeq ($(CONFIG_SKYTEL),y)
	make -C $(SRCBASE)/router/kromo/skytel/ install
else
ifeq ($(CONFIG_ERC),y)
	make -C $(SRCBASE)/router/kromo/dd-wrt/erc/common/ install
else
ifeq ($(CONFIG_CARLSONWIRELESS),y)
	make -C $(SRCBASE)/router/kromo/dd-wrt/carlsonwireless/common/ install
else
ifeq ($(CONFIG_ESPOD),y)
	make -C $(SRCBASE)/router/kromo/dd-wrt/espod/common/ install
else
ifeq ($(CONFIG_IPR),y)
	make -C $(SRCBASE)/router/kromo/dd-wrt/ipr/common/ install
else
ifeq ($(CONFIG_ONNET),y)
	make -C $(SRCBASE)/router/kromo/dd-wrt/onnet/common/ install
else 
ifeq ($(CONFIG_RAYTRONIK),y)
	make -C $(SRCBASE)/router/kromo/dd-wrt/raytronik/common/ install
else
	make -C $(SRCBASE)/router/kromo/common/ install
endif
endif
endif
endif
endif
endif
endif
endif
endif
endif

ifeq ($(CONFIG_OMNI),y)
	make -C $(SRCBASE)/router/kromo/omni-wifi/ install
endif

#ifeq ($(CONFIG_DDLAN),y)
#	make -C $(SRCBASE)/router/kromo/ddlan/ install
#endif


stripfiles:
	../../../../tools/strip $(INSTALLDIR)/www/*.asp
	../../../../tools/strip $(INSTALLDIR)/www/*.htm
	../../../../tools/strip $(INSTALLDIR)/www/*.html
	../../../../tools/strip $(INSTALLDIR)/www/help/*.asp


processlangfiles:
	../../../../tools/process_langfile $(INSTALLDIR)/www/lang_pack/english.js $(INSTALLDIR)/www/lang_pack/*.js

optimizefiles:
	mkdir -p $(INSTALLDIR)/www/style/elegant
	cp elegant/colorscheme.css $(INSTALLDIR)/www/style/elegant
	../../../../tools/write4 $(INSTALLDIR)/www/*.asp
	../../../../tools/write4 $(INSTALLDIR)/www/*.htm
	../../../../tools/write4 $(INSTALLDIR)/www/*.html
	../../../../tools/write4 $(INSTALLDIR)/www/help/*.asp
	-repugly $(INSTALLDIR)/www
ifeq ($(CONFIG_IAS),y)
	-repugly $(INSTALLDIR)/www/intatstart/js
	../../../../tools/removewhitespace.sh $(TOP)/../../tools $(INSTALLDIR)/www/intatstart/js
endif
ifeq ($(CONFIG_PWC),y)
	-repugly $(INSTALLDIR)/www/js
	../../../../tools/removewhitespace.sh $(TOP)/../../tools $(INSTALLDIR)/www/js
endif
	../../../../tools/removewhitespace.sh $(TOP)/../../tools $(INSTALLDIR)/www
	../../../../tools/removewhitespace.sh $(TOP)/../../tools $(INSTALLDIR)/www/help
	../../../../tools/removewhitespace.sh $(TOP)/../../tools $(INSTALLDIR)/www/lang_pack
	../../../../tools/removewhitespace.sh $(TOP)/../../tools $(INSTALLDIR)/www/images
	../../../../tools/removewhitespace.sh $(TOP)/../../tools $(INSTALLDIR)/www/ddwrt_inspired_themes
	../../../../tools/removewhitespace.sh $(TOP)/../../tools $(INSTALLDIR)/www/style
	../../../../tools/removewhitespace.sh $(TOP)/../../tools $(INSTALLDIR)/www/style/syslogd
	../../../../tools/removewhitespace.sh $(TOP)/../../tools $(INSTALLDIR)/www/style/blue
	../../../../tools/removewhitespace.sh $(TOP)/../../tools $(INSTALLDIR)/www/style/brainslayer
	../../../../tools/removewhitespace.sh $(TOP)/../../tools $(INSTALLDIR)/www/style/cyan
	../../../../tools/removewhitespace.sh $(TOP)/../../tools $(INSTALLDIR)/www/style/elegant
	../../../../tools/removewhitespace.sh $(TOP)/../../tools $(INSTALLDIR)/www/style/green
	../../../../tools/removewhitespace.sh $(TOP)/../../tools $(INSTALLDIR)/www/style/kromo
	../../../../tools/removewhitespace.sh $(TOP)/../../tools $(INSTALLDIR)/www/style/orange
	../../../../tools/removewhitespace.sh $(TOP)/../../tools $(INSTALLDIR)/www/style/pwc
	../../../../tools/removewhitespace.sh $(TOP)/../../tools $(INSTALLDIR)/www/style/pwc/ddwrt
	../../../../tools/removewhitespace.sh $(TOP)/../../tools $(INSTALLDIR)/www/style/red
	../../../../tools/removewhitespace.sh $(TOP)/../../tools $(INSTALLDIR)/www/style/wikar
	../../../../tools/removewhitespace.sh $(TOP)/../../tools $(INSTALLDIR)/www/style/xirian
	../../../../tools/removewhitespace.sh $(TOP)/../../tools $(INSTALLDIR)/www/style/yellow

	-repaccss $(INSTALLDIR)/www
	-repaccss $(INSTALLDIR)/www/help
	-repaccss $(INSTALLDIR)/www/lang_pack
	-repaccss $(INSTALLDIR)/www/images
	-repaccss $(INSTALLDIR)/www/ddwrt_inspired_themes
	-repaccss $(INSTALLDIR)/www/style
	-repaccss $(INSTALLDIR)/www/style/syslogd
	-repaccss $(INSTALLDIR)/www/style/blue
	-repaccss $(INSTALLDIR)/www/style/brainslayer
	-repaccss $(INSTALLDIR)/www/style/cyan
	-repaccss $(INSTALLDIR)/www/style/elegant
	-repaccss $(INSTALLDIR)/www/style/green
	-repaccss $(INSTALLDIR)/www/style/kromo
	-repaccss $(INSTALLDIR)/www/style/orange
	-repaccss $(INSTALLDIR)/www/style/pwc
	-repaccss $(INSTALLDIR)/www/style/pwc/ddwrt
	-repaccss $(INSTALLDIR)/www/style/red
	-repaccss $(INSTALLDIR)/www/style/wikar
	-repaccss $(INSTALLDIR)/www/style/xirian
	-repaccss $(INSTALLDIR)/www/style/yellow
ifeq ($(CONFIG_ELEGANT),y)
	cp elegant/style.css $(INSTALLDIR)/www/style/elegant
	cp elegant/fresh.css $(INSTALLDIR)/www/style/elegant
endif
ifeq ($(CONFIG_ANTAIRA),y)
	cp elegant/fresh.css $(INSTALLDIR)/www/style/elegant
endif
ifeq ($(CONFIG_RED),y)
	cp red/style.css $(INSTALLDIR)/www/style/red
endif
ifeq ($(CONFIG_BLUE),y)
	cp blue/style.css $(INSTALLDIR)/www/style/blue
endif
ifeq ($(CONFIG_YELLOW),y)
	cp yellow/style.css $(INSTALLDIR)/www/style/yellow
endif
ifeq ($(CONFIG_CYAN),y)
	cp cyan/style.css $(INSTALLDIR)/www/style/cyan
endif
ifeq ($(CONFIG_GREEN),y)
	cp green/style.css $(INSTALLDIR)/www/style/green
endif
ifeq ($(CONFIG_ORANGE),y)
	cp orange/style.css $(INSTALLDIR)/www/style/orange
endif
ifneq ($(CONFIG_ORANGE),y)
ifeq ($(CONFIG_ELEGANT),y)
ifeq ($(CONFIG_RED),y)
ifeq ($(CONFIG_BLUE),y)
ifeq ($(CONFIG_CYAN),y)
ifeq ($(CONFIG_YELLOW),y)
ifeq ($(CONFIG_GREEN),y)
ifeq ($(CONFIG_BRAINSLAYER),y)
	cp orange/style.css $(INSTALLDIR)/www/style/orange
endif
endif
endif
endif
endif
endif
endif
endif
ifeq ($(CONFIG_XIRIAN),y)
	cp xirian/style.css $(INSTALLDIR)/www/style/xirian
endif
ifeq ($(CONFIG_BRAINSLAYER),y)
	cp brainslayer/style.css $(INSTALLDIR)/www/style/brainslayer
endif
ifeq ($(CONFIG_KROMO),y)
	cp kromo/style.css $(INSTALLDIR)/www/style/kromo
endif
ifeq ($(CONFIG_WIKAR),y)
	cp wikar/style.css $(INSTALLDIR)/www/style/wikar
endif

removeobsolete:
	cd $(INSTALLDIR)/www  &&  find . -iname "CVS" | xargs rm -rf
	cd $(INSTALLDIR)/www  &&  find . -iname ".svn" | xargs rm -rf

makewww:
	cd $(INSTALLDIR)/www && $(SRCBASE)/../tools/webcomp . $(SRCBASE)/../tools/filelist *.asp *.svg *.htm *.html *.ico help/*.asp *.js js/* ddwrt_inspired_themes/*.css ddwrt_inspired_themes/*.stylus lang_pack/* *.css help/*.css style/*.css style/*/* style/*/*/* style/*/*/*/* style/*/*/*/*/* intatstart/js/* intatstart/css/* intatstart/css/images/* intatstart/lang_pack/* intatstart/config/* help/*.png  images/*.png images/*.svg images/*/*.png style/*.png style/*.jpg images/*.gif images/*/*.gif images/*/*.svg images/*.jpg images/*.ico $(SRCBASE)/router/httpd/html.c $(SRCBASE)/router/httpd/www 


removepwc:
ifeq ($(CONFIG_PWC),y)
	rm -rf $(INSTALLDIR)/www/style/pwc/ddwrt
	rm -rf $(INSTALLDIR)/www/style/pwc/default
else
	@true
endif

cleanup:
	-cd $(INSTALLDIR)/www && rm -f *.asp *.svg *.htm *.html *.css *.js *.ico help/*.asp help/*.png help/*.css images/*.gif ddwrt_inspired_themes/*.css ddwrt_inspired_themes/*.stylus images/*.png images/*.jpg images/*.ico style/*.css style/*.png style/*.jpg style/*/* images/*/*.png images/*/*.gif js/* lang_pack/* intatstart/js/* intatstart/css/* intatstart/css/images/* intatstart/lang_pack/* intatstart/config/*
	rm -rf $(INSTALLDIR)/www/images
	rm -rf $(INSTALLDIR)/www/help
	rm -rf $(INSTALLDIR)/www/style/syslogd
	rm -rf $(INSTALLDIR)/www/ddwrt_inspired_themes
	rm -rf $(INSTALLDIR)/www/js
	rm -rf $(INSTALLDIR)/www/lang_pack
	-rm -rf $(INSTALLDIR)/www/intatstart
ifeq ($(CONFIG_SPEEDCHECKER_DEBUG),y)
	ln -s ../tmp/Speedchecker-dev.asp $(INSTALLDIR)/www/Speedchecker-dev.asp
endif

ifneq ($(NO_PROCESSLANGFILES),y)
PROCESSLANGFILES:= processlangfiles
endif

install: installwebfiles stripfiles $(PROCESSLANGFILES) optimizefiles removeobsolete makewww removepwc cleanup


postinstall: installwebfiles removeobsolete removepwc cleanup

webcomp:

clean:
