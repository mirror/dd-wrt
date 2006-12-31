#
# Makefile
# $Id: Makefile,v 1.26 1995/11/15 08:38:51 carnil Exp $

#
# Compilation options
#

CFLAGS = -ansi -pedantic -Wall -Wshadow -Wpointer-arith -Wcast-qual -Wcast-align -Waggregate-return -Wstrict-prototypes -Wmissing-prototypes -Wnested-externs

#
#
# The rules
#
#
SYSPGMS =	les bus lecs
MAN8 =		les.8 lecs.8 bus.8
LESBUSOBJS =	mem.o load.o units.o load_lex.o timers.o dump.o atm.o \
		events.o lane.o
LESOBJS =	$(LESBUSOBJS) packet.o connect.o db.o
BUSOBJS =       $(LESBUSOBJS) connect_bus.o
LECSOBJS =	lecs_db.o lecs_load.o lecs.o ldb.o mem_lecs.o atm_lecs.o
OBJS =		$(LESOBJS) $(BUSOBJS) $(LECSOBJS)

CLEAN = clean_lane
include ../Rules.make

load_lex.c : load_lex.l
	$(LEX) -oload_lex.c load_lex.l

lecs_db.c: lecs_db.l
	$(LEX) -olecs_db.c lecs_db.l 

les:	$(LESOBJS)
	@echo "Linking $@"
	@$(CC) $(LDFLAGS) -o $@ $(LESOBJS)

bus:	$(BUSOBJS)
	@echo "Linking $@"
	@$(CC) $(LDFLAGS) -o $@ $(BUSOBJS)

lecs:	$(LECSOBJS)
	@echo "Linking $@"
	$(CC) $(LDFLAGS) -o $@ $(LECSOBJS) $(LDLIBS)

clean_lane:
	rm -f $(SYSPGMS) *.o *.d *~ .*~ core *.bak *.tar* *.errs load_lex.c \
	lecs_db.c *~
