/* second.c - Phase II, table generation */

/* Written 1995-2000 by Werner Almesberger, EPFL-LRC/ICA */

#include <stdlib.h>

#include "common.h"
#include "qgen.h"
#include "file.h"


int sym_tables = 0,symbols = 0;
static int unique;


static void dump_required(FIELD *start,FIELD *leader,int group)
{
    FIELD *walk;

    for (walk = start; walk; walk = walk->next) {
	if (walk->structure) {
	    dump_required(walk->my_block,leader,group);
	    continue;
	}
	if (walk->brk) break;
	if (!walk->value) {
	    if (!leader->has_required) {
		to_c("static int required_%d[] = {\n",group);
		leader->has_required = 1;
	    }
	    to_c("    %d, /* %s */\n",walk->field,walk->id);
	}
	else if (walk->value->type == vt_length)
		dump_required(walk->value->block,leader,group);
    }
    if (leader == start && leader && leader->has_required)
	to_c("    -1\n};\n\n");
}


static void find_required(FIELD *start)
{
    FIELD *walk;
    TAG *scan;

    for (walk = start; walk; walk = walk->next) {
	if (walk->structure) {
	    find_required(walk->my_block);
	    continue;
	}
	if (walk->value) 
	    switch  (walk->value->type) {
		case vt_id:
		    break;
		case vt_case:
		case vt_multi:
		    for (scan = walk->value->tags; scan; scan = scan->next)
			if (scan->block) {
			    scan->block->has_required = 0;
			    dump_required(scan->block,scan->block,scan->group);
			    find_required(scan->block);
			}
		    break;
		case vt_length:
		    find_required(walk->value->block);
		    break;
		default:
		    abort();
	    }
    }
}


static void groups(FIELD *start,int group)
{
    FIELD *walk;
    TAG *scan;

    for (walk = start; walk; walk = walk->next) {
	if (walk->structure) {
	    groups(walk->my_block,group);
	    continue;
	}
	if (walk->value)
	    switch  (walk->value->type) {
		case vt_id:
		    break;
		case vt_case:
		case vt_multi:
		    for (scan = walk->value->tags; scan; scan = scan->next) {
			int start_field,length,offset;

			if (scan->block && scan->block->structure &&
			  scan->block->structure->instances < 0) {
			    start_field = scan->block->structure->first_field;
			    length = scan->block->structure->fields;
			    offset = scan->block->my_block->field-
			      scan->block->structure->first_field;
			}
			else {
			    start_field = -1;
			    length = offset = 0;
			}
			if (scan->id) to_test("    \"%s\",\n",scan->id);
			if (scan->block && scan->block->has_required)
			    if (scan->block->id)
				to_c("    { %d, required_%d, %d, %d, %d }, "
				  "/* %s */\n",group,scan->group,start_field,
				  length,offset,scan->block->id);
			    else to_c("    { %d, required_%d, %d, %d, %d },\n",
				  group,scan->group,start_field,length,offset);
			else to_c("    { %d, NULL, %d, %d, %d },\n",group,
			      start_field,length,offset);
			groups(scan->block,scan->group);
		    }
		    break;
		case vt_length:
		    groups(walk->value->block,group);
		    break;
		default:
		    abort();
	    }
    }
}


static void values(FIELD *start)
{
    FIELD *walk;
    TAG *scan;
    VALUE_LIST *tag;

    for (walk = start; walk; walk = walk->next) {
	if (walk->structure) {
	    values(walk->my_block);
	}
	if (walk->value)
	    switch (walk->value->type) {
		case vt_id:
		    break;
		case vt_case:
		    if (*walk->id != '_') {
			to_c("static int values_%d[] = { /* %s */\n",
			  walk->field,walk->id);
			for (scan = walk->value->tags; scan; scan = scan->next)
			  {
			    to_c("    %s, %d,\n",scan->value,scan->group);
			    for (tag = scan->more; tag; tag = tag->next)
				to_c("    %s, %d,\n",tag->value,scan->group);
			    if (scan->deflt) to_c("    -2, %d,\n",scan->group);
				/* could also skip while entry, but maybe we'll
				   want to use the tags later */
			}
			to_c("    -1, -1\n};\n\n");
		    }
		    /* fall through */
		case vt_multi:
		    for (scan = walk->value->tags; scan; scan = scan->next)
			values(scan->block);
		    break;
		case vt_length:
		    values(walk->value->block);
		    break;
		default:
		    abort();
	    }
    }
}


