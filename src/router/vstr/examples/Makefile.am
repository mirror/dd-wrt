# See if we can cast the automake spell. -*-Makefile-*-
#

#
AUTOMAKE_OPTIONS = no-dependencies 1.4

EX_B_DIR=$(top_builddir)/examples
EX_INST_DIR=$(libdir)/vstr-$(VERSION)/examples

GEN_CFLAGS = -I../include -I../../include @CFLAGS@ @WARNS@
if USE_EX_STATIC_LIBS
GEN_CFLAGS += -DCONF_FULL_STATIC=1
endif

GEN_LDLIBS = @LDFLAGS@ @LOCAL_EX_PROG_LIBS@


ex_cat_DEPENDENCIES = ex_utils.h
ex_cat_CFLAGS = $(GEN_CFLAGS)
ex_cat_LDADD  = $(GEN_LDLIBS)

ex_ccount_DEPENDENCIES = ex_utils.h
ex_ccount_CFLAGS = $(GEN_CFLAGS)
ex_ccount_LDADD  = $(GEN_LDLIBS)

ex_conf_SOURCES      = conf.c ex_conf.c conf_inline.c
ex_conf_DEPENDENCIES = conf.h ex_utils.h mk.h
ex_conf_CFLAGS = $(GEN_CFLAGS)
ex_conf_LDADD  = $(GEN_LDLIBS)

ex_hello_world_DEPENDENCIES = ex_utils.h
ex_hello_world_CFLAGS = $(GEN_CFLAGS)
ex_hello_world_LDADD  = $(GEN_LDLIBS)

ex_hello_world_s_DEPENDENCIES = ex_utils.h
ex_hello_world_s_CFLAGS = $(GEN_CFLAGS)
ex_hello_world_s_LDADD  = $(GEN_LDLIBS)

ex_hello_world_m_DEPENDENCIES = ex_utils.h
ex_hello_world_m_CFLAGS = $(GEN_CFLAGS)
ex_hello_world_m_LDADD  = $(GEN_LDLIBS)

ex_hello_world_x_DEPENDENCIES = ex_utils.h
ex_hello_world_x_CFLAGS = $(GEN_CFLAGS)
ex_hello_world_x_LDADD  = $(GEN_LDLIBS)

ex_hexdump_SOURCES      = hexdump.c ex_hexdump.c
ex_hexdump_DEPENDENCIES = hexdump.h ex_utils.h
ex_hexdump_CFLAGS = $(GEN_CFLAGS)
ex_hexdump_LDADD  = $(GEN_LDLIBS)

ex_dir_list2html_DEPENDENCIES = ex_utils.h
ex_dir_list2html_CFLAGS = $(GEN_CFLAGS)
ex_dir_list2html_LDADD  = $(GEN_LDLIBS)

ex_dir_filter_SOURCES      = opt.c bag.c ex_dir_filter.c
ex_dir_filter_DEPENDENCIES = opt.h bag.h ex_utils.h
ex_dir_filter_CFLAGS = $(GEN_CFLAGS)
ex_dir_filter_LDADD  = $(GEN_LDLIBS)

ex_dir_list_DEPENDENCIES = ex_utils.h
ex_dir_list_CFLAGS = $(GEN_CFLAGS)
ex_dir_list_LDADD  = $(GEN_LDLIBS)

ex_dir_sort_SOURCES      = opt.c bag.c ex_dir_sort.c
ex_dir_sort_DEPENDENCIES = opt.h bag.h ex_utils.h
ex_dir_sort_CFLAGS = $(GEN_CFLAGS)
ex_dir_sort_LDADD  = $(GEN_LDLIBS)

ex_nl_DEPENDENCIES = ex_utils.h
ex_nl_CFLAGS = $(GEN_CFLAGS)
ex_nl_LDADD  = $(GEN_LDLIBS)

ex_phone_names_DEPENDENCIES = ex_utils.h
ex_phone_names_CFLAGS = $(GEN_CFLAGS)
ex_phone_names_LDADD  = $(GEN_LDLIBS)

ex_rot13_DEPENDENCIES = ex_utils.h
ex_rot13_CFLAGS = $(GEN_CFLAGS)
ex_rot13_LDADD  = $(GEN_LDLIBS)

