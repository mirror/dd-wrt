main.o main.d : main.c config.h types.h dprint.h route.h error.h str.h \
  route_struct.h parser/msg_parser.h parser/../comp_defs.h \
  parser/../str.h parser/../lump_struct.h parser/../flags.h \
  parser/../ip_addr.h parser/../str.h parser/../dprint.h \
  parser/../md5utils.h parser/../config.h parser/parse_def.h \
  parser/parse_cseq.h parser/parse_to.h parser/parse_via.h \
  parser/parse_fline.h parser/hf.h udp_server.h ip_addr.h globals.h \
  mem/mem.h mem/../config.h mem/../dprint.h mem/f_malloc.h mem/shm_mem.h \
  mem/../lock_ops.h sr_module.h timer.h resolve.h parser/parse_hname2.h \
  parser/digest/digest_parser.h parser/digest/../../str.h fifo_server.h \
  name_alias.h hash_func.h pt.h script_cb.h ut.h comp_defs.h tcp_init.h \
  stats.h