static void fields(FIELD *start,int group)
{
    FIELD *walk;
    TAG *scan;
    VALUE_LIST *tag;

    for (walk = start; walk; walk = walk->next) {
	if (walk->structure) {
	    fields(walk->my_block,group);
	    continue;
	}
	if (*walk->id != '_') {
	    if (walk->value && walk->value->type == vt_case)
		to_c("    { %d, %d, %d, values_%d, %d }, /* %s */\n",group,
		  walk->pos,walk->size,walk->field,walk->var_len,walk->id);
	    else to_c("    { %d, %d, %d, NULL, %d }, /* %s */\n",group,
		  walk->pos,walk->size,walk->var_len,walk->id);
	}
	if (walk->value)
	    switch  (walk->value->type) {
		case vt_id:
		    break;
		case vt_case:
		case vt_multi:
		    for (scan = walk->value->tags; scan; scan = scan->next)
			fields(scan->block,scan->group);
		    to_test("static int unique_%d[] = { /* %s */\n",unique++,
		      walk->id);
		    for (scan = walk->value->tags; scan; scan = scan->next) {
			to_test("    %s,\n",scan->value);
			for (tag = scan->more; tag; tag = tag->next)
			    to_test("    %s,\n",tag->value);
		    }
		    to_test("    -1\n};\n\n");
		    break;
		case vt_length:
		    fields(walk->value->block,group);
		    break;
		default:
		    abort();
	    }
    }
}


static void symbolic_names(FIELD *start)
{
    FIELD *walk;
    NAME *name;
    TAG *scan;
    VALUE_LIST *tag;

    for (walk = start; walk; walk = walk->next) {
	if (walk->structure) {
	    symbolic_names(walk->my_block);
	    continue;
	}
	if (walk->name_list ? walk->name_list->id == -1 : !!walk->value) {
	    if (walk->name_list) {
		to_dump("static SYM_NAME dump_sym_%d[] = { /* %s */\n",
		  walk->seq,walk->name_list->list_name);
		sym_tables++;
		walk->name_list->id = walk->seq;
		for (name = walk->name_list->list; name; name = name->next) {
		    to_dump("    { %s, \"%s\" },\n",name->value,name->name);
		    symbols++;
		}
	    }
	    else {
		to_dump("static SYM_NAME dump_sym_%d[] = {\n",walk->seq);
		sym_tables++;
		switch (walk->value->type) {
		    case vt_id:
			to_dump("    { %s, \"%s\" },\n",walk->value->id,
			  walk->value->id);
			symbols++;
			break;
		    case vt_case:
		    case vt_multi:
			for (scan = walk->value->tags; scan; scan = scan->next)
			  {
			    to_dump("    { %s, \"%s\" },\n",scan->value,
			      scan->value);
			    symbols++;
			    for (tag = scan->more; tag; tag = tag->next) {
				to_dump("    { %s, \"%s\" },\n",tag->value,
				  tag->value);
				symbols++;
			    }
			}
			break;
		    case vt_length:
			break;
		    default:
			abort();
		}
	    }
	    to_dump("    { 0, NULL }\n};\n\n");
	}
	if (walk->value)
	    switch (walk->value->type) {
		case vt_id:
		    break;
		case vt_case:
		case vt_multi:
		    for (scan = walk->value->tags; scan; scan = scan->next)
			symbolic_names(scan->block);
		    break;
		case vt_length:
		    symbolic_names(walk->value->block);
		    break;
		default:
		    abort();
	    }
    }
}


void second(FIELD *def)
{
    int i;

    def->has_required = 0;
    to_c("\n/*\n");
    to_c(" * If a group contains required fields, these are listed in the\n");
    to_c(" * following arrays. Each list ends with -1. The variable names\n");
    to_c(" * end with the group number.\n */\n\n");
    dump_required(def,def,0);
    find_required(def);
    to_c("\n/*\n * Various information about groups.\n */\n\n");
    to_c("typedef struct {\n    int parent;\n    int *required;\n");
    to_c("    int start;\n    int length;\n    int offset;\n} GROUP;\n\n");
    to_c("static GROUP groups[] = {\n");
    if (def->has_required) to_c("    { -1, required_0 },\n");
    else to_c("    { -1, NULL },\n");
    to_test("static const char *groups[] = {\n");
    groups(def,0);
    to_test("    NULL\n};\n\n");
    to_c("};\n\n\n");
    to_c("/*\n * Named case selectors only have a limited set of valid\n");
    to_c(" * values. They are listed in the following arrays, each followed\n");
    to_c(" * by the number of the group it enables.\n */\n\n");
    values(def);
    to_c("\n/*\n * Various information about fields.\n */\n\n");
    to_c("typedef struct {\n    int parent;\n    int pos,size;\n");
    to_c("    int *values;\n    int actual;\n} FIELD;\n\n");
    to_c("static FIELD fields[] = {\n");
    fields(def,0);
    to_c("};\n\n");
    to_test("static int *unique[] = {\n");
    for (i = 0; i < unique; i++) to_test("    unique_%d,\n",i);
    to_test("    NULL\n};\n\n");
    if (dump) {
	to_dump("typedef struct {\n    unsigned long value;\n");
	to_dump("    const char *name;\n} SYM_NAME;\n\n");
	symbolic_names(def);
    }
}
