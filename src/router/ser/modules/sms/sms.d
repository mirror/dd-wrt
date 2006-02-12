sms.o sms.d : sms.c ../../sr_module.h ../../parser/msg_parser.h \
  ../../comp_defs.h ../../str.h ../../lump_struct.h ../../flags.h \
  ../../ip_addr.h ../../dprint.h ../../md5utils.h ../../config.h \
  ../../types.h ../../parser/parse_def.h ../../parser/parse_cseq.h \
  ../../parser/parse_to.h ../../parser/parse_via.h \
  ../../parser/parse_fline.h ../../parser/hf.h ../../error.h ../../ut.h \
  ../../globals.h ../../mem/mem.h ../../mem/f_malloc.h \
  ../../mem/shm_mem.h ../../lock_ops.h ../tm/tm_load.h ../tm/defs.h \
  ../tm/t_hooks.h ../tm/uac.h ../tm/dlg.h ../../parser/parse_rr.h \
  ../../parser/parse_nameaddr.h ../../parser/parse_param.h ../tm/t_fwd.h \
  ../../proxy.h ../tm/t_reply.h ../../tags.h ../../crc.h ../tm/h_table.h \
  ../../usr_avp.h ../tm/config.h ../../hash_func.h ../tm/lock.h \
  ../../locking.h ../../lock_alloc.h ../tm/timer.h ../tm/sip_msg.h \
  ../tm/t_lookup.h ../tm/t_funcs.h ../../udp_server.h \
  ../../msg_translator.h ../../timer.h ../../forward.h ../../route.h \
  ../../route_struct.h ../../stats.h ../../tcp_server.h \
  ../../parser/parse_uri.h ../tm/ut.h sms_funcs.h sms_report.h \
  libsms_modem.h
