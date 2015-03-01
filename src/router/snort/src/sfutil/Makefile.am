AUTOMAKE_OPTIONS=foreign no-dependencies

noinst_LIBRARIES = libsfutil.a

if HAVE_INTEL_SOFT_CPM
INTEL_SOFT_CPM_SOURCES = intel-soft-cpm.c intel-soft-cpm.h
endif

if BUILD_OPENSSL_MD5
OPENSSL_MD5 = \
    md5.c md5.h 
endif

if BUILD_OPENSSL_SHA
OPENSSL_SHA = \
    sha2.c sha2.h
endif

libsfutil_a_SOURCES = \
    sfghash.c sfghash.h \
    sfhashfcn.c sfhashfcn.h \
    sflsq.c sflsq.h \
    sfmemcap.c sfmemcap.h \
    sfthd.c sfthd.h \
    sfxhash.c sfxhash.h \
    ipobj.c ipobj.h \
    getopt_long.c getopt.h getopt1.h \
    acsmx.c acsmx.h \
    acsmx2.c acsmx2.h \
    sfksearch.c sfksearch.h \
    bnfa_search.c bnfa_search.h \
    mpse.c mpse.h \
    bitop.h bitop_funcs.h \
    util_math.c util_math.h \
    util_net.c util_net.h \
    util_str.c util_str.h \
    util_utf.c util_utf.h \
    util_jsnorm.c util_jsnorm.h \
    util_unfold.c util_unfold.h \
    asn1.c asn1.h \
    sfeventq.c sfeventq.h \
    sfsnprintfappend.c sfsnprintfappend.h \
    sfrt.c sfrt.h sfrt_trie.h sfrt_dir.c sfrt_dir.h \
    sfrt_flat.c sfrt_flat.h sfrt_flat_dir.c sfrt_flat_dir.h \
    segment_mem.c segment_mem.h \
    sfportobject.c sfportobject.h \
    sfrim.c  sfrim.h \
    sfprimetable.c sfprimetable.h \
    sf_ip.c sf_ip.h \
    sf_ipvar.c sf_ipvar.h \
    sf_vartable.c sf_vartable.h \
    sf_iph.c sf_iph.h \
    sf_textlog.c sf_textlog.h \
    sf_sechash.c sf_sechash.h \
    sfPolicy.c sfPolicy.h \
    sfPolicyUserData.c sfPolicyUserData.h \
    sfPolicyData.h \
    sfActionQueue.c sfActionQueue.h \
    sfrf.c sfrf.h \
    strvec.c strvec.h \
    sf_email_attach_decode.c sf_email_attach_decode.h \
    sf_base64decode.c sf_base64decode.h \
    Unified2_common.h \
    sf_seqnums.h \
	mpse_methods.h \
	sfdebug.h \
    $(INTEL_SOFT_CPM_SOURCES) \
    $(OPENSSL_MD5) \
    $(OPENSSL_SHA)

INCLUDES = @INCLUDES@

