rule.o rule.d : rule.c ../../sr_module.h ../../parser/msg_parser.h \
  ../../parser/../comp_defs.h ../../parser/../str.h \
  ../../parser/../lump_struct.h ../../parser/../flags.h \
  ../../parser/../ip_addr.h ../../parser/../str.h \
  ../../parser/../dprint.h ../../parser/../md5utils.h \
  ../../parser/../config.h ../../parser/../types.h \
  ../../parser/parse_def.h ../../parser/parse_cseq.h \
  ../../parser/parse_to.h ../../parser/parse_via.h \
  ../../parser/parse_fline.h ../../parser/hf.h rule.h
