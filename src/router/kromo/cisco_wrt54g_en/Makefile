include $(TOP)/.config

ifneq ($(wildcard $(SRCBASE)/cy_conf.mak),)
  include $(SRCBASE)/cy_conf.mak
endif

all:

install:
	rm -rf $(INSTALLDIR)/www
	install -d $(INSTALLDIR)/www
	install -m 644 $(wildcard *.asp) $(wildcard *.css) $(wildcard *.jpg) $(wildcard *.gif) $(wildcard *.js) $(wildcard *.htm) $(wildcard *.ico) $(wildcard *.html) $(INSTALLDIR)/www
	cp -a images $(INSTALLDIR)/www
	cp -a style $(INSTALLDIR)/www
ifneq ($(CONFIG_DIST),"micro")
	cp -a help $(INSTALLDIR)/www
endif
	cp -a lang_pack $(INSTALLDIR)/www
ifeq ($(CONFIG_NOMUI),y)
	rm -f $(INSTALLDIR)/www/lang_pack/*.js
	rm -f $(INSTALLDIR)/www/images/dd-wrt.gif	
	rm -f $(INSTALLDIR)/www/images/logo.png
	rm -f $(INSTALLDIR)/www/images/88*	
	cp lang_pack/english.js $(INSTALLDIR)/www/lang_pack/
endif
	$(TOP)/tools/l7filter $(INSTALLDIR)/www/lang_pack/*.js
	ln -s ../tmp/www $(INSTALLDIR)/www/user


ifeq ($(CONFIG_WIKAR),y)
	cp -a wikar $(INSTALLDIR)/www/style
endif




ifeq ($(CONFIG_3COM),y)
	mkdir $(INSTALLDIR)/www/style/3com
	cp 3com/style.css $(INSTALLDIR)/www/style/3com
	cp 3com/images/* $(INSTALLDIR)/www/images
endif

ifeq ($(CONFIG_RED),y)
	mkdir $(INSTALLDIR)/www/style/red
	cp red/style.css $(INSTALLDIR)/www/style/red
	cp red/style_ie.css $(INSTALLDIR)/www/style/red
	cp red/common.css $(INSTALLDIR)/www/style
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
	cp blue/style.css $(INSTALLDIR)/www/style/blue
	cp blue/style_ie.css $(INSTALLDIR)/www/style/blue
	cp blue/common.css $(INSTALLDIR)/www/style
	cp blue/logo.png $(INSTALLDIR)/www/style
endif

ifeq ($(CONFIG_ELEGANT),y)
	mkdir $(INSTALLDIR)/www/style/elegant
	cp elegant/style.css $(INSTALLDIR)/www/style/elegant
	cp elegant/style_ie.css $(INSTALLDIR)/www/style/elegant
	cp elegant/common.css $(INSTALLDIR)/www/style
	cp elegant/logo.png $(INSTALLDIR)/www/style
endif

ifeq ($(CONFIG_CYAN),y)
	mkdir $(INSTALLDIR)/www/style/cyan
	cp cyan/style.css $(INSTALLDIR)/www/style/cyan
	cp cyan/style_ie.css $(INSTALLDIR)/www/style/cyan
	cp cyan/common.css $(INSTALLDIR)/www/style
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
	cp yellow/style.css $(INSTALLDIR)/www/style/yellow
	cp yellow/style_ie.css $(INSTALLDIR)/www/style/yellow
	cp yellow/common.css $(INSTALLDIR)/www/style
	cp yellow/logo.png $(INSTALLDIR)/www/style
endif

ifeq ($(CONFIG_GREEN),y)
	mkdir $(INSTALLDIR)/www/style/green
	cp green/style.css $(INSTALLDIR)/www/style/green
	cp green/style_ie.css $(INSTALLDIR)/www/style/green
	cp green/common.css $(INSTALLDIR)/www/style
	cp green/logo.png $(INSTALLDIR)/www/style
endif

ifeq ($(CONFIG_SKYTEL),y)
	cp -a skytel $(INSTALLDIR)/www/style
endif

ifeq ($(CONFIG_NEWMEDIA),y)
	cp -f nmn/nmn_logo.png $(INSTALLDIR)/www/style/logo.png
endif

ifeq ($(CONFIG_GGEW),y)
	cp -f ggew/logo.png $(INSTALLDIR)/www/style/logo.png
endif

ifneq ($(CONFIG_MSSID),y)
	cp WEP.v23 $(INSTALLDIR)/www/WEP.asp
	cp WL_WPATable.v23 $(INSTALLDIR)/www/WL_WPATable.asp
	cp WPA.v23 $(INSTALLDIR)/www/WPA.asp
	cp WPA_Preshared.v23 $(INSTALLDIR)/www/WPA_Preshared.asp
	cp WPA_Radius.v23 $(INSTALLDIR)/www/WPA_Radius.asp	
	cp Wireless_Basic.v23 $(INSTALLDIR)/www/Wireless_Basic.asp
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

ifneq ($(BACKUP_RESTORE_SUPPORT),1)
	rm -f $(INSTALLDIR)/www/Backup_Restore.asp
endif

ifneq ($(CONFIG_UPNP),y)
	rm -f $(INSTALLDIR)/www/UPnP.asp
endif

ifneq ($(CONFIG_NOCAT),y)
	rm -f $(INSTALLDIR)/www/splash.html
	rm -f $(INSTALLDIR)/www/status.html
endif

ifeq ($(CONFIG_NEWMEDIA),y)
	$(MAKE) -C $(SRCBASE)/router/kromo/newmedia/ install
else

ifeq ($(CONFIG_RAMON),y)
	$(MAKE) -C $(SRCBASE)/router/kromo/newmedia/ install
else

ifeq ($(CONFIG_34TELECOM),y)
	$(MAKE) -C $(SRCBASE)/router/kromo/34telecom/ install
	mkdir $(INSTALLDIR)/www/style/cyan
	cp cyan/style.css $(INSTALLDIR)/www/style/cyan
	cp cyan/style_ie.css $(INSTALLDIR)/www/style/cyan
	cp cyan/common.css $(INSTALLDIR)/www/style
#	cp cyan/logo.png $(INSTALLDIR)/www/style
	cp -f 34telecom/logo.png $(INSTALLDIR)/www/style/
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
endif
ifeq ($(CONFIG_OMNI),y)
	$(MAKE) -C $(SRCBASE)/router/kromo/omni-wifi/ install
endif

ifeq ($(CONFIG_SER),y)
	$(MAKE) -C $(SRCBASE)/router/kromo/sipath/ install
endif

ifeq ($(CONFIG_DDLAN),y)
	$(MAKE) -C $(SRCBASE)/router/kromo/ddlan/ install
endif

	




webcomp:
	webcomp . list > webrom.c

clean:
