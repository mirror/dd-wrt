## $Id$
AUTOMAKE_OPTIONS=foreign no-dependencies

INCLUDES = -I${top_builddir}/src/dynamic-preprocessors/include -I${top_builddir}/src/dynamic-preprocessors/ssl_common -I${top_srcdir}/src/dynamic-preprocessors/libs -I${top_builddir}

if SO_WITH_STATIC_LIB

preproclibdir=$(pkglibdir)/dynamic_preproc

preproclib_LTLIBRARIES = libsf_dynamic_preproc.la
libsf_dynamic_preproc_la_CFLAGS = -fPIC -DPIC -DDYNAMIC_PREPROC_CONTEXT
libsf_dynamic_preproc_la_LDFLAGS = -static

libsf_dynamic_preproc_la_SOURCES = \
ssl_common/ssl.c \
ssl_common/ssl_config.c \
ssl_common/ssl_inspect.c \
ssl_common/ssl_ha.c 

nodist_libsf_dynamic_preproc_la_SOURCES = \
include/sf_dynamic_preproc_lib.c \
include/sf_ip.c \
include/sfrt.c \
include/sfrt_dir.c \
include/sfrt_flat.c \
include/sfrt_flat_dir.c \
include/segment_mem.c \
include/mempool.c \
include/sf_sdlist.c \
include/sfPolicyUserData.c \
include/util_unfold.c \
include/sf_base64decode.c \
include/sf_email_attach_decode.c \
include/reg_test.c \
libs/sfparser.c

preprocdir=$(pkgincludedir)/dynamic_preproc

preproc_HEADERS = \
ssl_common/ssl.h \
ssl_common/ssl_include.h \
ssl_common/ssl_session.h \
ssl_common/ssl_config.h \
ssl_common/ssl_ha.h \
ssl_common/ssl_inspect.h

nodist_preproc_HEADERS = \
libs/sfcommon.h \
libs/sf_preproc_info.h \
include/sf_snort_packet.h \
include/sf_protocols.h \
include/sf_snort_plugin_api.h \
include/sf_decompression.h \
include/sf_decompression_define.h \
include/sfPolicyUserData.h \
include/snort_debug.h \
include/snort_bounds.h \
include/cpuclock.h \
include/profiler.h \
include/bitop.h \
include/mempool.h \
include/sf_sdlist_types.h \
include/sf_ip.h \
include/sfrt_flat.h \
include/sfrt_flat_dir.h \
include/segment_mem.h \
include/sf_dynamic_common.h \
include/sf_dynamic_engine.h \
include/sf_dynamic_define.h \
include/sf_dynamic_meta.h \
include/sf_dynamic_preprocessor.h \
include/sf_dynamic_preproc_lib.h \
include/ipv6_port.h \
include/sfPolicy.h \
include/sfrt.h \
include/sfrt_dir.h \
include/sfrt_trie.h \
include/obfuscation.h \
include/packet_time.h \
include/session_api.h \
include/stream_api.h \
include/str_search.h \
include/preprocids.h \
include/sfcontrol.h \
include/sidechannel_define.h \
include/idle_processing.h \
include/sf_seqnums.h \
include/perf_indicators.h \
include/mpse_methods.h \
include/file_api.h \
include/reload_api.h \
include/smtp_api.h \
include/reg_test.h \
include/memory_stats.h

preproclib_LTLIBRARIES += libsf_dynamic_utils.la

libsf_dynamic_utils_la_CFLAGS = -fPIC -DPIC -DDYNAMIC_PREPROC_CONTEXT
libsf_dynamic_utils_la_LDFLAGS = -static

if FEAT_OPEN_APPID
nodist_libsf_dynamic_utils_la_SOURCES = include/sfprimetable.c include/sfxhash.c include/sfmemcap.c include/sfmemcap.h \
						  include/sfghash.c include/sfhashfcn.c include/sflsq.c include/md5.c

nodist_preproc_HEADERS += include/appId.h include/appIdApi.h include/thirdparty_appid_types.h \
						  include/thirdparty_appid_api.h include/dns_defs.h
else
nodist_libsf_dynamic_utils_la_SOURCES = include/sfmemcap.c include/sfmemcap.h
endif

if BUILD_SNORT_RELOAD
#appdata_adjuster
nodist_libsf_dynamic_utils_la_SOURCES += include/appdata_adjuster.c include/sfxhash.c include/sfhashfcn.c include/sfmemcap.c include/sfprimetable.c include/reg_test.h include/reg_test.c

nodist_preproc_HEADERS += include/appdata_adjuster.h
endif

all-local: $(LTLIBRARIES)
	$(MAKE) DESTDIR=`pwd`/build install-preproclibLTLIBRARIES

