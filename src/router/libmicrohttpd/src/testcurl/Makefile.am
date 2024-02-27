# This Makefile.am is in the public domain
EMPTY_ITEM =

@HEAVY_TESTS_NOTPARALLEL@

SUBDIRS = .

AM_CPPFLAGS = \
  -I$(top_srcdir)/src/include \
  -I$(top_srcdir)/src/microhttpd \
  -DMHD_CPU_COUNT=$(CPU_COUNT) \
  $(CPPFLAGS_ac) $(LIBCURL_CPPFLAGS)

AM_CFLAGS = $(CFLAGS_ac) @LIBGCRYPT_CFLAGS@

AM_LDFLAGS = $(LDFLAGS_ac)

AM_TESTS_ENVIRONMENT = $(TESTS_ENVIRONMENT_ac)

if USE_COVERAGE
  AM_CFLAGS += -fprofile-arcs -ftest-coverage
endif

if ENABLE_HTTPS
  SUBDIRS += https
endif

LDADD = \
  $(top_builddir)/src/microhttpd/libmicrohttpd.la \
  @LIBCURL@

$(top_builddir)/src/microhttpd/libmicrohttpd.la: $(top_builddir)/src/microhttpd/Makefile
	@echo ' cd $(top_builddir)/src/microhttpd && $(MAKE) $(AM_MAKEFLAGS) libmicrohttpd.la'; \
	$(am__cd) $(top_builddir)/src/microhttpd && $(MAKE) $(AM_MAKEFLAGS) libmicrohttpd.la

THREAD_ONLY_TESTS = \
  test_urlparse \
  test_long_header \
  test_long_header11 \
  test_iplimit11 \
  test_termination \
  $(EMPTY_ITEM)

if HEAVY_TESTS
THREAD_ONLY_TESTS += \
  test_add_conn_cleanup \
  test_add_conn_cleanup_nolisten \
  test_timeout \
  $(EMPTY_ITEM)
endif

if HAVE_POSIX_THREADS
if HEAVY_TESTS
THREAD_ONLY_TESTS += \
  perf_get_concurrent11 \
  $(EMPTY_ITEM)
endif

THREAD_ONLY_TESTS += \
  test_get_wait \
  test_get_wait11 \
  $(EMPTY_ITEM)

if HEAVY_TESTS
THREAD_ONLY_TESTS += \
  test_concurrent_stop \
  test_quiesce \
  $(EMPTY_ITEM)
endif

if HAVE_CURL_BINARY
THREAD_ONLY_TESTS += \
  test_quiesce_stream
endif
endif

if HEAVY_TESTS
if HAVE_POSIX_THREADS
THREAD_ONLY_TESTS += \
  perf_get_concurrent
endif
endif

if RUN_LIBCURL_TESTS
check_PROGRAMS = \
  test_get \
  test_head \
  test_head10 \
  test_get_iovec \
  test_get_sendfile \
  test_get_close \
  test_get_close10 \
  test_get_keep_alive \
  test_get_keep_alive10 \
  test_delete \
  test_patch \
  test_put \
  test_add_conn \
  test_add_conn_nolisten \
  test_process_headers \
  test_process_arguments \
  test_toolarge_method \
  test_toolarge_url \
  test_toolarge_request_header_name \
  test_toolarge_request_header_value \
  test_toolarge_request_headers \
  test_toolarge_reply_header_name \
  test_toolarge_reply_header_value \
  test_toolarge_reply_headers \
  test_tricky_url \
  test_tricky_header2 \
  test_large_put \
  test_get11 \
  test_get_iovec11 \
  test_get_sendfile11 \
  test_patch11 \
  test_put11 \
  test_large_put11 \
  test_large_put_inc11 \
  test_put_broken_len10 \
  test_put_broken_len \
  test_get_chunked \
  test_get_chunked_close \
  test_get_chunked_string \
  test_get_chunked_close_string \
  test_get_chunked_empty \
  test_get_chunked_close_empty \
  test_get_chunked_string_empty \
  test_get_chunked_close_string_empty \
  test_get_chunked_sized \
  test_get_chunked_close_sized \
  test_get_chunked_empty_sized \
  test_get_chunked_close_empty_sized \
  test_get_chunked_forced \
  test_get_chunked_close_forced \
  test_get_chunked_empty_forced \
  test_get_chunked_close_empty_forced \
  test_put_chunked \
  test_callback \
  test_get_header_fold \
  test_put_header_fold \
  test_put_large_header_fold \
  test_put_header_fold_last \
  test_put_header_fold_large \
  test_get_header_double_fold \
  test_put_header_double_fold \
  test_put_large_header_double_fold \
  test_put_header_double_fold_last \
  test_put_header_double_fold_large \
  $(EMPTY_ITEM)

