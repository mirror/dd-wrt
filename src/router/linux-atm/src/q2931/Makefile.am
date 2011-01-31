noinst_PROGRAMS = q.out.o qd.dump.o
check_PROGRAMS = q.test

q_test_SOURCES = qtest.c uni.h
q_test_DEPENDENCIES = q.test.c

#TESTS = $(check_PROGRAMS)

EXTRA_DIST = incl.pl mknl.pl msg.fmt TODO

CLEANFILES = q.out.h q.out.c q.test.c qd.out.h qd.out.c qd.dump.c qd.test.c \
		default.nl

NLS = atm_ai_msg atm_ai_ie atm_loc atm_cv atm_pu atm_na atm_cond atm_ie \
	atm_msg atm_np atm_ton atm_sat atm_prs atm_scrn atm_vpa atm_poe \
	q2931_cs atm_td atm_bc atm_tc atm_stc atm_upcc q2931_proto atm_flag \
	atm_aalp atm_fd atm_tag atm_l2 atm_l3 atm_tt atm_mc atm_hl atm_imd \
	atm_tdl atm_tni atm_nip atm_shi atm_oci atm_unfm atm_ofi atm_irs \
	atm_it atm_lit atm_lsi atm_tcs atm_css atm_eqo atm_eqp atm_aap \
	atm_asp atm_tor
SYMFILES = $(srcdir)/uni.h $(shell $(CC) $(CFLAGS) -E $(srcdir)/header.c | $(AWK) -f $(srcdir)/script.awk)

default.nl: mknl.pl $(SYMFILES)
	cat $(SYMFILES) | @PERL@ $(srcdir)/mknl.pl $(NLS) >default.nl || \
	{ rm -f default.nl; echo 1; }

q.out.h q.out.c q.test.c: msg.fmt default.nl
	$(CPP) $(DEFS) $(DEFAULT_INCLUDES) $(INCLUDES) - < $(srcdir)/msg.fmt | ../qgen/qgen

qd.out.h qd.out.c qd.dump.c: msg.fmt default.nl
	$(CPP) $(DEFS) $(DEFAULT_INCLUDES) $(INCLUDES) - < $(srcdir)/msg.fmt | ../qgen/qgen -D

q.out.o: q.out.c q.out.h qlib.c qlib.h op.h uni.h

qd.dump.o: qd.dump.c qd.out.c qlib.c qlib.h op.h uni.h

#qd.dump.standalone.o: qd.dump.c qd.out.c qlib.c qlib.h
#	$(CC) $(DEFS) $(DEFAULT_INCLUDES) -DSTANDALONE $(INCLUDES) $(CFLAGS) -c qd.dump.c \
#		-o qd.dump.standalone.o
