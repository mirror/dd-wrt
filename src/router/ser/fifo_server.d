fifo_server.o fifo_server.d : fifo_server.c dprint.h ut.h comp_defs.h config.h types.h \
  str.h error.h globals.h ip_addr.h fifo_server.h mem/mem.h \
  mem/../config.h mem/../dprint.h mem/f_malloc.h sr_module.h \
  parser/msg_parser.h parser/../comp_defs.h parser/../str.h \
  parser/../lump_struct.h parser/../flags.h parser/../ip_addr.h \
  parser/../md5utils.h parser/../str.h parser/../config.h \
  parser/parse_def.h parser/parse_cseq.h parser/parse_to.h \
  parser/parse_via.h parser/parse_fline.h parser/hf.h pt.h timer.h
