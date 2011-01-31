%{
/* ql.y - Q.2931 data structures description language */

/* Written 1995-1997 by Werner Almesberger, EPFL-LRC */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>

#include "common.h"
#include "qgen.h"
#include "file.h"


#define MAX_TOKEN 256
#define DEFAULT_NAMELIST_FILE "default.nl"


FIELD *def = NULL;
static STRUCTURE *structures = NULL;
static const char *abort_id; /* indicates abort flag */

void yyerror(const char *s);
int yylex(void);

static NAME_LIST *get_name_list(const char *name)
{
    static NAME_LIST *name_lists = NULL;
    FILE *file;
    NAME_LIST *list;
    NAME *last,*this;
    char line[MAX_TOKEN+1];
    char path[PATH_MAX+1];
    char *start,*here,*walk;
    int searching,found;

    for (list = name_lists; list; list = list->next)
	if (list->list_name == name) return list;
    sprintf(path,"%s.nl",name);
    if (!(file = fopen(path,"r")) && !(file = fopen(strcpy(path,
      DEFAULT_NAMELIST_FILE),"r"))) yyerror("can't open list file");
    list = alloc_t(NAME_LIST);
    list->list_name = name;
    list->list = last = NULL;
    list->id = -1;
    list->next = name_lists;
    name_lists = list;
    searching = 1;
    found = 0;
    while (fgets(line,MAX_TOKEN,file)) {
	for (start = line; *start && isspace(*start); start++);
	if (!*start || *start == '#') continue;
	if ((here = strchr(start,'\n'))) *here = 0;
	for (walk = strchr(start,0)-1; walk > start && isspace(*walk); walk--)
	    *walk = 0;
	if (*start == ':') {
	    if (!(searching = strcmp(start+1,name)))
	    {
		if (found) yyerror("multiple entries");
		else found = 1;
	    }
	    continue;
	}
	if (searching) continue;
	if (!(here = strchr(start,'='))) yyerror("invalid name list");
	*here++ = 0;
	for (walk = here-2; walk > start && isspace(*walk); walk--)
	    *walk = 0;
	while (*here && isspace(*here)) here++;
	this = alloc_t(NAME);
	this->value = stralloc(start);
	this->name = stralloc(here);
	this->next = NULL;
	if (last) last->next = this;
	else list->list = this;
	last = this;
    }
    (void) fclose(file);
    if (!found) yyerror("no symbol list entry found");
    return list;
}


static FIELD *copy_block(FIELD *orig_field)
{
    FIELD *copy,**new_field;

    copy = NULL;
    new_field = &copy;
    while (orig_field) {
	*new_field = alloc_t(FIELD);
	**new_field = *orig_field;
	if (orig_field->value) {
	    (*new_field)->value = alloc_t(VALUE);
	    *(*new_field)->value = *orig_field->value;
	    switch (orig_field->value->type) {
		case vt_length:
		    (*new_field)->value->block =
		      copy_block(orig_field->value->block);
		    break;
		case vt_case:
		case vt_multi:
		    {
			TAG *orig_tag,**new_tag;

			new_tag = &(*new_field)->value->tags;
			for (orig_tag = orig_field->value->tags; orig_tag;
			  orig_tag = orig_tag->next) {
			    VALUE_LIST *orig_value,**new_value;

			    *new_tag = alloc_t(TAG);
			    **new_tag = *orig_tag;
			    new_value = &(*new_tag)->more;
			    for (orig_value = orig_tag->more; orig_value;
			      orig_value = orig_value->next) {
				*new_value = alloc_t(VALUE_LIST);
				**new_value = *orig_value;
				new_value = &(*new_value)->next;
			    }
			    (*new_tag)->block = copy_block(orig_tag->block);
			    new_tag = &(*new_tag)->next;
			}
		    }
	    }
	}
	if (orig_field->structure)
	    yyerror("sorry, can't handle nested structures");
	new_field = &(*new_field)->next;
	orig_field = orig_field->next;
    }
    return copy;
}


%}

%union {
    const char *str;
    int num;
    FIELD *field;
    VALUE *value;
    VALUE_LIST *list;
    TAG *tag;
    NAME_LIST *nlist;
};

