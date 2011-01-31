/* qgen.c - constructor/parser generator for Q.2931-like data structures */
 
/* Written 1995-2000 by Werner Almesberger, EPFL-LRC/ICA */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "common.h"
#include "file.h"
#include "qgen.h"


extern int yyparse(void);

extern FIELD *def;
extern int group,field,offset,varlen_fields;
extern int constr_size,parser_size;
extern int sym_tables,symbols;


int debug = 0;
int dump = 0;


int main(int argc,char **argv)
{
    const char *prefix;

    if (argc == 2 && !strcmp(argv[1],"-d")) debug = 1;
    if (argc == 2 && !strcmp(argv[1],"-D")) dump = 1;
    prefix = getenv("PREFIX");
    if (!prefix) prefix = dump ? "qd" : "q";
    open_files(prefix);
    to_h("/* THIS IS A MACHINE-GENERATED FILE. DO NOT EDIT ! */\n\n");
    to_c("/* THIS IS A MACHINE-GENERATED FILE. DO NOT EDIT ! */\n\n");
    to_c("#if HAVE_CONFIG_H\n");
    to_c("#include <config.h>\n");
    to_c("#endif\n\n");
    to_test("/* THIS IS A MACHINE-GENERATED FILE. DO NOT EDIT ! */\n\n");
    to_test("#if HAVE_CONFIG_H\n");
    to_test("#include <config.h>\n");
    to_test("#endif\n\n");
    if (dump) {
	to_dump("/* THIS IS A MACHINE-GENERATED FILE. DO NOT EDIT ! */\n\n");
	to_dump("#if HAVE_CONFIG_H\n");
	to_dump("#include <config.h>\n");
	to_dump("#endif\n\n");
    }
    to_c("/* (optional) user includes go here */\n\n");
    to_test("/* (optional) user includes go here */\n\n");
    if (dump) to_dump("/* (optional) user includes go here */\n\n");
    if (yyparse()) return 1;
    to_test("\n#ifndef NULL\n#define NULL ((void *) 0)\n#endif\n\n");
    if (dump) to_dump("\n#ifndef NULL\n#define NULL ((void *) 0)\n#endif\n\n");
    to_h("#ifndef Q_OUT_H\n#define Q_OUT_H\n\n");
    to_c("\n#include <stdlib.h>\n#include <stdio.h>\n");
    to_c("#include <string.h>\n#include <sys/types.h>\n\n");
    to_c("#include \"op.h\"\n");
    to_c("#include \"%s.out.h\"\n",prefix);
    to_c("#include \"qlib.h\"\n\n");
    to_c("\n\nstatic void q_put(unsigned char *table,int pos,int size,"
      "unsigned long value);\n\n");
    first(def);
    second(def);
    third(def);
    to_h("#endif\n");
    to_c("\n/*\n * Sorry, this is necessary ...\n */\n\n");
    to_c("#include \"qlib.c\"\n");
    to_test("\n/*\n * Sorry, this is necessary ...\n */\n\n");
    to_test("#include \"qtest.c\"\n");
    if (dump) {
	to_dump("\n/*\n * Sorry, this is necessary ...\n */\n\n");
	to_dump("#define DUMP_MODE\n\n");
	to_dump("#include \"%s.out.c\"\n",prefix);
    }
    close_files();
    fprintf(stderr,"  %d groups, %d fields (%d var-len), construction area is "
      "%d bytes,\n",group,field,varlen_fields,(offset+7)/8);
    fprintf(stderr,"  %d words in constructor, %d words in parser",
      constr_size,parser_size);
    if (!dump) fprintf(stderr,".\n");
    else fprintf(stderr,",\n  %d symbolic names in %d tables.\n",symbols,
	  sym_tables);
    return 0;
}
