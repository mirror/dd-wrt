/* third.c - Phase III, code generation */

/* Written 1995-1997 by Werner Almesberger, EPFL-LRC */

#include <stdlib.h>
#include <stdio.h>

#include "common.h"
#include "qgen.h"
#include "file.h"


typedef struct _break {
    int pc;
    struct _break *next;
} BREAK;


int constr_size,parser_size;

static BREAK *brks = NULL;


static void construct(FIELD *start)
{
    FIELD *walk;
    TAG *scan;
    int patch_here;

    patch_here = 0; /* for gcc */
    for (walk = start; walk; walk = walk->next) {
	if (walk->structure) {
	    construct(walk->my_block);
	    continue;
	}
	if (!walk->value)
	    if (walk->var_len == -1)
		code("%s%d%d%d/* %s */\n","OP_COPY",walk->jump,walk->pos,
		  walk->size,walk->id);
	    else code("%s%d%d%d/* %s */\n","OP_COPYVAR",walk->var_len,
		  walk->pos,walk->size/8,walk->id);
	else switch (walk->value->type) {
		case vt_id:
		    code("%s%d%d%d/* %s */\n","OP_COPY",walk->jump,walk->pos,
		      walk->size,walk->id);
		    break;
		case vt_case:
		    if (*walk->id != '_')
			code("%s%d%d%d/* %s */\n","OP_COPY",walk->jump,
			  walk->pos,walk->size,walk->id);
		    for (scan = walk->value->tags; scan; scan = scan->next) {
			if (debug)
			    printf("C %s/%s(%d): %d\n",walk->id,scan->value,
			      scan->group,scan->deflt);
			if (!scan->deflt) {
			    code("%s%d%s\n","OP_IFGROUP",scan->group,"?");
			    patch_here = pc-1;
			}
			if (*walk->id == '_')
			    code("%s%d%d%d/* %s */\n","OP_COPY",walk->jump,
			      scan->pos,walk->size,scan->value);
			construct(scan->block);
			if (scan->next) {
			    code("%s%s\n","OP_JUMP","?");
			    scan->patch = pc-1;
			}
			if (!scan->deflt) patch(patch_here,pc-patch_here-1);
		    }
		    for (scan = walk->value->tags; scan && scan->next;
		      scan = scan->next)
			patch(scan->patch,pc-scan->patch-1);
		    break;
		case vt_multi:
		    for (scan = walk->value->tags; scan; scan = scan->next) {
			if (debug)
			    printf("M %s/%s(%d)\n",walk->id,scan->value,
			      scan->group);
			code("%s%d%s\n","OP_IFGROUP",scan->group,"?");
			scan->patch = pc-1;
			code("%s%d%d%d/* %s */\n","OP_COPY",walk->jump,
			  scan->pos,walk->size,scan->value);
			construct(scan->block);
			patch(scan->patch,pc-scan->patch-1);
		    }
		    break;
		case vt_length:
		    code("%s%d%d%d/* %s */\n","OP_BEGIN_LEN",walk->jump,
                      walk->pos,walk->size,walk->id);
		    construct(walk->value->block);
		    code("%s\n","OP_END_LEN");
		    break;
		default:
		    abort();
	    }
    }
}


