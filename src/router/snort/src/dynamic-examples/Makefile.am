## $Id$
AUTOMAKE_OPTIONS=foreign no-dependencies

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
	include/sf_decompression_define.h \
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
	include/pcap_pkthdr32.h \
	include/session_api.h \
	include/stream_api.h \
	include/str_search.h \
	include/sf_types.h \
	include/sfrt.h \
	include/sfrt.c \
	include/sfrt_dir.h \
	include/sfrt_dir.c \
	include/sfrt_trie.h \
	include/sfPolicyUserData.c \
	include/sfPolicyUserData.h \
	include/sfPolicy.h \
	include/treenodes.h \
	include/signature.h \
	include/plugin_enum.h \
	include/obfuscation.h \
	include/packet_time.h \
	include/rule_option_types.h \
	include/event.h \
	include/sfcontrol.h \
	include/sidechannel_define.h \
	include/idle_processing.h \
	include/file_api.h \
	include/mpse_methods.h \
	include/reload_api.h \
	include/smtp_api.h \
	include/memory_stats.h

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

replace_policy_globals = \
	if test -f $$dst_header; then \
		sed -e "/SharedObjectAddStarts/d" \
		    -e "/SharedObjectAddEnds/d" \
            -e "/SharedObjectDeleteBegins/,/SharedObjectDeleteEnds/d" \
		    -e "s/getDefaultPolicy()/_dpd.getDefaultPolicy()/" \
		        $$dst_header > $$dst_header.new; \
		mv $$dst_header.new $$dst_header; \
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
	sed -f $(srcdir)/../dynamic-preprocessors/treenodes.sed $$dst_header.new > $$dst_header

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

include/sfcontrol.h: $(srcdir)/../control/sfcontrol.h
	@src_header=$?; dst_header=$@; $(copy_headers)

include/sidechannel_define.h: $(srcdir)/../side-channel/sidechannel_define.h
	@src_header=$?; dst_header=$@; $(copy_headers)

include/idle_processing.h: $(srcdir)/../idle_processing.h
	@src_header=$?; dst_header=$@; $(copy_headers)

include/reload_api.h: $(srcdir)/../reload_api.h
	@src_header=$?; dst_header=$@; $(massage_headers)

include/smtp_api.h: $(srcdir)/../smtp_api.h
	@src_header=$?; dst_header=$@; $(massage_headers)

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

include/sf_decompression_define.h: $(srcdir)/../dynamic-plugins/sf_decompression_define.h
	@src_header=$?; dst_header=$@; $(copy_headers)

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

include/sfrt_trie.h: $(srcdir)/../sfutil/sfrt_trie.h
	@src_header=$?; dst_header=$@; $(copy_headers)

include/sfPolicyUserData.c: $(srcdir)/../sfutil/sfPolicyUserData.c
	@src_header=$?; dst_header=$@; $(copy_headers); $(replace_policy_globals)

include/sfPolicyUserData.h: $(srcdir)/../sfutil/sfPolicyUserData.h
	@src_header=$?; dst_header=$@; $(copy_headers); $(replace_policy_globals)

include/sfPolicy.h: $(srcdir)/../sfutil/sfPolicy.h
	@src_header=$?; dst_header=$@; $(copy_headers); $(replace_policy_globals)

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

# Session API/String Searching, massage it to use SFSnortPacket
include/session_api.h: $(srcdir)/../preprocessors/session_api.h
	@src_header=$?; dst_header=$@; $(massage_headers)

# Stream API/String Searching, massage it to use SFSnortPacket
include/stream_api.h: $(srcdir)/../preprocessors/stream_api.h
	@src_header=$?; dst_header=$@; $(massage_headers)

include/str_search.h: $(srcdir)/../preprocessors/str_search.h
	@src_header=$?; dst_header=$@; $(massage_headers)

include/mpse_methods.h: $(srcdir)/../sfutil/mpse_methods.h
	@src_header=$?; dst_header=$@; $(massage_headers)

include/treenodes.h: $(srcdir)/../treenodes.h
	@src_header=$?; dst_header=$@; $(copy_treenode_header)

include/signature.h: $(srcdir)/../signature.h
	@src_header=$?; dst_header=$@; $(copy_treenode_header)

include/plugin_enum.h: $(srcdir)/../plugin_enum.h
	@src_header=$?; dst_header=$@; $(copy_headers)
	
include/file_api.h: $(top_srcdir)/src/file-process/file_api.h
	@src_header=$?; dst_header=$@; $(copy_headers)

include/memory_stats.h: $(srcdir)/../memory_stats.h
	@src_header=$?; dst_header=$@; $(copy_headers)

INCLUDES = @INCLUDES@

if BUILD_DYNAMIC_EXAMPLES
SUBDIRS = dynamic-preprocessor dynamic-rule
endif

clean-local:
	rm -rf include build

