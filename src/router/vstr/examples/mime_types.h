#ifndef MIME_TYPES_H
#define MIME_TYPES_H

#include <vstr.h>

typedef struct Mime_types_data
{
 void (*pref_func)(Vstr_ref *);
 Vstr_base *ent_data;
 Vstr_sects *ents;
} Mime_types_data;

/* allow different default types, without having to load differednt files... */
typedef struct Mime_types
{
 Vstr_ref *ref;
 
 const Vstr_base *def_type_vs1;
 size_t           def_type_pos;
 size_t           def_type_len;
} Mime_types;

extern int mime_types_init(Mime_types *, const Vstr_base *, size_t, size_t);
extern void mime_types_exit(Mime_types *);

extern int mime_types_load_simple(Mime_types *, const char *);

extern int mime_types_match(const Mime_types *,
                            const Vstr_base *, size_t, size_t,
                            const Vstr_base **, size_t *, size_t *);
extern void mime_types_combine_filedata(Mime_types *, Mime_types *);

#endif
