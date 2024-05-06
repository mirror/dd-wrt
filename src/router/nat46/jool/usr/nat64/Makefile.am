# Note to myself: documentation tends to call these "PROGRAMS" "targets".
# "jool" is a "target".

bin_PROGRAMS = jool
jool_SOURCES = main-nat64.c

jool_CFLAGS  = ${WARNINGCFLAGS}
jool_CFLAGS += -I${top_srcdir}/src

jool_LDADD = ../argp/libjoolargp.la

dist_man_MANS = jool.8

if ENABLE_BASH_COMPLETION
bashcompletiondir = $(BASH_COMPLETION_DIR)
dist_bashcompletion_DATA = jool.bash
endif
