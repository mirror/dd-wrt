## Process this file with automake to produce Makefile.in

noinst_PROGRAMS = ciphertest aestest
ciphertest_SOURCES = cipher_test.c
ciphertest_LDADD = ../lib/libmcrypt.la $(LIBLTDL) 
aestest_SOURCES = aes_test.c
aestest_LDADD = ../lib/libmcrypt.la $(LIBLTDL) 
TESTS = ciphertest aestest
