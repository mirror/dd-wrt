bin_PROGRAMS=rtpproxy rtpproxy_debug
rtpproxy_SOURCES=main.c rtp.h rtp_server.c rtp_server.h \
  rtpp_defines.h rtpp_log.h rtpp_record.c rtpp_record.h rtpp_session.h \
  rtpp_util.c rtpp_util.h rtp.c rtp_resizer.c rtp_resizer.h rtpp_session.c \
  rtpp_command.c rtpp_command.h rtpp_network.h rtpp_network.c \
  rtpp_syslog_async.c rtpp_syslog_async.h rtpp_notify.c rtpp_notify.h \
  rtpp_command_async.h rtpp_command_async.c rtpp_proc_async.c \
  rtpp_proc_async.h rtpp_proc.h rtpp_proc.c rtpp_version.h rtpp_math.c \
  rtpp_math.h rtpp_wi.c rtpp_wi.h rtpp_wi_private.h rtpp_queue.c rtpp_queue.h \
  rtpp_netio_async.c rtpp_netio_async.h rtpp_record_private.h \
  rtpp_command_parse.c rtpp_command_parse.h rtpp_command_copy.c \
  rtpp_command_copy.h rtpp_command_ul.c rtpp_command_ul.h \
  rtpp_log_stand.c rtpp_log_stand.h rtpp_hash_table.c rtpp_hash_table.h \
  rtpp_types.h rtpp_pcache.c rtpp_pcache.h config_pp.h rtpp_stats.c \
  rtpp_stats.h rtpp_cfg_stable.h rtpp_command_stats.h rtpp_command_stats.c \
  rtp_info.h rtpp_command_private.h rtpp_controlfd.h rtpp_controlfd.c \
  rtpp_list.h rtpp_command_stream.h rtpp_command_stream.c
SRCS_DEBUG=	rtpp_memdeb.c rtpp_memdeb.h rtpp_memdeb_internal.h \
  rtpp_memdeb_stats.h rtpp_memdeb_test.c
rtpproxy_debug_SOURCES=${rtpproxy_SOURCES} ${SRCS_DEBUG}

rtpproxy_LDADD=-lm -lpthread
rtpproxy_debug_LDADD=${rtpproxy_LDADD}
rtpproxy_debug_LDFLAGS=-static
rtpproxy_debug_CPPFLAGS=-g3 -O0 -DRTPP_DEBUG -DRTPP_MEMDEB_STDOUT -include rtpp_memdeb.h

DEFS=	-DWITHOUT_SIPLOG -Wall -DRTPP_LOG_ADVANCED @DEFS@