ex_yes_DEPENDENCIES = ex_utils.h
ex_yes_CFLAGS = $(GEN_CFLAGS)
ex_yes_LDADD  = $(GEN_LDLIBS)

ex_mon_cp_DEPENDENCIES = ex_utils.h
ex_mon_cp_CFLAGS = $(GEN_CFLAGS)
ex_mon_cp_LDADD  = $(GEN_LDLIBS)

ex_lookup_ip_DEPENDENCIES = ex_utils.h
ex_lookup_ip_CFLAGS = $(GEN_CFLAGS)
ex_lookup_ip_LDADD  = $(GEN_LDLIBS)

ex_csv_DEPENDENCIES = ex_utils.h
ex_csv_CFLAGS = $(GEN_CFLAGS)
ex_csv_LDADD  = $(GEN_LDLIBS)

ex_highlight_DEPENDENCIES = ex_utils.h
ex_highlight_CFLAGS = $(GEN_CFLAGS)
ex_highlight_LDADD  = $(GEN_LDLIBS)

ex_ssi_SOURCES      = opt.c ex_ssi.c
ex_ssi_DEPENDENCIES = opt.h ex_utils.h
ex_ssi_CFLAGS = $(GEN_CFLAGS)
ex_ssi_LDADD  = $(GEN_LDLIBS)

ex_rainbow_DEPENDENCIES = ex_utils.h
ex_rainbow_CFLAGS = $(GEN_CFLAGS)
ex_rainbow_LDADD  = $(GEN_LDLIBS)

ex_changelog_diff_DEPENDENCIES = ex_utils.h
ex_changelog_diff_CFLAGS = $(GEN_CFLAGS)
ex_changelog_diff_LDADD  = $(GEN_LDLIBS)

ex_slowcat_SOURCES      = opt.c ex_slowcat.c
ex_slowcat_DEPENDENCIES = opt.h ex_utils.h
ex_slowcat_CFLAGS   = $(GEN_CFLAGS) $(TIMER_Q_CFLAGS)
ex_slowcat_LDADD    = $(GEN_LDLIBS) $(TIMER_Q_LDLIBS)

# For command line stuff that uses events...
CMDLINE_FILES_SRCS = date.c evnt.c hexdump.c opt.c vlg.c
CMDLINE_FILES_HDRS = date.h evnt.h hexdump.h opt.h vlg.h

ex_dns_SOURCES      = dns.c $(CMDLINE_FILES_SRCS) ex_dns.c
ex_dns_DEPENDENCIES = dns.h $(CMDLINE_FILES_HDRS)
ex_dns_CFLAGS   = $(GEN_CFLAGS) $(SOCKET_POLL_CFLAGS) $(TIMER_Q_CFLAGS)
ex_dns_LDADD    = $(GEN_LDLIBS) $(SOCKET_POLL_LDLIBS) $(TIMER_Q_LDLIBS)

ex_cntl_SOURCES      = $(CMDLINE_FILES_SRCS) ex_cntl.c
ex_cntl_DEPENDENCIES = $(CMDLINE_FILES_HDRS) mk.h
ex_cntl_CFLAGS   = $(GEN_CFLAGS) $(SOCKET_POLL_CFLAGS) $(TIMER_Q_CFLAGS)
ex_cntl_LDADD    = $(GEN_LDLIBS) $(SOCKET_POLL_LDLIBS) $(TIMER_Q_LDLIBS)

DAEMON_FILES_SRCS = bag.c conf.c cntl.c date.c evnt.c hexdump.c \
                    opt.c opt_policy.c opt_serv.c vlg.c \
                    conf_inline.c opt_policy_inline.c
DAEMON_FILES_HDRS = bag.h conf.h cntl.h date.h evnt.h hexdump.h \
                    opt.h opt_policy.h opt_serv.h vlg.h

ex_httpd_SOURCES      = mime_types.c httpd_conf_main.c httpd_conf_req.c \
			httpd_policy.c httpd_policy_inline.c \
                        $(DAEMON_FILES_SRCS) httpd.c ex_httpd.c
ex_httpd_DEPENDENCIES = mime_types.h httpd_conf_main.h httpd_conf_req.h \
			httpd_policy.h \
                        $(DAEMON_FILES_HDRS) httpd.h httpd_err_codes.h
