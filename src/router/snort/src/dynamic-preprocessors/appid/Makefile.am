## $Id
AUTOMAKE_OPTIONS=foreign no-dependencies

INCLUDES = -I${top_builddir}/src/dynamic-preprocessors/include  -I${srcdir}/../libs \
		-I$(srcdir)/util \
		-I$(srcdir)/service_plugins \
		-I$(srcdir)/client_plugins  \
		-I$(srcdir)/detector_plugins  

dynamicpreprocessordir = ${libdir}/snort_dynamicpreprocessor

dynamicpreprocessor_LTLIBRARIES = libsf_appid_preproc.la


libsf_appid_preproc_la_LDFLAGS = -export-dynamic -module @XCCFLAGS@
if SO_WITH_STATIC_LIB
libsf_appid_preproc_la_LIBADD = ../libsf_dynamic_preproc.la $(LUA_LIBS)
else
nodist_libsf_appid_preproc_la_SOURCES = \
../include/sf_dynamic_preproc_lib.c \
../include/sf_ip.c \
../include/sfPolicyUserData.c \
../include/sfxhash.c \
../include/sfghash.c \
../include/sflsq.c \
../include/sfhashfcn.c \
../include/sfmemcap.c \
../include/sfprimetable.c

libsf_appid_preproc_la_LIBADD = $(LUA_LIBS)
endif

libsf_appid_preproc_la_CFLAGS = -DDYNAMIC_PREPROC_CONTEXT $(LUA_CFLAGS)


libsf_appid_preproc_la_SOURCES =  \
	commonAppMatcher.c  \
	flow.c \
	rna_flow.h \
	fw_appid.c \
	hostPortAppCache.c \
	luaDetectorApi.c \
	luaDetectorApi.h \
	luaDetectorFlowApi.c \
	luaDetectorModule.c \
	luaDetectorModule.h \
	service_state.c \
	spp_appid.c \
	appId.h \
	commonAppMatcher.h \
	flow.h \
	hostPortAppCache.h \
	httpCommon.h \
	luaDetectorFlowApi.h \
	service_state.h \
	spp_appid.h \
	attribute.h \
	flow_error.h \
	fw_appid.h \
	host_tracker.h \
	appInfoTable.c \
	appInfoTable.h \
	appIdStats.c \
	appIdStats.h \
	appIdConfig.c \
	appIdConfig.h

libsf_appid_preproc_la_SOURCES +=  \
	client_plugins/client_app_base.c \
	client_plugins/client_app_ym.c \
	client_plugins/client_app_msn.c \
	client_plugins/client_app_aim.c \
	client_plugins/client_app_smtp.c \
	client_plugins/client_app_msn.h \
	client_plugins/client_app_aim.h \
	client_plugins/client_app_smtp.h \
	client_plugins/client_app_ym.h \
	client_plugins/client_app_api.h \
	client_plugins/client_app_base.h \
	client_plugins/client_app_bit.c \
	client_plugins/client_app_bit_tracker.c \
	client_plugins/client_app_rtp.c \
	client_plugins/client_app_ssh.c \
	client_plugins/client_app_timbuktu.c \
	client_plugins/client_app_tns.c \
	client_plugins/client_app_vnc.c

libsf_appid_preproc_la_SOURCES +=  \
	service_plugins/service_util.h \
	service_plugins/service_base.h \
	service_plugins/service_base.c \
	service_plugins/service_api.h \
	service_plugins/dcerpc.h \
	service_plugins/dcerpc.c \
	service_plugins/service_mysql.c \
	service_plugins/service_ssh.h \
	service_plugins/service_radius.c \
	service_plugins/service_telnet.c \
	service_plugins/service_rexec.h \
	service_plugins/service_nntp.h \
	service_plugins/service_bootp.c \
	service_plugins/service_dns.h \
	service_plugins/service_ntp.c \
	service_plugins/service_rsync.h \
	service_plugins/service_flap.c \
	service_plugins/service_battle_field.h \
	service_plugins/service_rshell.c \
	service_plugins/service_netbios.c \
	service_plugins/service_ftp.h \
	service_plugins/service_snmp.c \
	service_plugins/service_radius.h \
	service_plugins/service_ntp.h \
	service_plugins/service_battle_field.c \
	service_plugins/service_rshell.h \
	service_plugins/service_dns.c \
	service_plugins/service_direct_connect.h \
	service_plugins/service_bootp.h \
	service_plugins/service_mysql.h \
	service_plugins/service_rexec.c \
	service_plugins/service_rfb.h \
	service_plugins/service_rfb.c \
	service_plugins/service_dcerpc.h \
	service_plugins/service_lpr.h \
	service_plugins/service_ssl.h \
	service_plugins/service_MDNS.h \
	service_plugins/service_rpc.c \
	service_plugins/service_flap.h \
	service_plugins/service_telnet.h \
	service_plugins/service_smtp.c \
	service_plugins/service_bgp.c \
	service_plugins/service_direct_connect.c \
	service_plugins/service_irc.c \
	service_plugins/service_rlogin.h \
	service_plugins/service_rpc.h \
	service_plugins/service_ssh.c \
	service_plugins/service_tftp.c \
	service_plugins/service_ftp.c \
	service_plugins/service_rsync.c \
	service_plugins/service_rlogin.c \
	service_plugins/service_tftp.h \
	service_plugins/service_netbios.h \
	service_plugins/service_lpr.c \
	service_plugins/service_pattern.c \
	service_plugins/service_nntp.c \
	service_plugins/service_snmp.h \
	service_plugins/service_bgp.h \
	service_plugins/service_MDNS.c \
	service_plugins/service_smtp.h \
	service_plugins/service_irc.h \
	service_plugins/service_dcerpc.c \
	service_plugins/service_pattern.h \
	service_plugins/service_ssl.c \
	service_plugins/service_bit.c \
	service_plugins/service_timbuktu.c \
	service_plugins/service_tns.c \
        service_plugins/service_rtmp.h \
        service_plugins/service_rtmp.c

libsf_appid_preproc_la_SOURCES +=  \
	detector_plugins/detector_api.h \
	detector_plugins/detector_base.h \
	detector_plugins/detector_base.c \
	detector_plugins/http_url_patterns.c \
	detector_plugins/detector_http.c \
	detector_plugins/detector_http.h \
	detector_plugins/http_url_patterns.h \
	detector_plugins/detector_imap.c  \
	detector_plugins/detector_kerberos.c  \
	detector_plugins/detector_pop3.c \
	detector_plugins/detector_sip.c \
	detector_plugins/detector_sip.h 

libsf_appid_preproc_la_SOURCES +=  \
	util/common_util.h \
	util/common_util.c \
	util/sf_multi_mpse.c \
	util/ip_funcs.h \
	util/sf_mlmp.h \
	util/sf_multi_mpse.h \
	util/sf_mlmp.c \
	util/fw_avltree.c \
	util/fw_avltree.h \
	util/OutputFile.c \
	util/OutputFile.h

EXTRA_DIST = 

all-local: $(LTLIBRARIES)
	$(MAKE) DESTDIR=`pwd`/../build install-dynamicpreprocessorLTLIBRARIES