if ENABLE_COOKIE
check_PROGRAMS += \
  test_parse_cookies_discp_p2 \
  test_parse_cookies_discp_p1 \
  test_parse_cookies_discp_zero \
  test_parse_cookies_discp_n2 \
  test_parse_cookies_discp_n3
endif

if HEAVY_TESTS
check_PROGRAMS += \
  perf_get
endif

if ENABLE_BAUTH
check_PROGRAMS += \
  test_basicauth test_basicauth_preauth \
  test_basicauth_oldapi test_basicauth_preauth_oldapi
endif

if HAVE_POSTPROCESSOR
check_PROGRAMS += \
  test_post \
  test_postform \
  test_post_loop \
  test_post11 \
  test_postform11 \
  test_post_loop11
endif


if ENABLE_DAUTH
if ENABLE_MD5
THREAD_ONLY_TESTS += \
  test_digestauth \
  test_digestauth_with_arguments \
  test_digestauth_concurrent
endif
if ENABLE_SHA256
THREAD_ONLY_TESTS += \
  test_digestauth_sha256
endif

if ENABLE_MD5
check_PROGRAMS += \
  test_digestauth_emu_ext \
  test_digestauth_emu_ext_oldapi \
  test_digestauth2 \
  test_digestauth2_rfc2069 \
  test_digestauth2_rfc2069_userdigest \
  test_digestauth2_oldapi1 \
  test_digestauth2_oldapi2 \
  test_digestauth2_userhash \
  test_digestauth2_userdigest \
  test_digestauth2_oldapi1_userdigest \
  test_digestauth2_oldapi2_userdigest \
  test_digestauth2_userhash_userdigest \
  test_digestauth2_bind_all \
  test_digestauth2_bind_uri \
  test_digestauth2_oldapi1_bind_all \
  test_digestauth2_oldapi1_bind_uri
endif
if ENABLE_SHA256
check_PROGRAMS += \
  test_digestauth2_sha256 \
  test_digestauth2_sha256_userhash \
  test_digestauth2_oldapi2_sha256 \
  test_digestauth2_sha256_userdigest \
  test_digestauth2_oldapi2_sha256_userdigest \
  test_digestauth2_sha256_userhash_userdigest
endif
endif

if HEAVY_TESTS
if HAVE_FORK_WAITPID
if HAVE_CURL_BINARY
check_PROGRAMS += test_get_response_cleanup
endif
endif
endif

if USE_POSIX_THREADS
check_PROGRAMS += \
  $(THREAD_ONLY_TESTS)
endif
if USE_W32_THREADS
check_PROGRAMS += \
  $(THREAD_ONLY_TESTS)
endif

TESTS = $(check_PROGRAMS)
endif

test_concurrent_stop_SOURCES = \
  test_concurrent_stop.c
test_concurrent_stop_CFLAGS = \
  $(AM_CFLAGS) $(PTHREAD_CFLAGS)
test_concurrent_stop_LDADD = \
  $(PTHREAD_LIBS) $(LDADD)

test_get_SOURCES = \
  test_get.c mhd_has_in_name.h mhd_has_param.h

test_head_SOURCES = \
  test_head.c mhd_has_in_name.h mhd_has_param.h

