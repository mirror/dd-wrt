# Note to myself: documentation tends to call these "PROGRAMS" "targets".
# "jool_siit" is a "target".

bin_PROGRAMS = jool_siit
jool_siit_SOURCES = main-siit.c

jool_siit_CFLAGS  = ${WARNINGCFLAGS}
jool_siit_CFLAGS += -I${top_srcdir}/src

jool_siit_LDADD = ../argp/libjoolargp.la

dist_man_MANS = jool_siit.8

if ENABLE_BASH_COMPLETION
bashcompletiondir = $(BASH_COMPLETION_DIR)
dist_bashcompletion_DATA = jool_siit.bash
endif
