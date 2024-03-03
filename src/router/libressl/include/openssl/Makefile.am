include $(top_srcdir)/Makefile.am.common

if !ENABLE_LIBTLS_ONLY
opensslincludedir=$(includedir)/openssl

BUILT_SOURCES = opensslconf.h
CLEANFILES = opensslconf.h

opensslconf.h: Makefile
	-echo "generating opensslconf.h ..."
if HOST_AARCH64
	-cp $(top_srcdir)/include/arch/aarch64/opensslconf.h opensslconf.h
endif
if HOST_ARM
	-cp $(top_srcdir)/include/arch/arm/opensslconf.h opensslconf.h
endif
if HOST_I386
	-cp $(top_srcdir)/include/arch/i386/opensslconf.h opensslconf.h
endif
if HOST_MIPS
	-cp $(top_srcdir)/include/arch/mips/opensslconf.h opensslconf.h
endif
if HOST_MIPS64
	-cp $(top_srcdir)/include/arch/mips64/opensslconf.h opensslconf.h
endif
if HOST_POWERPC
	-cp $(top_srcdir)/include/arch/powerpc/opensslconf.h opensslconf.h
endif
if HOST_POWERPC64
	-cp $(top_srcdir)/include/arch/powerpc64/opensslconf.h opensslconf.h
endif
if HOST_RISCV64
	-cp $(top_srcdir)/include/arch/riscv64/opensslconf.h opensslconf.h
endif
if HOST_SPARC64
	-cp $(top_srcdir)/include/arch/sparc64/opensslconf.h opensslconf.h
endif
if HOST_X86_64
	-cp $(top_srcdir)/include/arch/amd64/opensslconf.h opensslconf.h
endif

opensslinclude_HEADERS = opensslconf.h
opensslinclude_HEADERS += aes.h
opensslinclude_HEADERS += asn1.h
opensslinclude_HEADERS += asn1t.h
opensslinclude_HEADERS += bio.h
opensslinclude_HEADERS += blowfish.h
opensslinclude_HEADERS += bn.h
opensslinclude_HEADERS += buffer.h
opensslinclude_HEADERS += camellia.h
opensslinclude_HEADERS += cast.h
opensslinclude_HEADERS += chacha.h
opensslinclude_HEADERS += cmac.h
opensslinclude_HEADERS += cms.h
opensslinclude_HEADERS += comp.h
opensslinclude_HEADERS += conf.h
opensslinclude_HEADERS += conf_api.h
opensslinclude_HEADERS += crypto.h
opensslinclude_HEADERS += ct.h
opensslinclude_HEADERS += curve25519.h
opensslinclude_HEADERS += des.h
opensslinclude_HEADERS += dh.h
opensslinclude_HEADERS += dsa.h
opensslinclude_HEADERS += dtls1.h
opensslinclude_HEADERS += ec.h
opensslinclude_HEADERS += ecdh.h
opensslinclude_HEADERS += ecdsa.h
opensslinclude_HEADERS += engine.h
opensslinclude_HEADERS += err.h
opensslinclude_HEADERS += evp.h
opensslinclude_HEADERS += gost.h
opensslinclude_HEADERS += hkdf.h
opensslinclude_HEADERS += hmac.h
opensslinclude_HEADERS += idea.h
opensslinclude_HEADERS += kdf.h
opensslinclude_HEADERS += lhash.h
opensslinclude_HEADERS += md4.h
opensslinclude_HEADERS += md5.h
opensslinclude_HEADERS += modes.h
opensslinclude_HEADERS += obj_mac.h
opensslinclude_HEADERS += objects.h
opensslinclude_HEADERS += ocsp.h
opensslinclude_HEADERS += opensslfeatures.h
opensslinclude_HEADERS += opensslv.h
opensslinclude_HEADERS += ossl_typ.h
opensslinclude_HEADERS += pem.h
opensslinclude_HEADERS += pem2.h
opensslinclude_HEADERS += pkcs12.h
opensslinclude_HEADERS += pkcs7.h
opensslinclude_HEADERS += poly1305.h
opensslinclude_HEADERS += rand.h
opensslinclude_HEADERS += rc2.h
opensslinclude_HEADERS += rc4.h
opensslinclude_HEADERS += ripemd.h
opensslinclude_HEADERS += rsa.h
opensslinclude_HEADERS += safestack.h
opensslinclude_HEADERS += sha.h
opensslinclude_HEADERS += sm3.h
opensslinclude_HEADERS += sm4.h
opensslinclude_HEADERS += srtp.h
opensslinclude_HEADERS += ssl.h
opensslinclude_HEADERS += ssl2.h
opensslinclude_HEADERS += ssl23.h
opensslinclude_HEADERS += ssl3.h
opensslinclude_HEADERS += stack.h
opensslinclude_HEADERS += tls1.h
opensslinclude_HEADERS += ts.h
opensslinclude_HEADERS += txt_db.h
opensslinclude_HEADERS += ui.h
opensslinclude_HEADERS += ui_compat.h
opensslinclude_HEADERS += whrlpool.h
opensslinclude_HEADERS += x509.h
opensslinclude_HEADERS += x509_vfy.h
opensslinclude_HEADERS += x509v3.h
endif