test_head10_SOURCES = \
  test_head.c mhd_has_in_name.h mhd_has_param.h

test_quiesce_SOURCES = \
  test_quiesce.c mhd_has_param.h mhd_has_in_name.h
test_quiesce_CFLAGS = \
  $(AM_CFLAGS) $(PTHREAD_CFLAGS)
test_quiesce_LDADD = \
  $(PTHREAD_LIBS) $(LDADD)

test_quiesce_stream_SOURCES = \
  test_quiesce_stream.c
test_quiesce_stream_CFLAGS = \
  $(AM_CFLAGS) $(PTHREAD_CFLAGS)
test_quiesce_stream_LDADD = \
  $(PTHREAD_LIBS) $(LDADD)

test_callback_SOURCES = \
  test_callback.c

perf_get_SOURCES = \
  perf_get.c \
  mhd_has_in_name.h

perf_get_concurrent_SOURCES = \
  perf_get_concurrent.c \
  mhd_has_in_name.h
perf_get_concurrent_CFLAGS = \
  $(AM_CFLAGS) $(PTHREAD_CFLAGS)
perf_get_concurrent_LDADD = \
  $(PTHREAD_LIBS) $(LDADD)

perf_get_concurrent11_SOURCES = \
  perf_get_concurrent.c \
  mhd_has_in_name.h
perf_get_concurrent11_CFLAGS = \
  $(AM_CFLAGS) $(PTHREAD_CFLAGS)
perf_get_concurrent11_LDADD = \
  $(PTHREAD_LIBS) $(LDADD)

test_basicauth_SOURCES = \
  test_basicauth.c

test_basicauth_preauth_SOURCES = \
  test_basicauth.c

test_basicauth_oldapi_SOURCES = \
  test_basicauth.c

test_basicauth_preauth_oldapi_SOURCES = \
  test_basicauth.c

test_digestauth_SOURCES = \
  test_digestauth.c
test_digestauth_LDADD = \
  @LIBGCRYPT_LIBS@ $(LDADD)

test_digestauth_sha256_SOURCES = \
  test_digestauth_sha256.c
test_digestauth_sha256_LDADD = \
  @LIBGCRYPT_LIBS@ $(LDADD)

test_digestauth_with_arguments_SOURCES = \
  test_digestauth_with_arguments.c
test_digestauth_with_arguments_LDADD = \
  @LIBGCRYPT_LIBS@ $(LDADD)

test_digestauth_concurrent_SOURCES = \
  test_digestauth_concurrent.c
test_digestauth_concurrent_CFLAGS = \
  $(AM_CFLAGS) $(PTHREAD_CFLAGS)
test_digestauth_concurrent_LDADD = \
  @LIBGCRYPT_LIBS@ $(LDADD) $(PTHREAD_LIBS) $(LDADD)

test_digestauth_emu_ext_SOURCES = \
  test_digestauth_emu_ext.c

test_digestauth_emu_ext_oldapi_SOURCES = \
  test_digestauth_emu_ext.c

test_digestauth2_SOURCES = \
  test_digestauth2.c mhd_has_param.h mhd_has_in_name.h

test_digestauth2_rfc2069_SOURCES = \
  test_digestauth2.c mhd_has_param.h mhd_has_in_name.h

test_digestauth2_rfc2069_userdigest_SOURCES = \
  test_digestauth2.c mhd_has_param.h mhd_has_in_name.h

test_digestauth2_oldapi1_SOURCES = \
  test_digestauth2.c mhd_has_param.h mhd_has_in_name.h

test_digestauth2_oldapi2_SOURCES = \
  test_digestauth2.c mhd_has_param.h mhd_has_in_name.h

test_digestauth2_userhash_SOURCES = \
  test_digestauth2.c mhd_has_param.h mhd_has_in_name.h

test_digestauth2_sha256_SOURCES = \
  test_digestauth2.c mhd_has_param.h mhd_has_in_name.h

