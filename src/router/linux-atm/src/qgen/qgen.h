/* qgen.h - constructor/parser generator for Q.2931-like data structures */
 
/* Written 1995-1997 by Werner Almesberger, EPFL-LRC */
 
 
#ifndef QGEN_H
#define QGEN_H

typedef enum { vt_id,vt_case,vt_multi,vt_length } VALUE_TYPE;

typedef struct _macro {
    /* --- the following fields are initialized by the parser */
    const char *id;
    struct _field *block;
    int instances; /* this is later modified in the first pass
		      (> 1 becomes -1) */
    struct _macro *next;
    /* --- the following fields are initialized in the first phase */
    int first_field; /* number of the first field in the first instance */
    int fields; /* number of fields covered by this structure */
    int groups; /* number of groups covered by this structure */
} STRUCTURE;

typedef struct _name {
    /* --- the following fields are initialized by the parser */
    const char *value;
    const char *name;
    struct _name *next;
} NAME;

typedef struct _name_list {
    /* --- the following fields are initialized by the parser */
    const char *list_name;
    NAME *list;
    int id; /* initialized to -1 = unassigned */
    struct _name_list *next;
} NAME_LIST;

typedef struct {
    /* --- the following fields are initialized by the parser */
    VALUE_TYPE type;
    const char *recovery; /* only valid if vt_length; NULL if not used */
    const char *abort_id; /* non-NULL to request application-specific error */
    const char *id; /* for value-only and for default tag */
    struct _field *block; /* length */
    struct _tag *tags; /* case or multi */
    /* --- the following fields are initialized in the first phase */
    /* id of default tag */
} VALUE;

typedef struct _field {
    /* --- the following fields are initialized by the parser */
    const char *id;
    NAME_LIST *name_list;
    int size;
    int var_len;
    int pos; /* modified in the first phase */
    int flush;
    VALUE *value;
    int brk; /* != 0: may break before this field */
    struct _field *next;
    STRUCTURE *structure; /* NULL if this entry isn't a structure */
    struct _field *my_block; /* this instance of the structure's body
				(undefined if structure == NULL) */
    /* --- the following fields are initialized in the first phase */
    int field; /* field number */
    int jump; /* move by that many bytes before writing that field */
    int group; /* group number, only in first element of group */
    int seq; /* sequence number - for dumping */
    /* --- the following fields are initialized in the second phase */
    int has_required;
    /* --- the following fields are initialized in the third phase */
    int patch;
} FIELD;

typedef struct _id_list {
    /* --- the following fields are initialized by the parser */
    const char *value;
    struct _id_list *next;
    /* --- the following fields are initialized in the third phase */
    int patch;
} VALUE_LIST;

typedef struct _tag {
    /* --- the following fields are initialized by the parser */
    const char *id; /* group id */
    VALUE_LIST *more; /* only for named selectors */
    const char *value;
    FIELD *block;
    const char *abort_id; /* non-NULL to request application-specific error */
    struct _tag *next;
    int deflt;
    /* --- the following fields are initialized in the first phase */
    int group;
    int pos;
    /* --- the following fields are initialized in the third phase */
    int patch;
} TAG;


void first(FIELD *def);
void second(FIELD *def);
void third(FIELD *def);

#endif