ex_httpd_CFLAGS   = $(GEN_CFLAGS) $(SOCKET_POLL_CFLAGS) $(TIMER_Q_CFLAGS)
ex_httpd_LDADD    = $(GEN_LDLIBS) $(SOCKET_POLL_LDLIBS) $(TIMER_Q_LDLIBS)

ex_echod_SOURCES      = $(DAEMON_FILES_SRCS) ex_echod.c
ex_echod_DEPENDENCIES = $(DAEMON_FILES_HDRS)
ex_echod_CFLAGS   = $(GEN_CFLAGS) $(SOCKET_POLL_CFLAGS) $(TIMER_Q_CFLAGS)
ex_echod_LDADD    = $(GEN_LDLIBS) $(SOCKET_POLL_LDLIBS) $(TIMER_Q_LDLIBS)

ex_ypservd_SOURCES      = $(DAEMON_FILES_SRCS) ex_ypservd.c
ex_ypservd_DEPENDENCIES = $(DAEMON_FILES_HDRS)
ex_ypservd_CFLAGS   = $(GEN_CFLAGS) $(SOCKET_POLL_CFLAGS) $(TIMER_Q_CFLAGS)
ex_ypservd_LDADD    = $(GEN_LDLIBS) $(SOCKET_POLL_LDLIBS) $(TIMER_Q_LDLIBS)

ex_zcat_DEPENDENCIES = ex_utils.h
ex_zcat_CFLAGS = $(GEN_CFLAGS) $(ZLIB_CFLAGS) $(BZ2_CFLAGS)
ex_zcat_LDADD  = $(GEN_LDLIBS) $(ZLIB_LDLIBS) $(BZ2_LDLIBS)

ex_sock_filter_DEPENDENCIES = ex_utils.h
ex_sock_filter_CFLAGS = $(GEN_CFLAGS)
ex_sock_filter_LDADD  = $(GEN_LDLIBS)

ex_perf_sgopenssl_DEPENDENCIES = ex_perf.h
ex_perf_sgopenssl_CFLAGS = $(GEN_CFLAGS) $(OPENSSL_CFLAGS) -Wno-aggregate-return
ex_perf_sgopenssl_LDADD  = $(GEN_LDLIBS) $(OPENSSL_LDLIBS)

ex_gmp_factorials_DEPENDENCIES = ex_utils.h
ex_gmp_factorials_CFLAGS = $(GEN_CFLAGS) $(GMP_CFLAGS)
ex_gmp_factorials_LDADD  = $(GEN_LDLIBS) $(GMP_LDLIBS)
ex_gmp_nums_DEPENDENCIES = ex_utils.h
ex_gmp_nums_CFLAGS = $(GEN_CFLAGS) $(GMP_CFLAGS)
ex_gmp_nums_LDADD  = $(GEN_LDLIBS) $(GMP_LDLIBS)
ex_gmp_num_roman_DEPENDENCIES = ex_utils.h
ex_gmp_num_roman_CFLAGS = $(GEN_CFLAGS) $(GMP_CFLAGS)
ex_gmp_num_roman_LDADD  = $(GEN_LDLIBS) $(GMP_LDLIBS)
ex_gmp_num_words_DEPENDENCIES = ex_utils.h
ex_gmp_num_words_CFLAGS = $(GEN_CFLAGS) $(GMP_CFLAGS)
ex_gmp_num_words_LDADD  = $(GEN_LDLIBS) $(GMP_LDLIBS)

ex_perf_nodesize_foreach_DEPENDENCIES = ex_perf.h
ex_perf_nodesize_foreach_CFLAGS = $(GEN_CFLAGS) $(GLIB20_CFLAGS) -Wno-aggregate-return
ex_perf_nodesize_foreach_LDADD  = $(GEN_LDLIBS) $(GLIB20_LDLIBS)

ex_netstr_DEPENDENCIES = ex_perf.h
ex_netstr_CFLAGS = $(GEN_CFLAGS)
ex_netstr_LDADD  = $(GEN_LDLIBS)

