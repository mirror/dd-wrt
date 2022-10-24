EXTRA_DIST        = annotate.conf a.conf b.conf spdir check_confuse.h

TESTS             = keyval
TESTS            += suite_single
TESTS            += suite_dup
TESTS            += suite_func
TESTS            += suite_list
TESTS            += suite_validate
TESTS            += suite_ptr
TESTS            += list_plus_syntax
TESTS            += section_title_dupes
TESTS            += single_title_sections
TESTS            += section_remove
TESTS            += section_add
TESTS            += section_getopt
TESTS            += quote_before_print
TESTS            += include
TESTS            += searchpath
TESTS            += env
TESTS            += ignore_parm
TESTS            += annotate
TESTS            += empty_string
TESTS            += setopt_ptr
TESTS            += setmulti_reset
TESTS            += print_filter
TESTS            += modified_flag

check_PROGRAMS    = $(TESTS)

DEFS              = -DSRC_DIR='"$(srcdir)"'
LDFLAGS           = -static
LDADD             = -L../src ../src/libconfuse.la $(LTLIBINTL)
CLEANFILES        = *~


