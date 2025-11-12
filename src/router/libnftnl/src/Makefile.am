include $(top_srcdir)/Make_global.am
lib_LTLIBRARIES = libnftnl.la

libnftnl_la_LIBADD = ${LIBMNL_LIBS}
libnftnl_la_LDFLAGS = -Wl,--version-script=$(srcdir)/libnftnl.map	\
		      -version-info $(LIBVERSION)

libnftnl_la_SOURCES = utils.c		\
		      batch.c		\
		      flowtable.c	\
		      common.c		\
		      gen.c		\
		      table.c		\
		      trace.c		\
		      chain.c		\
		      object.c		\
		      rule.c		\
		      set.c		\
		      set_elem.c	\
		      str_array.c	\
		      ruleset.c		\
		      udata.c		\
		      expr.c		\
		      expr_ops.c	\
		      expr/bitwise.c	\
		      expr/byteorder.c	\
		      expr/cmp.c	\
		      expr/range.c	\
		      expr/connlimit.c	\
		      expr/counter.c	\
		      expr/ct.c		\
		      expr/data_reg.c	\
		      expr/dup.c	\
		      expr/exthdr.c	\
		      expr/flow_offload.c \
		      expr/fib.c	\
		      expr/fwd.c	\
		      expr/last.c	\
		      expr/limit.c	\
		      expr/log.c	\
		      expr/lookup.c	\
		      expr/dynset.c	\
		      expr/immediate.c	\
		      expr/inner.c	\
		      expr/match.c	\
		      expr/meta.c	\
		      expr/numgen.c	\
		      expr/nat.c	\
		      expr/tproxy.c	\
		      expr/objref.c	\
		      expr/payload.c	\
		      expr/queue.c	\
		      expr/quota.c	\
		      expr/reject.c	\
		      expr/rt.c		\
		      expr/target.c	\
		      expr/tunnel.c	\
		      expr/masq.c	\
		      expr/redir.c	\
		      expr/hash.c	\
		      expr/socket.c	\
		      expr/synproxy.c	\
		      expr/osf.c	\
		      expr/xfrm.c	\
		      obj/counter.c	\
		      obj/ct_helper.c	\
		      obj/quota.c	\
		      obj/tunnel.c	\
		      obj/limit.c	\
		      obj/synproxy.c	\
		      obj/ct_timeout.c 	\
		      obj/secmark.c	\
		      obj/ct_expect.c 	\
		      libnftnl.map