test_digestauth2_oldapi2_sha256_SOURCES = \
  test_digestauth2.c mhd_has_param.h mhd_has_in_name.h

test_digestauth2_sha256_userhash_SOURCES = \
  test_digestauth2.c mhd_has_param.h mhd_has_in_name.h

test_digestauth2_userdigest_SOURCES = \
  test_digestauth2.c mhd_has_param.h mhd_has_in_name.h

test_digestauth2_oldapi1_userdigest_SOURCES = \
  test_digestauth2.c mhd_has_param.h mhd_has_in_name.h

test_digestauth2_oldapi2_userdigest_SOURCES = \
  test_digestauth2.c mhd_has_param.h mhd_has_in_name.h

test_digestauth2_userhash_userdigest_SOURCES = \
  test_digestauth2.c mhd_has_param.h mhd_has_in_name.h

test_digestauth2_sha256_userdigest_SOURCES = \
  test_digestauth2.c mhd_has_param.h mhd_has_in_name.h

test_digestauth2_oldapi2_sha256_userdigest_SOURCES = \
  test_digestauth2.c mhd_has_param.h mhd_has_in_name.h

test_digestauth2_sha256_userhash_userdigest_SOURCES = \
  test_digestauth2.c mhd_has_param.h mhd_has_in_name.h

test_digestauth2_bind_all_SOURCES = \
  test_digestauth2.c mhd_has_param.h mhd_has_in_name.h

test_digestauth2_bind_uri_SOURCES = \
  test_digestauth2.c mhd_has_param.h mhd_has_in_name.h

test_digestauth2_oldapi1_bind_all_SOURCES = \
  test_digestauth2.c mhd_has_param.h mhd_has_in_name.h

test_digestauth2_oldapi1_bind_uri_SOURCES = \
  test_digestauth2.c mhd_has_param.h mhd_has_in_name.h

test_get_iovec_SOURCES = \
  test_get_iovec.c mhd_has_in_name.h

test_get_sendfile_SOURCES = \
  test_get_sendfile.c mhd_has_in_name.h

test_get_wait_SOURCES = \
  test_get_wait.c \
  mhd_has_in_name.h
test_get_wait_CFLAGS = \
  $(PTHREAD_CFLAGS) $(AM_CFLAGS)
test_get_wait_LDADD = \
  $(PTHREAD_LIBS) $(LDADD)

test_get_wait11_SOURCES = \
  test_get_wait.c \
  mhd_has_in_name.h
test_get_wait11_CFLAGS = \
  $(PTHREAD_CFLAGS) $(AM_CFLAGS)
test_get_wait11_LDADD = \
  $(PTHREAD_LIBS) $(LDADD)

test_urlparse_SOURCES = \
  test_urlparse.c mhd_has_in_name.h

test_get_response_cleanup_SOURCES = \
  test_get_response_cleanup.c mhd_has_in_name.h

test_get_chunked_SOURCES = \
  test_get_chunked.c mhd_has_in_name.h

test_get_chunked_close_SOURCES = \
  test_get_chunked.c mhd_has_in_name.h

test_get_chunked_string_SOURCES = \
  test_get_chunked.c mhd_has_in_name.h

test_get_chunked_close_string_SOURCES = \
  test_get_chunked.c mhd_has_in_name.h

test_get_chunked_empty_SOURCES = \
  test_get_chunked.c mhd_has_in_name.h

test_get_chunked_close_empty_SOURCES = \
  test_get_chunked.c mhd_has_in_name.h

test_get_chunked_string_empty_SOURCES = \
  test_get_chunked.c mhd_has_in_name.h

test_get_chunked_close_string_empty_SOURCES = \
  test_get_chunked.c mhd_has_in_name.h

test_get_chunked_sized_SOURCES = \
  test_get_chunked.c mhd_has_in_name.h

test_get_chunked_close_sized_SOURCES = \
  test_get_chunked.c mhd_has_in_name.h

test_get_chunked_empty_sized_SOURCES = \
  test_get_chunked.c mhd_has_in_name.h

