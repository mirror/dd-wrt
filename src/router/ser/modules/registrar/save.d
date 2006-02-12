save.o save.d : save.c ../../comp_defs.h save.h ../../parser/msg_parser.h \
  ../../str.h ../../lump_struct.h ../../flags.h ../../ip_addr.h \
  ../../dprint.h ../../md5utils.h ../../config.h ../../types.h \
  ../../parser/parse_def.h ../../parser/parse_cseq.h \
  ../../parser/parse_to.h ../../parser/parse_via.h \
  ../../parser/parse_fline.h ../../parser/hf.h ../../trim.h ../../ut.h \
  ../usrloc/usrloc.h ../usrloc/dlist.h ../usrloc/udomain.h \
  ../../locking.h ../../lock_ops.h ../../lock_alloc.h ../../mem/mem.h \
  ../../mem/f_malloc.h ../../mem/shm_mem.h ../usrloc/urecord.h \
  ../usrloc/hslot.h ../usrloc/ucontact.h ../usrloc/notify.h common.h \
  sip_msg.h ../../parser/contact/parse_contact.h \
  ../../parser/contact/contact.h ../../parser/parse_param.h rerrno.h \
  reply.h reg_mod.h regtime.h