endif

BUILT_SOURCES = \
	include/snort_bounds.h \
	include/snort_debug.h \
	include/preprocids.h \
	include/profiler.h \
	include/cpuclock.h \
	include/sf_dynamic_common.h \
	include/sf_dynamic_engine.h \
	include/sf_dynamic_define.h \
	include/sf_dynamic_meta.h \
	include/sf_dynamic_preprocessor.h \
	include/sf_dynamic_preproc_lib.c \
	include/sf_dynamic_preproc_lib.h \
	include/sfghash.h \
	include/sfhashfcn.h \
	include/bitop.h \
	include/sf_ip.h \
	include/sf_ip.c \
	include/sf_ipvar.h \
	include/sf_vartable.h \
	include/ipv6_port.h \
	include/sfsnort_dynamic_detection_lib.c \
	include/sfsnort_dynamic_detection_lib.h \
	include/sf_snort_packet.h \
	include/sf_protocols.h \
	include/sf_snort_plugin_api.h \
	include/sf_decompression.h \
	include/sf_decompression_define.h \
	include/pcap_pkthdr32.h \
	include/session_api.h \
	include/stream_api.h \
	include/str_search.h \
	include/sf_types.h \
	include/sfrt.h \
	include/sfrt.c \
	include/sfrt_dir.h \
	include/sfrt_dir.c \
	include/sfrt_flat.h \
	include/sfrt_flat.c \
	include/sfrt_flat_dir.h \
	include/sfrt_flat_dir.c \
	include/sfrt_trie.h \
	include/segment_mem.h \
	include/segment_mem.c \
	include/mempool.h \
	include/mempool.c \
	include/sfmemcap.h \
	include/sfmemcap.c \
	include/sf_sdlist.h \
	include/sf_sdlist_types.h \
	include/sf_sdlist.c \
	include/sfPolicyUserData.c \
	include/sfPolicyUserData.h \
	include/sfPolicy.h \
	include/util_unfold.h \
	include/util_unfold.c \
	include/sf_base64decode.h \
	include/sf_base64decode.c \
	include/sf_email_attach_decode.h \
	include/sf_email_attach_decode.c \
	include/treenodes.h \
	include/signature.h \
	include/plugin_enum.h \
	include/obfuscation.h \
	include/packet_time.h \
	include/rule_option_types.h \
	include/event.h \
	include/Unified2_common.h \
	include/sfcontrol.h \
	include/sidechannel_define.h \
	include/idle_processing.h \
	include/sf_seqnums.h \
	include/perf_indicators.h \
	include/file_api.h \
	include/file_mail_common.h \
	include/mpse_methods.h \
	include/sfdebug.h \
	include/sip_common.h \
	include/cip_common.h \
	include/reload_api.h \
	include/smtp_api.h \
	include/reg_test.h \
	include/reg_test.c \
	ssl_common/ssl.h \
	ssl_common/ssl.c \
	ssl_common/ssl_include.h \
	ssl_common/ssl_config.h \
	ssl_common/ssl_config.c \
	ssl_common/ssl_session.h \
	ssl_common/ssl_inspect.h \
	ssl_common/ssl_inspect.c \
	ssl_common/ssl_ha.h \
	ssl_common/ssl_ha.c \
	libs/sfparser.c \
	libs/sfcommon.h \
	include/memory_stats.h

if FEAT_OPEN_APPID
BUILT_SOURCES += include/appId.h  include/appIdApi.h include/thirdparty_appid_types.h \
				 include/thirdparty_appid_api.h \
				 include/sfprimetable.h include/sfprimetable.c include/sfxhash.h \
				 include/sfxhash.c \
				 include/sfghash.c include/sfhashfcn.c include/sflsq.h include/sflsq.c \
				 include/md5.h include/md5.c \
				 include/dns_defs.h
endif

if BUILD_SNORT_RELOAD
BUILT_SOURCES += include/sfprimetable.h \
        include/sfprimetable.c \
        include/sfmemcap.h \
        include/sfmemcap.c \
        include/sfhashfcn.h \
        include/sfhashfcn.c \
        include/sfxhash.h \
        include/sfxhash.c \
        include/appdata_adjuster.h \
        include/appdata_adjuster.c
endif