%token		TOK_BREAK TOK_CASE TOK_DEF TOK_DEFAULT TOK_LENGTH TOK_MULTI
%token		TOK_RECOVER TOK_ABORT
%token <str>	TOK_ID TOK_INCLUDE TOK_STRING

%type <field>	rep_block block fields field field_cont
%type <num>	opt_break opt_pos decimal opt_more
%type <value>	opt_val value
%type <tag>	tags rep_tags
%type <list>	list
%type <str>	opt_id opt_recover
%type <nlist>	opt_name_list

%%

all:
    includes structures block
	{
	    STRUCTURE *walk;

	    def = $3;
	    for (walk = structures; walk; walk = walk->next)
		if (!walk->instances)
		    fprintf(stderr,"unused structure: %s\n",walk->id);
	}
    ;

includes:
    | TOK_INCLUDE includes
	{
	    to_c("#%s\n",$1);
	    to_test("#%s\n",$1);
	    if (dump) to_dump("#%s\n",$1);
	}
    ;

structures:
    | structures structure
    ;

structure:
    TOK_DEF TOK_ID '=' block
	{
	    STRUCTURE *n;

	    n = alloc_t(STRUCTURE);
	    n->id = $2;
	    n->block = $4;
	    n->instances = 0;
	    n->next = structures;
	    structures = n;
	}
    ;

rep_block:
	{
	    abort_id = NULL;
	}
      block
	{
	    $$ = $2;
	}
    ;

block:
    TOK_ID
	{
	    STRUCTURE *walk;

	    for (walk = structures; walk; walk = walk->next)
		if (walk->id == $1) break;
	    if (!walk) yyerror("no such structure");
	    walk->instances++;
	    $$ = alloc_t(FIELD);
	    $$->id = NULL;
	    $$->name_list = NULL;
	    $$->value = NULL;
	    $$->brk = 0;
	    $$->structure = walk;
	    $$->my_block = copy_block(walk->block);
	    $$->next = NULL;
	    abort_id = NULL;
	}
    | '{' fields '}'
	{
	    $$ = $2;
	    abort_id = NULL;
	}
    | TOK_ABORT TOK_ID
	{
	    $$ = NULL;
	    abort_id = $2;
	}
    ;

fields:
	{
	    $$ = NULL;
	}
    | field fields
	{
	    $$ = $1;
	    $1->next = $2;
	}
    ;

field:
    opt_break TOK_ID opt_name_list '<' field_cont
	{
	    TAG *walk;

	    $$ = $5;
	    $$->name_list = $3;
	    $$->brk = $1;
	    $$->id = $2;
	    if ($$->var_len == -2) {
		if (*$$->id == '_') yyerror("var-len field must be named");
	    }
	    else if (*$$->id == '_' && !$$->value)
		    yyerror("unnamed fields must have value");
	    if (*$$->id == '_' && $$->value && $$->value->type == vt_case)
		for (walk = $$->value->tags; walk; walk = walk->next)
		    if (walk->more)
			yyerror("value list only allowed in named case "
			  "selections");
	    if (*$$->id != '_' && $$->value && $$->value->type == vt_multi)
		yyerror("multi selectors must be unnamed");
	}
    ;

opt_break:
	{
	    $$ = 0;
	}
    | TOK_BREAK
	{
	    $$ = 1;
	}
    ;

field_cont:
     '-' decimal '>'
	{
	    $$ = alloc_t(FIELD);
	    $$->size = $2;
	    $$->var_len = -2; /* hack */
	    if ($2 & 7) yyerror("var-len field must have integral size");
	    $$->pos = 0;
	    $$->flush = 1;
	    $$->value = NULL;
	    $$->structure = NULL;
	    $$->next = NULL;
	}
     | decimal opt_pos opt_more '>' opt_val
	{
	    $$ = alloc_t(FIELD);
	    $$->size = $1;
	    $$->var_len = -1;
	    $$->pos = $2;
	    $$->flush = !$3;
	    if ($$->pos == -1)
	    {
		if ($$->size & 7)
		    yyerror("position required for small fields");
		else $$->pos = 0;
	    }
	    $$->value = $5;
	    $$->structure = NULL;
	    $$->next = NULL;
	}
    ;

