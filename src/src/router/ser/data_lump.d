data_lump.o data_lump.d : data_lump.c data_lump.h lump_struct.h parser/msg_parser.h \
  parser/../comp_defs.h parser/../str.h parser/../lump_struct.h \
  parser/../flags.h parser/../ip_addr.h parser/../str.h \
  parser/../dprint.h parser/../md5utils.h parser/../config.h \
  parser/../types.h parser/parse_def.h parser/parse_cseq.h \
  parser/parse_to.h parser/parse_via.h parser/parse_fline.h parser/hf.h \
  dprint.h mem/mem.h mem/../config.h mem/../dprint.h mem/q_malloc.h \
  globals.h types.h ip_addr.h str.h error.h