sed_ipv6_headers = \
	sed -e "s/->iph->ip_src/->ip4_header->source/" \
	    -e "s/->iph->ip_dst/->ip4_header->destination/" \
	    -e "s/->iph->/->ip4_header->/" \
	    -e "s/->iph$$/->ip4_header/" \
	    -e "s/orig_iph/orig_ipv4h/" \
	    -e "s/ip_verhl/version_headerlength/" \
	    -e "s/ip_tos/type_service/" \
	    -e "s/ip_len/data_length/" \
	    -e "s/ip_id/identifier/" \
	    -e "s/ip_off/offset/" \
	    -e "s/ip_ttl/time_to_live/" \
	    -e "s/ip_proto/proto/" \
	    -e "s/ip_csum/checksum/" \
	    $$dst_header.new > $$dst_header

massage_ipv6_headers = \
	mkdir -p include; \
	mkdir -p build; \
	if test -f $$dst_header; then \
		x=`diff $$src_header $$dst_header.new >> /dev/null`; \
		if test "$$x" != "0"; then \
			echo "Updating " $$dst_header; \
			cp $$src_header $$dst_header.new; \
			$(sed_ipv6_headers); \
		fi \
	else \
		echo "Updating " $$dst_header; \
		cp $$src_header $$dst_header.new; \
		$(sed_ipv6_headers); \
	fi

sed_headers = \
	sed -e "s/Packet /SFSnortPacket /" \
	    -e "s/SnortPktHdr /SFSnortPktHdr /" \
	    -e "s/decode\.h/sf_snort_packet.h/" \
	    -e "/sfportobject\.h/d" \
	    -e "s/PortObject \*/void */g" \
	    $$dst_header.new > $$dst_header

massage_headers = \
	mkdir -p include; \
	mkdir -p build; \
	if test -f $$dst_header; then \
		x=`diff $$src_header $$dst_header.new.new >> /dev/null`; \
		if test "$$x" != "0"; then \
			echo "Updating " $$dst_header; \
			cp $$src_header $$dst_header.new; \
			$(sed_headers); \
		fi \
	else \
		echo "Updating " $$dst_header; \
		cp $$src_header $$dst_header.new; \
		$(sed_headers); \
	fi

sed_debug_header = \
	sed -e "s/DebugMessageFile = /*_dpd.debugMsgFile = /" \
	    -e "s/DebugMessageLine = /*_dpd.debugMsgLine = /" \
	    -e "s/; DebugMessageFunc$$/; _dpd.debugMsg/" \
	    -e "s/; DebugWideMessageFunc$$/; _dpd.debugWideMsg/" \
	    $$dst_header.new > $$dst_header

copy_debug_header = \
	mkdir -p include; \
	mkdir -p build; \
	if test -f $$dst_header; then \
		x=`diff $$src_header $$dst_header.new.new >> /dev/null`; \
		if test "$$x" != "0"; then \
			echo "Updating " $$dst_header; \
			cp $$src_header $$dst_header.new; \
			$(sed_debug_header); \
		fi \
	else \
		echo "Updating " $$dst_header; \
		cp $$src_header $$dst_header.new; \
		$(sed_debug_header); \
	fi

copy_error_message = \
        if test -f $$dst_header; then \
	   sed -e "s/ErrorMessage/_dpd.errMsg/" \
	       -e "s/LogMessage/_dpd.logMsg/" \
	       -e "s/FatalError/_dpd.fatalMsg/" \
	       -e "/util.h/d" \
	       -e "/snort.h/d" \
                        $$dst_header > $$dst_header.new; \
                mv -f $$dst_header.new $$dst_header; \
        fi

copy_no_static_hash = \
        if test -f $$dst_header; then \
                echo "Updating " $$dst_header; \
                sed -e "s/\#ifndef MODULUS_HASH/\#ifdef STATIC_HASH/" \
                        $$dst_header > $$dst_header.new; \
                mv -f $$dst_header.new $$dst_header; \
        fi

replace_policy_globals = \
	if test -f $$dst_header; then \
		sed -e "/SharedObjectAddStarts/d" \
		    -e "/SharedObjectAddEnds/d" \
            -e "/SharedObjectDeleteBegins/,/SharedObjectDeleteEnds/d" \
		    -e "s/getDefaultPolicy()/_dpd.getDefaultPolicy()/" \
		    -e "s/SnortStrnStr/_dpd.SnortStrnStr/" \
		    -e "s/SnortStrncpy/_dpd.SnortStrncpy/" \
		    -e "s/ReloadAdjustRegister/_dpd.reloadAdjustRegister/" \
		    -e "s/session_api/_dpd.sessionAPI/" \
		        $$dst_header > $$dst_header.new; \
		mv -f $$dst_header.new $$dst_header; \
	fi

copy_headers = \
	mkdir -p include; \
	mkdir -p build; \
	if test -f $$dst_header; then \
		x=`diff $$src_header $$dst_header.new.new >> /dev/null`; \
		if test "$$x" != "0"; then \
			echo "Updating " $$dst_header; \
			cp $$src_header $$dst_header; \
		fi \
	else \
		echo "Updating " $$dst_header; \
		cp $$src_header $$dst_header; \
	fi

