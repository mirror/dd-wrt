receive.o receive.d : receive.c receive.h ip_addr.h str.h dprint.h globals.h types.h \
  route.h config.h error.h route_struct.h parser/msg_parser.h \
  parser/../comp_defs.h parser/../str.h parser/../lump_struct.h \
  parser/../flags.h parser/../ip_addr.h parser/../md5utils.h \
  parser/../str.h parser/../config.h parser/parse_def.h \
  parser/parse_cseq.h parser/parse_to.h parser/parse_via.h \
  parser/parse_fline.h parser/hf.h forward.h proxy.h stats.h udp_server.h \
  tcp_server.h action.h mem/mem.h mem/../config.h mem/../dprint.h \
  mem/f_malloc.h script_cb.h dset.h usr_avp.h
