source=cf-parse.tab.c cf-lex.c conf.c
root-rel=../

include ../Rules

conf-src=$(srcdir)/conf
conf-fragments=$(conf-src)/confbase.Y @CONFS@ $(addsuffix /config.Y,$(static-dir-paths))

ifdef DEBUG
BISON_DEBUG=-t
#FLEX_DEBUG=-d
endif

cf-parse.tab.h: cf-parse.tab.c

cf-parse.tab.c: cf-parse.y
	$(BISON) -bcf-parse -dv -pcf_ $(BISON_DEBUG) cf-parse.y

cf-parse.y: $(conf-fragments) $(conf-src)/gen_parser.m4
	$(M4) -P $(conf-src)/gen_parser.m4 $(conf-fragments) >cf-parse.y

keywords.h: $(conf-fragments) $(conf-src)/gen_keywords.m4
	$(M4) -P $(conf-src)/gen_keywords.m4 $(conf-fragments) >keywords.h

commands.h: $(conf-fragments) $(conf-src)/gen_commands.m4 $(srcdir)/client/cmds.m4
	$(M4) -P $(conf-src)/gen_commands.m4 $(srcdir)/client/cmds.m4 $(conf-fragments) | sort >commands.h

cf-lex.c: cf-lex.l
	$(FLEX) $(FLEX_DEBUG) -s -B -8 -ocf-lex.c -Pcf_ cf-lex.l

depend: keywords.h commands.h cf-parse.tab.c cf-lex.c