sed_treenode_header = \
	sed -f $(srcdir)/treenodes.sed $$dst_header.new > $$dst_header

copy_treenode_header = \
	mkdir -p include; \
	mkdir -p build; \
	if test -f $$dst_header; then \
		x=`diff $$src_header $$dst_header.new.new >> /dev/null`; \
		if test "$$x" != "0"; then \
			echo "Updating " $$dst_header; \
			cp $$src_header $$dst_header.new; \
			$(sed_treenode_header); \
		fi \
	else \
		echo "Updating " $$dst_header; \
		cp $$src_header $$dst_header.new; \
		$(sed_treenode_header); \
	fi

# From main src tree
include/snort_debug.h: $(srcdir)/../snort_debug.h
	@src_header=$?; dst_header=$@; $(copy_debug_header)

include/preprocids.h: $(srcdir)/../preprocids.h
	@src_header=$?; dst_header=$@; $(copy_headers)

include/profiler.h: $(srcdir)/../profiler.h
	@src_header=$?; dst_header=$@; $(copy_headers)

include/cpuclock.h: $(srcdir)/../cpuclock.h
	@src_header=$?; dst_header=$@; $(copy_headers)

include/pcap_pkthdr32.h: $(srcdir)/../pcap_pkthdr32.h
	@src_header=$?; dst_header=$@; $(copy_headers)

include/snort_bounds.h: $(srcdir)/../snort_bounds.h
	@src_header=$?; dst_header=$@; $(copy_headers)

include/ipv6_port.h: $(srcdir)/../ipv6_port.h
	@src_header=$?; dst_header=$@; $(massage_ipv6_headers)

include/sf_types.h: $(srcdir)/../sf_types.h
	@src_header=$?; dst_header=$@; $(copy_headers)

include/obfuscation.h: $(srcdir)/../obfuscation.h
	@src_header=$?; dst_header=$@; $(massage_headers)

include/packet_time.h: $(srcdir)/../packet_time.h
	@src_header=$?; dst_header=$@; $(massage_headers)

include/rule_option_types.h: $(srcdir)/../rule_option_types.h
	@src_header=$?; dst_header=$@; $(copy_headers)

include/event.h: $(srcdir)/../event.h
	@src_header=$?; dst_header=$@; $(copy_headers)

include/sidechannel_define.h: $(srcdir)/../side-channel/sidechannel_define.h
	@src_header=$?; dst_header=$@; $(massage_headers)

include/reload_api.h: $(srcdir)/../reload_api.h
	@src_header=$?; dst_header=$@; $(massage_headers)

include/smtp_api.h: $(srcdir)/../smtp_api.h
	@src_header=$?; dst_header=$@; $(massage_headers)

include/reg_test.h: $(srcdir)/../reg_test.h
	@src_header=$?; dst_header=$@; $(copy_headers)

include/reg_test.c: $(srcdir)/../reg_test.c
	@src_header=$?; dst_header=$@; $(copy_headers)

# From dynamic-plugins
include/sf_dynamic_common.h: $(srcdir)/../dynamic-plugins/sf_dynamic_common.h
	@src_header=$?; dst_header=$@; $(copy_headers)

include/sf_dynamic_engine.h: $(srcdir)/../dynamic-plugins/sf_dynamic_engine.h
	@src_header=$?; dst_header=$@; $(copy_headers)

include/sf_dynamic_define.h: $(srcdir)/../dynamic-plugins/sf_dynamic_define.h
	@src_header=$?; dst_header=$@; $(copy_headers)

include/sf_dynamic_meta.h: $(srcdir)/../dynamic-plugins/sf_dynamic_meta.h
	@src_header=$?; dst_header=$@; $(copy_headers)

include/sf_dynamic_preprocessor.h: $(srcdir)/../dynamic-plugins/sf_dynamic_preprocessor.h
	@src_header=$?; dst_header=$@; $(massage_headers)

# From dynamic-plugins/sf_preproc_example
include/sf_dynamic_preproc_lib.c: $(srcdir)/../dynamic-plugins/sf_preproc_example/sf_dynamic_preproc_lib.c
	@src_header=$?; dst_header=$@; $(copy_headers)

include/sf_dynamic_preproc_lib.h: $(srcdir)/../dynamic-plugins/sf_preproc_example/sf_dynamic_preproc_lib.h
	@src_header=$?; dst_header=$@; $(copy_headers)

