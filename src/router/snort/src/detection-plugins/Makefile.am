## $Id
EXTRA_DIST=detection_leaf_node.c
AUTOMAKE_OPTIONS=foreign no-dependencies

noinst_LIBRARIES = libspd.a

BUILT_SOURCES = sf_snort_plugin_hdropts.c \
sf_snort_packet.h \
sf_snort_plugin_api.h \
sf_decompression.h \
sf_decompression_define.h

nodist_libspd_a_SOURCES = sf_snort_plugin_hdropts.c \
sf_snort_packet.h \
sf_snort_plugin_api.h \
sf_decompression.h \
sf_decompression_define.h

libspd_a_SOURCES = \
detection_options.c detection_options.h \
sp_asn1.c sp_asn1.h \
sp_asn1_detect.c sp_asn1_detect.h \
sp_byte_check.c sp_byte_check.h \
sp_byte_jump.c sp_byte_jump.h \
sp_byte_extract.c sp_byte_extract.h \
sp_byte_math.c sp_byte_math.h \
sp_clientserver.c sp_clientserver.h \
sp_cvs.c sp_cvs.h \
sp_dsize_check.c sp_dsize_check.h \
sp_flowbits.c sp_flowbits.h \
sp_ftpbounce.c sp_ftpbounce.h \
sp_hdr_opt_wrap.c sp_hdr_opt_wrap.h \
sp_icmp_code_check.c sp_icmp_code_check.h \
sp_icmp_id_check.c sp_icmp_id_check.h \
sp_icmp_seq_check.c sp_icmp_seq_check.h \
sp_icmp_type_check.c sp_icmp_type_check.h \
sp_ip_fragbits.c sp_ip_fragbits.h \
sp_ip_id_check.c sp_ip_id_check.h \
sp_ip_proto.c sp_ip_proto.h \
sp_ip_same_check.c sp_ip_same_check.h \
sp_ip_tos_check.c sp_ip_tos_check.h \
sp_ipoption_check.c sp_ipoption_check.h \
sp_isdataat.c sp_isdataat.h \
sp_pattern_match.c sp_pattern_match.h  \
sp_pcre.c sp_pcre.h \
sp_replace.c sp_replace.h \
sp_rpc_check.c sp_rpc_check.h \
sp_session.c sp_session.h \
sp_tcp_ack_check.c sp_tcp_ack_check.h \
sp_tcp_flag_check.h sp_tcp_flag_check.c \
sp_tcp_seq_check.c sp_tcp_seq_check.h \
sp_tcp_win_check.c sp_tcp_win_check.h \
sp_ttl_check.c sp_ttl_check.h \
sp_urilen_check.c sp_urilen_check.h \
sp_file_data.c sp_file_data.h \
sp_base64_decode.c sp_base64_decode.h \
sp_base64_data.c sp_base64_data.h \
sp_pkt_data.c sp_pkt_data.h \
sp_file_type.c sp_file_type.h

if BUILD_REACT
libspd_a_SOURCES += sp_react.c sp_react.h
endif

if BUILD_RESPOND3
libspd_a_SOURCES += sp_respond3.c sp_respond.h
endif

if FEAT_OPEN_APPID
libspd_a_SOURCES += sp_appid.c sp_appid.h 
endif
copy_files = \
	if test -f $$dst_file; then \
		x=`diff $$src_file $$dst_file >> /dev/null`; \
		if test "$$x" != "0"; then \
			echo "Updating " $$dst_file; \
			cp $$src_file $$dst_file; \
		fi \
	else \
		echo "Updating " $$dst_file; \
		cp $$src_file $$dst_file; \
	fi

sed_hdropts = \
	sed -e "s/_ded.errMsg/ErrorMessage/g" \
		-e "s/sf_snort_packet.h/decode.h/g" \
	    -e "s/SFSnortPacket/Packet/g" \
		-e "s/ip4_header/iph/g" \
		-e "s/tcp_header/tcph/g" \
		-e "s/proto/ip_proto/g" \
		-e "s/type_service/ip_tos/g" \
		-e "s/time_to_live/ip_ttl/g" \
		-e "s/num_ip_options/ip_option_count/g" \
		-e "s/IPOptions/Options/g" \
		-e "s/option_code/code/g" \
		-e "s/acknowledgement/th_ack/g" \
		-e "s/sequence/th_seq/g" \
		-e "s/tcph->flags/tcph->th_flags/g" \
		-e "s/tcph->window/tcph->th_win/g" \
		-e "s/num_tcp_options/tcp_option_count/g" \
		-e "s/icmp_header/icmph/g" \
		-e "s/ICMP_ECHO_REPLY/ICMP_ECHOREPLY/g" \
		-e "s/ICMP_ECHO_REQUEST/ICMP_ECHO/g" \
		-e "s/icmph_union.echo.id/s_icmp_id/g" \
		-e "s/icmph_union.echo.seq/s_icmp_seq/g" \
	$$src_file > $$dst_file;

sf_snort_plugin_hdropts.c: ../dynamic-plugins/sf_engine/sf_snort_plugin_hdropts.c
	@src_file=$?; dst_file=$@; $(sed_hdropts)

sf_snort_plugin_api.h: ../dynamic-plugins/sf_engine/sf_snort_plugin_api.h
	@src_file=$?; dst_file=$@; $(copy_files)

sf_decompression.h: ../dynamic-plugins/sf_engine/sf_decompression.h
	@src_file=$?; dst_file=$@; $(copy_files)

sf_decompression_define.h: ../dynamic-plugins/sf_decompression_define.h
	@src_file=$?; dst_file=$@; $(copy_files)

sf_snort_packet.h: ../dynamic-plugins/sf_engine/sf_snort_packet.h
	@src_file=$?; dst_file=$@; $(copy_files)

clean-local:
	rm -rf sf_snort_plugin_hdropts.c sf_snort_packet.h sf_snort_plugin_api.h sf_decompression.h sf_decompression_define.h

INCLUDES = @INCLUDES@
