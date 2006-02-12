common.o common.d : common.c ../../dprint.h ../../ut.h ../../comp_defs.h \
  ../../config.h ../../types.h ../../str.h ../../parser/parse_uri.h \
  ../../parser/msg_parser.h ../../lump_struct.h ../../flags.h \
  ../../ip_addr.h ../../md5utils.h ../../parser/parse_def.h \
  ../../parser/parse_cseq.h ../../parser/parse_to.h \
  ../../parser/parse_via.h ../../parser/parse_fline.h ../../parser/hf.h \
  common.h rerrno.h reg_mod.h ../usrloc/usrloc.h ../usrloc/dlist.h \
  ../usrloc/udomain.h ../../locking.h ../../lock_ops.h ../../lock_alloc.h \
  ../../mem/mem.h ../../mem/f_malloc.h ../../mem/shm_mem.h \
  ../usrloc/urecord.h ../usrloc/hslot.h ../usrloc/ucontact.h \
  ../usrloc/notify.h