# From Utils
include/sfghash.h: $(srcdir)/../sfutil/sfghash.h
	@src_header=$?; dst_header=$@; $(copy_headers)

include/sfhashfcn.h: $(srcdir)/../sfutil/sfhashfcn.h
	@src_header=$?; dst_header=$@; $(copy_headers)

include/bitop.h: $(srcdir)/../sfutil/bitop.h
	@src_header=$?; dst_header=$@; $(copy_headers)

include/sf_ip.h: $(srcdir)/../sfutil/sf_ip.h
	@src_header=$?; dst_header=$@; $(copy_headers)

include/sf_ip.c: $(srcdir)/../sfutil/sf_ip.c
	@src_header=$?; dst_header=$@; $(copy_headers)

include/sf_ipvar.h: $(srcdir)/../sfutil/sf_ipvar.h
	@src_header=$?; dst_header=$@; $(copy_headers)

include/sf_vartable.h: $(srcdir)/../sfutil/sf_vartable.h
	@src_header=$?; dst_header=$@; $(copy_headers)

include/sfrt.h: $(srcdir)/../sfutil/sfrt.h
	@src_header=$?; dst_header=$@; $(copy_headers)

include/sfrt.c: $(srcdir)/../sfutil/sfrt.c
	@src_header=$?; dst_header=$@; $(copy_headers)

include/sfrt_dir.h: $(srcdir)/../sfutil/sfrt_dir.h
	@src_header=$?; dst_header=$@; $(copy_headers)

include/sfrt_dir.c: $(srcdir)/../sfutil/sfrt_dir.c
	@src_header=$?; dst_header=$@; $(copy_headers)
	
include/sfrt_flat.h: $(srcdir)/../sfutil/sfrt_flat.h
	@src_header=$?; dst_header=$@; $(copy_headers)

include/sfrt_flat.c: $(srcdir)/../sfutil/sfrt_flat.c
	@src_header=$?; dst_header=$@; $(copy_headers)

include/sfrt_flat_dir.h: $(srcdir)/../sfutil/sfrt_flat_dir.h
	@src_header=$?; dst_header=$@; $(copy_headers)

include/sfrt_flat_dir.c: $(srcdir)/../sfutil/sfrt_flat_dir.c
	@src_header=$?; dst_header=$@; $(copy_headers)
	
include/sfrt_trie.h: $(srcdir)/../sfutil/sfrt_trie.h
	@src_header=$?; dst_header=$@; $(copy_headers)

include/segment_mem.c: $(srcdir)/../sfutil/segment_mem.c
	@src_header=$?; dst_header=$@; $(copy_headers)

include/segment_mem.h: $(srcdir)/../sfutil/segment_mem.h
	@src_header=$?; dst_header=$@; $(copy_headers)

include/mempool.h: $(srcdir)/../mempool.h
	@src_header=$?; dst_header=$@; $(copy_headers); $(copy_error_message); $(replace_policy_globals)

include/mempool.c: $(srcdir)/../mempool.c
	@src_header=$?; dst_header=$@; $(copy_headers); $(copy_error_message); $(replace_policy_globals)

include/sfmemcap.h: $(srcdir)/../sfutil/sfmemcap.h
	    @src_header=$?; dst_header=$@; $(copy_headers); $(copy_error_message); $(replace_policy_globals)

include/sfmemcap.c: $(srcdir)/../sfutil/sfmemcap.c
	    @src_header=$?; dst_header=$@; $(copy_headers); $(copy_error_message); $(replace_policy_globals)

include/sf_sdlist.h: $(srcdir)/../sf_sdlist.h
	@src_header=$?; dst_header=$@; $(copy_headers); $(copy_error_message); $(replace_policy_globals)

include/sf_sdlist_types.h: $(srcdir)/../sf_sdlist_types.h
	@src_header=$?; dst_header=$@; $(copy_headers); $(copy_error_message); $(replace_policy_globals)

include/sf_sdlist.c: $(srcdir)/../sf_sdlist.c
	@src_header=$?; dst_header=$@; $(copy_headers); $(copy_error_message); $(replace_policy_globals)

include/sfPolicyUserData.c: $(srcdir)/../sfutil/sfPolicyUserData.c
	@src_header=$?; dst_header=$@; $(copy_headers); $(replace_policy_globals)

include/sfPolicyUserData.h: $(srcdir)/../sfutil/sfPolicyUserData.h
	@src_header=$?; dst_header=$@; $(copy_headers); $(replace_policy_globals)

include/sfPolicy.h: $(srcdir)/../sfutil/sfPolicy.h
	@src_header=$?; dst_header=$@; $(copy_headers); $(replace_policy_globals)

