/* first.c - Phase I, input data preprocessing */

/* Written 1995-1997 by Werner Almesberger, EPFL-LRC */

#include <stdlib.h>
#include <stdio.h>

#include "common.h"
#include "qgen.h"
#include "file.h"


int field = 0;
int group = 0;
int offset = 0;
int varlen_fields = 0;

static unsigned char mask = 0;
static int seq = 0;


#define ALLOC(field) ({ \
    if (mask & beg_mask) \
	die("bit collision (%s: 0x%02X+0x%02X)",walk->id,mask,beg_mask); \
    field->pos += offset; \
    if (next <= 8) { \
	mask |= end_mask; \
	walk->jump = 0; \
    } \
    else { \
	mask = end_mask; \
	walk->jump = (next-1) >> 3; \
    } \
    if (walk->flush && walk->size) { \
	if (!mask) die("nothing to flush (%s)",walk->id); \
	walk->jump++; \
	mask = 0; \
    } })


static void process(FIELD *start,int defines)
{
    FIELD *walk;
    TAG *scan;
    unsigned char beg_mask,end_mask,orig_mask;
    int tmp,shift,next;

    for (walk = start; walk; walk = walk->next) {
	walk->field = field; /* record it even if this isn't a real field (we
				need to know the first field number later in
				second.c and this way we don't have to search
				for it) */
	if (walk->structure) {
	    int start_field,start_group;

	    start_field = field;
	    start_group = group;
	    process(walk->my_block,walk->structure->instances > 0);
	    if (walk->structure->instances > 1) {
		walk->structure->instances = -1;
		walk->structure->first_field = start_field;
		walk->structure->fields = field-start_field;
		walk->structure->groups = group-start_group;
	    }
	    continue;
	}
	walk->seq = seq++; /* number of fields, always increases */
	if (*walk->id != '_') {
	    if (defines) to_h("#define QF_%s %d\n",walk->id,field);
	    to_test("    \"%s\",\n",walk->id);
	    field++;
	}
	if (mask && walk->brk)
	    die("mask 0x%02x at break (%s)\n",mask,walk->id);
	/* compute position and adjust masks */
	tmp = walk->size > 7 ? 0xff : (1 << walk->size)-1;
	beg_mask = tmp << walk->pos;
	tmp = 0xff & ~((1 << (8-(walk->size & 7)))-1);
	if (walk->size && !tmp) tmp = 0xff;
	next = walk->pos+walk->size;
	shift = 8-(next & 7);
	if (shift == 8) shift = 0;
	end_mask = tmp >> shift;
	if (debug)
	    printf("mask 0x%02X, beg 0x%02X, end 0x%02X, offset %d, flush %d, "
	      "%s\n",mask,beg_mask,end_mask,offset,walk->flush,walk->id);
	/* handle values */
	if (!walk->value) {
	    if (walk->var_len == -2) walk->var_len = varlen_fields++;
	    ALLOC(walk);
	    offset += walk->jump*8;
	}
	else switch (walk->value->type) {
		case vt_id:
		    ALLOC(walk);
		    to_c("    q_put(q_initial,%d,%d,%s); /* %s */\n",walk->pos,
		      walk->size,walk->value->id,walk->id);
		    offset += walk->jump*8;
		    break;
		case vt_case:
		    if (*walk->id != '_') {
			ALLOC(walk);
			offset += walk->jump*8;
#if 0 /* bad idea */
			for (scan = walk->value->tags; scan; scan = scan->next)
			    if (scan->deflt) {
				to_c("    q_put(q_initial,%d,%d,%s); "
				  "/* %s */\n",walk->pos,walk->size,
				  scan->value,walk->id);
			    }
#endif
		    }
		    /* fall through */
#if 0
		    if (*walk->id != '_') {
			ALLOC(walk);
			offset += walk->jump*8;
			orig_mask = mask;
			for (scan = walk->value->tags; scan; scan = scan->next)
			  {
			    if (scan->id && defines)
				to_h("#define QG_%s %d\n",scan->id,-group-1);
			    scan->group = group++;
			    scan->pos = walk->pos;
			    mask = orig_mask;
			    to_c("    q_put(q_initial,%d,%d,%s); /* %s */\n",
			      walk->pos,walk->size,scan->value,walk->id);
			    process(scan->block,defines);
			    if (mask) offset += 8;
			}
			break;
		    }
#endif
		    /* fall through */
		case vt_multi:
		    orig_mask = mask;
		    for (scan = walk->value->tags; scan; scan = scan->next) {
			if (scan->id && defines)
			    to_h("#define QG_%s %d\n",scan->id,-group-1);
			scan->group = group++;
			scan->pos = walk->pos;
			if (debug)
			    printf("| %s/%s (walk->flush = %d)\n",walk->id,
			      scan->value,walk->flush);
			if (*walk->id != '_') mask = orig_mask;
			else {
			    ALLOC(scan);
			    if (debug)
				printf("| jump = %d (mask 0x%02x)\n",
				  walk->jump,mask);
			    offset += walk->jump*8;
			    to_c("    q_put(q_initial,%d,%d,%s); /* %s */\n",
			      scan->pos,walk->size,scan->value,walk->id);
			}
			process(scan->block,defines);
			if (*walk->id != '_' && mask) die("EF129");
		    }
		    break;
		case vt_length:
		    ALLOC(walk);
		    offset += walk->jump*8;
		    process(walk->value->block,defines);
		    break;
		default:
		    abort();
	    }
    }
}


void first(FIELD *def)
{
    def->group = group++;
    to_h("/*\n * Identifiers to access numbers of fields and of named\n");
    to_h(" * groups (unnamed groups don't need that). Field numbers are\n");
    to_h(" * zero-based. Group numbers are negative and -1-based.\n */\n\n");
    to_c("static unsigned char q_initial[Q_DATA_BYTES];\n\n");
    to_c("/*\n * Initialization of constant data. Could also do this in\n");
    to_c(" * the translator and output the resulting byte stream.\n */\n\n");
    to_c("static void q_init_global(void)\n{\n");
    to_c("    memset(q_initial,0,sizeof(q_initial));\n");
    to_test("static const char *fields[] = {\n");
    process(def,1);
    to_test("    NULL\n};\n\n");
    to_c("}\n\n");
    if (mask) offset += 8;
    to_h("\n/*\n * Sizes of various tables which are allocated at run-time.\n");
    to_h(" */\n");
    to_h("\n#define Q_DATA_BYTES %d\n",offset/8);
    to_h("#define Q_GROUPS %d\n",group);
    to_h("#define Q_FIELDS %d\n",field);
    to_h("#define Q_VARLEN_FIELDS %d\n\n",varlen_fields);
}