test_get_chunked_close_empty_sized_SOURCES = \
  test_get_chunked.c mhd_has_in_name.h

test_get_chunked_forced_SOURCES = \
  test_get_chunked.c mhd_has_in_name.h

test_get_chunked_close_forced_SOURCES = \
  test_get_chunked.c mhd_has_in_name.h

test_get_chunked_empty_forced_SOURCES = \
  test_get_chunked.c mhd_has_in_name.h

test_get_chunked_close_empty_forced_SOURCES = \
  test_get_chunked.c mhd_has_in_name.h

test_post_SOURCES = \
  test_post.c mhd_has_in_name.h

test_process_headers_SOURCES = \
  test_process_headers.c mhd_has_in_name.h

test_parse_cookies_discp_zero_SOURCES = \
  test_parse_cookies.c mhd_has_in_name.h mhd_has_param.h

test_parse_cookies_discp_p2_SOURCES = \
  $(test_parse_cookies_discp_zero_SOURCES)

test_parse_cookies_discp_p1_SOURCES = \
  $(test_parse_cookies_discp_zero_SOURCES)

test_parse_cookies_discp_n2_SOURCES = \
  $(test_parse_cookies_discp_zero_SOURCES)

test_parse_cookies_discp_n3_SOURCES = \
  $(test_parse_cookies_discp_zero_SOURCES)

test_process_arguments_SOURCES = \
  test_process_arguments.c mhd_has_in_name.h

test_postform_SOURCES = \
  test_postform.c mhd_has_in_name.h
test_postform_LDADD = \
  @LIBGCRYPT_LIBS@ $(LDADD)

test_post_loop_SOURCES = \
  test_post_loop.c mhd_has_in_name.h

test_delete_SOURCES = \
  test_delete.c mhd_has_in_name.h

test_patch_SOURCES = \
  test_patch.c mhd_has_in_name.h

test_patch11_SOURCES = \
  test_patch.c mhd_has_in_name.h

test_put_SOURCES = \
  test_put.c mhd_has_in_name.h

test_put_chunked_SOURCES = \
  test_put_chunked.c

test_add_conn_SOURCES = \
  test_add_conn.c mhd_has_in_name.h mhd_has_param.h
test_add_conn_CFLAGS = \
  $(PTHREAD_CFLAGS) $(AM_CFLAGS)
test_add_conn_LDADD = \
  $(PTHREAD_LIBS) $(LDADD)

test_add_conn_nolisten_SOURCES = \
  test_add_conn.c mhd_has_in_name.h mhd_has_param.h
test_add_conn_nolisten_CFLAGS = \
  $(PTHREAD_CFLAGS) $(AM_CFLAGS)
test_add_conn_nolisten_LDADD = \
  $(PTHREAD_LIBS) $(LDADD)

test_add_conn_cleanup_SOURCES = \
  test_add_conn.c mhd_has_in_name.h mhd_has_param.h
test_add_conn_cleanup_CFLAGS = \
  $(PTHREAD_CFLAGS) $(AM_CFLAGS)
test_add_conn_cleanup_LDADD = \
  $(PTHREAD_LIBS) $(LDADD)

test_add_conn_cleanup_nolisten_SOURCES = \
  test_add_conn.c mhd_has_in_name.h mhd_has_param.h
test_add_conn_cleanup_nolisten_CFLAGS = \
  $(PTHREAD_CFLAGS) $(AM_CFLAGS)
test_add_conn_cleanup_nolisten_LDADD = \
  $(PTHREAD_LIBS) $(LDADD)

test_get11_SOURCES = \
  test_get.c mhd_has_in_name.h mhd_has_param.h

test_get_iovec11_SOURCES = \
  test_get_iovec.c mhd_has_in_name.h

test_get_sendfile11_SOURCES = \
  test_get_sendfile.c mhd_has_in_name.h

test_get_close_SOURCES = \
  test_get_close_keep_alive.c mhd_has_in_name.h mhd_has_param.h

