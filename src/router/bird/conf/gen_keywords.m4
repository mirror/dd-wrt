m4_divert(-1)m4_dnl
#
#	BIRD -- Generator of Configuration Keyword List
#
#	(c) 1998--2000 Martin Mares <mj@atrey.karlin.mff.cuni.cz>
#
#	Can be freely distributed and used under the terms of the GNU GPL.
#

# Common aliases
m4_define(DNL, `m4_dnl')

# Diversions used:
#	1	keywords

# Simple iterator
m4_define(CF_itera, `m4_ifelse($#, 1, [[CF_iter($1)]], [[CF_iter($1)[[]]CF_itera(m4_shift($@))]])')
m4_define(CF_iterate, `m4_define([[CF_iter]], m4_defn([[$1]]))CF_itera($2)')

# We include all the headers
m4_define(CF_HDR, `m4_divert(0)')
m4_define(CF_DECLS, `m4_divert(-1)')
m4_define(CF_DEFINES, `m4_divert(-1)')

# Keywords are translated to C initializers
m4_define(CF_handle_kw, `m4_divert(1){ "m4_translit($1,[[A-Z]],[[a-z]])", $1, NULL },
m4_divert(-1)')
m4_define(CF_keywd, `m4_ifdef([[CF_tok_$1]],,[[m4_define([[CF_tok_$1]],1)CF_handle_kw($1)]])')
m4_define(CF_KEYWORDS, `m4_define([[CF_toks]],[[]])CF_iterate([[CF_keywd]], [[$@]])m4_ifelse(CF_toks,,,%token[[]]CF_toks
)DNL')

# CLI commands generate keywords as well
m4_define(CF_CLI, `CF_KEYWORDS(m4_translit($1, [[ ]], [[,]]))
')

# Enums are translated to C initializers: use CF_ENUM(typename, prefix, values)
m4_define(CF_enum, `m4_divert(1){ "CF_enum_prefix[[]]$1", -((CF_enum_type<<16) | CF_enum_prefix[[]]$1), NULL },
m4_divert(-1)')
m4_define(CF_ENUM, `m4_define([[CF_enum_type]],$1)m4_define([[CF_enum_prefix]],$2)CF_iterate([[CF_enum]], [[m4_shift(m4_shift($@))]])DNL')

# After all configuration templates end, we generate the 
m4_m4wrap(`
m4_divert(0)
static struct keyword keyword_list[] = {
m4_undivert(1){ NULL, -1, NULL } };
')

# As we are processing C source, we must access all M4 primitives via
# m4_* and also set different quoting convention: `[[' and ']]'
m4_changequote([[,]])
