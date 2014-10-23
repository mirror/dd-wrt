if WITH_PPP_PLUGIN
pppd_plugindir = $(PPPD_PLUGIN_DIR)

pppd_plugin_LTLIBRARIES      = sstp-pppd-plugin.la
sstp_pppd_plugin_la_CFLAGS   = -I$(top_srcdir)/include
sstp_pppd_plugin_la_LDFLAGS  = -module -avoid-version
sstp_pppd_plugin_la_LIBADD   = ../libsstp-api/libsstp_api.la
sstp_pppd_plugin_la_SOURCES  = sstp-plugin.c 

endif
