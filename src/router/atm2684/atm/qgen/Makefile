LIBS=-lfl # lex may want  -ll  here
OBJS=common.o file.o first.o lex.yy.o qgen.o second.o third.o y.tab.o
TRASH=q.out.h q.out.c q.test.c qd.out.h qd.out.c qd.test.c qd.dump.c default.nl
# q40.out.h q40.out.c q40.test.c
PGMS=qgen q.out.o qd.dump.o q.dump #qtest
# q40.out.o
NLS=atm_ai_msg atm_ai_ie atm_loc atm_cv atm_pu atm_na atm_cond atm_ie atm_msg \
  atm_np atm_ton atm_sat atm_prs atm_scrn atm_vpa atm_poe q2931_cs atm_td \
  atm_bc atm_tc atm_stc atm_upcc q2931_proto atm_flag atm_aalp atm_fd atm_tag \
  atm_l2 atm_l3 atm_tt atm_mc atm_hl atm_imd atm_tdl atm_tni atm_nip atm_shi \
  atm_oci atm_unfm atm_ofi atm_irs atm_it atm_lit atm_lsi atm_tcs atm_css \
  atm_eqo atm_eqp atm_aap atm_asp atm_tor
SYMFILES=uni.h $(shell perl incl.pl $(CFLAGS) linux/atmsap.h)

NOLIBATMDEP=yes
include ../Rules.make

qgen:				$(OBJS)
				$(CC) $(LDFLAGS) -o qgen $(OBJS) $(LIBS)

q.out.h q.out.c q.test.c:	qgen msg.fmt default.nl
				$(CC) $(STANDARDS) -E - <msg.fmt | ./qgen

#q40.out.o:			q40.out.c q40.out.h qlib.c qlib.h
#				$(CC) $(CFLAGS) -DUNI40 -c q40.out.c
#
#q40.out.h q40.out.c q40.test.c:	qgen uni40 default.nl
#				$(CC) $(STANDARDS) -E - <uni40 | \
#				  PREFIX=q40 ./qgen

qd.out.h qd.out.c qd.dump.c:	qgen msg.fmt default.nl
				$(CC) $(STANDARDS) -E - <msg.fmt | ./qgen -D

lex.yy.c:			ql.l qgen.h y.tab.h
				$(LEX) ql.l
 
y.tab.c y.tab.h:		ql.y qgen.h
				$(YACC) -d ql.y

default.nl:			mknl.pl $(SYMFILES)
				cat $(SYMFILES) | \
				  perl mknl.pl $(NLS) >default.nl || \
				  { rm -f default.nl; echo 1; }

q.out.o:			q.out.c q.out.h qlib.c qlib.h
				$(CC) $(CFLAGS) -c $(STANDARDS) q.out.c

q.test:				q.test.c qtest.c
				$(CC) $(CFLAGS) -o q.test $(STANDARDS) q.test.c

q.dump:				qd.dump.c q.out.h qd.out.c qlib.h qlib.c
				$(CC) $(CFLAGS) -DSTANDALONE -o q.dump \
				  $(STANDARDS) qd.dump.c common.o

qd.dump.o:			qd.dump.c qd.out.c qlib.h qlib.c
				$(CC) $(CFLAGS) -c $(STANDARDS) qd.dump.c

qtest:				q.test
				./q.test