ex_perf_nodesize_DEPENDENCIES = ex_perf.h
ex_perf_nodesize_CFLAGS = $(GEN_CFLAGS) -Wno-aggregate-return
ex_perf_nodesize_LDADD  = $(GEN_LDLIBS)

ex_perf_add_DEPENDENCIES = ex_perf.h
ex_perf_add_CFLAGS = $(GEN_CFLAGS) -Wno-aggregate-return
ex_perf_add_LDADD  = $(GEN_LDLIBS)


# BEG: Not a Vstr app.
ex_tdns_CFLAGS   = -pthread $(GEN_CFLAGS) 
ex_tdns_LDADD    = $(GEN_LDLIBS)
ex_tdns_LDFLAGS  = -pthread
# END: Not a Vstr app.


examplesdir = $(EX_INST_DIR)
examples_PROGRAMS = ex_netstr

if MALLINFO_AVAIL
examples_PROGRAMS += ex_perf_nodesize ex_perf_add
endif

# Turn on Programs we can compile...

if HAVE_POSIX_HOST

hi_DEP =   ex_highlight
hi_APP = ./ex_highlight
ls_DEP =   ex_dir_list ex_dir_filter ex_dir_sort ex_dir_list2html
ls1_APP = ./ex_dir_list --size --follow -- 
ls2_APP = | ./ex_dir_filter \
 -A .. --deny-name-beg . \
 -D index.html -D dir_list.css \
 --deny-name-end .gz --deny-name-end .bz2 \
 --deny-name-end ~ --deny-name-end \\\# | \
 ./ex_dir_sort --sort=cmp | \
 ./ex_dir_list2html --name


# Don't build by default, requires threads...
# examples_PROGRAMS += ex_tdns

examples_PROGRAMS += \
  ex_cat \
  ex_hello_world ex_hello_world_s ex_hello_world_m ex_hello_world_x \
  ex_hexdump ex_dir_list2html ex_dir_filter ex_dir_list ex_dir_sort ex_nl \
  ex_phone_names ex_rot13 \
  ex_yes ex_mon_cp ex_lookup_ip ex_csv $(hi_DEP) \
  ex_ssi ex_rainbow ex_changelog_diff ex_ccount ex_sock_filter ex_conf

if TIMER_Q_AVAIL
examples_PROGRAMS += ex_slowcat

if SOCKET_POLL_AVAIL
examples_PROGRAMS += ex_cntl ex_dns ex_echod
#examples_PROGRAMS += ex_httpd
#examples_PROGRAMS += ex_ypservd
endif
# Timer_q
endif

if BZ2_AVAIL
if ZLIB_AVAIL
examples_PROGRAMS += ex_zcat
endif
endif

if GMP_AVAIL
examples_PROGRAMS += \
  ex_gmp_factorials ex_gmp_nums ex_gmp_num_roman ex_gmp_num_words
endif

if OPENSSL_AVAIL
if MALLINFO_AVAIL
examples_PROGRAMS += ex_perf_sgopenssl
endif
endif

if GLIB20_AVAIL
if MALLINFO_AVAIL
examples_PROGRAMS += ex_perf_nodesize_foreach
endif
endif


if PERL_USR_BIN_AVAIL
TESTS_ENVIRONMENT = SRCDIR="$(srcdir)"
TESTS = tst/tst_cat.pl tst/tst_hexdump.pl tst/tst_nl.pl tst/tst_rot13.pl \
        tst/tst_sock_filter.pl tst/tst_conf.pl \
        tst/tst_echod.pl
#TESTS += tst/tst_httpd_cmd.pl tst/tst_httpd_conf.pl
#TESTS += tst/tst_httpd_pubonly.pl
#TESTS += tst/tst_httpd_none.pl
#TESTS += tst/tst_httpd_nonvhosts.pl
#TESTS += tst/tst_httpd_vhosts.pl
endif

else
# !HAVE_POSIX_HOST

# Use pre-installed version...
hi_DEP =
hi_APP = echo -- hi
ls_DEP =
ls1_APP = echo -- ls1
ls2_APP = echo -- ls2

endif

