## $Id
AUTOMAKE_OPTIONS=foreign no-dependencies

INCLUDES = @INCLUDES@

lib_LTLIBRARIES = libsf_engine.la

libdir = ${exec_prefix}/lib/snort_dynamicengine

libsf_engine_la_LDFLAGS = -shared -export-dynamic -module @XCCFLAGS@

BUILT_SOURCES = \
sfhashfcn.c \
sfhashfcn.h \
sfghash.c \
sfghash.h \
sfprimetable.c \
sfprimetable.h \
ipv6_port.h \
sf_ip.c \
sf_ip.h \
debug.h \
sf_types.h

nodist_libsf_engine_la_SOURCES = \
sfhashfcn.c \
sfhashfcn.h \
sfghash.c \
sfghash.h \
sfprimetable.c \
sfprimetable.h \
ipv6_port.h \
sf_ip.c \
sf_ip.h \
debug.h \
sf_types.h

libsf_engine_la_SOURCES = \
bmh.c \
bmh.h \
sf_snort_detection_engine.c \
sf_snort_packet.h \
sf_snort_plugin_api.c \
sf_snort_plugin_api.h \
sf_snort_plugin_byte.c \
sf_snort_plugin_content.c \
sf_snort_plugin_hdropts.c \
sf_snort_plugin_loop.c \
sf_snort_plugin_pcre.c \
sf_snort_plugin_rc4.c

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

copy_no_static_hash = \
	if test -f $$dst_file; then \
		x=`diff $$src_file $$dst_file.new >> /dev/null`; \
		if test "$$x" != "0"; then \
			echo "Updating " $$dst_file; \
			cp $$src_file $$dst_file.new; \
			sed -e "s/\#ifndef MODULUS_HASH/\#ifdef STATIC_HASH/" $$dst_file.new > $$dst_file; \
		fi \
	else \
		echo "Updating " $$dst_file; \
		cp $$src_file $$dst_file.new; \
		sed -e "s/\#ifndef MODULUS_HASH/\#ifdef STATIC_HASH/" $$dst_file.new > $$dst_file; \
	fi

sed_debug_header = \
	sed -e "s/DebugMessageFile = /*_ded.debugMsgFile = /" \
	    -e "s/DebugMessageLine = /*_ded.debugMsgLine = /" \
	    -e "s/; DebugMessageFunc$$/; _ded.debugMsg/" \
	    -e "s/; DebugWideMessageFunc$$/; _ded.debugWideMsg/" \
	    $$dst_file.new > $$dst_file

copy_debug_header = \
	if test -f $$dst_file; then \
		x=`diff $$src_file $$dst_file.new.new >> /dev/null`; \
		if test "$$x" != "0"; then \
			echo "Updating " $$dst_file; \
			cp $$src_file $$dst_file.new; \
			$(sed_debug_header); \
		fi \
	else \
		echo "Updating " $$dst_file; \
		cp $$src_file $$dst_file.new; \
		$(sed_debug_header); \
	fi

sed_ipv6_headers = \
	sed -e "s/->iph->ip_src/->ip4_header->source/" \
	    -e "s/->iph->ip_dst/->ip4_header->destination/" \
	    -e "s/->iph->/->ip4_header->/" \
	    -e "s/->iph$$/->ip4_header/" \
	    -e "s/orig_iph/orig_ip4h/" \
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

ipv6_port.h: ../../ipv6_port.h 
	@src_header=$?; dst_header=$@; $(massage_ipv6_headers)

sf_ip.h: ../../sfutil/sf_ip.h
	@src_file=$?; dst_file=$@; $(copy_files)

sf_ip.c: ../../sfutil/sf_ip.c
	@src_file=$?; dst_file=$@; $(copy_files)

debug.h: ../../debug.h
	@src_file=$?; dst_file=$@; $(copy_debug_header)

sfghash.h: ../../sfutil/sfghash.h
	@src_file=$?; dst_file=$@; $(copy_files)

sfghash.c: ../../sfutil/sfghash.c
	@src_file=$?; dst_file=$@; $(copy_files)

sfhashfcn.h: ../../sfutil/sfhashfcn.h
	@src_file=$?; dst_file=$@; $(copy_files)

sfhashfcn.c: ../../sfutil/sfhashfcn.c
	@src_file=$?; dst_file=$@; $(copy_no_static_hash)

sfprimetable.h: ../../sfutil/sfprimetable.h
	@src_file=$?; dst_file=$@; $(copy_files)

sfprimetable.c: ../../sfutil/sfprimetable.c
	@src_file=$?; dst_file=$@; $(copy_files)

sf_types.h: ../../sf_types.h
	@src_file=$?; dst_file=$@; $(copy_files)

SUBDIRS = examples

clean-local:
	rm -rf sfhashfcn.c sfhashfcn.c.new sfghash.c sfprimetable.c sf_ip.c sf_ip.h ipv6_port.h debug.h debug.h.new sfprimetable.h sfghash.h ipv6_port.h.new sfhashfcn.h sf_types.h
