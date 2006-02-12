tcp_main.o tcp_main.d : tcp_main.c ip_addr.h str.h dprint.h pass_fd.h tcp_conn.h \
  locking.h lock_ops.h lock_alloc.h mem/mem.h mem/../config.h \
  mem/../types.h mem/../dprint.h mem/f_malloc.h mem/shm_mem.h \
  mem/../lock_ops.h globals.h types.h pt.h timer.h sr_module.h \
  parser/msg_parser.h parser/../comp_defs.h parser/../str.h \
  parser/../lump_struct.h parser/../flags.h parser/../ip_addr.h \
  parser/../md5utils.h parser/../str.h parser/../config.h \
  parser/parse_def.h parser/parse_cseq.h parser/parse_to.h \
  parser/parse_via.h parser/parse_fline.h parser/hf.h tcp_server.h \
  tcp_init.h