include/util_unfold.h: $(srcdir)/../sfutil/util_unfold.h
	@src_header=$?; dst_header=$@; $(copy_headers)

include/util_unfold.c: $(srcdir)/../sfutil/util_unfold.c
	@src_header=$?; dst_header=$@; $(copy_headers)

include/sf_base64decode.h: $(srcdir)/../sfutil/sf_base64decode.h
	@src_header=$?; dst_header=$@; $(copy_headers)

include/sf_base64decode.c: $(srcdir)/../sfutil/sf_base64decode.c
	@src_header=$?; dst_header=$@; $(copy_headers)

include/sf_email_attach_decode.h: $(srcdir)/../sfutil/sf_email_attach_decode.h
	@src_header=$?; dst_header=$@; $(copy_headers)

include/sf_email_attach_decode.c: $(srcdir)/../sfutil/sf_email_attach_decode.c
	@src_header=$?; dst_header=$@; $(copy_headers); $(copy_error_message); $(replace_policy_globals)

include/Unified2_common.h: $(srcdir)/../sfutil/Unified2_common.h
	@src_header=$?; dst_header=$@; $(copy_headers)

# From dynamic-plugins/sf_engine/examples
include/sfsnort_dynamic_detection_lib.c: $(srcdir)/../dynamic-plugins/sf_engine/examples/sfsnort_dynamic_detection_lib.c
	@src_header=$?; dst_header=$@; $(copy_headers)

include/sfsnort_dynamic_detection_lib.h: $(srcdir)/../dynamic-plugins/sf_engine/examples/sfsnort_dynamic_detection_lib.h
	@src_header=$?; dst_header=$@; $(copy_headers)

# From dynamic-plugins/sf_engine
include/sf_snort_packet.h: $(srcdir)/../dynamic-plugins/sf_engine/sf_snort_packet.h
	@src_header=$?; dst_header=$@; $(copy_headers)

include/sf_protocols.h: $(srcdir)/../sf_protocols.h
	@src_header=$?; dst_header=$@; $(copy_headers)

include/sf_snort_plugin_api.h: $(srcdir)/../dynamic-plugins/sf_engine/sf_snort_plugin_api.h
	@src_header=$?; dst_header=$@; $(copy_headers)

include/sf_decompression.h: $(srcdir)/../dynamic-plugins/sf_engine/sf_decompression.h
	@src_header=$?; dst_header=$@; $(copy_headers)

include/sf_decompression_define.h: $(srcdir)/../dynamic-plugins/sf_decompression_define.h
	@src_header=$?; dst_header=$@; $(copy_headers)

# Session API/String Searching, massage it to use SFSnortPacket
include/session_api.h: $(srcdir)/../preprocessors/session_api.h
	@src_header=$?; dst_header=$@; $(massage_headers)

# Stream API/String Searching, massage it to use SFSnortPacket
include/stream_api.h: $(srcdir)/../preprocessors/stream_api.h
	@src_header=$?; dst_header=$@; $(massage_headers)

include/str_search.h: $(srcdir)/../preprocessors/str_search.h
	@src_header=$?; dst_header=$@; $(massage_headers)

include/treenodes.h: $(srcdir)/../treenodes.h
	@src_header=$?; dst_header=$@; $(copy_treenode_header)

include/signature.h: $(srcdir)/../signature.h
	@src_header=$?; dst_header=$@; $(copy_treenode_header)

include/plugin_enum.h: $(srcdir)/../plugin_enum.h
	@src_header=$?; dst_header=$@; $(copy_headers)

include/sfcontrol.h: $(top_srcdir)/src/control/sfcontrol.h
	@src_header=$?; dst_header=$@; $(copy_headers)

include/idle_processing.h: $(top_srcdir)/src/idle_processing.h
	@src_header=$?; dst_header=$@; $(copy_headers)

include/sf_seqnums.h: $(top_srcdir)/src/sfutil/sf_seqnums.h
	@src_header=$?; dst_header=$@; $(copy_headers)

include/perf_indicators.h: $(srcdir)/../preprocessors/perf_indicators.h
	@src_header=$?; dst_header=$@; $(copy_headers)

include/file_api.h: $(top_srcdir)/src/file-process/file_api.h
	@src_header=$?; dst_header=$@; $(copy_headers)

include/file_mail_common.h: $(top_srcdir)/src/file-process/file_mail_common.h
	@src_header=$?; dst_header=$@; $(copy_headers)
	
include/sfdebug.h: $(srcdir)/../sfutil/sfdebug.h
	@src_header=$?; dst_header=$@; $(copy_headers)

include/mpse_methods.h: $(srcdir)/../sfutil/mpse_methods.h
	@src_header=$?; dst_header=$@; $(copy_headers)