example_code_html = \
  html/app.h.html \
  html/bag.c.html \
  html/bag.h.html \
  html/conf.c.html \
  html/conf.h.html \
  html/conf_inline.c.html \
  html/cntl.c.html \
  html/cntl.h.html \
  html/date.c.html \
  html/date.h.html \
  html/dns.c.html \
  html/dns.h.html \
  html/evnt.c.html \
  html/evnt.h.html \
  html/mime_types.c.html \
  html/mime_types.h.html \
  html/ex_cat.c.html \
  html/ex_ccount.c.html \
  html/ex_changelog_diff.c.html \
  html/ex_conf.c.html \
  html/ex_cntl.c.html \
  html/ex_csv.c.html \
  html/ex_dir_filter.c.html \
  html/ex_dir_list.c.html \
  html/ex_dir_sort.c.html \
  html/ex_dns.c.html \
  html/ex_echod.c.html \
  html/ex_gmp_factorials.c.html \
  html/ex_gmp_num_roman.c.html \
  html/ex_gmp_nums.c.html \
  html/ex_gmp_num_words.c.html \
  html/ex_hello_world.c.html \
  html/ex_hello_world.h.html \
  html/ex_hello_world_m.c.html \
  html/ex_hello_world_s.c.html \
  html/ex_hello_world_x.c.html \
  html/ex_hexdump.c.html \
  html/ex_highlight.c.html \
  html/ex_dir_list2html.c.html \
  html/ex_httpd.c.html \
  html/ex_lookup_ip.c.html \
  html/ex_mon_cp.c.html \
  html/ex_netstr.c.html \
  html/ex_nl.c.html \
  html/ex_perf_add.c.html \
  html/ex_perf.h.html \
  html/ex_perf_nodesize.c.html \
  html/ex_perf_nodesize_foreach.c.html \
  html/ex_perf_sgopenssl.c.html \
  html/ex_phone_names.c.html \
  html/ex_rainbow.c.html \
  html/ex_rot13.c.html \
  html/ex_slowcat.c.html \
  html/ex_ssi.c.html \
  html/ex_utils.h.html \
  html/ex_yes.c.html \
  html/ex_ypservd.c.html \
  html/ex_zcat.c.html \
  html/hexdump.c.html \
  html/hexdump.h.html \
  html/httpd.c.html \
  html/httpd.h.html \
  html/httpd_err_codes.h.html \
  html/httpd_conf_main.c.html \
  html/httpd_conf_main.h.html \
  html/httpd_conf_req.c.html \
  html/httpd_conf_req.h.html \
  html/httpd_policy.c.html \
  html/httpd_policy.h.html \
  html/httpd_policy_inline.c.html \
  html/mk.h.html \
  html/opt.h.html \
  html/opt_serv.c.html \
  html/opt_serv.h.html \
  html/opt_policy.c.html \
  html/opt_policy.h.html \
  html/opt_policy_inline.c.html \
  html/vlg.c.html \
  html/vlg.h.html
#  html/ex_tdns.c.html 


htmldir   = $(EX_INST_DIR)/html
html_DATA = \
  html/index.html \
  html/dir_list.css \
  html/f_c.css \
  \
  $(example_code_html)

examplesscriptsdir = $(EX_INST_DIR)/scripts
examplesscripts_DATA = \
  highlight2style.pl \
  nodesize_plots.pl \
  nodesize_split.pl

extradatadir = $(EX_INST_DIR)/data
extradata_DATA = \
  hexdump_data \
  mime_types_extra.txt \
  README \
  zcat_data_1.bz2 \
  zcat_data_1.gz \
  zcat_data_2.bz2 \
  zcat_data_2.gz


initddir = $(sysconfdir)/init.d
initd_SCRIPTS = \
  init.d/jechod


CLEANFILES=$(html_DATA) html

html/f_c.css: ../Documentation/f_c.css
		-@mkdir html
		-$(LN_S) ../$< html

html/dir_list.css: ../examples/dir_list.css
		-@mkdir html
		-$(LN_S) ../$< html

html/index.html: html/dir_list.css $(ls_DEP) $(example_code_html)
		$(ls1_APP) html $(ls2_APP) 'Vstr example code' > $@
		$(srcdir)/gzip-r.pl --zero --type=all html

html/%.html: % html/f_c.css $(hi_DEP)
		$(hi_APP) --css-filename=f_c.css $< > $@

