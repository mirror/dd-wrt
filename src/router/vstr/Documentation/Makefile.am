docmaindir = $(datadir)/doc/vstr-devel-$(VERSION)
docmain_DATA =                         \
                basic_cat.c.html \
                comparison.html \
                constants.txt \
                constants.html \
                design.html \
                f_c.css \
                functions.txt \
                functions.html \
		internal_structure.dia \
                structs.txt \
                structs.html \
                namespace.html \
                overview.html \
                printf_comparison.html \
                req_trunc_io.html \
                security.html \
                speed.html \
                security_problems.html \
		size_cmp.gnumeric \
                cost_tutorial.html \
                tutorial.html

doclicdir := $(datadir)/doc/vstr-devel-$(VERSION)/compare_licenses/
doclic_DATA =                         \
                compare_licenses/SafeStr.txt \
                compare_licenses/bstring.txt \
                compare_licenses/cords.txt \
                compare_licenses/ossp-str.txt \
                compare_licenses/rope.txt \
                compare_licenses/sz.txt \
                compare_licenses/toolbox.txt


man_MANS = \
                vstr.3 \
                vstr_const.3

EXTRA_DIST = $(man_MANS)

CLEANFILES = $(man_MANS)

TUT_SSI_FILES= \
  tutorial.shtml \
  ../examples/ex_hello_world.c \
  ../examples/ex_hello_world.h \
  ../examples/ex_hello_world_s.c \
  ../examples/ex_hello_world_m.c \
  ../examples/ex_hello_world_x.c \
  ../examples/ex_utils.h \
  ../examples/ex_cat.c \
  ../examples/ex_nl.c \
  ../examples/ex_hexdump.c \
  ../examples/ex_lookup_ip.c \
  tut/mpz1.c \
  tut/mpz2.c \
  tut/mpz3.c \
  tut/mpz4.c \
  ../examples/ex_gmp_factorials.c \
  ../examples/ex_gmp_nums.c

COST_TUT_SSI_FILES= \
  cost_tutorial.shtml \
  cost_tut/add_buf_add_ptr.c \
  cost_tut/pos_add_buf.c \
  cost_tut/add_buf_sub_buf.c \
  cost_tut/add_buf_sub_ptr.c \
  cost_tut/add_ptr_sub_ptr.c \
  cost_tut/iter_mov_add_del.c

SEC_SSI_FILES= \
  security.shtml \
  sec/alloc1.c \
  sec/alloc2.c \
  sec/alloc3.c \
  sec/alloc4.c \
  sec/alloc5.c \
  sec/sprintf1.c \
  sec/sprintf2.c \
  sec/strcpy1.c \
  sec/strncpy1.c \
  sec/strncpy2.c \
  sec/strncpy3.c \
  sec/strncpy4.c \
  sec/strncpy5.c \
  sec/strncpy6.c \
  sec/asprintf1.c \
  sec/snprintf1.c \
  sec/snprintf2.c

security_problems.html: security_problems.txt sec_txt2html.pl
		$(srcdir)/sec_txt2html.pl

functions.html: functions.txt txt2html.pl
		$(srcdir)/txt2html.pl

constants.html: constants.txt txt2html.pl
		$(srcdir)/txt2html.pl

structs.html: structs.txt txt2html.pl
		$(srcdir)/txt2html.pl

functions.3: functions.txt txt2man.pl
		$(srcdir)/txt2man.pl

constants.3: constants.txt txt2man.pl
		$(srcdir)/txt2man.pl

if USE_COMPILE_EXAMPLES

if HAVE_POSIX_HOST

ssi_DEP = ../examples/ex_ssi
ssi_APP = ../examples/ex_ssi --prefix-path="`pwd`/../examples"

else

# can't use jssi, because it calls ex_highlight inline
ssi_DEP =
ssi_APP = cat

endif

else

# can't use jssi, because it calls ex_highlight inline
ssi_DEP =
ssi_APP = cat

endif



tutorial.html: $(TUT_SSI_FILES) $(ssi_DEP)
		-@$(LN_S) $(top_srcdir)/Documentation/f_c.css f_c.css
		$(ssi_APP) $< > $@

cost_tutorial.html: $(COST_TUT_SSI_FILES) $(ssi_DEP)
		-@$(LN_S) $(top_srcdir)/Documentation/f_c.css f_c.css
		$(ssi_APP) $< > $@

security.html: $(SEC_SSI_FILES) $(ssi_DEP)
		-@$(LN_S) $(top_srcdir)/Documentation/f_c.css f_c.css
		$(ssi_APP) $< > $@

vstr.3: ./functions.3
		cp -p $< $@

vstr_const.3: ./constants.3
		cp -p $< $@