include/sip_common.h: $(srcdir)/../preprocessors/sip_common.h
	@src_header=$?; dst_header=$@; $(copy_headers)

include/cip_common.h: $(srcdir)/../preprocessors/cip_common.h
	@src_header=$?; dst_header=$@; $(copy_headers)

include/appId.h: $(srcdir)/appid/appId.h
	@src_header=$?; dst_header=$@; $(copy_headers)

if FEAT_OPEN_APPID
include/appIdApi.h: $(srcdir)/../appIdApi.h
	@src_header=$?; dst_header=$@; $(copy_headers)

include/thirdparty_appid_types.h: $(srcdir)/appid/thirdparty_appid_types.h
	@src_header=$?; dst_header=$@; $(copy_headers)

include/thirdparty_appid_api.h: $(srcdir)/appid/thirdparty_appid_api.h
	@src_header=$?; dst_header=$@; $(copy_headers)

include/dns_defs.h: $(srcdir)/appid/dns_defs.h
	@src_header=$?; dst_header=$@; $(copy_headers)

include/md5.c: $(srcdir)/../sfutil/md5.c
	@src_header=$?; dst_header=$@; $(copy_headers)

include/md5.h: $(srcdir)/../sfutil/md5.h
	@src_header=$?; dst_header=$@; $(copy_headers)

include/sfprimetable.h: $(srcdir)/../sfutil/sfprimetable.h
	@src_header=$?; dst_header=$@; $(copy_headers); $(copy_error_message); $(replace_policy_globals)

include/sfprimetable.c: $(srcdir)/../sfutil/sfprimetable.c
	@src_header=$?; dst_header=$@; $(copy_headers); $(copy_error_message); $(replace_policy_globals)

include/sfxhash.h: $(srcdir)/../sfutil/sfxhash.h
	@src_header=$?; dst_header=$@; $(copy_headers); $(copy_error_message); $(replace_policy_globals)

include/sfxhash.c: $(srcdir)/../sfutil/sfxhash.c
	@src_header=$?; dst_header=$@; $(copy_headers); $(copy_error_message); $(replace_policy_globals)

include/sfghash.c: $(srcdir)/../sfutil/sfghash.c
	@src_header=$?; dst_header=$@; $(copy_headers); $(copy_error_message); $(replace_policy_globals)

include/sfhashfcn.c: $(srcdir)/../sfutil/sfhashfcn.c
	@src_header=$?; dst_header=$@; $(copy_headers); $(copy_error_message); $(replace_policy_globals); $(copy_no_static_hash)

include/sflsq.h: $(srcdir)/../sfutil/sflsq.h
	@src_header=$?; dst_header=$@; $(copy_headers); $(copy_error_message); $(replace_policy_globals)

include/sflsq.c: $(srcdir)/../sfutil/sflsq.c
	@src_header=$?; dst_header=$@; $(copy_headers); $(copy_error_message); $(replace_policy_globals)
else
if BUILD_SNORT_RELOAD
include/sfxhash.h: $(srcdir)/../sfutil/sfxhash.h
	@src_header=$?; dst_header=$@; $(copy_headers); $(copy_error_message); $(replace_policy_globals)

include/sfxhash.c: $(srcdir)/../sfutil/sfxhash.c
	@src_header=$?; dst_header=$@; $(copy_headers); $(copy_error_message); $(replace_policy_globals)

include/sfprimetable.h: $(srcdir)/../sfutil/sfprimetable.h
	@src_header=$?; dst_header=$@; $(copy_headers); $(copy_error_message); $(replace_policy_globals)

include/sfprimetable.c: $(srcdir)/../sfutil/sfprimetable.c
	@src_header=$?; dst_header=$@; $(copy_headers); $(copy_error_message); $(replace_policy_globals)

include/sfhashfcn.c: $(srcdir)/../sfutil/sfhashfcn.c
	@src_header=$?; dst_header=$@; $(copy_headers); $(copy_error_message); $(replace_policy_globals); $(copy_no_static_hash)
endif
endif

if BUILD_SNORT_RELOAD
include/appdata_adjuster.c: $(srcdir)/../reload-adjust/appdata_adjuster.c
	@src_header=$?; dst_header=$@; $(copy_headers); $(copy_error_message); $(replace_policy_globals)

include/appdata_adjuster.h: $(srcdir)/../reload-adjust/appdata_adjuster.h
	@src_header=$?; dst_header=$@; $(copy_headers)
endif

include/memory_stats.h: $(srcdir)/../memory_stats.h
	@src_header=$?; dst_header=$@; $(copy_headers)