opt_pos:
	{
	    $$ = -1;
	}
    | '@' decimal
	{
	    $$ = $2-1;
	    if ($$ < 0 || $$ > 7) yyerror("invalid position");
	}
    ;

decimal:
    TOK_ID
	{
	    char *end;

	    $$ = strtoul($1,&end,10);
	    if (*end) yyerror("no a decimal number");
	}
    ;

opt_more:
	{
	    $$ = 0;
	}
    | ',' TOK_ID
	{
	    if (strcmp($2,"more")) yyerror("\"more\" expected");
	    $$ = 1;
	}
    ;

opt_val:
	{
	    $$ = NULL;
	}
    | '=' value
	{
	    $$ = $2;
	}
    ;

value:
    TOK_ID
	{
	    $$ = alloc_t(VALUE);
	    $$->type = vt_id;
	    $$->id = $1;
	}
    | TOK_CASE '{' tags '}'
	{
	    $$ = alloc_t(VALUE);
	    $$->type = vt_case;
	    $$->id = NULL;
	    $$->tags = $3;
	}
    | TOK_MULTI '{' rep_tags '}'
	{
	    $$ = alloc_t(VALUE);
	    $$->type = vt_multi;
	    $$->tags = $3;
	}
    | opt_recover TOK_LENGTH block
	{
	    $$ = alloc_t(VALUE);
	    $$->type = vt_length;
	    $$->recovery = $1;
	    $$->block = $3;
	    $$->abort_id = abort_id;
	}
    ;

opt_recover:
	{
	    $$ = NULL;
	}
    | TOK_RECOVER TOK_ID
	{
	    $$ = $2;
	}
    ;

opt_name_list:
	{
	    $$ = NULL;
	}
    | TOK_STRING
	{
	    $$ = get_name_list($1);
	}
    ;

tags:
	{
	    $$ = NULL;
	}
    | TOK_DEFAULT TOK_ID opt_id list block
	{
	    $$ = alloc_t(TAG);
	    $$->deflt = 1;
	    if ($3) {
		$$->id = $2;
		$$->value = $3;
	    }
	    else {
		$$->id = NULL;
		$$->value = $2;
	    }
	    $$->more = $4;
	    $$->block = $5;
	    $$->next = NULL;
	    $$->abort_id = abort_id;
	}
    | TOK_ID opt_id list block
	{
	    $<tag>$ = alloc_t(TAG);
	    $<tag>$->abort_id = abort_id;
	}
	  tags
	{
	    $$ = $<tag>5;
	    $$->deflt = 0;
	    if ($2) {
		$$->id = $1;
		$$->value = $2;
	    }
	    else {
		$$->id = NULL;
		$$->value = $1;
	    }
	    $$->more = $3;
	    $$->block = $4;
	    $$->next = $6;
	}
    ;

rep_tags:
	{
	    $$ = NULL;
	}
    | TOK_DEFAULT TOK_ID opt_id list rep_block
	{
	    $$ = alloc_t(TAG);
	    $$->deflt = 1;
	    if ($3) {
		$$->id = $2;
		$$->value = $3;
	    }
	    else {
		$$->id = NULL;
		$$->value = $2;
	    }
	    $$->more = $4;
	    $$->block = $5;
	    $$->next = NULL;
	}
    | TOK_ID opt_id list rep_block
	{
	    $<tag>$ = alloc_t(TAG);
	    $<tag>$->abort_id = abort_id;
	}
	    rep_tags
	{
	    $$ = $<tag>5;
	    $$->deflt = 0;
	    if ($2) {
		$$->id = $1;
		$$->value = $2;
	    }
	    else {
		$$->id = NULL;
		$$->value = $1;
	    }
	    $$->more = $3;
	    $$->block = $4;
	    $$->next = $6;
	}
    ;

opt_id:
	{
	    $$ = NULL;
	}
    | ':' TOK_ID
	{
	    $$ = $2;
	}
    ;

list:
	{
	    $$ = NULL;
	}
    | ',' TOK_ID list
	{
	    $$ = alloc_t(VALUE_LIST);
	    $$->value = $2;
	    $$->next = $3;
	}
    ;