test_get_close10_SOURCES = \
  test_get_close_keep_alive.c mhd_has_in_name.h mhd_has_param.h

test_get_keep_alive_SOURCES = \
  test_get_close_keep_alive.c mhd_has_in_name.h mhd_has_param.h

test_get_keep_alive10_SOURCES = \
  test_get_close_keep_alive.c mhd_has_in_name.h mhd_has_param.h

test_post11_SOURCES = \
  test_post.c mhd_has_in_name.h

test_postform11_SOURCES = \
  test_postform.c mhd_has_in_name.h
test_postform11_LDADD = \
  @LIBGCRYPT_LIBS@ $(LDADD)

test_post_loop11_SOURCES = \
  test_post_loop.c mhd_has_in_name.h

test_put11_SOURCES = \
  test_put.c mhd_has_in_name.h

test_large_put_SOURCES = \
  test_large_put.c mhd_has_in_name.h mhd_has_param.h

test_large_put11_SOURCES = \
  test_large_put.c mhd_has_in_name.h mhd_has_param.h

test_large_put_inc11_SOURCES = \
  test_large_put.c mhd_has_in_name.h mhd_has_param.h

test_long_header_SOURCES = \
  test_long_header.c mhd_has_in_name.h

test_long_header11_SOURCES = \
  test_long_header.c mhd_has_in_name.h

test_iplimit11_SOURCES = \
  test_iplimit.c mhd_has_in_name.h

test_termination_SOURCES = \
  test_termination.c

test_timeout_SOURCES = \
  test_timeout.c mhd_has_in_name.h

test_toolarge_method_SOURCES = \
  test_toolarge.c mhd_has_in_name.h mhd_has_param.h

test_toolarge_url_SOURCES = \
  test_toolarge.c mhd_has_in_name.h mhd_has_param.h

test_toolarge_request_header_name_SOURCES = \
  test_toolarge.c mhd_has_in_name.h mhd_has_param.h

test_toolarge_request_header_value_SOURCES = \
  test_toolarge.c mhd_has_in_name.h mhd_has_param.h

test_toolarge_request_headers_SOURCES = \
  test_toolarge.c mhd_has_in_name.h mhd_has_param.h

test_toolarge_reply_header_name_SOURCES = \
  test_toolarge.c mhd_has_in_name.h mhd_has_param.h

test_toolarge_reply_header_value_SOURCES = \
  test_toolarge.c mhd_has_in_name.h mhd_has_param.h

test_toolarge_reply_headers_SOURCES = \
  test_toolarge.c mhd_has_in_name.h mhd_has_param.h

test_tricky_url_SOURCES = \
  test_tricky.c mhd_has_in_name.h mhd_has_param.h

test_tricky_header2_SOURCES = \
  test_tricky.c mhd_has_in_name.h mhd_has_param.h

test_put_broken_len_SOURCES = \
  test_put_broken_len.c mhd_has_in_name.h mhd_has_param.h

test_put_broken_len10_SOURCES = $(test_put_broken_len_SOURCES)

test_put_header_fold_SOURCES = \
  test_put_header_fold.c mhd_has_in_name.h mhd_has_param.h

test_put_large_header_fold_SOURCES = $(test_put_header_fold_SOURCES)

test_get_header_fold_SOURCES = $(test_put_header_fold_SOURCES)

test_put_header_fold_last_SOURCES = $(test_put_header_fold_SOURCES)

test_put_header_fold_large_SOURCES = $(test_put_header_fold_SOURCES)

test_put_header_double_fold_SOURCES = $(test_put_header_fold_SOURCES)

test_put_large_header_double_fold_SOURCES = $(test_put_large_header_fold_SOURCES)

test_get_header_double_fold_SOURCES = $(test_put_header_fold_SOURCES)

test_put_header_double_fold_last_SOURCES = $(test_put_header_fold_last_SOURCES)

test_put_header_double_fold_large_SOURCES = $(test_put_header_fold_large_SOURCES)
