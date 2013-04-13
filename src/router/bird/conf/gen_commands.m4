m4_divert(-1)m4_dnl
#
#	BIRD -- Generator of CLI Command List
#
#	(c) 2000 Martin Mares <mj@atrey.karlin.mff.cuni.cz>
#
#	Can be freely distributed and used under the terms of the GNU GPL.
#

m4_define(CF_CLI, `m4_divert(0){ "m4_translit($1,A-Z,a-z)", "$3", "$4", 1 },
m4_divert(-1)')

m4_define(CF_CLI_CMD, `m4_divert(0){ "m4_translit($1,A-Z,a-z)", "$2", "$3", 1 },
m4_divert(-1)')

m4_define(CF_CLI_HELP, `m4_divert(0){ "m4_translit($1,A-Z,a-z)", "$2", "$3", 0 },
m4_divert(-1)')

# As we are processing C source, we must access all M4 primitives via
# m4_* and also set different quoting convention: `[[' and ']]'
m4_changequote([[,]])