if FEAT_FILE_INSPECT
FILE_INSPECT_DIR = file
endif

SUBDIRS = . libs ftptelnet pop imap smtp ssh dns ssl dcerpc2 sdf sip reputation gtp modbus dnp3 s7commplus $(FILE_INSPECT_DIR)

if FEAT_OPEN_APPID
SUBDIRS += appid
endif

clean-local:
	rm -rf include build

EXTRA_DIST = \
dynamic_preprocessors.vcxproj \
dynamic_preprocessors.dsp \
sf_dynamic_initialize/sf_dynamic_initialize.vcxproj \
sf_dynamic_initialize/sf_dynamic_initialize.dsp \
treenodes.sed

srcinstdir = $(exec_prefix)/src/snort_dynamicsrc

exported_files = \
include/sf_dynamic_common.h \
include/sf_dynamic_define.h \
include/sf_dynamic_engine.h \
include/sf_dynamic_meta.h \
include/sf_dynamic_preprocessor.h \
include/sf_dynamic_preproc_lib.h \
include/sf_dynamic_preproc_lib.c \
include/sf_ip.h \
include/sf_snort_packet.h \
include/sf_protocols.h \
include/sf_snort_plugin_api.h \
include/sf_decompression.h \
include/sf_decompression_define.h \
include/sf_types.h \
include/sfsnort_dynamic_detection_lib.h \
include/sfsnort_dynamic_detection_lib.c \
include/pcap_pkthdr32.h \
include/str_search.h \
include/session_api.h \
include/stream_api.h \
include/snort_debug.h \
include/profiler.h \
include/sfghash.h \
include/sfhashfcn.h \
include/sfmemcap.h \
include/bitop.h \
include/preprocids.h \
include/sfPolicyUserData.h \
include/util_unfold.h \
include/util_unfold.c \
include/sf_base64decode.h \
include/sf_base64decode.c \
include/sf_email_attach_decode.h \
include/sf_email_attach_decode.c \
include/treenodes.h \
include/signature.h \
include/plugin_enum.h \
include/sfPolicyUserData.c \
include/obfuscation.h \
include/sidechannel_define.h \
include/rule_option_types.h \
include/event.h \
include/Unified2_common.h \
include/sfcontrol.h \
include/idle_processing.h \
include/sf_seqnums.h \
include/perf_indicators.h \
include/file_api.h \
include/file_mail_common.h \
include/mpse_methods.h \
include/sfdebug.h \
include/sip_common.h \
include/cip_common.h \
include/reload_api.h \
include/reg_test.h \
include/reg_test.c \
ssl_common/ssl.h \
ssl_common/ssl.c \
ssl_common/ssl_include.h \
ssl_common/ssl_config.h \
ssl_common/ssl_config.c \
ssl_common/ssl_session.h \
ssl_common/ssl_inspect.h \
ssl_common/ssl_inspect.c \
ssl_common/ssl_ha.h \
ssl_common/ssl_ha.c \
libs/sfparser.c \
include/memory_stats.h

if FEAT_OPEN_APPID
exported_files += include/appId.h include/appIdApi.h include/thirdparty_appid_types.h \
				  include/thirdparty_appid_api.h \
				  include/sfprimetable.h include/sfxhash.h include/sfhashfcn.h \
				  include/md5.h \
				  include/dns_defs.h
endif

if BUILD_SNORT_RELOAD
exported_files += include/sfprimetable.h \
include/sfprimetable.c \
include/sfmemcap.h \
include/sfmemcap.c \
include/sfhashfcn.h \
include/sfhashfcn.c \
include/sfxhash.h \
include/sfxhash.c \
include/appdata_adjuster.h \
include/appdata_adjuster.c
endif

install-data-local:
	@for f in $(exported_files); do \
## Compute the filename only
		truefile=`echo $$f | sed -e "s/.*\///"`; \
## Make the install directory.
		$(mkinstalldirs) $(DESTDIR)$(srcinstdir); \
## Find the header file -- in our case it might be in srcdir or
## it might be in the build directory.  "p" is the variable that
## names the actual file we will install.
		if test -f $(srcdir)/$$f; then p=$(srcdir)/$$f; else p=$$f; fi; \
## Actually install the file.
		$(INSTALL_DATA) $$p $(DESTDIR)$(srcinstdir)/$$truefile; \
	done

uninstall-local:
	@for f in $(exported_files); do \
## Compute the filename only
		truefile=`echo $$f | sed -e "s/.*\///"`; \
## Make the install directory.
		$(mkinstalldirs) $(DESTDIR)$(srcinstdir); \
## Actually install the file.
		rm -f $(DESTDIR)$(srcinstdir)/$$truefile; \
	done
