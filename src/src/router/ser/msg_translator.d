msg_translator.o msg_translator.d : msg_translator.c comp_defs.h msg_translator.h \
  parser/msg_parser.h parser/../comp_defs.h parser/../str.h \
  parser/../lump_struct.h parser/../flags.h parser/../ip_addr.h \
  parser/../str.h parser/../dprint.h parser/../md5utils.h \
  parser/../config.h parser/../types.h parser/parse_def.h \
  parser/parse_cseq.h parser/parse_to.h parser/parse_via.h \
  parser/parse_fline.h parser/hf.h ip_addr.h globals.h types.h str.h \
  error.h mem/mem.h mem/../config.h mem/../dprint.h mem/q_malloc.h \
  dprint.h config.h md5utils.h data_lump.h lump_struct.h data_lump_rpl.h \
  resolve.h ut.h pt.h timer.h