static void parser(FIELD *start,int level,int group)
{
    FIELD *walk;
    TAG *scan;
    VALUE_LIST *tag;
    int count,patch_here;
    BREAK *brk,*tmp_brks,*next;

    patch_here = 0; /* for gcc */
    for (walk = start; walk; walk = walk->next) {
	if (walk->structure) {
	    parser(walk->my_block,level,group);
	    continue;
	}
	if (walk->brk) {
	    code("%s%s\n","OP_IFEND","?");
	    brk = alloc_t(BREAK);
	    brk->pc = pc-1;
	    brk->next = brks;
	    brks = brk;
	}
	if (dump) {
	    if ((!walk->value || walk->value->type != vt_multi) && walk->size)
		code("%s%d\n","OP_DUMP",walk->seq);
	    if (walk->value || walk->name_list)
		to_dump("    { %d, dump_sym_%d, \"%s\" },\n",level,
		  walk->name_list ? walk->name_list->id : walk->seq,walk->id);
	    else to_dump("    { %d, NULL, \"%s\" },\n",level,walk->id);
        }
	if (!walk->value) {
	    if (walk->var_len != -1)
		code("%s%d%d%d/* %s */\n","OP_COPYVAR",walk->var_len,walk->pos,
		  walk->size/8,walk->id);
	    else if (*walk->id != '_' || walk->jump || dump)
		    code("%s%d%d%d/* %s */\n","OP_COPY",walk->jump,walk->pos,
		      walk->size,walk->id);
	}
	else switch (walk->value->type) {
		case vt_id:
		    if (*walk->id != '_' || walk->jump || dump)
			code("%s%d%d%d/* %s */\n","OP_COPY",walk->jump,
			  walk->pos,walk->size,walk->id);
		    break;
		case vt_multi:
		    code("%s%s/* %s */\n","OP_IFEND","?",walk->id);
		    walk->patch = pc-1;
		    if (dump) code("%s%d\n","OP_DUMP",walk->seq);
		    /* fall through */
		case vt_case:
		    if (*walk->id != '_' || dump)
			code("%s%d%d%d/* %s */\n","OP_COPY",0,walk->pos,
			  walk->size,walk->id); /* don't move */
		    count = 0;
		    for (scan = walk->value->tags; scan; scan = scan->next) {
			count++;
			for (tag = scan->more; tag; tag = tag->next) count++;
		    }
		    code("%s%d%d%d%d/* %s */\n","OP_CASE",walk->jump,
		      walk->pos,walk->size,count,walk->id);
		    for (scan = walk->value->tags; scan; scan = scan->next) {
			code("%s%d%s\n",scan->deflt ? "-1" : scan->value,
			  scan->group,"?");
			scan->patch = pc-1;
			for (tag = scan->more; tag; tag = tag->next) {
			    code("%s%d%s\n",tag->value,scan->group,"?");
			    tag->patch = pc-1;
			}
		    }
		    for (scan = walk->value->tags; scan; scan = scan->next) {
			patch(scan->patch,pc-scan->patch-1);
			for (tag = scan->more; tag; tag = tag->next)
			    patch(tag->patch,pc-tag->patch-1);
			if (!scan->block && scan->abort_id)
			    code("%s%s\n","OP_ABORT",scan->abort_id);
			parser(scan->block,level+1,scan->group);
			if (scan->next) {
			    code("%s%s\n","OP_JUMP","?");
			    scan->patch = pc-1;
			}
		    }
		    for (scan = walk->value->tags; scan && scan->next;
		      scan = scan->next)
			patch(scan->patch,pc-scan->patch-1);
		    if (walk->value->type == vt_multi) {
			code("%s%d\n","OP_JUMP",walk->patch-pc-3);
			patch(walk->patch,pc-walk->patch-1);
		    }
		    break;
		case vt_length:
		    code("%s%d%d%d/* %s */\n","OP_BEGIN_LEN",walk->jump,
                      walk->pos,walk->size,walk->id);
		    if (walk->value->recovery) {
			code("%s%s%d%d\n","OP_BEGIN_REC",walk->value->recovery,
			    group,"?");
			patch_here = pc-1;
		    }
		    tmp_brks = brks;
		    if (!walk->value->block && walk->value->abort_id) 
			code("%s%s\n","OP_ABORT",walk->value->abort_id);
		    parser(walk->value->block,level+1,group);
		    if (walk->value->recovery) {
			code("%s\n","OP_END_REC");
			patch(patch_here,pc);
		    }
		    for (brk = brks; brk; brk = next) {
			next = brk->next;
			patch(brk->pc,pc-brk->pc-1);
			free(brk);
		    }
		    brks = tmp_brks;
		    code("%s /* %s */\n","OP_END_LEN",walk->id);
		    break;
		default:
		    abort();
	    }
    }
}


void third(FIELD *def)
{
    if (dump) constr_size = 0;
    else {
	to_c("\n/*\n");
	to_c(" * \"Microcode\" used to construct messages. It copies all\n");
	to_c(" * fields from the construction area to the resulting message.");
	to_c("\n */\n\n");
	to_c("static int construct[] = {\n");
	begin_code();
	construct(def);
	constr_size = end_code()+1;
	to_c("    OP_END\n};\n\n");
    }
    to_c("\n/*\n * \"Microcode\" used to parse messages. It detects the\n");
    to_c(" * presence of fields and copies them from the message to the\n");
    to_c(" * construction area.\n */\n\n");
    to_c("static int parse[] = {\n");
    if (dump) {
	to_dump("typedef struct {\n    int level;\n    const SYM_NAME *sym;\n");
	to_dump("    const char *name;\n");
	to_dump("} DUMP_FIELD;\n\n");
	to_dump("static DUMP_FIELD dump_fields[] = {\n");
    }
    begin_code();
    parser(def,0,0);
    parser_size = end_code()+1;
    to_c("    OP_END\n};\n\n");
    if (dump) to_dump("};\n\n");
}
