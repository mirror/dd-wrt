#! /usr/bin/env perl

package configdata;

use strict;
use warnings;

use Exporter;
#use vars qw(@ISA @EXPORT);
our @ISA = qw(Exporter);
our @EXPORT = qw(%config %target %disabled %withargs %unified_info @disablables);

our %config = (
  AR => "powerpc-linux-uclibc-ar",
  ARFLAGS => [ "r" ],
  CC => "ccache powerpc-linux-uclibc-gcc",
  CFLAGS => [ "-Os -pipe -funit-at-a-time -mcpu=e300c3 -mtune=e300c3 -fno-caller-saves -mhard-float", "-Os", "-pipe", "-funit-at-a-time", "-mcpu=e300c3", "-mtune=e300c3", "-fno-caller-saves", "-mhard-float", "-ffunction-sections", "-fdata-sections" ],
  CPPDEFINES => [ "NDEBUG", "_GNU_SOURCE", "OPENSSL_SMALL_FOOTPRINT" ],
  CPPFLAGS => [  ],
  CPPINCLUDES => [  ],
  CXX => "ccache powerpc-linux-uclibc-g++",
  CXXFLAGS => [ "-Wall", "-Os", "-pipe", "-funit-at-a-time", "-mcpu=e300c3", "-mtune=e300c3", "-fno-caller-saves", "-mhard-float", "-ffunction-sections", "-fdata-sections" ],
  HASHBANGPERL => "/usr/bin/env perl",
  LDFLAGS => [ "-Wl,--gc-sections", "-L/home/seg/DEV/mpc83xx/src/router/libucontext" ],
  LDLIBS => [ "-ldl", "-lrt", "-lucontext" ],
  PERL => "/usr/bin/perl",
  RANLIB => "powerpc-linux-uclibc-ranlib",
  RC => "windres",
  RCFLAGS => [  ],
  afalgeng => "",
  b32 => "1",
  b64 => "0",
  b64l => "0",
  bn_ll => "1",
  build_file => "Makefile",
  build_file_templates => [ "Configurations/common0.tmpl", "Configurations/unix-Makefile.tmpl", "Configurations/common.tmpl" ],
  build_infos => [ "./build.info", "crypto/build.info", "ssl/build.info", "engines/build.info", "apps/build.info", "test/build.info", "util/build.info", "tools/build.info", "fuzz/build.info", "crypto/objects/build.info", "crypto/md4/build.info", "crypto/md5/build.info", "crypto/sha/build.info", "crypto/hmac/build.info", "crypto/ripemd/build.info", "crypto/poly1305/build.info", "crypto/blake2/build.info", "crypto/des/build.info", "crypto/aes/build.info", "crypto/rc4/build.info", "crypto/bf/build.info", "crypto/cast/build.info", "crypto/chacha/build.info", "crypto/modes/build.info", "crypto/bn/build.info", "crypto/ec/build.info", "crypto/rsa/build.info", "crypto/dsa/build.info", "crypto/dh/build.info", "crypto/dso/build.info", "crypto/engine/build.info", "crypto/buffer/build.info", "crypto/bio/build.info", "crypto/stack/build.info", "crypto/lhash/build.info", "crypto/rand/build.info", "crypto/err/build.info", "crypto/evp/build.info", "crypto/asn1/build.info", "crypto/pem/build.info", "crypto/x509/build.info", "crypto/x509v3/build.info", "crypto/conf/build.info", "crypto/txt_db/build.info", "crypto/pkcs7/build.info", "crypto/pkcs12/build.info", "crypto/comp/build.info", "crypto/ocsp/build.info", "crypto/ui/build.info", "crypto/cms/build.info", "crypto/ts/build.info", "crypto/srp/build.info", "crypto/cmac/build.info", "crypto/ct/build.info", "crypto/async/build.info", "crypto/kdf/build.info", "crypto/store/build.info", "test/ossl_shim/build.info" ],
  build_type => "release",
  builddir => ".",
  cflags => [ "-Wa,--noexecstack" ],
  conf_files => [ "Configurations/00-base-templates.conf", "Configurations/10-main.conf" ],
  cppflags => [  ],
  cxxflags => [  ],
  defines => [ "NDEBUG" ],
  dirs => [ "crypto", "ssl", "engines", "apps", "test", "util", "tools", "fuzz" ],
  dynamic_engines => "1",
  engdirs => [ "afalg" ],
  ex_libs => [  ],
  export_var_as_fn => "0",
  includes => [  ],
  lflags => [  ],
  lib_defines => [ "OPENSSL_PIC" ],
  libdir => "/usr/lib",
  major => "1",
  makedepprog => "\$(CROSS_COMPILE)ccache powerpc-linux-uclibc-gcc",
  minor => "1.1",
  openssl_algorithm_defines => [ "OPENSSL_NO_ARIA", "OPENSSL_NO_CAMELLIA", "OPENSSL_NO_IDEA", "OPENSSL_NO_MD2", "OPENSSL_NO_MDC2", "OPENSSL_NO_RC2", "OPENSSL_NO_RC5", "OPENSSL_NO_SEED", "OPENSSL_NO_SIPHASH", "OPENSSL_NO_SM2", "OPENSSL_NO_SM3", "OPENSSL_NO_SM4", "OPENSSL_NO_WHIRLPOOL" ],
  openssl_api_defines => [  ],
  openssl_other_defines => [ "OPENSSL_RAND_SEED_OS", "OPENSSL_NO_ASAN", "OPENSSL_NO_CRYPTO_MDEBUG", "OPENSSL_NO_CRYPTO_MDEBUG_BACKTRACE", "OPENSSL_NO_DEVCRYPTOENG", "OPENSSL_NO_EC2M", "OPENSSL_NO_EC_NISTP_64_GCC_128", "OPENSSL_NO_EGD", "OPENSSL_NO_ERR", "OPENSSL_NO_EXTERNAL_TESTS", "OPENSSL_NO_FUZZ_AFL", "OPENSSL_NO_FUZZ_LIBFUZZER", "OPENSSL_NO_GOST", "OPENSSL_NO_HEARTBEATS", "OPENSSL_NO_MSAN", "OPENSSL_NO_SCTP", "OPENSSL_NO_SSL_TRACE", "OPENSSL_NO_SSL3", "OPENSSL_NO_TESTS", "OPENSSL_NO_UBSAN", "OPENSSL_NO_UNIT_TEST", "OPENSSL_NO_WEAK_SSL_CIPHERS", "OPENSSL_NO_STATIC_ENGINE" ],
  openssl_sys_defines => [  ],
  openssl_thread_defines => [ "OPENSSL_THREADS" ],
  openssldir => "/etc/ssl",
  options => "--prefix=/usr --libdir=/usr/lib --openssldir=/etc/ssl -Os -pipe -funit-at-a-time -mcpu=e300c3 -mtune=e300c3 -fno-caller-saves -mhard-float -ffunction-sections -fdata-sections -Wl,--gc-sections -DNDEBUG -D_GNU_SOURCE -ldl -lrt -L/home/seg/DEV/mpc83xx/src/router/libucontext -lucontext enable-threads enable-ssl3-method -DOPENSSL_SMALL_FOOTPRINT no-aria no-asan no-buildtest-c++ no-camellia no-crypto-mdebug no-crypto-mdebug-backtrace no-devcryptoeng no-ec2m no-ec_nistp_64_gcc_128 no-egd no-err no-external-tests no-fuzz-afl no-fuzz-libfuzzer no-gost no-heartbeats no-idea no-md2 no-mdc2 no-msan no-rc2 no-rc5 no-sctp no-seed no-siphash no-sm2 no-sm3 no-sm4 no-ssl-trace no-ssl3 no-tests no-ubsan no-unit-test no-weak-ssl-ciphers no-whirlpool no-zlib no-zlib-dynamic",
  perl_archname => "x86_64-linux-thread-multi",
  perl_cmd => "/usr/bin/perl",
  perl_version => "5.26.1",
  perlargv => [ "linux-generic32", "--prefix=/usr", "--libdir=/usr/lib", "--openssldir=/etc/ssl", "-Os", "-pipe", "-funit-at-a-time", "-mcpu=e300c3", "-mtune=e300c3", "-fno-caller-saves", "-mhard-float", "-ffunction-sections", "-fdata-sections", "-Wl,--gc-sections", "-DNDEBUG", "-D_GNU_SOURCE", "-ldl", "-lrt", "-L/home/seg/DEV/mpc83xx/src/router/libucontext", "-lucontext", "no-idea", "no-md2", "no-mdc2", "no-rc5", "no-camellia", "no-whirlpool", "no-seed", "-no-gost", "no-ssl3", "no-heartbeats", "no-rc2", "no-weak-ssl-ciphers", "no-zlib", "no-aria", "no-devcryptoeng", "no-siphash", "no-sm2", "no-sm3", "no-sm4", "no-tests", "no-external-tests", "no-err", "threads", "no-ssl2", "enable-ssl3-method", "no-ec2m", "no-heartbeats", "no-egd", "no-devcryptoeng", "-DOPENSSL_SMALL_FOOTPRINT" ],
  perlenv => {
      "AR" => "powerpc-linux-uclibc-ar",
      "BUILDFILE" => undef,
      "CC" => "ccache powerpc-linux-uclibc-gcc",
      "CFLAGS" => "-Os -pipe -funit-at-a-time -mcpu=e300c3 -mtune=e300c3 -fno-caller-saves -mhard-float",
      "CPPFLAGS" => undef,
      "CROSS_COMPILE" => "",
      "CXX" => "ccache powerpc-linux-uclibc-g++",
      "CXXFLAGS" => undef,
      "HASHBANGPERL" => undef,
      "LDFLAGS" => undef,
      "LDLIBS" => undef,
      "OPENSSL_LOCAL_CONFIG_DIR" => undef,
      "PERL" => undef,
      "RANLIB" => "powerpc-linux-uclibc-ranlib",
      "RC" => undef,
      "RCFLAGS" => undef,
      "WINDRES" => undef,
      "__CNF_CFLAGS" => undef,
      "__CNF_CPPDEFINES" => undef,
      "__CNF_CPPFLAGS" => undef,
      "__CNF_CPPINCLUDES" => undef,
      "__CNF_CXXFLAGS" => undef,
      "__CNF_LDFLAGS" => undef,
      "__CNF_LDLIBS" => undef,
  },
  prefix => "/usr",
  processor => "",
  rc4_int => "unsigned char",
  sdirs => [ "objects", "md4", "md5", "sha", "hmac", "ripemd", "poly1305", "blake2", "des", "aes", "rc4", "bf", "cast", "chacha", "modes", "bn", "ec", "rsa", "dsa", "dh", "dso", "engine", "buffer", "bio", "stack", "lhash", "rand", "err", "evp", "asn1", "pem", "x509", "x509v3", "conf", "txt_db", "pkcs7", "pkcs12", "comp", "ocsp", "ui", "cms", "ts", "srp", "cmac", "ct", "async", "kdf", "store" ],
  shlib_major => "1",
  shlib_minor => "1",
  shlib_version_history => "",
  shlib_version_number => "1.1",
  sourcedir => ".",
  target => "linux-generic32",
  tdirs => [ "ossl_shim" ],
  version => "1.1.1n",
  version_num => "0x101010efL",
);

our %target = (
  AR => "ar",
  ARFLAGS => "r",
  CC => "gcc",
  CFLAGS => "-Wall",
  CXX => "g++",
  CXXFLAGS => "-Wall",
  HASHBANGPERL => "/usr/bin/env perl",
  RANLIB => "ranlib",
  RC => "windres",
  _conf_fname_int => [ "Configurations/00-base-templates.conf", "Configurations/00-base-templates.conf", "Configurations/10-main.conf", "Configurations/shared-info.pl" ],
  aes_asm_src => "aes_core.c aes_cbc.c",
  aes_obj => "aes_core.o aes_cbc.o",
  apps_aux_src => "",
  apps_init_src => "",
  apps_obj => "",
  bf_asm_src => "bf_enc.c",
  bf_obj => "bf_enc.o",
  bn_asm_src => "bn_asm.c",
  bn_obj => "bn_asm.o",
  bn_ops => "BN_LLONG RC4_CHAR",
  build_file => "Makefile",
  build_scheme => [ "unified", "unix" ],
  cast_asm_src => "c_enc.c",
  cast_obj => "c_enc.o",
  cflags => "-pthread",
  chacha_asm_src => "chacha_enc.c",
  chacha_obj => "chacha_enc.o",
  cmll_asm_src => "camellia.c cmll_misc.c cmll_cbc.c",
  cmll_obj => "camellia.o cmll_misc.o cmll_cbc.o",
  cppflags => "",
  cpuid_asm_src => "mem_clr.c",
  cpuid_obj => "mem_clr.o",
  cxxflags => "-std=c++11 -pthread",
  defines => [  ],
  des_asm_src => "des_enc.c fcrypt_b.c",
  des_obj => "des_enc.o fcrypt_b.o",
  disable => [  ],
  dso_extension => ".so",
  dso_scheme => "dlfcn",
  ec_asm_src => "",
  ec_obj => "",
  enable => [ "afalgeng" ],
  ex_libs => "-ldl -pthread",
  exe_extension => "",
  includes => [  ],
  keccak1600_asm_src => "keccak1600.c",
  keccak1600_obj => "keccak1600.o",
  lflags => "",
  lib_cflags => "",
  lib_cppflags => "-DOPENSSL_USE_NODELETE",
  lib_defines => [  ],
  md5_asm_src => "",
  md5_obj => "",
  modes_asm_src => "",
  modes_obj => "",
  module_cflags => "-fPIC",
  module_cxxflags => "",
  module_ldflags => "-Wl,-znodelete -shared -Wl,-Bsymbolic",
  padlock_asm_src => "",
  padlock_obj => "",
  poly1305_asm_src => "",
  poly1305_obj => "",
  rc4_asm_src => "rc4_enc.c rc4_skey.c",
  rc4_obj => "rc4_enc.o rc4_skey.o",
  rc5_asm_src => "rc5_enc.c",
  rc5_obj => "rc5_enc.o",
  rmd160_asm_src => "",
  rmd160_obj => "",
  shared_cflag => "-fPIC",
  shared_defflag => "-Wl,--version-script=",
  shared_defines => [  ],
  shared_extension => ".so.\$(SHLIB_VERSION_NUMBER)",
  shared_extension_simple => ".so",
  shared_ldflag => "-Wl,-znodelete -shared -Wl,-Bsymbolic",
  shared_rcflag => "",
  shared_sonameflag => "-Wl,-soname=",
  shared_target => "linux-shared",
  template => "1",
  thread_defines => [  ],
  thread_scheme => "pthreads",
  unistd => "<unistd.h>",
  uplink_aux_src => "",
  uplink_obj => "",
  wp_asm_src => "wp_block.c",
  wp_obj => "wp_block.o",
);

our %available_protocols = (
  tls => [ "ssl3", "tls1", "tls1_1", "tls1_2", "tls1_3" ],
  dtls => [ "dtls1", "dtls1_2" ],
);

our @disablables = (
  "afalgeng",
  "aria",
  "asan",
  "asm",
  "async",
  "autoalginit",
  "autoerrinit",
  "autoload-config",
  "bf",
  "blake2",
  "buildtest-c\\+\\+",
  "camellia",
  "capieng",
  "cast",
  "chacha",
  "cmac",
  "cms",
  "comp",
  "crypto-mdebug",
  "crypto-mdebug-backtrace",
  "ct",
  "deprecated",
  "des",
  "devcryptoeng",
  "dgram",
  "dh",
  "dsa",
  "dso",
  "dtls",
  "dynamic-engine",
  "ec",
  "ec2m",
  "ecdh",
  "ecdsa",
  "ec_nistp_64_gcc_128",
  "egd",
  "engine",
  "err",
  "external-tests",
  "filenames",
  "fuzz-libfuzzer",
  "fuzz-afl",
  "gost",
  "heartbeats",
  "hw(-.+)?",
  "idea",
  "makedepend",
  "md2",
  "md4",
  "mdc2",
  "msan",
  "multiblock",
  "nextprotoneg",
  "pinshared",
  "ocb",
  "ocsp",
  "pic",
  "poly1305",
  "posix-io",
  "psk",
  "rc2",
  "rc4",
  "rc5",
  "rdrand",
  "rfc3779",
  "rmd160",
  "scrypt",
  "sctp",
  "seed",
  "shared",
  "siphash",
  "sm2",
  "sm3",
  "sm4",
  "sock",
  "srp",
  "srtp",
  "sse2",
  "ssl",
  "ssl-trace",
  "static-engine",
  "stdio",
  "tests",
  "threads",
  "tls",
  "ts",
  "ubsan",
  "ui-console",
  "unit-test",
  "whirlpool",
  "weak-ssl-ciphers",
  "zlib",
  "zlib-dynamic",
  "ssl3",
  "ssl3-method",
  "tls1",
  "tls1-method",
  "tls1_1",
  "tls1_1-method",
  "tls1_2",
  "tls1_2-method",
  "tls1_3",
  "dtls1",
  "dtls1-method",
  "dtls1_2",
  "dtls1_2-method",
);

our %disabled = (
  "aria" => "option",
  "asan" => "default",
  "buildtest-c++" => "default",
  "camellia" => "option",
  "crypto-mdebug" => "default",
  "crypto-mdebug-backtrace" => "default",
  "devcryptoeng" => "option",
  "ec2m" => "option",
  "ec_nistp_64_gcc_128" => "default",
  "egd" => "option",
  "err" => "option",
  "external-tests" => "option",
  "fuzz-afl" => "default",
  "fuzz-libfuzzer" => "default",
  "gost" => "option",
  "heartbeats" => "option",
  "idea" => "option",
  "md2" => "option",
  "mdc2" => "option",
  "msan" => "default",
  "rc2" => "option",
  "rc5" => "option",
  "sctp" => "default",
  "seed" => "option",
  "siphash" => "option",
  "sm2" => "option",
  "sm3" => "option",
  "sm4" => "option",
  "ssl-trace" => "default",
  "ssl3" => "option",
  "tests" => "option",
  "ubsan" => "default",
  "unit-test" => "default",
  "weak-ssl-ciphers" => "option",
  "whirlpool" => "option",
  "zlib" => "option",
  "zlib-dynamic" => "default",
);

our %withargs = (
);

our %unified_info = (
    "depends" =>
        {
            "" =>
                [
                    "include/crypto/bn_conf.h",
                    "include/crypto/dso_conf.h",
                    "include/openssl/opensslconf.h",
                ],
            "apps/asn1pars.o" =>
                [
                    "apps/progs.h",
                ],
            "apps/ca.o" =>
                [
                    "apps/progs.h",
                ],
            "apps/ciphers.o" =>
                [
                    "apps/progs.h",
                ],
            "apps/cms.o" =>
                [
                    "apps/progs.h",
                ],
            "apps/crl.o" =>
                [
                    "apps/progs.h",
                ],
            "apps/crl2p7.o" =>
                [
                    "apps/progs.h",
                ],
            "apps/dgst.o" =>
                [
                    "apps/progs.h",
                ],
            "apps/dhparam.o" =>
                [
                    "apps/progs.h",
                ],
            "apps/dsa.o" =>
                [
                    "apps/progs.h",
                ],
            "apps/dsaparam.o" =>
                [
                    "apps/progs.h",
                ],
            "apps/ec.o" =>
                [
                    "apps/progs.h",
                ],
            "apps/ecparam.o" =>
                [
                    "apps/progs.h",
                ],
            "apps/enc.o" =>
                [
                    "apps/progs.h",
                ],
            "apps/engine.o" =>
                [
                    "apps/progs.h",
                ],
            "apps/errstr.o" =>
                [
                    "apps/progs.h",
                ],
            "apps/gendsa.o" =>
                [
                    "apps/progs.h",
                ],
            "apps/genpkey.o" =>
                [
                    "apps/progs.h",
                ],
            "apps/genrsa.o" =>
                [
                    "apps/progs.h",
                ],
            "apps/nseq.o" =>
                [
                    "apps/progs.h",
                ],
            "apps/ocsp.o" =>
                [
                    "apps/progs.h",
                ],
            "apps/openssl" =>
                [
                    "apps/libapps.a",
                    "libssl",
                ],
            "apps/openssl.o" =>
                [
                    "apps/progs.h",
                ],
            "apps/passwd.o" =>
                [
                    "apps/progs.h",
                ],
            "apps/pkcs12.o" =>
                [
                    "apps/progs.h",
                ],
            "apps/pkcs7.o" =>
                [
                    "apps/progs.h",
                ],
            "apps/pkcs8.o" =>
                [
                    "apps/progs.h",
                ],
            "apps/pkey.o" =>
                [
                    "apps/progs.h",
                ],
            "apps/pkeyparam.o" =>
                [
                    "apps/progs.h",
                ],
            "apps/pkeyutl.o" =>
                [
                    "apps/progs.h",
                ],
            "apps/prime.o" =>
                [
                    "apps/progs.h",
                ],
            "apps/progs.h" =>
                [
                    "configdata.pm",
                ],
            "apps/rand.o" =>
                [
                    "apps/progs.h",
                ],
            "apps/rehash.o" =>
                [
                    "apps/progs.h",
                ],
            "apps/req.o" =>
                [
                    "apps/progs.h",
                ],
            "apps/rsa.o" =>
                [
                    "apps/progs.h",
                ],
            "apps/rsautl.o" =>
                [
                    "apps/progs.h",
                ],
            "apps/s_client.o" =>
                [
                    "apps/progs.h",
                ],
            "apps/s_server.o" =>
                [
                    "apps/progs.h",
                ],
            "apps/s_time.o" =>
                [
                    "apps/progs.h",
                ],
            "apps/sess_id.o" =>
                [
                    "apps/progs.h",
                ],
            "apps/smime.o" =>
                [
                    "apps/progs.h",
                ],
            "apps/speed.o" =>
                [
                    "apps/progs.h",
                ],
            "apps/spkac.o" =>
                [
                    "apps/progs.h",
                ],
            "apps/srp.o" =>
                [
                    "apps/progs.h",
                ],
            "apps/storeutl.o" =>
                [
                    "apps/progs.h",
                ],
            "apps/ts.o" =>
                [
                    "apps/progs.h",
                ],
            "apps/verify.o" =>
                [
                    "apps/progs.h",
                ],
            "apps/version.o" =>
                [
                    "apps/progs.h",
                ],
            "apps/x509.o" =>
                [
                    "apps/progs.h",
                ],
            "crypto/aes/aes-586.s" =>
                [
                    "crypto/perlasm/x86asm.pl",
                ],
            "crypto/aes/aesni-586.s" =>
                [
                    "crypto/perlasm/x86asm.pl",
                ],
            "crypto/aes/aest4-sparcv9.S" =>
                [
                    "crypto/perlasm/sparcv9_modes.pl",
                ],
            "crypto/aes/vpaes-586.s" =>
                [
                    "crypto/perlasm/x86asm.pl",
                ],
            "crypto/bf/bf-586.s" =>
                [
                    "crypto/perlasm/cbc.pl",
                    "crypto/perlasm/x86asm.pl",
                ],
            "crypto/bn/bn-586.s" =>
                [
                    "crypto/perlasm/x86asm.pl",
                ],
            "crypto/bn/co-586.s" =>
                [
                    "crypto/perlasm/x86asm.pl",
                ],
            "crypto/bn/x86-gf2m.s" =>
                [
                    "crypto/perlasm/x86asm.pl",
                ],
            "crypto/bn/x86-mont.s" =>
                [
                    "crypto/perlasm/x86asm.pl",
                ],
            "crypto/buildinf.h" =>
                [
                    "configdata.pm",
                ],
            "crypto/cast/cast-586.s" =>
                [
                    "crypto/perlasm/cbc.pl",
                    "crypto/perlasm/x86asm.pl",
                ],
            "crypto/cversion.o" =>
                [
                    "crypto/buildinf.h",
                ],
            "crypto/des/crypt586.s" =>
                [
                    "crypto/perlasm/cbc.pl",
                    "crypto/perlasm/x86asm.pl",
                ],
            "crypto/des/des-586.s" =>
                [
                    "crypto/perlasm/cbc.pl",
                    "crypto/perlasm/x86asm.pl",
                ],
            "crypto/rc4/rc4-586.s" =>
                [
                    "crypto/perlasm/x86asm.pl",
                ],
            "crypto/ripemd/rmd-586.s" =>
                [
                    "crypto/perlasm/x86asm.pl",
                ],
            "crypto/sha/sha1-586.s" =>
                [
                    "crypto/perlasm/x86asm.pl",
                ],
            "crypto/sha/sha256-586.s" =>
                [
                    "crypto/perlasm/x86asm.pl",
                ],
            "crypto/sha/sha512-586.s" =>
                [
                    "crypto/perlasm/x86asm.pl",
                ],
            "crypto/x86cpuid.s" =>
                [
                    "crypto/perlasm/x86asm.pl",
                ],
            "engines/afalg" =>
                [
                    "libcrypto",
                ],
            "engines/capi" =>
                [
                    "libcrypto",
                ],
            "engines/dasync" =>
                [
                    "libcrypto",
                ],
            "engines/ossltest" =>
                [
                    "libcrypto",
                ],
            "engines/padlock" =>
                [
                    "libcrypto",
                ],
            "include/crypto/bn_conf.h" =>
                [
                    "configdata.pm",
                ],
            "include/crypto/dso_conf.h" =>
                [
                    "configdata.pm",
                ],
            "include/openssl/opensslconf.h" =>
                [
                    "configdata.pm",
                ],
            "libcrypto.map" =>
                [
                    "util/libcrypto.num",
                ],
            "libssl" =>
                [
                    "libcrypto",
                ],
            "libssl.map" =>
                [
                    "util/libssl.num",
                ],
        },
    "dirinfo" =>
        {
            "apps" =>
                {
                    "products" =>
                        {
                            "bin" =>
                                [
                                    "apps/openssl",
                                ],
                            "lib" =>
                                [
                                    "apps/libapps.a",
                                ],
                            "script" =>
                                [
                                    "apps/CA.pl",
                                    "apps/tsget.pl",
                                ],
                        },
                },
            "crypto" =>
                {
                    "deps" =>
                        [
                            "crypto/cpt_err.o",
                            "crypto/cryptlib.o",
                            "crypto/ctype.o",
                            "crypto/cversion.o",
                            "crypto/ebcdic.o",
                            "crypto/ex_data.o",
                            "crypto/getenv.o",
                            "crypto/init.o",
                            "crypto/mem.o",
                            "crypto/mem_clr.o",
                            "crypto/mem_dbg.o",
                            "crypto/mem_sec.o",
                            "crypto/o_dir.o",
                            "crypto/o_fips.o",
                            "crypto/o_fopen.o",
                            "crypto/o_init.o",
                            "crypto/o_str.o",
                            "crypto/o_time.o",
                            "crypto/threads_none.o",
                            "crypto/threads_pthread.o",
                            "crypto/threads_win.o",
                            "crypto/uid.o",
                        ],
                    "products" =>
                        {
                            "lib" =>
                                [
                                    "libcrypto",
                                ],
                        },
                },
            "crypto/aes" =>
                {
                    "deps" =>
                        [
                            "crypto/aes/aes_cbc.o",
                            "crypto/aes/aes_cfb.o",
                            "crypto/aes/aes_core.o",
                            "crypto/aes/aes_ecb.o",
                            "crypto/aes/aes_ige.o",
                            "crypto/aes/aes_misc.o",
                            "crypto/aes/aes_ofb.o",
                            "crypto/aes/aes_wrap.o",
                        ],
                    "products" =>
                        {
                            "lib" =>
                                [
                                    "libcrypto",
                                ],
                        },
                },
            "crypto/asn1" =>
                {
                    "deps" =>
                        [
                            "crypto/asn1/a_bitstr.o",
                            "crypto/asn1/a_d2i_fp.o",
                            "crypto/asn1/a_digest.o",
                            "crypto/asn1/a_dup.o",
                            "crypto/asn1/a_gentm.o",
                            "crypto/asn1/a_i2d_fp.o",
                            "crypto/asn1/a_int.o",
                            "crypto/asn1/a_mbstr.o",
                            "crypto/asn1/a_object.o",
                            "crypto/asn1/a_octet.o",
                            "crypto/asn1/a_print.o",
                            "crypto/asn1/a_sign.o",
                            "crypto/asn1/a_strex.o",
                            "crypto/asn1/a_strnid.o",
                            "crypto/asn1/a_time.o",
                            "crypto/asn1/a_type.o",
                            "crypto/asn1/a_utctm.o",
                            "crypto/asn1/a_utf8.o",
                            "crypto/asn1/a_verify.o",
                            "crypto/asn1/ameth_lib.o",
                            "crypto/asn1/asn1_err.o",
                            "crypto/asn1/asn1_gen.o",
                            "crypto/asn1/asn1_item_list.o",
                            "crypto/asn1/asn1_lib.o",
                            "crypto/asn1/asn1_par.o",
                            "crypto/asn1/asn_mime.o",
                            "crypto/asn1/asn_moid.o",
                            "crypto/asn1/asn_mstbl.o",
                            "crypto/asn1/asn_pack.o",
                            "crypto/asn1/bio_asn1.o",
                            "crypto/asn1/bio_ndef.o",
                            "crypto/asn1/d2i_pr.o",
                            "crypto/asn1/d2i_pu.o",
                            "crypto/asn1/evp_asn1.o",
                            "crypto/asn1/f_int.o",
                            "crypto/asn1/f_string.o",
                            "crypto/asn1/i2d_pr.o",
                            "crypto/asn1/i2d_pu.o",
                            "crypto/asn1/n_pkey.o",
                            "crypto/asn1/nsseq.o",
                            "crypto/asn1/p5_pbe.o",
                            "crypto/asn1/p5_pbev2.o",
                            "crypto/asn1/p5_scrypt.o",
                            "crypto/asn1/p8_pkey.o",
                            "crypto/asn1/t_bitst.o",
                            "crypto/asn1/t_pkey.o",
                            "crypto/asn1/t_spki.o",
                            "crypto/asn1/tasn_dec.o",
                            "crypto/asn1/tasn_enc.o",
                            "crypto/asn1/tasn_fre.o",
                            "crypto/asn1/tasn_new.o",
                            "crypto/asn1/tasn_prn.o",
                            "crypto/asn1/tasn_scn.o",
                            "crypto/asn1/tasn_typ.o",
                            "crypto/asn1/tasn_utl.o",
                            "crypto/asn1/x_algor.o",
                            "crypto/asn1/x_bignum.o",
                            "crypto/asn1/x_info.o",
                            "crypto/asn1/x_int64.o",
                            "crypto/asn1/x_long.o",
                            "crypto/asn1/x_pkey.o",
                            "crypto/asn1/x_sig.o",
                            "crypto/asn1/x_spki.o",
                            "crypto/asn1/x_val.o",
                        ],
                    "products" =>
                        {
                            "lib" =>
                                [
                                    "libcrypto",
                                ],
                        },
                },
            "crypto/async" =>
                {
                    "deps" =>
                        [
                            "crypto/async/async.o",
                            "crypto/async/async_err.o",
                            "crypto/async/async_wait.o",
                        ],
                    "products" =>
                        {
                            "lib" =>
                                [
                                    "libcrypto",
                                ],
                        },
                },
            "crypto/async/arch" =>
                {
                    "deps" =>
                        [
                            "crypto/async/arch/async_null.o",
                            "crypto/async/arch/async_posix.o",
                            "crypto/async/arch/async_win.o",
                        ],
                    "products" =>
                        {
                            "lib" =>
                                [
                                    "libcrypto",
                                ],
                        },
                },
            "crypto/bf" =>
                {
                    "deps" =>
                        [
                            "crypto/bf/bf_cfb64.o",
                            "crypto/bf/bf_ecb.o",
                            "crypto/bf/bf_enc.o",
                            "crypto/bf/bf_ofb64.o",
                            "crypto/bf/bf_skey.o",
                        ],
                    "products" =>
                        {
                            "lib" =>
                                [
                                    "libcrypto",
                                ],
                        },
                },
            "crypto/bio" =>
                {
                    "deps" =>
                        [
                            "crypto/bio/b_addr.o",
                            "crypto/bio/b_dump.o",
                            "crypto/bio/b_print.o",
                            "crypto/bio/b_sock.o",
                            "crypto/bio/b_sock2.o",
                            "crypto/bio/bf_buff.o",
                            "crypto/bio/bf_lbuf.o",
                            "crypto/bio/bf_nbio.o",
                            "crypto/bio/bf_null.o",
                            "crypto/bio/bio_cb.o",
                            "crypto/bio/bio_err.o",
                            "crypto/bio/bio_lib.o",
                            "crypto/bio/bio_meth.o",
                            "crypto/bio/bss_acpt.o",
                            "crypto/bio/bss_bio.o",
                            "crypto/bio/bss_conn.o",
                            "crypto/bio/bss_dgram.o",
                            "crypto/bio/bss_fd.o",
                            "crypto/bio/bss_file.o",
                            "crypto/bio/bss_log.o",
                            "crypto/bio/bss_mem.o",
                            "crypto/bio/bss_null.o",
                            "crypto/bio/bss_sock.o",
                        ],
                    "products" =>
                        {
                            "lib" =>
                                [
                                    "libcrypto",
                                ],
                        },
                },
            "crypto/blake2" =>
                {
                    "deps" =>
                        [
                            "crypto/blake2/blake2b.o",
                            "crypto/blake2/blake2s.o",
                            "crypto/blake2/m_blake2b.o",
                            "crypto/blake2/m_blake2s.o",
                        ],
                    "products" =>
                        {
                            "lib" =>
                                [
                                    "libcrypto",
                                ],
                        },
                },
            "crypto/bn" =>
                {
                    "deps" =>
                        [
                            "crypto/bn/bn_add.o",
                            "crypto/bn/bn_asm.o",
                            "crypto/bn/bn_blind.o",
                            "crypto/bn/bn_const.o",
                            "crypto/bn/bn_ctx.o",
                            "crypto/bn/bn_depr.o",
                            "crypto/bn/bn_dh.o",
                            "crypto/bn/bn_div.o",
                            "crypto/bn/bn_err.o",
                            "crypto/bn/bn_exp.o",
                            "crypto/bn/bn_exp2.o",
                            "crypto/bn/bn_gcd.o",
                            "crypto/bn/bn_gf2m.o",
                            "crypto/bn/bn_intern.o",
                            "crypto/bn/bn_kron.o",
                            "crypto/bn/bn_lib.o",
                            "crypto/bn/bn_mod.o",
                            "crypto/bn/bn_mont.o",
                            "crypto/bn/bn_mpi.o",
                            "crypto/bn/bn_mul.o",
                            "crypto/bn/bn_nist.o",
                            "crypto/bn/bn_prime.o",
                            "crypto/bn/bn_print.o",
                            "crypto/bn/bn_rand.o",
                            "crypto/bn/bn_recp.o",
                            "crypto/bn/bn_shift.o",
                            "crypto/bn/bn_sqr.o",
                            "crypto/bn/bn_sqrt.o",
                            "crypto/bn/bn_srp.o",
                            "crypto/bn/bn_word.o",
                            "crypto/bn/bn_x931p.o",
                        ],
                    "products" =>
                        {
                            "lib" =>
                                [
                                    "libcrypto",
                                ],
                        },
                },
            "crypto/buffer" =>
                {
                    "deps" =>
                        [
                            "crypto/buffer/buf_err.o",
                            "crypto/buffer/buffer.o",
                        ],
                    "products" =>
                        {
                            "lib" =>
                                [
                                    "libcrypto",
                                ],
                        },
                },
            "crypto/cast" =>
                {
                    "deps" =>
                        [
                            "crypto/cast/c_cfb64.o",
                            "crypto/cast/c_ecb.o",
                            "crypto/cast/c_enc.o",
                            "crypto/cast/c_ofb64.o",
                            "crypto/cast/c_skey.o",
                        ],
                    "products" =>
                        {
                            "lib" =>
                                [
                                    "libcrypto",
                                ],
                        },
                },
            "crypto/chacha" =>
                {
                    "deps" =>
                        [
                            "crypto/chacha/chacha_enc.o",
                        ],
                    "products" =>
                        {
                            "lib" =>
                                [
                                    "libcrypto",
                                ],
                        },
                },
            "crypto/cmac" =>
                {
                    "deps" =>
                        [
                            "crypto/cmac/cm_ameth.o",
                            "crypto/cmac/cm_pmeth.o",
                            "crypto/cmac/cmac.o",
                        ],
                    "products" =>
                        {
                            "lib" =>
                                [
                                    "libcrypto",
                                ],
                        },
                },
            "crypto/cms" =>
                {
                    "deps" =>
                        [
                            "crypto/cms/cms_asn1.o",
                            "crypto/cms/cms_att.o",
                            "crypto/cms/cms_cd.o",
                            "crypto/cms/cms_dd.o",
                            "crypto/cms/cms_enc.o",
                            "crypto/cms/cms_env.o",
                            "crypto/cms/cms_err.o",
                            "crypto/cms/cms_ess.o",
                            "crypto/cms/cms_io.o",
                            "crypto/cms/cms_kari.o",
                            "crypto/cms/cms_lib.o",
                            "crypto/cms/cms_pwri.o",
                            "crypto/cms/cms_sd.o",
                            "crypto/cms/cms_smime.o",
                        ],
                    "products" =>
                        {
                            "lib" =>
                                [
                                    "libcrypto",
                                ],
                        },
                },
            "crypto/comp" =>
                {
                    "deps" =>
                        [
                            "crypto/comp/c_zlib.o",
                            "crypto/comp/comp_err.o",
                            "crypto/comp/comp_lib.o",
                        ],
                    "products" =>
                        {
                            "lib" =>
                                [
                                    "libcrypto",
                                ],
                        },
                },
            "crypto/conf" =>
                {
                    "deps" =>
                        [
                            "crypto/conf/conf_api.o",
                            "crypto/conf/conf_def.o",
                            "crypto/conf/conf_err.o",
                            "crypto/conf/conf_lib.o",
                            "crypto/conf/conf_mall.o",
                            "crypto/conf/conf_mod.o",
                            "crypto/conf/conf_sap.o",
                            "crypto/conf/conf_ssl.o",
                        ],
                    "products" =>
                        {
                            "lib" =>
                                [
                                    "libcrypto",
                                ],
                        },
                },
            "crypto/ct" =>
                {
                    "deps" =>
                        [
                            "crypto/ct/ct_b64.o",
                            "crypto/ct/ct_err.o",
                            "crypto/ct/ct_log.o",
                            "crypto/ct/ct_oct.o",
                            "crypto/ct/ct_policy.o",
                            "crypto/ct/ct_prn.o",
                            "crypto/ct/ct_sct.o",
                            "crypto/ct/ct_sct_ctx.o",
                            "crypto/ct/ct_vfy.o",
                            "crypto/ct/ct_x509v3.o",
                        ],
                    "products" =>
                        {
                            "lib" =>
                                [
                                    "libcrypto",
                                ],
                        },
                },
            "crypto/des" =>
                {
                    "deps" =>
                        [
                            "crypto/des/cbc_cksm.o",
                            "crypto/des/cbc_enc.o",
                            "crypto/des/cfb64ede.o",
                            "crypto/des/cfb64enc.o",
                            "crypto/des/cfb_enc.o",
                            "crypto/des/des_enc.o",
                            "crypto/des/ecb3_enc.o",
                            "crypto/des/ecb_enc.o",
                            "crypto/des/fcrypt.o",
                            "crypto/des/fcrypt_b.o",
                            "crypto/des/ofb64ede.o",
                            "crypto/des/ofb64enc.o",
                            "crypto/des/ofb_enc.o",
                            "crypto/des/pcbc_enc.o",
                            "crypto/des/qud_cksm.o",
                            "crypto/des/rand_key.o",
                            "crypto/des/set_key.o",
                            "crypto/des/str2key.o",
                            "crypto/des/xcbc_enc.o",
                        ],
                    "products" =>
                        {
                            "lib" =>
                                [
                                    "libcrypto",
                                ],
                        },
                },
            "crypto/dh" =>
                {
                    "deps" =>
                        [
                            "crypto/dh/dh_ameth.o",
                            "crypto/dh/dh_asn1.o",
                            "crypto/dh/dh_check.o",
                            "crypto/dh/dh_depr.o",
                            "crypto/dh/dh_err.o",
                            "crypto/dh/dh_gen.o",
                            "crypto/dh/dh_kdf.o",
                            "crypto/dh/dh_key.o",
                            "crypto/dh/dh_lib.o",
                            "crypto/dh/dh_meth.o",
                            "crypto/dh/dh_pmeth.o",
                            "crypto/dh/dh_prn.o",
                            "crypto/dh/dh_rfc5114.o",
                            "crypto/dh/dh_rfc7919.o",
                        ],
                    "products" =>
                        {
                            "lib" =>
                                [
                                    "libcrypto",
                                ],
                        },
                },
            "crypto/dsa" =>
                {
                    "deps" =>
                        [
                            "crypto/dsa/dsa_ameth.o",
                            "crypto/dsa/dsa_asn1.o",
                            "crypto/dsa/dsa_depr.o",
                            "crypto/dsa/dsa_err.o",
                            "crypto/dsa/dsa_gen.o",
                            "crypto/dsa/dsa_key.o",
                            "crypto/dsa/dsa_lib.o",
                            "crypto/dsa/dsa_meth.o",
                            "crypto/dsa/dsa_ossl.o",
                            "crypto/dsa/dsa_pmeth.o",
                            "crypto/dsa/dsa_prn.o",
                            "crypto/dsa/dsa_sign.o",
                            "crypto/dsa/dsa_vrf.o",
                        ],
                    "products" =>
                        {
                            "lib" =>
                                [
                                    "libcrypto",
                                ],
                        },
                },
            "crypto/dso" =>
                {
                    "deps" =>
                        [
                            "crypto/dso/dso_dl.o",
                            "crypto/dso/dso_dlfcn.o",
                            "crypto/dso/dso_err.o",
                            "crypto/dso/dso_lib.o",
                            "crypto/dso/dso_openssl.o",
                            "crypto/dso/dso_vms.o",
                            "crypto/dso/dso_win32.o",
                        ],
                    "products" =>
                        {
                            "lib" =>
                                [
                                    "libcrypto",
                                ],
                        },
                },
            "crypto/ec" =>
                {
                    "deps" =>
                        [
                            "crypto/ec/curve25519.o",
                            "crypto/ec/ec2_oct.o",
                            "crypto/ec/ec2_smpl.o",
                            "crypto/ec/ec_ameth.o",
                            "crypto/ec/ec_asn1.o",
                            "crypto/ec/ec_check.o",
                            "crypto/ec/ec_curve.o",
                            "crypto/ec/ec_cvt.o",
                            "crypto/ec/ec_err.o",
                            "crypto/ec/ec_key.o",
                            "crypto/ec/ec_kmeth.o",
                            "crypto/ec/ec_lib.o",
                            "crypto/ec/ec_mult.o",
                            "crypto/ec/ec_oct.o",
                            "crypto/ec/ec_pmeth.o",
                            "crypto/ec/ec_print.o",
                            "crypto/ec/ecdh_kdf.o",
                            "crypto/ec/ecdh_ossl.o",
                            "crypto/ec/ecdsa_ossl.o",
                            "crypto/ec/ecdsa_sign.o",
                            "crypto/ec/ecdsa_vrf.o",
                            "crypto/ec/eck_prn.o",
                            "crypto/ec/ecp_mont.o",
                            "crypto/ec/ecp_nist.o",
                            "crypto/ec/ecp_nistp224.o",
                            "crypto/ec/ecp_nistp256.o",
                            "crypto/ec/ecp_nistp521.o",
                            "crypto/ec/ecp_nistputil.o",
                            "crypto/ec/ecp_oct.o",
                            "crypto/ec/ecp_smpl.o",
                            "crypto/ec/ecx_meth.o",
                        ],
                    "products" =>
                        {
                            "lib" =>
                                [
                                    "libcrypto",
                                ],
                        },
                },
            "crypto/ec/curve448" =>
                {
                    "deps" =>
                        [
                            "crypto/ec/curve448/curve448.o",
                            "crypto/ec/curve448/curve448_tables.o",
                            "crypto/ec/curve448/eddsa.o",
                            "crypto/ec/curve448/f_generic.o",
                            "crypto/ec/curve448/scalar.o",
                        ],
                    "products" =>
                        {
                            "lib" =>
                                [
                                    "libcrypto",
                                ],
                        },
                },
            "crypto/ec/curve448/arch_32" =>
                {
                    "deps" =>
                        [
                            "crypto/ec/curve448/arch_32/f_impl.o",
                        ],
                    "products" =>
                        {
                            "lib" =>
                                [
                                    "libcrypto",
                                ],
                        },
                },
            "crypto/engine" =>
                {
                    "deps" =>
                        [
                            "crypto/engine/eng_all.o",
                            "crypto/engine/eng_cnf.o",
                            "crypto/engine/eng_ctrl.o",
                            "crypto/engine/eng_dyn.o",
                            "crypto/engine/eng_err.o",
                            "crypto/engine/eng_fat.o",
                            "crypto/engine/eng_init.o",
                            "crypto/engine/eng_lib.o",
                            "crypto/engine/eng_list.o",
                            "crypto/engine/eng_openssl.o",
                            "crypto/engine/eng_pkey.o",
                            "crypto/engine/eng_rdrand.o",
                            "crypto/engine/eng_table.o",
                            "crypto/engine/tb_asnmth.o",
                            "crypto/engine/tb_cipher.o",
                            "crypto/engine/tb_dh.o",
                            "crypto/engine/tb_digest.o",
                            "crypto/engine/tb_dsa.o",
                            "crypto/engine/tb_eckey.o",
                            "crypto/engine/tb_pkmeth.o",
                            "crypto/engine/tb_rand.o",
                            "crypto/engine/tb_rsa.o",
                        ],
                    "products" =>
                        {
                            "lib" =>
                                [
                                    "libcrypto",
                                ],
                        },
                },
            "crypto/err" =>
                {
                    "deps" =>
                        [
                            "crypto/err/err.o",
                            "crypto/err/err_all.o",
                            "crypto/err/err_prn.o",
                        ],
                    "products" =>
                        {
                            "lib" =>
                                [
                                    "libcrypto",
                                ],
                        },
                },
            "crypto/evp" =>
                {
                    "deps" =>
                        [
                            "crypto/evp/bio_b64.o",
                            "crypto/evp/bio_enc.o",
                            "crypto/evp/bio_md.o",
                            "crypto/evp/bio_ok.o",
                            "crypto/evp/c_allc.o",
                            "crypto/evp/c_alld.o",
                            "crypto/evp/cmeth_lib.o",
                            "crypto/evp/digest.o",
                            "crypto/evp/e_aes.o",
                            "crypto/evp/e_aes_cbc_hmac_sha1.o",
                            "crypto/evp/e_aes_cbc_hmac_sha256.o",
                            "crypto/evp/e_aria.o",
                            "crypto/evp/e_bf.o",
                            "crypto/evp/e_camellia.o",
                            "crypto/evp/e_cast.o",
                            "crypto/evp/e_chacha20_poly1305.o",
                            "crypto/evp/e_des.o",
                            "crypto/evp/e_des3.o",
                            "crypto/evp/e_idea.o",
                            "crypto/evp/e_null.o",
                            "crypto/evp/e_old.o",
                            "crypto/evp/e_rc2.o",
                            "crypto/evp/e_rc4.o",
                            "crypto/evp/e_rc4_hmac_md5.o",
                            "crypto/evp/e_rc5.o",
                            "crypto/evp/e_seed.o",
                            "crypto/evp/e_sm4.o",
                            "crypto/evp/e_xcbc_d.o",
                            "crypto/evp/encode.o",
                            "crypto/evp/evp_cnf.o",
                            "crypto/evp/evp_enc.o",
                            "crypto/evp/evp_err.o",
                            "crypto/evp/evp_key.o",
                            "crypto/evp/evp_lib.o",
                            "crypto/evp/evp_pbe.o",
                            "crypto/evp/evp_pkey.o",
                            "crypto/evp/m_md2.o",
                            "crypto/evp/m_md4.o",
                            "crypto/evp/m_md5.o",
                            "crypto/evp/m_md5_sha1.o",
                            "crypto/evp/m_mdc2.o",
                            "crypto/evp/m_null.o",
                            "crypto/evp/m_ripemd.o",
                            "crypto/evp/m_sha1.o",
                            "crypto/evp/m_sha3.o",
                            "crypto/evp/m_sigver.o",
                            "crypto/evp/m_wp.o",
                            "crypto/evp/names.o",
                            "crypto/evp/p5_crpt.o",
                            "crypto/evp/p5_crpt2.o",
                            "crypto/evp/p_dec.o",
                            "crypto/evp/p_enc.o",
                            "crypto/evp/p_lib.o",
                            "crypto/evp/p_open.o",
                            "crypto/evp/p_seal.o",
                            "crypto/evp/p_sign.o",
                            "crypto/evp/p_verify.o",
                            "crypto/evp/pbe_scrypt.o",
                            "crypto/evp/pmeth_fn.o",
                            "crypto/evp/pmeth_gn.o",
                            "crypto/evp/pmeth_lib.o",
                        ],
                    "products" =>
                        {
                            "lib" =>
                                [
                                    "libcrypto",
                                ],
                        },
                },
            "crypto/hmac" =>
                {
                    "deps" =>
                        [
                            "crypto/hmac/hm_ameth.o",
                            "crypto/hmac/hm_pmeth.o",
                            "crypto/hmac/hmac.o",
                        ],
                    "products" =>
                        {
                            "lib" =>
                                [
                                    "libcrypto",
                                ],
                        },
                },
            "crypto/kdf" =>
                {
                    "deps" =>
                        [
                            "crypto/kdf/hkdf.o",
                            "crypto/kdf/kdf_err.o",
                            "crypto/kdf/scrypt.o",
                            "crypto/kdf/tls1_prf.o",
                        ],
                    "products" =>
                        {
                            "lib" =>
                                [
                                    "libcrypto",
                                ],
                        },
                },
            "crypto/lhash" =>
                {
                    "deps" =>
                        [
                            "crypto/lhash/lh_stats.o",
                            "crypto/lhash/lhash.o",
                        ],
                    "products" =>
                        {
                            "lib" =>
                                [
                                    "libcrypto",
                                ],
                        },
                },
            "crypto/md4" =>
                {
                    "deps" =>
                        [
                            "crypto/md4/md4_dgst.o",
                            "crypto/md4/md4_one.o",
                        ],
                    "products" =>
                        {
                            "lib" =>
                                [
                                    "libcrypto",
                                ],
                        },
                },
            "crypto/md5" =>
                {
                    "deps" =>
                        [
                            "crypto/md5/md5_dgst.o",
                            "crypto/md5/md5_one.o",
                        ],
                    "products" =>
                        {
                            "lib" =>
                                [
                                    "libcrypto",
                                ],
                        },
                },
            "crypto/modes" =>
                {
                    "deps" =>
                        [
                            "crypto/modes/cbc128.o",
                            "crypto/modes/ccm128.o",
                            "crypto/modes/cfb128.o",
                            "crypto/modes/ctr128.o",
                            "crypto/modes/cts128.o",
                            "crypto/modes/gcm128.o",
                            "crypto/modes/ocb128.o",
                            "crypto/modes/ofb128.o",
                            "crypto/modes/wrap128.o",
                            "crypto/modes/xts128.o",
                        ],
                    "products" =>
                        {
                            "lib" =>
                                [
                                    "libcrypto",
                                ],
                        },
                },
            "crypto/objects" =>
                {
                    "deps" =>
                        [
                            "crypto/objects/o_names.o",
                            "crypto/objects/obj_dat.o",
                            "crypto/objects/obj_err.o",
                            "crypto/objects/obj_lib.o",
                            "crypto/objects/obj_xref.o",
                        ],
                    "products" =>
                        {
                            "lib" =>
                                [
                                    "libcrypto",
                                ],
                        },
                },
            "crypto/ocsp" =>
                {
                    "deps" =>
                        [
                            "crypto/ocsp/ocsp_asn.o",
                            "crypto/ocsp/ocsp_cl.o",
                            "crypto/ocsp/ocsp_err.o",
                            "crypto/ocsp/ocsp_ext.o",
                            "crypto/ocsp/ocsp_ht.o",
                            "crypto/ocsp/ocsp_lib.o",
                            "crypto/ocsp/ocsp_prn.o",
                            "crypto/ocsp/ocsp_srv.o",
                            "crypto/ocsp/ocsp_vfy.o",
                            "crypto/ocsp/v3_ocsp.o",
                        ],
                    "products" =>
                        {
                            "lib" =>
                                [
                                    "libcrypto",
                                ],
                        },
                },
            "crypto/pem" =>
                {
                    "deps" =>
                        [
                            "crypto/pem/pem_all.o",
                            "crypto/pem/pem_err.o",
                            "crypto/pem/pem_info.o",
                            "crypto/pem/pem_lib.o",
                            "crypto/pem/pem_oth.o",
                            "crypto/pem/pem_pk8.o",
                            "crypto/pem/pem_pkey.o",
                            "crypto/pem/pem_sign.o",
                            "crypto/pem/pem_x509.o",
                            "crypto/pem/pem_xaux.o",
                            "crypto/pem/pvkfmt.o",
                        ],
                    "products" =>
                        {
                            "lib" =>
                                [
                                    "libcrypto",
                                ],
                        },
                },
            "crypto/pkcs12" =>
                {
                    "deps" =>
                        [
                            "crypto/pkcs12/p12_add.o",
                            "crypto/pkcs12/p12_asn.o",
                            "crypto/pkcs12/p12_attr.o",
                            "crypto/pkcs12/p12_crpt.o",
                            "crypto/pkcs12/p12_crt.o",
                            "crypto/pkcs12/p12_decr.o",
                            "crypto/pkcs12/p12_init.o",
                            "crypto/pkcs12/p12_key.o",
                            "crypto/pkcs12/p12_kiss.o",
                            "crypto/pkcs12/p12_mutl.o",
                            "crypto/pkcs12/p12_npas.o",
                            "crypto/pkcs12/p12_p8d.o",
                            "crypto/pkcs12/p12_p8e.o",
                            "crypto/pkcs12/p12_sbag.o",
                            "crypto/pkcs12/p12_utl.o",
                            "crypto/pkcs12/pk12err.o",
                        ],
                    "products" =>
                        {
                            "lib" =>
                                [
                                    "libcrypto",
                                ],
                        },
                },
            "crypto/pkcs7" =>
                {
                    "deps" =>
                        [
                            "crypto/pkcs7/bio_pk7.o",
                            "crypto/pkcs7/pk7_asn1.o",
                            "crypto/pkcs7/pk7_attr.o",
                            "crypto/pkcs7/pk7_doit.o",
                            "crypto/pkcs7/pk7_lib.o",
                            "crypto/pkcs7/pk7_mime.o",
                            "crypto/pkcs7/pk7_smime.o",
                            "crypto/pkcs7/pkcs7err.o",
                        ],
                    "products" =>
                        {
                            "lib" =>
                                [
                                    "libcrypto",
                                ],
                        },
                },
            "crypto/poly1305" =>
                {
                    "deps" =>
                        [
                            "crypto/poly1305/poly1305.o",
                            "crypto/poly1305/poly1305_ameth.o",
                            "crypto/poly1305/poly1305_pmeth.o",
                        ],
                    "products" =>
                        {
                            "lib" =>
                                [
                                    "libcrypto",
                                ],
                        },
                },
            "crypto/rand" =>
                {
                    "deps" =>
                        [
                            "crypto/rand/drbg_ctr.o",
                            "crypto/rand/drbg_lib.o",
                            "crypto/rand/rand_egd.o",
                            "crypto/rand/rand_err.o",
                            "crypto/rand/rand_lib.o",
                            "crypto/rand/rand_unix.o",
                            "crypto/rand/rand_vms.o",
                            "crypto/rand/rand_win.o",
                            "crypto/rand/randfile.o",
                        ],
                    "products" =>
                        {
                            "lib" =>
                                [
                                    "libcrypto",
                                ],
                        },
                },
            "crypto/rc4" =>
                {
                    "deps" =>
                        [
                            "crypto/rc4/rc4_enc.o",
                            "crypto/rc4/rc4_skey.o",
                        ],
                    "products" =>
                        {
                            "lib" =>
                                [
                                    "libcrypto",
                                ],
                        },
                },
            "crypto/ripemd" =>
                {
                    "deps" =>
                        [
                            "crypto/ripemd/rmd_dgst.o",
                            "crypto/ripemd/rmd_one.o",
                        ],
                    "products" =>
                        {
                            "lib" =>
                                [
                                    "libcrypto",
                                ],
                        },
                },
            "crypto/rsa" =>
                {
                    "deps" =>
                        [
                            "crypto/rsa/rsa_ameth.o",
                            "crypto/rsa/rsa_asn1.o",
                            "crypto/rsa/rsa_chk.o",
                            "crypto/rsa/rsa_crpt.o",
                            "crypto/rsa/rsa_depr.o",
                            "crypto/rsa/rsa_err.o",
                            "crypto/rsa/rsa_gen.o",
                            "crypto/rsa/rsa_lib.o",
                            "crypto/rsa/rsa_meth.o",
                            "crypto/rsa/rsa_mp.o",
                            "crypto/rsa/rsa_none.o",
                            "crypto/rsa/rsa_oaep.o",
                            "crypto/rsa/rsa_ossl.o",
                            "crypto/rsa/rsa_pk1.o",
                            "crypto/rsa/rsa_pmeth.o",
                            "crypto/rsa/rsa_prn.o",
                            "crypto/rsa/rsa_pss.o",
                            "crypto/rsa/rsa_saos.o",
                            "crypto/rsa/rsa_sign.o",
                            "crypto/rsa/rsa_ssl.o",
                            "crypto/rsa/rsa_x931.o",
                            "crypto/rsa/rsa_x931g.o",
                        ],
                    "products" =>
                        {
                            "lib" =>
                                [
                                    "libcrypto",
                                ],
                        },
                },
            "crypto/sha" =>
                {
                    "deps" =>
                        [
                            "crypto/sha/keccak1600.o",
                            "crypto/sha/sha1_one.o",
                            "crypto/sha/sha1dgst.o",
                            "crypto/sha/sha256.o",
                            "crypto/sha/sha512.o",
                        ],
                    "products" =>
                        {
                            "lib" =>
                                [
                                    "libcrypto",
                                ],
                        },
                },
            "crypto/srp" =>
                {
                    "deps" =>
                        [
                            "crypto/srp/srp_lib.o",
                            "crypto/srp/srp_vfy.o",
                        ],
                    "products" =>
                        {
                            "lib" =>
                                [
                                    "libcrypto",
                                ],
                        },
                },
            "crypto/stack" =>
                {
                    "deps" =>
                        [
                            "crypto/stack/stack.o",
                        ],
                    "products" =>
                        {
                            "lib" =>
                                [
                                    "libcrypto",
                                ],
                        },
                },
            "crypto/store" =>
                {
                    "deps" =>
                        [
                            "crypto/store/loader_file.o",
                            "crypto/store/store_err.o",
                            "crypto/store/store_init.o",
                            "crypto/store/store_lib.o",
                            "crypto/store/store_register.o",
                            "crypto/store/store_strings.o",
                        ],
                    "products" =>
                        {
                            "lib" =>
                                [
                                    "libcrypto",
                                ],
                        },
                },
            "crypto/ts" =>
                {
                    "deps" =>
                        [
                            "crypto/ts/ts_asn1.o",
                            "crypto/ts/ts_conf.o",
                            "crypto/ts/ts_err.o",
                            "crypto/ts/ts_lib.o",
                            "crypto/ts/ts_req_print.o",
                            "crypto/ts/ts_req_utils.o",
                            "crypto/ts/ts_rsp_print.o",
                            "crypto/ts/ts_rsp_sign.o",
                            "crypto/ts/ts_rsp_utils.o",
                            "crypto/ts/ts_rsp_verify.o",
                            "crypto/ts/ts_verify_ctx.o",
                        ],
                    "products" =>
                        {
                            "lib" =>
                                [
                                    "libcrypto",
                                ],
                        },
                },
            "crypto/txt_db" =>
                {
                    "deps" =>
                        [
                            "crypto/txt_db/txt_db.o",
                        ],
                    "products" =>
                        {
                            "lib" =>
                                [
                                    "libcrypto",
                                ],
                        },
                },
            "crypto/ui" =>
                {
                    "deps" =>
                        [
                            "crypto/ui/ui_err.o",
                            "crypto/ui/ui_lib.o",
                            "crypto/ui/ui_null.o",
                            "crypto/ui/ui_openssl.o",
                            "crypto/ui/ui_util.o",
                        ],
                    "products" =>
                        {
                            "lib" =>
                                [
                                    "libcrypto",
                                ],
                        },
                },
            "crypto/x509" =>
                {
                    "deps" =>
                        [
                            "crypto/x509/by_dir.o",
                            "crypto/x509/by_file.o",
                            "crypto/x509/t_crl.o",
                            "crypto/x509/t_req.o",
                            "crypto/x509/t_x509.o",
                            "crypto/x509/x509_att.o",
                            "crypto/x509/x509_cmp.o",
                            "crypto/x509/x509_d2.o",
                            "crypto/x509/x509_def.o",
                            "crypto/x509/x509_err.o",
                            "crypto/x509/x509_ext.o",
                            "crypto/x509/x509_lu.o",
                            "crypto/x509/x509_meth.o",
                            "crypto/x509/x509_obj.o",
                            "crypto/x509/x509_r2x.o",
                            "crypto/x509/x509_req.o",
                            "crypto/x509/x509_set.o",
                            "crypto/x509/x509_trs.o",
                            "crypto/x509/x509_txt.o",
                            "crypto/x509/x509_v3.o",
                            "crypto/x509/x509_vfy.o",
                            "crypto/x509/x509_vpm.o",
                            "crypto/x509/x509cset.o",
                            "crypto/x509/x509name.o",
                            "crypto/x509/x509rset.o",
                            "crypto/x509/x509spki.o",
                            "crypto/x509/x509type.o",
                            "crypto/x509/x_all.o",
                            "crypto/x509/x_attrib.o",
                            "crypto/x509/x_crl.o",
                            "crypto/x509/x_exten.o",
                            "crypto/x509/x_name.o",
                            "crypto/x509/x_pubkey.o",
                            "crypto/x509/x_req.o",
                            "crypto/x509/x_x509.o",
                            "crypto/x509/x_x509a.o",
                        ],
                    "products" =>
                        {
                            "lib" =>
                                [
                                    "libcrypto",
                                ],
                        },
                },
            "crypto/x509v3" =>
                {
                    "deps" =>
                        [
                            "crypto/x509v3/pcy_cache.o",
                            "crypto/x509v3/pcy_data.o",
                            "crypto/x509v3/pcy_lib.o",
                            "crypto/x509v3/pcy_map.o",
                            "crypto/x509v3/pcy_node.o",
                            "crypto/x509v3/pcy_tree.o",
                            "crypto/x509v3/v3_addr.o",
                            "crypto/x509v3/v3_admis.o",
                            "crypto/x509v3/v3_akey.o",
                            "crypto/x509v3/v3_akeya.o",
                            "crypto/x509v3/v3_alt.o",
                            "crypto/x509v3/v3_asid.o",
                            "crypto/x509v3/v3_bcons.o",
                            "crypto/x509v3/v3_bitst.o",
                            "crypto/x509v3/v3_conf.o",
                            "crypto/x509v3/v3_cpols.o",
                            "crypto/x509v3/v3_crld.o",
                            "crypto/x509v3/v3_enum.o",
                            "crypto/x509v3/v3_extku.o",
                            "crypto/x509v3/v3_genn.o",
                            "crypto/x509v3/v3_ia5.o",
                            "crypto/x509v3/v3_info.o",
                            "crypto/x509v3/v3_int.o",
                            "crypto/x509v3/v3_lib.o",
                            "crypto/x509v3/v3_ncons.o",
                            "crypto/x509v3/v3_pci.o",
                            "crypto/x509v3/v3_pcia.o",
                            "crypto/x509v3/v3_pcons.o",
                            "crypto/x509v3/v3_pku.o",
                            "crypto/x509v3/v3_pmaps.o",
                            "crypto/x509v3/v3_prn.o",
                            "crypto/x509v3/v3_purp.o",
                            "crypto/x509v3/v3_skey.o",
                            "crypto/x509v3/v3_sxnet.o",
                            "crypto/x509v3/v3_tlsf.o",
                            "crypto/x509v3/v3_utl.o",
                            "crypto/x509v3/v3err.o",
                        ],
                    "products" =>
                        {
                            "lib" =>
                                [
                                    "libcrypto",
                                ],
                        },
                },
            "engines" =>
                {
                    "products" =>
                        {
                            "dso" =>
                                [
                                    "engines/afalg",
                                    "engines/capi",
                                    "engines/dasync",
                                    "engines/ossltest",
                                    "engines/padlock",
                                ],
                        },
                },
            "ssl" =>
                {
                    "deps" =>
                        [
                            "ssl/bio_ssl.o",
                            "ssl/d1_lib.o",
                            "ssl/d1_msg.o",
                            "ssl/d1_srtp.o",
                            "ssl/methods.o",
                            "ssl/packet.o",
                            "ssl/pqueue.o",
                            "ssl/s3_cbc.o",
                            "ssl/s3_enc.o",
                            "ssl/s3_lib.o",
                            "ssl/s3_msg.o",
                            "ssl/ssl_asn1.o",
                            "ssl/ssl_cert.o",
                            "ssl/ssl_ciph.o",
                            "ssl/ssl_conf.o",
                            "ssl/ssl_err.o",
                            "ssl/ssl_init.o",
                            "ssl/ssl_lib.o",
                            "ssl/ssl_mcnf.o",
                            "ssl/ssl_rsa.o",
                            "ssl/ssl_sess.o",
                            "ssl/ssl_stat.o",
                            "ssl/ssl_txt.o",
                            "ssl/ssl_utst.o",
                            "ssl/t1_enc.o",
                            "ssl/t1_lib.o",
                            "ssl/t1_trce.o",
                            "ssl/tls13_enc.o",
                            "ssl/tls_srp.o",
                        ],
                    "products" =>
                        {
                            "lib" =>
                                [
                                    "libssl",
                                ],
                        },
                },
            "ssl/record" =>
                {
                    "deps" =>
                        [
                            "ssl/record/dtls1_bitmap.o",
                            "ssl/record/rec_layer_d1.o",
                            "ssl/record/rec_layer_s3.o",
                            "ssl/record/ssl3_buffer.o",
                            "ssl/record/ssl3_record.o",
                            "ssl/record/ssl3_record_tls13.o",
                        ],
                    "products" =>
                        {
                            "lib" =>
                                [
                                    "libssl",
                                ],
                        },
                },
            "ssl/statem" =>
                {
                    "deps" =>
                        [
                            "ssl/statem/extensions.o",
                            "ssl/statem/extensions_clnt.o",
                            "ssl/statem/extensions_cust.o",
                            "ssl/statem/extensions_srvr.o",
                            "ssl/statem/statem.o",
                            "ssl/statem/statem_clnt.o",
                            "ssl/statem/statem_dtls.o",
                            "ssl/statem/statem_lib.o",
                            "ssl/statem/statem_srvr.o",
                        ],
                    "products" =>
                        {
                            "lib" =>
                                [
                                    "libssl",
                                ],
                        },
                },
            "tools" =>
                {
                    "products" =>
                        {
                            "script" =>
                                [
                                    "tools/c_rehash",
                                ],
                        },
                },
            "util" =>
                {
                    "products" =>
                        {
                            "script" =>
                                [
                                    "util/shlib_wrap.sh",
                                ],
                        },
                },
        },
    "engines" =>
        [
            "engines/afalg",
            "engines/capi",
            "engines/dasync",
            "engines/ossltest",
            "engines/padlock",
        ],
    "extra" =>
        [
            "crypto/alphacpuid.pl",
            "crypto/arm64cpuid.pl",
            "crypto/armv4cpuid.pl",
            "crypto/ia64cpuid.S",
            "crypto/pariscid.pl",
            "crypto/ppccpuid.pl",
            "crypto/x86_64cpuid.pl",
            "crypto/x86cpuid.pl",
            "ms/applink.c",
            "ms/uplink-x86.pl",
            "ms/uplink.c",
        ],
    "generate" =>
        {
            "apps/progs.h" =>
                [
                    "apps/progs.pl",
                    "\$(APPS_OPENSSL)",
                ],
            "crypto/aes/aes-586.s" =>
                [
                    "crypto/aes/asm/aes-586.pl",
                    "\$(PERLASM_SCHEME)",
                    "\$(LIB_CFLAGS)",
                    "\$(LIB_CPPFLAGS)",
                    "\$(PROCESSOR)",
                ],
            "crypto/aes/aes-armv4.S" =>
                [
                    "crypto/aes/asm/aes-armv4.pl",
                    "\$(PERLASM_SCHEME)",
                ],
            "crypto/aes/aes-ia64.s" =>
                [
                    "crypto/aes/asm/aes-ia64.S",
                ],
            "crypto/aes/aes-mips.S" =>
                [
                    "crypto/aes/asm/aes-mips.pl",
                    "\$(PERLASM_SCHEME)",
                    "-Icrypto",
                ],
            "crypto/aes/aes-parisc.s" =>
                [
                    "crypto/aes/asm/aes-parisc.pl",
                    "\$(PERLASM_SCHEME)",
                ],
            "crypto/aes/aes-ppc.s" =>
                [
                    "crypto/aes/asm/aes-ppc.pl",
                    "\$(PERLASM_SCHEME)",
                ],
            "crypto/aes/aes-s390x.S" =>
                [
                    "crypto/aes/asm/aes-s390x.pl",
                    "\$(PERLASM_SCHEME)",
                ],
            "crypto/aes/aes-sparcv9.S" =>
                [
                    "crypto/aes/asm/aes-sparcv9.pl",
                    "\$(PERLASM_SCHEME)",
                ],
            "crypto/aes/aes-x86_64.s" =>
                [
                    "crypto/aes/asm/aes-x86_64.pl",
                    "\$(PERLASM_SCHEME)",
                ],
            "crypto/aes/aesfx-sparcv9.S" =>
                [
                    "crypto/aes/asm/aesfx-sparcv9.pl",
                    "\$(PERLASM_SCHEME)",
                ],
            "crypto/aes/aesni-mb-x86_64.s" =>
                [
                    "crypto/aes/asm/aesni-mb-x86_64.pl",
                    "\$(PERLASM_SCHEME)",
                ],
            "crypto/aes/aesni-sha1-x86_64.s" =>
                [
                    "crypto/aes/asm/aesni-sha1-x86_64.pl",
                    "\$(PERLASM_SCHEME)",
                ],
            "crypto/aes/aesni-sha256-x86_64.s" =>
                [
                    "crypto/aes/asm/aesni-sha256-x86_64.pl",
                    "\$(PERLASM_SCHEME)",
                ],
            "crypto/aes/aesni-x86.s" =>
                [
                    "crypto/aes/asm/aesni-x86.pl",
                    "\$(PERLASM_SCHEME)",
                    "\$(LIB_CFLAGS)",
                    "\$(LIB_CPPFLAGS)",
                    "\$(PROCESSOR)",
                ],
            "crypto/aes/aesni-x86_64.s" =>
                [
                    "crypto/aes/asm/aesni-x86_64.pl",
                    "\$(PERLASM_SCHEME)",
                ],
            "crypto/aes/aesp8-ppc.s" =>
                [
                    "crypto/aes/asm/aesp8-ppc.pl",
                    "\$(PERLASM_SCHEME)",
                ],
            "crypto/aes/aest4-sparcv9.S" =>
                [
                    "crypto/aes/asm/aest4-sparcv9.pl",
                    "\$(PERLASM_SCHEME)",
                ],
            "crypto/aes/aesv8-armx.S" =>
                [
                    "crypto/aes/asm/aesv8-armx.pl",
                    "\$(PERLASM_SCHEME)",
                ],
            "crypto/aes/bsaes-armv7.S" =>
                [
                    "crypto/aes/asm/bsaes-armv7.pl",
                    "\$(PERLASM_SCHEME)",
                ],
            "crypto/aes/bsaes-x86_64.s" =>
                [
                    "crypto/aes/asm/bsaes-x86_64.pl",
                    "\$(PERLASM_SCHEME)",
                ],
            "crypto/aes/vpaes-armv8.S" =>
                [
                    "crypto/aes/asm/vpaes-armv8.pl",
                    "\$(PERLASM_SCHEME)",
                ],
            "crypto/aes/vpaes-ppc.s" =>
                [
                    "crypto/aes/asm/vpaes-ppc.pl",
                    "\$(PERLASM_SCHEME)",
                ],
            "crypto/aes/vpaes-x86.s" =>
                [
                    "crypto/aes/asm/vpaes-x86.pl",
                    "\$(PERLASM_SCHEME)",
                    "\$(LIB_CFLAGS)",
                    "\$(LIB_CPPFLAGS)",
                    "\$(PROCESSOR)",
                ],
            "crypto/aes/vpaes-x86_64.s" =>
                [
                    "crypto/aes/asm/vpaes-x86_64.pl",
                    "\$(PERLASM_SCHEME)",
                ],
            "crypto/alphacpuid.s" =>
                [
                    "crypto/alphacpuid.pl",
                ],
            "crypto/arm64cpuid.S" =>
                [
                    "crypto/arm64cpuid.pl",
                    "\$(PERLASM_SCHEME)",
                ],
            "crypto/armv4cpuid.S" =>
                [
                    "crypto/armv4cpuid.pl",
                    "\$(PERLASM_SCHEME)",
                ],
            "crypto/bf/bf-586.s" =>
                [
                    "crypto/bf/asm/bf-586.pl",
                    "\$(PERLASM_SCHEME)",
                    "\$(LIB_CFLAGS)",
                    "\$(LIB_CPPFLAGS)",
                    "\$(PROCESSOR)",
                ],
            "crypto/bn/alpha-mont.S" =>
                [
                    "crypto/bn/asm/alpha-mont.pl",
                    "\$(PERLASM_SCHEME)",
                ],
            "crypto/bn/armv4-gf2m.S" =>
                [
                    "crypto/bn/asm/armv4-gf2m.pl",
                    "\$(PERLASM_SCHEME)",
                ],
            "crypto/bn/armv4-mont.S" =>
                [
                    "crypto/bn/asm/armv4-mont.pl",
                    "\$(PERLASM_SCHEME)",
                ],
            "crypto/bn/armv8-mont.S" =>
                [
                    "crypto/bn/asm/armv8-mont.pl",
                    "\$(PERLASM_SCHEME)",
                ],
            "crypto/bn/bn-586.s" =>
                [
                    "crypto/bn/asm/bn-586.pl",
                    "\$(PERLASM_SCHEME)",
                    "\$(LIB_CFLAGS)",
                    "\$(LIB_CPPFLAGS)",
                    "\$(PROCESSOR)",
                ],
            "crypto/bn/bn-ia64.s" =>
                [
                    "crypto/bn/asm/ia64.S",
                ],
            "crypto/bn/bn-mips.S" =>
                [
                    "crypto/bn/asm/mips.pl",
                    "\$(PERLASM_SCHEME)",
                ],
            "crypto/bn/bn-ppc.s" =>
                [
                    "crypto/bn/asm/ppc.pl",
                    "\$(PERLASM_SCHEME)",
                ],
            "crypto/bn/co-586.s" =>
                [
                    "crypto/bn/asm/co-586.pl",
                    "\$(PERLASM_SCHEME)",
                    "\$(LIB_CFLAGS)",
                    "\$(LIB_CPPFLAGS)",
                    "\$(PROCESSOR)",
                ],
            "crypto/bn/ia64-mont.s" =>
                [
                    "crypto/bn/asm/ia64-mont.pl",
                    "\$(LIB_CFLAGS)",
                    "\$(LIB_CPPFLAGS)",
                ],
            "crypto/bn/mips-mont.S" =>
                [
                    "crypto/bn/asm/mips-mont.pl",
                    "\$(PERLASM_SCHEME)",
                ],
            "crypto/bn/parisc-mont.s" =>
                [
                    "crypto/bn/asm/parisc-mont.pl",
                    "\$(PERLASM_SCHEME)",
                ],
            "crypto/bn/ppc-mont.s" =>
                [
                    "crypto/bn/asm/ppc-mont.pl",
                    "\$(PERLASM_SCHEME)",
                ],
            "crypto/bn/ppc64-mont.s" =>
                [
                    "crypto/bn/asm/ppc64-mont.pl",
                    "\$(PERLASM_SCHEME)",
                ],
            "crypto/bn/rsaz-avx2.s" =>
                [
                    "crypto/bn/asm/rsaz-avx2.pl",
                    "\$(PERLASM_SCHEME)",
                ],
            "crypto/bn/rsaz-x86_64.s" =>
                [
                    "crypto/bn/asm/rsaz-x86_64.pl",
                    "\$(PERLASM_SCHEME)",
                ],
            "crypto/bn/s390x-gf2m.s" =>
                [
                    "crypto/bn/asm/s390x-gf2m.pl",
                    "\$(PERLASM_SCHEME)",
                ],
            "crypto/bn/s390x-mont.S" =>
                [
                    "crypto/bn/asm/s390x-mont.pl",
                    "\$(PERLASM_SCHEME)",
                ],
            "crypto/bn/sparct4-mont.S" =>
                [
                    "crypto/bn/asm/sparct4-mont.pl",
                    "\$(PERLASM_SCHEME)",
                ],
            "crypto/bn/sparcv9-gf2m.S" =>
                [
                    "crypto/bn/asm/sparcv9-gf2m.pl",
                    "\$(PERLASM_SCHEME)",
                ],
            "crypto/bn/sparcv9-mont.S" =>
                [
                    "crypto/bn/asm/sparcv9-mont.pl",
                    "\$(PERLASM_SCHEME)",
                ],
            "crypto/bn/sparcv9a-mont.S" =>
                [
                    "crypto/bn/asm/sparcv9a-mont.pl",
                    "\$(PERLASM_SCHEME)",
                ],
            "crypto/bn/vis3-mont.S" =>
                [
                    "crypto/bn/asm/vis3-mont.pl",
                    "\$(PERLASM_SCHEME)",
                ],
            "crypto/bn/x86-gf2m.s" =>
                [
                    "crypto/bn/asm/x86-gf2m.pl",
                    "\$(PERLASM_SCHEME)",
                    "\$(LIB_CFLAGS)",
                    "\$(LIB_CPPFLAGS)",
                    "\$(PROCESSOR)",
                ],
            "crypto/bn/x86-mont.s" =>
                [
                    "crypto/bn/asm/x86-mont.pl",
                    "\$(PERLASM_SCHEME)",
                    "\$(LIB_CFLAGS)",
                    "\$(LIB_CPPFLAGS)",
                    "\$(PROCESSOR)",
                ],
            "crypto/bn/x86_64-gf2m.s" =>
                [
                    "crypto/bn/asm/x86_64-gf2m.pl",
                    "\$(PERLASM_SCHEME)",
                ],
            "crypto/bn/x86_64-mont.s" =>
                [
                    "crypto/bn/asm/x86_64-mont.pl",
                    "\$(PERLASM_SCHEME)",
                ],
            "crypto/bn/x86_64-mont5.s" =>
                [
                    "crypto/bn/asm/x86_64-mont5.pl",
                    "\$(PERLASM_SCHEME)",
                ],
            "crypto/buildinf.h" =>
                [
                    "util/mkbuildinf.pl",
                    "\"\$(CC)",
                    "\$(LIB_CFLAGS)",
                    "\$(CPPFLAGS_Q)\"",
                    "\"\$(PLATFORM)\"",
                ],
            "crypto/cast/cast-586.s" =>
                [
                    "crypto/cast/asm/cast-586.pl",
                    "\$(PERLASM_SCHEME)",
                    "\$(LIB_CFLAGS)",
                    "\$(LIB_CPPFLAGS)",
                    "\$(PROCESSOR)",
                ],
            "crypto/chacha/chacha-armv4.S" =>
                [
                    "crypto/chacha/asm/chacha-armv4.pl",
                    "\$(PERLASM_SCHEME)",
                ],
            "crypto/chacha/chacha-armv8.S" =>
                [
                    "crypto/chacha/asm/chacha-armv8.pl",
                    "\$(PERLASM_SCHEME)",
                ],
            "crypto/chacha/chacha-ppc.s" =>
                [
                    "crypto/chacha/asm/chacha-ppc.pl",
                    "\$(PERLASM_SCHEME)",
                ],
            "crypto/chacha/chacha-s390x.S" =>
                [
                    "crypto/chacha/asm/chacha-s390x.pl",
                    "\$(PERLASM_SCHEME)",
                ],
            "crypto/chacha/chacha-x86.s" =>
                [
                    "crypto/chacha/asm/chacha-x86.pl",
                    "\$(PERLASM_SCHEME)",
                    "\$(LIB_CFLAGS)",
                    "\$(LIB_CPPFLAGS)",
                    "\$(PROCESSOR)",
                ],
            "crypto/chacha/chacha-x86_64.s" =>
                [
                    "crypto/chacha/asm/chacha-x86_64.pl",
                    "\$(PERLASM_SCHEME)",
                ],
            "crypto/des/crypt586.s" =>
                [
                    "crypto/des/asm/crypt586.pl",
                    "\$(PERLASM_SCHEME)",
                    "\$(LIB_CFLAGS)",
                    "\$(LIB_CPPFLAGS)",
                ],
            "crypto/des/des-586.s" =>
                [
                    "crypto/des/asm/des-586.pl",
                    "\$(PERLASM_SCHEME)",
                    "\$(LIB_CFLAGS)",
                    "\$(LIB_CPPFLAGS)",
                ],
            "crypto/des/des_enc-sparc.S" =>
                [
                    "crypto/des/asm/des_enc.m4",
                ],
            "crypto/des/dest4-sparcv9.S" =>
                [
                    "crypto/des/asm/dest4-sparcv9.pl",
                    "\$(PERLASM_SCHEME)",
                ],
            "crypto/ec/ecp_nistz256-armv4.S" =>
                [
                    "crypto/ec/asm/ecp_nistz256-armv4.pl",
                    "\$(PERLASM_SCHEME)",
                ],
            "crypto/ec/ecp_nistz256-armv8.S" =>
                [
                    "crypto/ec/asm/ecp_nistz256-armv8.pl",
                    "\$(PERLASM_SCHEME)",
                ],
            "crypto/ec/ecp_nistz256-avx2.s" =>
                [
                    "crypto/ec/asm/ecp_nistz256-avx2.pl",
                    "\$(PERLASM_SCHEME)",
                ],
            "crypto/ec/ecp_nistz256-ppc64.s" =>
                [
                    "crypto/ec/asm/ecp_nistz256-ppc64.pl",
                    "\$(PERLASM_SCHEME)",
                ],
            "crypto/ec/ecp_nistz256-sparcv9.S" =>
                [
                    "crypto/ec/asm/ecp_nistz256-sparcv9.pl",
                    "\$(PERLASM_SCHEME)",
                ],
            "crypto/ec/ecp_nistz256-x86.s" =>
                [
                    "crypto/ec/asm/ecp_nistz256-x86.pl",
                    "\$(PERLASM_SCHEME)",
                    "\$(LIB_CFLAGS)",
                    "\$(LIB_CPPFLAGS)",
                    "\$(PROCESSOR)",
                ],
            "crypto/ec/ecp_nistz256-x86_64.s" =>
                [
                    "crypto/ec/asm/ecp_nistz256-x86_64.pl",
                    "\$(PERLASM_SCHEME)",
                ],
            "crypto/ec/x25519-ppc64.s" =>
                [
                    "crypto/ec/asm/x25519-ppc64.pl",
                    "\$(PERLASM_SCHEME)",
                ],
            "crypto/ec/x25519-x86_64.s" =>
                [
                    "crypto/ec/asm/x25519-x86_64.pl",
                    "\$(PERLASM_SCHEME)",
                ],
            "crypto/ia64cpuid.s" =>
                [
                    "crypto/ia64cpuid.S",
                ],
            "crypto/md5/md5-586.s" =>
                [
                    "crypto/md5/asm/md5-586.pl",
                    "\$(PERLASM_SCHEME)",
                    "\$(LIB_CFLAGS)",
                    "\$(LIB_CPPFLAGS)",
                ],
            "crypto/md5/md5-sparcv9.S" =>
                [
                    "crypto/md5/asm/md5-sparcv9.pl",
                    "\$(PERLASM_SCHEME)",
                ],
            "crypto/md5/md5-x86_64.s" =>
                [
                    "crypto/md5/asm/md5-x86_64.pl",
                    "\$(PERLASM_SCHEME)",
                ],
            "crypto/modes/aesni-gcm-x86_64.s" =>
                [
                    "crypto/modes/asm/aesni-gcm-x86_64.pl",
                    "\$(PERLASM_SCHEME)",
                ],
            "crypto/modes/ghash-alpha.S" =>
                [
                    "crypto/modes/asm/ghash-alpha.pl",
                    "\$(PERLASM_SCHEME)",
                ],
            "crypto/modes/ghash-armv4.S" =>
                [
                    "crypto/modes/asm/ghash-armv4.pl",
                    "\$(PERLASM_SCHEME)",
                ],
            "crypto/modes/ghash-ia64.s" =>
                [
                    "crypto/modes/asm/ghash-ia64.pl",
                    "\$(LIB_CFLAGS)",
                    "\$(LIB_CPPFLAGS)",
                ],
            "crypto/modes/ghash-parisc.s" =>
                [
                    "crypto/modes/asm/ghash-parisc.pl",
                    "\$(PERLASM_SCHEME)",
                ],
            "crypto/modes/ghash-s390x.S" =>
                [
                    "crypto/modes/asm/ghash-s390x.pl",
                    "\$(PERLASM_SCHEME)",
                ],
            "crypto/modes/ghash-sparcv9.S" =>
                [
                    "crypto/modes/asm/ghash-sparcv9.pl",
                    "\$(PERLASM_SCHEME)",
                ],
            "crypto/modes/ghash-x86.s" =>
                [
                    "crypto/modes/asm/ghash-x86.pl",
                    "\$(PERLASM_SCHEME)",
                    "\$(LIB_CFLAGS)",
                    "\$(LIB_CPPFLAGS)",
                    "\$(PROCESSOR)",
                ],
            "crypto/modes/ghash-x86_64.s" =>
                [
                    "crypto/modes/asm/ghash-x86_64.pl",
                    "\$(PERLASM_SCHEME)",
                ],
            "crypto/modes/ghashp8-ppc.s" =>
                [
                    "crypto/modes/asm/ghashp8-ppc.pl",
                    "\$(PERLASM_SCHEME)",
                ],
            "crypto/modes/ghashv8-armx.S" =>
                [
                    "crypto/modes/asm/ghashv8-armx.pl",
                    "\$(PERLASM_SCHEME)",
                ],
            "crypto/pariscid.s" =>
                [
                    "crypto/pariscid.pl",
                    "\$(PERLASM_SCHEME)",
                ],
            "crypto/poly1305/poly1305-armv4.S" =>
                [
                    "crypto/poly1305/asm/poly1305-armv4.pl",
                    "\$(PERLASM_SCHEME)",
                ],
            "crypto/poly1305/poly1305-armv8.S" =>
                [
                    "crypto/poly1305/asm/poly1305-armv8.pl",
                    "\$(PERLASM_SCHEME)",
                ],
            "crypto/poly1305/poly1305-mips.S" =>
                [
                    "crypto/poly1305/asm/poly1305-mips.pl",
                    "\$(PERLASM_SCHEME)",
                ],
            "crypto/poly1305/poly1305-ppc.s" =>
                [
                    "crypto/poly1305/asm/poly1305-ppc.pl",
                    "\$(PERLASM_SCHEME)",
                ],
            "crypto/poly1305/poly1305-ppcfp.s" =>
                [
                    "crypto/poly1305/asm/poly1305-ppcfp.pl",
                    "\$(PERLASM_SCHEME)",
                ],
            "crypto/poly1305/poly1305-s390x.S" =>
                [
                    "crypto/poly1305/asm/poly1305-s390x.pl",
                    "\$(PERLASM_SCHEME)",
                ],
            "crypto/poly1305/poly1305-sparcv9.S" =>
                [
                    "crypto/poly1305/asm/poly1305-sparcv9.pl",
                    "\$(PERLASM_SCHEME)",
                ],
            "crypto/poly1305/poly1305-x86.s" =>
                [
                    "crypto/poly1305/asm/poly1305-x86.pl",
                    "\$(PERLASM_SCHEME)",
                    "\$(LIB_CFLAGS)",
                    "\$(LIB_CPPFLAGS)",
                    "\$(PROCESSOR)",
                ],
            "crypto/poly1305/poly1305-x86_64.s" =>
                [
                    "crypto/poly1305/asm/poly1305-x86_64.pl",
                    "\$(PERLASM_SCHEME)",
                ],
            "crypto/ppccpuid.s" =>
                [
                    "crypto/ppccpuid.pl",
                    "\$(PERLASM_SCHEME)",
                ],
            "crypto/rc4/rc4-586.s" =>
                [
                    "crypto/rc4/asm/rc4-586.pl",
                    "\$(PERLASM_SCHEME)",
                    "\$(LIB_CFLAGS)",
                    "\$(LIB_CPPFLAGS)",
                    "\$(PROCESSOR)",
                ],
            "crypto/rc4/rc4-md5-x86_64.s" =>
                [
                    "crypto/rc4/asm/rc4-md5-x86_64.pl",
                    "\$(PERLASM_SCHEME)",
                ],
            "crypto/rc4/rc4-parisc.s" =>
                [
                    "crypto/rc4/asm/rc4-parisc.pl",
                    "\$(PERLASM_SCHEME)",
                ],
            "crypto/rc4/rc4-s390x.s" =>
                [
                    "crypto/rc4/asm/rc4-s390x.pl",
                    "\$(PERLASM_SCHEME)",
                ],
            "crypto/rc4/rc4-x86_64.s" =>
                [
                    "crypto/rc4/asm/rc4-x86_64.pl",
                    "\$(PERLASM_SCHEME)",
                ],
            "crypto/ripemd/rmd-586.s" =>
                [
                    "crypto/ripemd/asm/rmd-586.pl",
                    "\$(PERLASM_SCHEME)",
                    "\$(LIB_CFLAGS)",
                    "\$(LIB_CPPFLAGS)",
                ],
            "crypto/s390xcpuid.S" =>
                [
                    "crypto/s390xcpuid.pl",
                    "\$(PERLASM_SCHEME)",
                ],
            "crypto/sha/keccak1600-armv4.S" =>
                [
                    "crypto/sha/asm/keccak1600-armv4.pl",
                    "\$(PERLASM_SCHEME)",
                ],
            "crypto/sha/keccak1600-armv8.S" =>
                [
                    "crypto/sha/asm/keccak1600-armv8.pl",
                    "\$(PERLASM_SCHEME)",
                ],
            "crypto/sha/keccak1600-ppc64.s" =>
                [
                    "crypto/sha/asm/keccak1600-ppc64.pl",
                    "\$(PERLASM_SCHEME)",
                ],
            "crypto/sha/keccak1600-s390x.S" =>
                [
                    "crypto/sha/asm/keccak1600-s390x.pl",
                    "\$(PERLASM_SCHEME)",
                ],
            "crypto/sha/keccak1600-x86_64.s" =>
                [
                    "crypto/sha/asm/keccak1600-x86_64.pl",
                    "\$(PERLASM_SCHEME)",
                ],
            "crypto/sha/sha1-586.s" =>
                [
                    "crypto/sha/asm/sha1-586.pl",
                    "\$(PERLASM_SCHEME)",
                    "\$(LIB_CFLAGS)",
                    "\$(LIB_CPPFLAGS)",
                    "\$(PROCESSOR)",
                ],
            "crypto/sha/sha1-alpha.S" =>
                [
                    "crypto/sha/asm/sha1-alpha.pl",
                    "\$(PERLASM_SCHEME)",
                ],
            "crypto/sha/sha1-armv4-large.S" =>
                [
                    "crypto/sha/asm/sha1-armv4-large.pl",
                    "\$(PERLASM_SCHEME)",
                ],
            "crypto/sha/sha1-armv8.S" =>
                [
                    "crypto/sha/asm/sha1-armv8.pl",
                    "\$(PERLASM_SCHEME)",
                ],
            "crypto/sha/sha1-ia64.s" =>
                [
                    "crypto/sha/asm/sha1-ia64.pl",
                    "\$(LIB_CFLAGS)",
                    "\$(LIB_CPPFLAGS)",
                ],
            "crypto/sha/sha1-mb-x86_64.s" =>
                [
                    "crypto/sha/asm/sha1-mb-x86_64.pl",
                    "\$(PERLASM_SCHEME)",
                ],
            "crypto/sha/sha1-mips.S" =>
                [
                    "crypto/sha/asm/sha1-mips.pl",
                    "\$(PERLASM_SCHEME)",
                ],
            "crypto/sha/sha1-parisc.s" =>
                [
                    "crypto/sha/asm/sha1-parisc.pl",
                    "\$(PERLASM_SCHEME)",
                ],
            "crypto/sha/sha1-ppc.s" =>
                [
                    "crypto/sha/asm/sha1-ppc.pl",
                    "\$(PERLASM_SCHEME)",
                ],
            "crypto/sha/sha1-s390x.S" =>
                [
                    "crypto/sha/asm/sha1-s390x.pl",
                    "\$(PERLASM_SCHEME)",
                ],
            "crypto/sha/sha1-sparcv9.S" =>
                [
                    "crypto/sha/asm/sha1-sparcv9.pl",
                    "\$(PERLASM_SCHEME)",
                ],
            "crypto/sha/sha1-x86_64.s" =>
                [
                    "crypto/sha/asm/sha1-x86_64.pl",
                    "\$(PERLASM_SCHEME)",
                ],
            "crypto/sha/sha256-586.s" =>
                [
                    "crypto/sha/asm/sha256-586.pl",
                    "\$(PERLASM_SCHEME)",
                    "\$(LIB_CFLAGS)",
                    "\$(LIB_CPPFLAGS)",
                    "\$(PROCESSOR)",
                ],
            "crypto/sha/sha256-armv4.S" =>
                [
                    "crypto/sha/asm/sha256-armv4.pl",
                    "\$(PERLASM_SCHEME)",
                ],
            "crypto/sha/sha256-armv8.S" =>
                [
                    "crypto/sha/asm/sha512-armv8.pl",
                    "\$(PERLASM_SCHEME)",
                ],
            "crypto/sha/sha256-ia64.s" =>
                [
                    "crypto/sha/asm/sha512-ia64.pl",
                    "\$(LIB_CFLAGS)",
                    "\$(LIB_CPPFLAGS)",
                ],
            "crypto/sha/sha256-mb-x86_64.s" =>
                [
                    "crypto/sha/asm/sha256-mb-x86_64.pl",
                    "\$(PERLASM_SCHEME)",
                ],
            "crypto/sha/sha256-mips.S" =>
                [
                    "crypto/sha/asm/sha512-mips.pl",
                    "\$(PERLASM_SCHEME)",
                ],
            "crypto/sha/sha256-parisc.s" =>
                [
                    "crypto/sha/asm/sha512-parisc.pl",
                    "\$(PERLASM_SCHEME)",
                ],
            "crypto/sha/sha256-ppc.s" =>
                [
                    "crypto/sha/asm/sha512-ppc.pl",
                    "\$(PERLASM_SCHEME)",
                ],
            "crypto/sha/sha256-s390x.S" =>
                [
                    "crypto/sha/asm/sha512-s390x.pl",
                    "\$(PERLASM_SCHEME)",
                ],
            "crypto/sha/sha256-sparcv9.S" =>
                [
                    "crypto/sha/asm/sha512-sparcv9.pl",
                    "\$(PERLASM_SCHEME)",
                ],
            "crypto/sha/sha256-x86_64.s" =>
                [
                    "crypto/sha/asm/sha512-x86_64.pl",
                    "\$(PERLASM_SCHEME)",
                ],
            "crypto/sha/sha256p8-ppc.s" =>
                [
                    "crypto/sha/asm/sha512p8-ppc.pl",
                    "\$(PERLASM_SCHEME)",
                ],
            "crypto/sha/sha512-586.s" =>
                [
                    "crypto/sha/asm/sha512-586.pl",
                    "\$(PERLASM_SCHEME)",
                    "\$(LIB_CFLAGS)",
                    "\$(LIB_CPPFLAGS)",
                    "\$(PROCESSOR)",
                ],
            "crypto/sha/sha512-armv4.S" =>
                [
                    "crypto/sha/asm/sha512-armv4.pl",
                    "\$(PERLASM_SCHEME)",
                ],
            "crypto/sha/sha512-armv8.S" =>
                [
                    "crypto/sha/asm/sha512-armv8.pl",
                    "\$(PERLASM_SCHEME)",
                ],
            "crypto/sha/sha512-ia64.s" =>
                [
                    "crypto/sha/asm/sha512-ia64.pl",
                    "\$(LIB_CFLAGS)",
                    "\$(LIB_CPPFLAGS)",
                ],
            "crypto/sha/sha512-mips.S" =>
                [
                    "crypto/sha/asm/sha512-mips.pl",
                    "\$(PERLASM_SCHEME)",
                ],
            "crypto/sha/sha512-parisc.s" =>
                [
                    "crypto/sha/asm/sha512-parisc.pl",
                    "\$(PERLASM_SCHEME)",
                ],
            "crypto/sha/sha512-ppc.s" =>
                [
                    "crypto/sha/asm/sha512-ppc.pl",
                    "\$(PERLASM_SCHEME)",
                ],
            "crypto/sha/sha512-s390x.S" =>
                [
                    "crypto/sha/asm/sha512-s390x.pl",
                    "\$(PERLASM_SCHEME)",
                ],
            "crypto/sha/sha512-sparcv9.S" =>
                [
                    "crypto/sha/asm/sha512-sparcv9.pl",
                    "\$(PERLASM_SCHEME)",
                ],
            "crypto/sha/sha512-x86_64.s" =>
                [
                    "crypto/sha/asm/sha512-x86_64.pl",
                    "\$(PERLASM_SCHEME)",
                ],
            "crypto/sha/sha512p8-ppc.s" =>
                [
                    "crypto/sha/asm/sha512p8-ppc.pl",
                    "\$(PERLASM_SCHEME)",
                ],
            "crypto/uplink-ia64.s" =>
                [
                    "ms/uplink-ia64.pl",
                    "\$(PERLASM_SCHEME)",
                ],
            "crypto/uplink-x86.s" =>
                [
                    "ms/uplink-x86.pl",
                    "\$(PERLASM_SCHEME)",
                ],
            "crypto/uplink-x86_64.s" =>
                [
                    "ms/uplink-x86_64.pl",
                    "\$(PERLASM_SCHEME)",
                ],
            "crypto/x86_64cpuid.s" =>
                [
                    "crypto/x86_64cpuid.pl",
                    "\$(PERLASM_SCHEME)",
                ],
            "crypto/x86cpuid.s" =>
                [
                    "crypto/x86cpuid.pl",
                    "\$(PERLASM_SCHEME)",
                    "\$(LIB_CFLAGS)",
                    "\$(LIB_CPPFLAGS)",
                    "\$(PROCESSOR)",
                ],
            "engines/e_padlock-x86.s" =>
                [
                    "engines/asm/e_padlock-x86.pl",
                    "\$(PERLASM_SCHEME)",
                    "\$(LIB_CFLAGS)",
                    "\$(LIB_CPPFLAGS)",
                    "\$(PROCESSOR)",
                ],
            "engines/e_padlock-x86_64.s" =>
                [
                    "engines/asm/e_padlock-x86_64.pl",
                    "\$(PERLASM_SCHEME)",
                ],
            "include/crypto/bn_conf.h" =>
                [
                    "include/crypto/bn_conf.h.in",
                ],
            "include/crypto/dso_conf.h" =>
                [
                    "include/crypto/dso_conf.h.in",
                ],
            "include/openssl/opensslconf.h" =>
                [
                    "include/openssl/opensslconf.h.in",
                ],
            "libcrypto.map" =>
                [
                    "util/mkdef.pl",
                    "crypto",
                    "linux",
                ],
            "libssl.map" =>
                [
                    "util/mkdef.pl",
                    "ssl",
                    "linux",
                ],
        },
    "includes" =>
        {
            "apps/app_rand.o" =>
                [
                    ".",
                    "include",
                ],
            "apps/apps.o" =>
                [
                    ".",
                    "include",
                ],
            "apps/asn1pars.o" =>
                [
                    ".",
                    "include",
                    "apps",
                ],
            "apps/bf_prefix.o" =>
                [
                    ".",
                    "include",
                ],
            "apps/ca.o" =>
                [
                    ".",
                    "include",
                    "apps",
                ],
            "apps/ciphers.o" =>
                [
                    ".",
                    "include",
                    "apps",
                ],
            "apps/cms.o" =>
                [
                    ".",
                    "include",
                    "apps",
                ],
            "apps/crl.o" =>
                [
                    ".",
                    "include",
                    "apps",
                ],
            "apps/crl2p7.o" =>
                [
                    ".",
                    "include",
                    "apps",
                ],
            "apps/dgst.o" =>
                [
                    ".",
                    "include",
                    "apps",
                ],
            "apps/dhparam.o" =>
                [
                    ".",
                    "include",
                    "apps",
                ],
            "apps/dsa.o" =>
                [
                    ".",
                    "include",
                    "apps",
                ],
            "apps/dsaparam.o" =>
                [
                    ".",
                    "include",
                    "apps",
                ],
            "apps/ec.o" =>
                [
                    ".",
                    "include",
                    "apps",
                ],
            "apps/ecparam.o" =>
                [
                    ".",
                    "include",
                    "apps",
                ],
            "apps/enc.o" =>
                [
                    ".",
                    "include",
                    "apps",
                ],
            "apps/engine.o" =>
                [
                    ".",
                    "include",
                    "apps",
                ],
            "apps/errstr.o" =>
                [
                    ".",
                    "include",
                    "apps",
                ],
            "apps/gendsa.o" =>
                [
                    ".",
                    "include",
                    "apps",
                ],
            "apps/genpkey.o" =>
                [
                    ".",
                    "include",
                    "apps",
                ],
            "apps/genrsa.o" =>
                [
                    ".",
                    "include",
                    "apps",
                ],
            "apps/nseq.o" =>
                [
                    ".",
                    "include",
                    "apps",
                ],
            "apps/ocsp.o" =>
                [
                    ".",
                    "include",
                    "apps",
                ],
            "apps/openssl.o" =>
                [
                    ".",
                    "include",
                    "apps",
                ],
            "apps/opt.o" =>
                [
                    ".",
                    "include",
                ],
            "apps/passwd.o" =>
                [
                    ".",
                    "include",
                    "apps",
                ],
            "apps/pkcs12.o" =>
                [
                    ".",
                    "include",
                    "apps",
                ],
            "apps/pkcs7.o" =>
                [
                    ".",
                    "include",
                    "apps",
                ],
            "apps/pkcs8.o" =>
                [
                    ".",
                    "include",
                    "apps",
                ],
            "apps/pkey.o" =>
                [
                    ".",
                    "include",
                    "apps",
                ],
            "apps/pkeyparam.o" =>
                [
                    ".",
                    "include",
                    "apps",
                ],
            "apps/pkeyutl.o" =>
                [
                    ".",
                    "include",
                    "apps",
                ],
            "apps/prime.o" =>
                [
                    ".",
                    "include",
                    "apps",
                ],
            "apps/progs.h" =>
                [
                    ".",
                ],
            "apps/rand.o" =>
                [
                    ".",
                    "include",
                    "apps",
                ],
            "apps/rehash.o" =>
                [
                    ".",
                    "include",
                    "apps",
                ],
            "apps/req.o" =>
                [
                    ".",
                    "include",
                    "apps",
                ],
            "apps/rsa.o" =>
                [
                    ".",
                    "include",
                    "apps",
                ],
            "apps/rsautl.o" =>
                [
                    ".",
                    "include",
                    "apps",
                ],
            "apps/s_cb.o" =>
                [
                    ".",
                    "include",
                ],
            "apps/s_client.o" =>
                [
                    ".",
                    "include",
                    "apps",
                ],
            "apps/s_server.o" =>
                [
                    ".",
                    "include",
                    "apps",
                ],
            "apps/s_socket.o" =>
                [
                    ".",
                    "include",
                ],
            "apps/s_time.o" =>
                [
                    ".",
                    "include",
                    "apps",
                ],
            "apps/sess_id.o" =>
                [
                    ".",
                    "include",
                    "apps",
                ],
            "apps/smime.o" =>
                [
                    ".",
                    "include",
                    "apps",
                ],
            "apps/speed.o" =>
                [
                    ".",
                    "include",
                    "apps",
                ],
            "apps/spkac.o" =>
                [
                    ".",
                    "include",
                    "apps",
                ],
            "apps/srp.o" =>
                [
                    ".",
                    "include",
                    "apps",
                ],
            "apps/storeutl.o" =>
                [
                    ".",
                    "include",
                    "apps",
                ],
            "apps/ts.o" =>
                [
                    ".",
                    "include",
                    "apps",
                ],
            "apps/verify.o" =>
                [
                    ".",
                    "include",
                    "apps",
                ],
            "apps/version.o" =>
                [
                    ".",
                    "include",
                    "apps",
                ],
            "apps/x509.o" =>
                [
                    ".",
                    "include",
                    "apps",
                ],
            "crypto/aes/aes-armv4.o" =>
                [
                    "crypto",
                ],
            "crypto/aes/aes-mips.o" =>
                [
                    "crypto",
                ],
            "crypto/aes/aes-s390x.o" =>
                [
                    "crypto",
                ],
            "crypto/aes/aes-sparcv9.o" =>
                [
                    "crypto",
                ],
            "crypto/aes/aes_cbc.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/aes/aes_cfb.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/aes/aes_core.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/aes/aes_ecb.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/aes/aes_ige.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/aes/aes_misc.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/aes/aes_ofb.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/aes/aes_wrap.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/aes/aesfx-sparcv9.o" =>
                [
                    "crypto",
                ],
            "crypto/aes/aest4-sparcv9.o" =>
                [
                    "crypto",
                ],
            "crypto/aes/aesv8-armx.o" =>
                [
                    "crypto",
                ],
            "crypto/aes/bsaes-armv7.o" =>
                [
                    "crypto",
                ],
            "crypto/arm64cpuid.o" =>
                [
                    "crypto",
                ],
            "crypto/armv4cpuid.o" =>
                [
                    "crypto",
                ],
            "crypto/asn1/a_bitstr.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/asn1/a_d2i_fp.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/asn1/a_digest.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/asn1/a_dup.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/asn1/a_gentm.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/asn1/a_i2d_fp.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/asn1/a_int.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/asn1/a_mbstr.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/asn1/a_object.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/asn1/a_octet.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/asn1/a_print.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/asn1/a_sign.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/asn1/a_strex.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/asn1/a_strnid.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/asn1/a_time.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/asn1/a_type.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/asn1/a_utctm.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/asn1/a_utf8.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/asn1/a_verify.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/asn1/ameth_lib.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/asn1/asn1_err.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/asn1/asn1_gen.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/asn1/asn1_item_list.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/asn1/asn1_lib.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/asn1/asn1_par.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/asn1/asn_mime.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/asn1/asn_moid.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/asn1/asn_mstbl.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/asn1/asn_pack.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/asn1/bio_asn1.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/asn1/bio_ndef.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/asn1/d2i_pr.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/asn1/d2i_pu.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/asn1/evp_asn1.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/asn1/f_int.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/asn1/f_string.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/asn1/i2d_pr.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/asn1/i2d_pu.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/asn1/n_pkey.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/asn1/nsseq.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/asn1/p5_pbe.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/asn1/p5_pbev2.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/asn1/p5_scrypt.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/asn1/p8_pkey.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/asn1/t_bitst.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/asn1/t_pkey.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/asn1/t_spki.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/asn1/tasn_dec.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/asn1/tasn_enc.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/asn1/tasn_fre.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/asn1/tasn_new.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/asn1/tasn_prn.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/asn1/tasn_scn.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/asn1/tasn_typ.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/asn1/tasn_utl.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/asn1/x_algor.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/asn1/x_bignum.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/asn1/x_info.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/asn1/x_int64.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/asn1/x_long.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/asn1/x_pkey.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/asn1/x_sig.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/asn1/x_spki.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/asn1/x_val.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/async/arch/async_null.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/async/arch/async_posix.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/async/arch/async_win.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/async/async.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/async/async_err.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/async/async_wait.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/bf/bf_cfb64.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/bf/bf_ecb.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/bf/bf_enc.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/bf/bf_ofb64.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/bf/bf_skey.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/bio/b_addr.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/bio/b_dump.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/bio/b_print.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/bio/b_sock.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/bio/b_sock2.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/bio/bf_buff.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/bio/bf_lbuf.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/bio/bf_nbio.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/bio/bf_null.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/bio/bio_cb.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/bio/bio_err.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/bio/bio_lib.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/bio/bio_meth.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/bio/bss_acpt.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/bio/bss_bio.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/bio/bss_conn.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/bio/bss_dgram.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/bio/bss_fd.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/bio/bss_file.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/bio/bss_log.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/bio/bss_mem.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/bio/bss_null.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/bio/bss_sock.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/blake2/blake2b.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/blake2/blake2s.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/blake2/m_blake2b.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/blake2/m_blake2s.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/bn/armv4-gf2m.o" =>
                [
                    "crypto",
                ],
            "crypto/bn/armv4-mont.o" =>
                [
                    "crypto",
                ],
            "crypto/bn/bn-mips.o" =>
                [
                    "crypto",
                ],
            "crypto/bn/bn_add.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/bn/bn_asm.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/bn/bn_blind.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/bn/bn_const.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/bn/bn_ctx.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/bn/bn_depr.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/bn/bn_dh.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/bn/bn_div.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/bn/bn_err.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/bn/bn_exp.o" =>
                [
                    ".",
                    "include",
                    "crypto",
                ],
            "crypto/bn/bn_exp2.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/bn/bn_gcd.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/bn/bn_gf2m.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/bn/bn_intern.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/bn/bn_kron.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/bn/bn_lib.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/bn/bn_mod.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/bn/bn_mont.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/bn/bn_mpi.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/bn/bn_mul.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/bn/bn_nist.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/bn/bn_prime.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/bn/bn_print.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/bn/bn_rand.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/bn/bn_recp.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/bn/bn_shift.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/bn/bn_sqr.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/bn/bn_sqrt.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/bn/bn_srp.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/bn/bn_word.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/bn/bn_x931p.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/bn/mips-mont.o" =>
                [
                    "crypto",
                ],
            "crypto/bn/sparct4-mont.o" =>
                [
                    "crypto",
                ],
            "crypto/bn/sparcv9-gf2m.o" =>
                [
                    "crypto",
                ],
            "crypto/bn/sparcv9-mont.o" =>
                [
                    "crypto",
                ],
            "crypto/bn/sparcv9a-mont.o" =>
                [
                    "crypto",
                ],
            "crypto/bn/vis3-mont.o" =>
                [
                    "crypto",
                ],
            "crypto/buffer/buf_err.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/buffer/buffer.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/buildinf.h" =>
                [
                    ".",
                ],
            "crypto/cast/c_cfb64.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/cast/c_ecb.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/cast/c_enc.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/cast/c_ofb64.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/cast/c_skey.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/chacha/chacha-armv4.o" =>
                [
                    "crypto",
                ],
            "crypto/chacha/chacha-armv8.o" =>
                [
                    "crypto",
                ],
            "crypto/chacha/chacha-s390x.o" =>
                [
                    "crypto",
                ],
            "crypto/chacha/chacha_enc.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/cmac/cm_ameth.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/cmac/cm_pmeth.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/cmac/cmac.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/cms/cms_asn1.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/cms/cms_att.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/cms/cms_cd.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/cms/cms_dd.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/cms/cms_enc.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/cms/cms_env.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/cms/cms_err.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/cms/cms_ess.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/cms/cms_io.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/cms/cms_kari.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/cms/cms_lib.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/cms/cms_pwri.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/cms/cms_sd.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/cms/cms_smime.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/comp/c_zlib.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/comp/comp_err.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/comp/comp_lib.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/conf/conf_api.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/conf/conf_def.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/conf/conf_err.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/conf/conf_lib.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/conf/conf_mall.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/conf/conf_mod.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/conf/conf_sap.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/conf/conf_ssl.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/cpt_err.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/cryptlib.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/ct/ct_b64.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/ct/ct_err.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/ct/ct_log.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/ct/ct_oct.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/ct/ct_policy.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/ct/ct_prn.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/ct/ct_sct.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/ct/ct_sct_ctx.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/ct/ct_vfy.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/ct/ct_x509v3.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/ctype.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/cversion.o" =>
                [
                    ".",
                    "include",
                    "crypto",
                ],
            "crypto/des/cbc_cksm.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/des/cbc_enc.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/des/cfb64ede.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/des/cfb64enc.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/des/cfb_enc.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/des/des_enc.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/des/dest4-sparcv9.o" =>
                [
                    "crypto",
                ],
            "crypto/des/ecb3_enc.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/des/ecb_enc.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/des/fcrypt.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/des/fcrypt_b.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/des/ofb64ede.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/des/ofb64enc.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/des/ofb_enc.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/des/pcbc_enc.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/des/qud_cksm.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/des/rand_key.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/des/set_key.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/des/str2key.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/des/xcbc_enc.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/dh/dh_ameth.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/dh/dh_asn1.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/dh/dh_check.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/dh/dh_depr.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/dh/dh_err.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/dh/dh_gen.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/dh/dh_kdf.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/dh/dh_key.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/dh/dh_lib.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/dh/dh_meth.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/dh/dh_pmeth.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/dh/dh_prn.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/dh/dh_rfc5114.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/dh/dh_rfc7919.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/dsa/dsa_ameth.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/dsa/dsa_asn1.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/dsa/dsa_depr.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/dsa/dsa_err.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/dsa/dsa_gen.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/dsa/dsa_key.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/dsa/dsa_lib.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/dsa/dsa_meth.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/dsa/dsa_ossl.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/dsa/dsa_pmeth.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/dsa/dsa_prn.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/dsa/dsa_sign.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/dsa/dsa_vrf.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/dso/dso_dl.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/dso/dso_dlfcn.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/dso/dso_err.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/dso/dso_lib.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/dso/dso_openssl.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/dso/dso_vms.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/dso/dso_win32.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/ebcdic.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/ec/curve25519.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/ec/curve448/arch_32/f_impl.o" =>
                [
                    ".",
                    "include",
                    "crypto/ec/curve448/arch_32",
                    "crypto/ec/curve448",
                ],
            "crypto/ec/curve448/curve448.o" =>
                [
                    ".",
                    "include",
                    "crypto/ec/curve448/arch_32",
                    "crypto/ec/curve448",
                ],
            "crypto/ec/curve448/curve448_tables.o" =>
                [
                    ".",
                    "include",
                    "crypto/ec/curve448/arch_32",
                    "crypto/ec/curve448",
                ],
            "crypto/ec/curve448/eddsa.o" =>
                [
                    ".",
                    "include",
                    "crypto/ec/curve448/arch_32",
                    "crypto/ec/curve448",
                ],
            "crypto/ec/curve448/f_generic.o" =>
                [
                    ".",
                    "include",
                    "crypto/ec/curve448/arch_32",
                    "crypto/ec/curve448",
                ],
            "crypto/ec/curve448/scalar.o" =>
                [
                    ".",
                    "include",
                    "crypto/ec/curve448/arch_32",
                    "crypto/ec/curve448",
                ],
            "crypto/ec/ec2_oct.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/ec/ec2_smpl.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/ec/ec_ameth.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/ec/ec_asn1.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/ec/ec_check.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/ec/ec_curve.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/ec/ec_cvt.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/ec/ec_err.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/ec/ec_key.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/ec/ec_kmeth.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/ec/ec_lib.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/ec/ec_mult.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/ec/ec_oct.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/ec/ec_pmeth.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/ec/ec_print.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/ec/ecdh_kdf.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/ec/ecdh_ossl.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/ec/ecdsa_ossl.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/ec/ecdsa_sign.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/ec/ecdsa_vrf.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/ec/eck_prn.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/ec/ecp_mont.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/ec/ecp_nist.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/ec/ecp_nistp224.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/ec/ecp_nistp256.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/ec/ecp_nistp521.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/ec/ecp_nistputil.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/ec/ecp_nistz256-armv4.o" =>
                [
                    "crypto",
                ],
            "crypto/ec/ecp_nistz256-armv8.o" =>
                [
                    "crypto",
                ],
            "crypto/ec/ecp_nistz256-sparcv9.o" =>
                [
                    "crypto",
                ],
            "crypto/ec/ecp_oct.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/ec/ecp_smpl.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/ec/ecx_meth.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/engine/eng_all.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/engine/eng_cnf.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/engine/eng_ctrl.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/engine/eng_dyn.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/engine/eng_err.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/engine/eng_fat.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/engine/eng_init.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/engine/eng_lib.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/engine/eng_list.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/engine/eng_openssl.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/engine/eng_pkey.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/engine/eng_rdrand.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/engine/eng_table.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/engine/tb_asnmth.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/engine/tb_cipher.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/engine/tb_dh.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/engine/tb_digest.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/engine/tb_dsa.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/engine/tb_eckey.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/engine/tb_pkmeth.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/engine/tb_rand.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/engine/tb_rsa.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/err/err.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/err/err_all.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/err/err_prn.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/evp/bio_b64.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/evp/bio_enc.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/evp/bio_md.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/evp/bio_ok.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/evp/c_allc.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/evp/c_alld.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/evp/cmeth_lib.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/evp/digest.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/evp/e_aes.o" =>
                [
                    ".",
                    "include",
                    "crypto",
                    "crypto/modes",
                ],
            "crypto/evp/e_aes_cbc_hmac_sha1.o" =>
                [
                    ".",
                    "include",
                    "crypto/modes",
                ],
            "crypto/evp/e_aes_cbc_hmac_sha256.o" =>
                [
                    ".",
                    "include",
                    "crypto/modes",
                ],
            "crypto/evp/e_aria.o" =>
                [
                    ".",
                    "include",
                    "crypto",
                    "crypto/modes",
                ],
            "crypto/evp/e_bf.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/evp/e_camellia.o" =>
                [
                    ".",
                    "include",
                    "crypto",
                    "crypto/modes",
                ],
            "crypto/evp/e_cast.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/evp/e_chacha20_poly1305.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/evp/e_des.o" =>
                [
                    ".",
                    "include",
                    "crypto",
                ],
            "crypto/evp/e_des3.o" =>
                [
                    ".",
                    "include",
                    "crypto",
                ],
            "crypto/evp/e_idea.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/evp/e_null.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/evp/e_old.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/evp/e_rc2.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/evp/e_rc4.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/evp/e_rc4_hmac_md5.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/evp/e_rc5.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/evp/e_seed.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/evp/e_sm4.o" =>
                [
                    ".",
                    "include",
                    "crypto",
                    "crypto/modes",
                ],
            "crypto/evp/e_xcbc_d.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/evp/encode.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/evp/evp_cnf.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/evp/evp_enc.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/evp/evp_err.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/evp/evp_key.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/evp/evp_lib.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/evp/evp_pbe.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/evp/evp_pkey.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/evp/m_md2.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/evp/m_md4.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/evp/m_md5.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/evp/m_md5_sha1.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/evp/m_mdc2.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/evp/m_null.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/evp/m_ripemd.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/evp/m_sha1.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/evp/m_sha3.o" =>
                [
                    ".",
                    "include",
                    "crypto",
                ],
            "crypto/evp/m_sigver.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/evp/m_wp.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/evp/names.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/evp/p5_crpt.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/evp/p5_crpt2.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/evp/p_dec.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/evp/p_enc.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/evp/p_lib.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/evp/p_open.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/evp/p_seal.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/evp/p_sign.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/evp/p_verify.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/evp/pbe_scrypt.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/evp/pmeth_fn.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/evp/pmeth_gn.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/evp/pmeth_lib.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/ex_data.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/getenv.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/hmac/hm_ameth.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/hmac/hm_pmeth.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/hmac/hmac.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/init.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/kdf/hkdf.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/kdf/kdf_err.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/kdf/scrypt.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/kdf/tls1_prf.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/lhash/lh_stats.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/lhash/lhash.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/md4/md4_dgst.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/md4/md4_one.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/md5/md5-sparcv9.o" =>
                [
                    "crypto",
                ],
            "crypto/md5/md5_dgst.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/md5/md5_one.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/mem.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/mem_clr.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/mem_dbg.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/mem_sec.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/modes/cbc128.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/modes/ccm128.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/modes/cfb128.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/modes/ctr128.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/modes/cts128.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/modes/gcm128.o" =>
                [
                    ".",
                    "include",
                    "crypto",
                ],
            "crypto/modes/ghash-armv4.o" =>
                [
                    "crypto",
                ],
            "crypto/modes/ghash-s390x.o" =>
                [
                    "crypto",
                ],
            "crypto/modes/ghash-sparcv9.o" =>
                [
                    "crypto",
                ],
            "crypto/modes/ghashv8-armx.o" =>
                [
                    "crypto",
                ],
            "crypto/modes/ocb128.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/modes/ofb128.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/modes/wrap128.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/modes/xts128.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/o_dir.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/o_fips.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/o_fopen.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/o_init.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/o_str.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/o_time.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/objects/o_names.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/objects/obj_dat.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/objects/obj_err.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/objects/obj_lib.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/objects/obj_xref.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/ocsp/ocsp_asn.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/ocsp/ocsp_cl.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/ocsp/ocsp_err.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/ocsp/ocsp_ext.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/ocsp/ocsp_ht.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/ocsp/ocsp_lib.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/ocsp/ocsp_prn.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/ocsp/ocsp_srv.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/ocsp/ocsp_vfy.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/ocsp/v3_ocsp.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/pem/pem_all.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/pem/pem_err.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/pem/pem_info.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/pem/pem_lib.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/pem/pem_oth.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/pem/pem_pk8.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/pem/pem_pkey.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/pem/pem_sign.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/pem/pem_x509.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/pem/pem_xaux.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/pem/pvkfmt.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/pkcs12/p12_add.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/pkcs12/p12_asn.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/pkcs12/p12_attr.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/pkcs12/p12_crpt.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/pkcs12/p12_crt.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/pkcs12/p12_decr.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/pkcs12/p12_init.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/pkcs12/p12_key.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/pkcs12/p12_kiss.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/pkcs12/p12_mutl.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/pkcs12/p12_npas.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/pkcs12/p12_p8d.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/pkcs12/p12_p8e.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/pkcs12/p12_sbag.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/pkcs12/p12_utl.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/pkcs12/pk12err.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/pkcs7/bio_pk7.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/pkcs7/pk7_asn1.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/pkcs7/pk7_attr.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/pkcs7/pk7_doit.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/pkcs7/pk7_lib.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/pkcs7/pk7_mime.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/pkcs7/pk7_smime.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/pkcs7/pkcs7err.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/poly1305/poly1305-armv4.o" =>
                [
                    "crypto",
                ],
            "crypto/poly1305/poly1305-armv8.o" =>
                [
                    "crypto",
                ],
            "crypto/poly1305/poly1305-mips.o" =>
                [
                    "crypto",
                ],
            "crypto/poly1305/poly1305-sparcv9.o" =>
                [
                    "crypto",
                ],
            "crypto/poly1305/poly1305.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/poly1305/poly1305_ameth.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/poly1305/poly1305_pmeth.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/rand/drbg_ctr.o" =>
                [
                    ".",
                    "include",
                    "crypto/modes",
                ],
            "crypto/rand/drbg_lib.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/rand/rand_egd.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/rand/rand_err.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/rand/rand_lib.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/rand/rand_unix.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/rand/rand_vms.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/rand/rand_win.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/rand/randfile.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/rc4/rc4_enc.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/rc4/rc4_skey.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/ripemd/rmd_dgst.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/ripemd/rmd_one.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/rsa/rsa_ameth.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/rsa/rsa_asn1.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/rsa/rsa_chk.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/rsa/rsa_crpt.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/rsa/rsa_depr.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/rsa/rsa_err.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/rsa/rsa_gen.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/rsa/rsa_lib.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/rsa/rsa_meth.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/rsa/rsa_mp.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/rsa/rsa_none.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/rsa/rsa_oaep.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/rsa/rsa_ossl.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/rsa/rsa_pk1.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/rsa/rsa_pmeth.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/rsa/rsa_prn.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/rsa/rsa_pss.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/rsa/rsa_saos.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/rsa/rsa_sign.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/rsa/rsa_ssl.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/rsa/rsa_x931.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/rsa/rsa_x931g.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/s390xcpuid.o" =>
                [
                    "crypto",
                ],
            "crypto/sha/keccak1600-armv4.o" =>
                [
                    "crypto",
                ],
            "crypto/sha/keccak1600.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/sha/sha1-armv4-large.o" =>
                [
                    "crypto",
                ],
            "crypto/sha/sha1-armv8.o" =>
                [
                    "crypto",
                ],
            "crypto/sha/sha1-mips.o" =>
                [
                    "crypto",
                ],
            "crypto/sha/sha1-s390x.o" =>
                [
                    "crypto",
                ],
            "crypto/sha/sha1-sparcv9.o" =>
                [
                    "crypto",
                ],
            "crypto/sha/sha1_one.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/sha/sha1dgst.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/sha/sha256-armv4.o" =>
                [
                    "crypto",
                ],
            "crypto/sha/sha256-armv8.o" =>
                [
                    "crypto",
                ],
            "crypto/sha/sha256-mips.o" =>
                [
                    "crypto",
                ],
            "crypto/sha/sha256-s390x.o" =>
                [
                    "crypto",
                ],
            "crypto/sha/sha256-sparcv9.o" =>
                [
                    "crypto",
                ],
            "crypto/sha/sha256.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/sha/sha512-armv4.o" =>
                [
                    "crypto",
                ],
            "crypto/sha/sha512-armv8.o" =>
                [
                    "crypto",
                ],
            "crypto/sha/sha512-mips.o" =>
                [
                    "crypto",
                ],
            "crypto/sha/sha512-s390x.o" =>
                [
                    "crypto",
                ],
            "crypto/sha/sha512-sparcv9.o" =>
                [
                    "crypto",
                ],
            "crypto/sha/sha512.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/srp/srp_lib.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/srp/srp_vfy.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/stack/stack.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/store/loader_file.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/store/store_err.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/store/store_init.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/store/store_lib.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/store/store_register.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/store/store_strings.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/threads_none.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/threads_pthread.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/threads_win.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/ts/ts_asn1.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/ts/ts_conf.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/ts/ts_err.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/ts/ts_lib.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/ts/ts_req_print.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/ts/ts_req_utils.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/ts/ts_rsp_print.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/ts/ts_rsp_sign.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/ts/ts_rsp_utils.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/ts/ts_rsp_verify.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/ts/ts_verify_ctx.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/txt_db/txt_db.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/ui/ui_err.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/ui/ui_lib.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/ui/ui_null.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/ui/ui_openssl.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/ui/ui_util.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/uid.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/x509/by_dir.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/x509/by_file.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/x509/t_crl.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/x509/t_req.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/x509/t_x509.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/x509/x509_att.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/x509/x509_cmp.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/x509/x509_d2.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/x509/x509_def.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/x509/x509_err.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/x509/x509_ext.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/x509/x509_lu.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/x509/x509_meth.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/x509/x509_obj.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/x509/x509_r2x.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/x509/x509_req.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/x509/x509_set.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/x509/x509_trs.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/x509/x509_txt.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/x509/x509_v3.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/x509/x509_vfy.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/x509/x509_vpm.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/x509/x509cset.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/x509/x509name.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/x509/x509rset.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/x509/x509spki.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/x509/x509type.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/x509/x_all.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/x509/x_attrib.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/x509/x_crl.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/x509/x_exten.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/x509/x_name.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/x509/x_pubkey.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/x509/x_req.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/x509/x_x509.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/x509/x_x509a.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/x509v3/pcy_cache.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/x509v3/pcy_data.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/x509v3/pcy_lib.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/x509v3/pcy_map.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/x509v3/pcy_node.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/x509v3/pcy_tree.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/x509v3/v3_addr.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/x509v3/v3_admis.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/x509v3/v3_akey.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/x509v3/v3_akeya.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/x509v3/v3_alt.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/x509v3/v3_asid.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/x509v3/v3_bcons.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/x509v3/v3_bitst.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/x509v3/v3_conf.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/x509v3/v3_cpols.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/x509v3/v3_crld.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/x509v3/v3_enum.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/x509v3/v3_extku.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/x509v3/v3_genn.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/x509v3/v3_ia5.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/x509v3/v3_info.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/x509v3/v3_int.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/x509v3/v3_lib.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/x509v3/v3_ncons.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/x509v3/v3_pci.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/x509v3/v3_pcia.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/x509v3/v3_pcons.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/x509v3/v3_pku.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/x509v3/v3_pmaps.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/x509v3/v3_prn.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/x509v3/v3_purp.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/x509v3/v3_skey.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/x509v3/v3_sxnet.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/x509v3/v3_tlsf.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/x509v3/v3_utl.o" =>
                [
                    ".",
                    "include",
                ],
            "crypto/x509v3/v3err.o" =>
                [
                    ".",
                    "include",
                ],
            "engines/e_afalg.o" =>
                [
                    "include",
                ],
            "engines/e_capi.o" =>
                [
                    "include",
                ],
            "engines/e_dasync.o" =>
                [
                    "include",
                ],
            "engines/e_ossltest.o" =>
                [
                    "include",
                ],
            "engines/e_padlock.o" =>
                [
                    "include",
                ],
            "include/crypto/bn_conf.h" =>
                [
                    ".",
                ],
            "include/crypto/dso_conf.h" =>
                [
                    ".",
                ],
            "include/openssl/opensslconf.h" =>
                [
                    ".",
                ],
            "ssl/bio_ssl.o" =>
                [
                    ".",
                    "include",
                ],
            "ssl/d1_lib.o" =>
                [
                    ".",
                    "include",
                ],
            "ssl/d1_msg.o" =>
                [
                    ".",
                    "include",
                ],
            "ssl/d1_srtp.o" =>
                [
                    ".",
                    "include",
                ],
            "ssl/methods.o" =>
                [
                    ".",
                    "include",
                ],
            "ssl/packet.o" =>
                [
                    ".",
                    "include",
                ],
            "ssl/pqueue.o" =>
                [
                    ".",
                    "include",
                ],
            "ssl/record/dtls1_bitmap.o" =>
                [
                    ".",
                    "include",
                ],
            "ssl/record/rec_layer_d1.o" =>
                [
                    ".",
                    "include",
                ],
            "ssl/record/rec_layer_s3.o" =>
                [
                    ".",
                    "include",
                ],
            "ssl/record/ssl3_buffer.o" =>
                [
                    ".",
                    "include",
                ],
            "ssl/record/ssl3_record.o" =>
                [
                    ".",
                    "include",
                ],
            "ssl/record/ssl3_record_tls13.o" =>
                [
                    ".",
                    "include",
                ],
            "ssl/s3_cbc.o" =>
                [
                    ".",
                    "include",
                ],
            "ssl/s3_enc.o" =>
                [
                    ".",
                    "include",
                ],
            "ssl/s3_lib.o" =>
                [
                    ".",
                    "include",
                ],
            "ssl/s3_msg.o" =>
                [
                    ".",
                    "include",
                ],
            "ssl/ssl_asn1.o" =>
                [
                    ".",
                    "include",
                ],
            "ssl/ssl_cert.o" =>
                [
                    ".",
                    "include",
                ],
            "ssl/ssl_ciph.o" =>
                [
                    ".",
                    "include",
                ],
            "ssl/ssl_conf.o" =>
                [
                    ".",
                    "include",
                ],
            "ssl/ssl_err.o" =>
                [
                    ".",
                    "include",
                ],
            "ssl/ssl_init.o" =>
                [
                    ".",
                    "include",
                ],
            "ssl/ssl_lib.o" =>
                [
                    ".",
                    "include",
                ],
            "ssl/ssl_mcnf.o" =>
                [
                    ".",
                    "include",
                ],
            "ssl/ssl_rsa.o" =>
                [
                    ".",
                    "include",
                ],
            "ssl/ssl_sess.o" =>
                [
                    ".",
                    "include",
                ],
            "ssl/ssl_stat.o" =>
                [
                    ".",
                    "include",
                ],
            "ssl/ssl_txt.o" =>
                [
                    ".",
                    "include",
                ],
            "ssl/ssl_utst.o" =>
                [
                    ".",
                    "include",
                ],
            "ssl/statem/extensions.o" =>
                [
                    ".",
                    "include",
                ],
            "ssl/statem/extensions_clnt.o" =>
                [
                    ".",
                    "include",
                ],
            "ssl/statem/extensions_cust.o" =>
                [
                    ".",
                    "include",
                ],
            "ssl/statem/extensions_srvr.o" =>
                [
                    ".",
                    "include",
                ],
            "ssl/statem/statem.o" =>
                [
                    ".",
                    "include",
                ],
            "ssl/statem/statem_clnt.o" =>
                [
                    ".",
                    "include",
                ],
            "ssl/statem/statem_dtls.o" =>
                [
                    ".",
                    "include",
                ],
            "ssl/statem/statem_lib.o" =>
                [
                    ".",
                    "include",
                ],
            "ssl/statem/statem_srvr.o" =>
                [
                    ".",
                    "include",
                ],
            "ssl/t1_enc.o" =>
                [
                    ".",
                    "include",
                ],
            "ssl/t1_lib.o" =>
                [
                    ".",
                    "include",
                ],
            "ssl/t1_trce.o" =>
                [
                    ".",
                    "include",
                ],
            "ssl/tls13_enc.o" =>
                [
                    ".",
                    "include",
                ],
            "ssl/tls_srp.o" =>
                [
                    ".",
                    "include",
                ],
        },
    "install" =>
        {
            "engines" =>
                [
                    "engines/afalg",
                    "engines/capi",
                    "engines/padlock",
                ],
            "libraries" =>
                [
                    "libcrypto",
                    "libssl",
                ],
            "programs" =>
                [
                    "apps/openssl",
                ],
            "scripts" =>
                [
                    "apps/CA.pl",
                    "apps/tsget.pl",
                    "tools/c_rehash",
                ],
        },
    "ldadd" =>
        {
        },
    "libraries" =>
        [
            "apps/libapps.a",
            "libcrypto",
            "libssl",
        ],
    "overrides" =>
        [
        ],
    "programs" =>
        [
            "apps/openssl",
        ],
    "rawlines" =>
        [
            "##### SHA assembler implementations",
            "",
            "# GNU make \"catch all\"",
            "crypto/sha/sha1-%.S:	crypto/sha/asm/sha1-%.pl",
            "	CC=\"\$(CC)\" \$(PERL) \$< \$(PERLASM_SCHEME) \$\@",
            "crypto/sha/sha256-%.S:	crypto/sha/asm/sha512-%.pl",
            "	CC=\"\$(CC)\" \$(PERL) \$< \$(PERLASM_SCHEME) \$\@",
            "crypto/sha/sha512-%.S:	crypto/sha/asm/sha512-%.pl",
            "	CC=\"\$(CC)\" \$(PERL) \$< \$(PERLASM_SCHEME) \$\@",
            "crypto/poly1305/poly1305-%.S:	crypto/poly1305/asm/poly1305-%.pl",
            "	CC=\"\$(CC)\" \$(PERL) \$< \$(PERLASM_SCHEME) \$\@",
            "##### AES assembler implementations",
            "",
            "# GNU make \"catch all\"",
            "crypto/aes/aes-%.S:	crypto/aes/asm/aes-%.pl",
            "	CC=\"\$(CC)\" \$(PERL) \$< \$(PERLASM_SCHEME) \$\@",
            "crypto/aes/bsaes-%.S:	crypto/aes/asm/bsaes-%.pl",
            "	CC=\"\$(CC)\" \$(PERL) \$< \$(PERLASM_SCHEME) \$\@",
            "",
            "# GNU make \"catch all\"",
            "crypto/rc4/rc4-%.s:	crypto/rc4/asm/rc4-%.pl",
            "	CC=\"\$(CC)\" \$(PERL) \$< \$(PERLASM_SCHEME) \$\@",
            "##### CHACHA assembler implementations",
            "",
            "crypto/chacha/chacha-%.S:	crypto/chacha/asm/chacha-%.pl",
            "	CC=\"\$(CC)\" \$(PERL) \$< \$(PERLASM_SCHEME) \$\@",
            "# GNU make \"catch all\"",
            "crypto/modes/ghash-%.S:	crypto/modes/asm/ghash-%.pl",
            "	CC=\"\$(CC)\" \$(PERL) \$< \$(PERLASM_SCHEME) \$\@",
            "crypto/ec/ecp_nistz256-%.S:	crypto/ec/asm/ecp_nistz256-%.pl",
            "	CC=\"\$(CC)\" \$(PERL) \$< \$(PERLASM_SCHEME) \$\@",
        ],
    "rename" =>
        {
        },
    "scripts" =>
        [
            "apps/CA.pl",
            "apps/tsget.pl",
            "tools/c_rehash",
            "util/shlib_wrap.sh",
        ],
    "shared_sources" =>
        {
            "libcrypto" =>
                [
                    "libcrypto.map",
                ],
            "libssl" =>
                [
                    "libssl.map",
                ],
        },
    "sharednames" =>
        {
            "libcrypto" => "libcrypto",
            "libssl" => "libssl",
        },
    "sources" =>
        {
            "apps/CA.pl" =>
                [
                    "apps/CA.pl.in",
                ],
            "apps/app_rand.o" =>
                [
                    "apps/app_rand.c",
                ],
            "apps/apps.o" =>
                [
                    "apps/apps.c",
                ],
            "apps/asn1pars.o" =>
                [
                    "apps/asn1pars.c",
                ],
            "apps/bf_prefix.o" =>
                [
                    "apps/bf_prefix.c",
                ],
            "apps/ca.o" =>
                [
                    "apps/ca.c",
                ],
            "apps/ciphers.o" =>
                [
                    "apps/ciphers.c",
                ],
            "apps/cms.o" =>
                [
                    "apps/cms.c",
                ],
            "apps/crl.o" =>
                [
                    "apps/crl.c",
                ],
            "apps/crl2p7.o" =>
                [
                    "apps/crl2p7.c",
                ],
            "apps/dgst.o" =>
                [
                    "apps/dgst.c",
                ],
            "apps/dhparam.o" =>
                [
                    "apps/dhparam.c",
                ],
            "apps/dsa.o" =>
                [
                    "apps/dsa.c",
                ],
            "apps/dsaparam.o" =>
                [
                    "apps/dsaparam.c",
                ],
            "apps/ec.o" =>
                [
                    "apps/ec.c",
                ],
            "apps/ecparam.o" =>
                [
                    "apps/ecparam.c",
                ],
            "apps/enc.o" =>
                [
                    "apps/enc.c",
                ],
            "apps/engine.o" =>
                [
                    "apps/engine.c",
                ],
            "apps/errstr.o" =>
                [
                    "apps/errstr.c",
                ],
            "apps/gendsa.o" =>
                [
                    "apps/gendsa.c",
                ],
            "apps/genpkey.o" =>
                [
                    "apps/genpkey.c",
                ],
            "apps/genrsa.o" =>
                [
                    "apps/genrsa.c",
                ],
            "apps/libapps.a" =>
                [
                    "apps/app_rand.o",
                    "apps/apps.o",
                    "apps/bf_prefix.o",
                    "apps/opt.o",
                    "apps/s_cb.o",
                    "apps/s_socket.o",
                ],
            "apps/nseq.o" =>
                [
                    "apps/nseq.c",
                ],
            "apps/ocsp.o" =>
                [
                    "apps/ocsp.c",
                ],
            "apps/openssl" =>
                [
                    "apps/asn1pars.o",
                    "apps/ca.o",
                    "apps/ciphers.o",
                    "apps/cms.o",
                    "apps/crl.o",
                    "apps/crl2p7.o",
                    "apps/dgst.o",
                    "apps/dhparam.o",
                    "apps/dsa.o",
                    "apps/dsaparam.o",
                    "apps/ec.o",
                    "apps/ecparam.o",
                    "apps/enc.o",
                    "apps/engine.o",
                    "apps/errstr.o",
                    "apps/gendsa.o",
                    "apps/genpkey.o",
                    "apps/genrsa.o",
                    "apps/nseq.o",
                    "apps/ocsp.o",
                    "apps/openssl.o",
                    "apps/passwd.o",
                    "apps/pkcs12.o",
                    "apps/pkcs7.o",
                    "apps/pkcs8.o",
                    "apps/pkey.o",
                    "apps/pkeyparam.o",
                    "apps/pkeyutl.o",
                    "apps/prime.o",
                    "apps/rand.o",
                    "apps/rehash.o",
                    "apps/req.o",
                    "apps/rsa.o",
                    "apps/rsautl.o",
                    "apps/s_client.o",
                    "apps/s_server.o",
                    "apps/s_time.o",
                    "apps/sess_id.o",
                    "apps/smime.o",
                    "apps/speed.o",
                    "apps/spkac.o",
                    "apps/srp.o",
                    "apps/storeutl.o",
                    "apps/ts.o",
                    "apps/verify.o",
                    "apps/version.o",
                    "apps/x509.o",
                ],
            "apps/openssl.o" =>
                [
                    "apps/openssl.c",
                ],
            "apps/opt.o" =>
                [
                    "apps/opt.c",
                ],
            "apps/passwd.o" =>
                [
                    "apps/passwd.c",
                ],
            "apps/pkcs12.o" =>
                [
                    "apps/pkcs12.c",
                ],
            "apps/pkcs7.o" =>
                [
                    "apps/pkcs7.c",
                ],
            "apps/pkcs8.o" =>
                [
                    "apps/pkcs8.c",
                ],
            "apps/pkey.o" =>
                [
                    "apps/pkey.c",
                ],
            "apps/pkeyparam.o" =>
                [
                    "apps/pkeyparam.c",
                ],
            "apps/pkeyutl.o" =>
                [
                    "apps/pkeyutl.c",
                ],
            "apps/prime.o" =>
                [
                    "apps/prime.c",
                ],
            "apps/rand.o" =>
                [
                    "apps/rand.c",
                ],
            "apps/rehash.o" =>
                [
                    "apps/rehash.c",
                ],
            "apps/req.o" =>
                [
                    "apps/req.c",
                ],
            "apps/rsa.o" =>
                [
                    "apps/rsa.c",
                ],
            "apps/rsautl.o" =>
                [
                    "apps/rsautl.c",
                ],
            "apps/s_cb.o" =>
                [
                    "apps/s_cb.c",
                ],
            "apps/s_client.o" =>
                [
                    "apps/s_client.c",
                ],
            "apps/s_server.o" =>
                [
                    "apps/s_server.c",
                ],
            "apps/s_socket.o" =>
                [
                    "apps/s_socket.c",
                ],
            "apps/s_time.o" =>
                [
                    "apps/s_time.c",
                ],
            "apps/sess_id.o" =>
                [
                    "apps/sess_id.c",
                ],
            "apps/smime.o" =>
                [
                    "apps/smime.c",
                ],
            "apps/speed.o" =>
                [
                    "apps/speed.c",
                ],
            "apps/spkac.o" =>
                [
                    "apps/spkac.c",
                ],
            "apps/srp.o" =>
                [
                    "apps/srp.c",
                ],
            "apps/storeutl.o" =>
                [
                    "apps/storeutl.c",
                ],
            "apps/ts.o" =>
                [
                    "apps/ts.c",
                ],
            "apps/tsget.pl" =>
                [
                    "apps/tsget.in",
                ],
            "apps/verify.o" =>
                [
                    "apps/verify.c",
                ],
            "apps/version.o" =>
                [
                    "apps/version.c",
                ],
            "apps/x509.o" =>
                [
                    "apps/x509.c",
                ],
            "crypto/aes/aes_cbc.o" =>
                [
                    "crypto/aes/aes_cbc.c",
                ],
            "crypto/aes/aes_cfb.o" =>
                [
                    "crypto/aes/aes_cfb.c",
                ],
            "crypto/aes/aes_core.o" =>
                [
                    "crypto/aes/aes_core.c",
                ],
            "crypto/aes/aes_ecb.o" =>
                [
                    "crypto/aes/aes_ecb.c",
                ],
            "crypto/aes/aes_ige.o" =>
                [
                    "crypto/aes/aes_ige.c",
                ],
            "crypto/aes/aes_misc.o" =>
                [
                    "crypto/aes/aes_misc.c",
                ],
            "crypto/aes/aes_ofb.o" =>
                [
                    "crypto/aes/aes_ofb.c",
                ],
            "crypto/aes/aes_wrap.o" =>
                [
                    "crypto/aes/aes_wrap.c",
                ],
            "crypto/asn1/a_bitstr.o" =>
                [
                    "crypto/asn1/a_bitstr.c",
                ],
            "crypto/asn1/a_d2i_fp.o" =>
                [
                    "crypto/asn1/a_d2i_fp.c",
                ],
            "crypto/asn1/a_digest.o" =>
                [
                    "crypto/asn1/a_digest.c",
                ],
            "crypto/asn1/a_dup.o" =>
                [
                    "crypto/asn1/a_dup.c",
                ],
            "crypto/asn1/a_gentm.o" =>
                [
                    "crypto/asn1/a_gentm.c",
                ],
            "crypto/asn1/a_i2d_fp.o" =>
                [
                    "crypto/asn1/a_i2d_fp.c",
                ],
            "crypto/asn1/a_int.o" =>
                [
                    "crypto/asn1/a_int.c",
                ],
            "crypto/asn1/a_mbstr.o" =>
                [
                    "crypto/asn1/a_mbstr.c",
                ],
            "crypto/asn1/a_object.o" =>
                [
                    "crypto/asn1/a_object.c",
                ],
            "crypto/asn1/a_octet.o" =>
                [
                    "crypto/asn1/a_octet.c",
                ],
            "crypto/asn1/a_print.o" =>
                [
                    "crypto/asn1/a_print.c",
                ],
            "crypto/asn1/a_sign.o" =>
                [
                    "crypto/asn1/a_sign.c",
                ],
            "crypto/asn1/a_strex.o" =>
                [
                    "crypto/asn1/a_strex.c",
                ],
            "crypto/asn1/a_strnid.o" =>
                [
                    "crypto/asn1/a_strnid.c",
                ],
            "crypto/asn1/a_time.o" =>
                [
                    "crypto/asn1/a_time.c",
                ],
            "crypto/asn1/a_type.o" =>
                [
                    "crypto/asn1/a_type.c",
                ],
            "crypto/asn1/a_utctm.o" =>
                [
                    "crypto/asn1/a_utctm.c",
                ],
            "crypto/asn1/a_utf8.o" =>
                [
                    "crypto/asn1/a_utf8.c",
                ],
            "crypto/asn1/a_verify.o" =>
                [
                    "crypto/asn1/a_verify.c",
                ],
            "crypto/asn1/ameth_lib.o" =>
                [
                    "crypto/asn1/ameth_lib.c",
                ],
            "crypto/asn1/asn1_err.o" =>
                [
                    "crypto/asn1/asn1_err.c",
                ],
            "crypto/asn1/asn1_gen.o" =>
                [
                    "crypto/asn1/asn1_gen.c",
                ],
            "crypto/asn1/asn1_item_list.o" =>
                [
                    "crypto/asn1/asn1_item_list.c",
                ],
            "crypto/asn1/asn1_lib.o" =>
                [
                    "crypto/asn1/asn1_lib.c",
                ],
            "crypto/asn1/asn1_par.o" =>
                [
                    "crypto/asn1/asn1_par.c",
                ],
            "crypto/asn1/asn_mime.o" =>
                [
                    "crypto/asn1/asn_mime.c",
                ],
            "crypto/asn1/asn_moid.o" =>
                [
                    "crypto/asn1/asn_moid.c",
                ],
            "crypto/asn1/asn_mstbl.o" =>
                [
                    "crypto/asn1/asn_mstbl.c",
                ],
            "crypto/asn1/asn_pack.o" =>
                [
                    "crypto/asn1/asn_pack.c",
                ],
            "crypto/asn1/bio_asn1.o" =>
                [
                    "crypto/asn1/bio_asn1.c",
                ],
            "crypto/asn1/bio_ndef.o" =>
                [
                    "crypto/asn1/bio_ndef.c",
                ],
            "crypto/asn1/d2i_pr.o" =>
                [
                    "crypto/asn1/d2i_pr.c",
                ],
            "crypto/asn1/d2i_pu.o" =>
                [
                    "crypto/asn1/d2i_pu.c",
                ],
            "crypto/asn1/evp_asn1.o" =>
                [
                    "crypto/asn1/evp_asn1.c",
                ],
            "crypto/asn1/f_int.o" =>
                [
                    "crypto/asn1/f_int.c",
                ],
            "crypto/asn1/f_string.o" =>
                [
                    "crypto/asn1/f_string.c",
                ],
            "crypto/asn1/i2d_pr.o" =>
                [
                    "crypto/asn1/i2d_pr.c",
                ],
            "crypto/asn1/i2d_pu.o" =>
                [
                    "crypto/asn1/i2d_pu.c",
                ],
            "crypto/asn1/n_pkey.o" =>
                [
                    "crypto/asn1/n_pkey.c",
                ],
            "crypto/asn1/nsseq.o" =>
                [
                    "crypto/asn1/nsseq.c",
                ],
            "crypto/asn1/p5_pbe.o" =>
                [
                    "crypto/asn1/p5_pbe.c",
                ],
            "crypto/asn1/p5_pbev2.o" =>
                [
                    "crypto/asn1/p5_pbev2.c",
                ],
            "crypto/asn1/p5_scrypt.o" =>
                [
                    "crypto/asn1/p5_scrypt.c",
                ],
            "crypto/asn1/p8_pkey.o" =>
                [
                    "crypto/asn1/p8_pkey.c",
                ],
            "crypto/asn1/t_bitst.o" =>
                [
                    "crypto/asn1/t_bitst.c",
                ],
            "crypto/asn1/t_pkey.o" =>
                [
                    "crypto/asn1/t_pkey.c",
                ],
            "crypto/asn1/t_spki.o" =>
                [
                    "crypto/asn1/t_spki.c",
                ],
            "crypto/asn1/tasn_dec.o" =>
                [
                    "crypto/asn1/tasn_dec.c",
                ],
            "crypto/asn1/tasn_enc.o" =>
                [
                    "crypto/asn1/tasn_enc.c",
                ],
            "crypto/asn1/tasn_fre.o" =>
                [
                    "crypto/asn1/tasn_fre.c",
                ],
            "crypto/asn1/tasn_new.o" =>
                [
                    "crypto/asn1/tasn_new.c",
                ],
            "crypto/asn1/tasn_prn.o" =>
                [
                    "crypto/asn1/tasn_prn.c",
                ],
            "crypto/asn1/tasn_scn.o" =>
                [
                    "crypto/asn1/tasn_scn.c",
                ],
            "crypto/asn1/tasn_typ.o" =>
                [
                    "crypto/asn1/tasn_typ.c",
                ],
            "crypto/asn1/tasn_utl.o" =>
                [
                    "crypto/asn1/tasn_utl.c",
                ],
            "crypto/asn1/x_algor.o" =>
                [
                    "crypto/asn1/x_algor.c",
                ],
            "crypto/asn1/x_bignum.o" =>
                [
                    "crypto/asn1/x_bignum.c",
                ],
            "crypto/asn1/x_info.o" =>
                [
                    "crypto/asn1/x_info.c",
                ],
            "crypto/asn1/x_int64.o" =>
                [
                    "crypto/asn1/x_int64.c",
                ],
            "crypto/asn1/x_long.o" =>
                [
                    "crypto/asn1/x_long.c",
                ],
            "crypto/asn1/x_pkey.o" =>
                [
                    "crypto/asn1/x_pkey.c",
                ],
            "crypto/asn1/x_sig.o" =>
                [
                    "crypto/asn1/x_sig.c",
                ],
            "crypto/asn1/x_spki.o" =>
                [
                    "crypto/asn1/x_spki.c",
                ],
            "crypto/asn1/x_val.o" =>
                [
                    "crypto/asn1/x_val.c",
                ],
            "crypto/async/arch/async_null.o" =>
                [
                    "crypto/async/arch/async_null.c",
                ],
            "crypto/async/arch/async_posix.o" =>
                [
                    "crypto/async/arch/async_posix.c",
                ],
            "crypto/async/arch/async_win.o" =>
                [
                    "crypto/async/arch/async_win.c",
                ],
            "crypto/async/async.o" =>
                [
                    "crypto/async/async.c",
                ],
            "crypto/async/async_err.o" =>
                [
                    "crypto/async/async_err.c",
                ],
            "crypto/async/async_wait.o" =>
                [
                    "crypto/async/async_wait.c",
                ],
            "crypto/bf/bf_cfb64.o" =>
                [
                    "crypto/bf/bf_cfb64.c",
                ],
            "crypto/bf/bf_ecb.o" =>
                [
                    "crypto/bf/bf_ecb.c",
                ],
            "crypto/bf/bf_enc.o" =>
                [
                    "crypto/bf/bf_enc.c",
                ],
            "crypto/bf/bf_ofb64.o" =>
                [
                    "crypto/bf/bf_ofb64.c",
                ],
            "crypto/bf/bf_skey.o" =>
                [
                    "crypto/bf/bf_skey.c",
                ],
            "crypto/bio/b_addr.o" =>
                [
                    "crypto/bio/b_addr.c",
                ],
            "crypto/bio/b_dump.o" =>
                [
                    "crypto/bio/b_dump.c",
                ],
            "crypto/bio/b_print.o" =>
                [
                    "crypto/bio/b_print.c",
                ],
            "crypto/bio/b_sock.o" =>
                [
                    "crypto/bio/b_sock.c",
                ],
            "crypto/bio/b_sock2.o" =>
                [
                    "crypto/bio/b_sock2.c",
                ],
            "crypto/bio/bf_buff.o" =>
                [
                    "crypto/bio/bf_buff.c",
                ],
            "crypto/bio/bf_lbuf.o" =>
                [
                    "crypto/bio/bf_lbuf.c",
                ],
            "crypto/bio/bf_nbio.o" =>
                [
                    "crypto/bio/bf_nbio.c",
                ],
            "crypto/bio/bf_null.o" =>
                [
                    "crypto/bio/bf_null.c",
                ],
            "crypto/bio/bio_cb.o" =>
                [
                    "crypto/bio/bio_cb.c",
                ],
            "crypto/bio/bio_err.o" =>
                [
                    "crypto/bio/bio_err.c",
                ],
            "crypto/bio/bio_lib.o" =>
                [
                    "crypto/bio/bio_lib.c",
                ],
            "crypto/bio/bio_meth.o" =>
                [
                    "crypto/bio/bio_meth.c",
                ],
            "crypto/bio/bss_acpt.o" =>
                [
                    "crypto/bio/bss_acpt.c",
                ],
            "crypto/bio/bss_bio.o" =>
                [
                    "crypto/bio/bss_bio.c",
                ],
            "crypto/bio/bss_conn.o" =>
                [
                    "crypto/bio/bss_conn.c",
                ],
            "crypto/bio/bss_dgram.o" =>
                [
                    "crypto/bio/bss_dgram.c",
                ],
            "crypto/bio/bss_fd.o" =>
                [
                    "crypto/bio/bss_fd.c",
                ],
            "crypto/bio/bss_file.o" =>
                [
                    "crypto/bio/bss_file.c",
                ],
            "crypto/bio/bss_log.o" =>
                [
                    "crypto/bio/bss_log.c",
                ],
            "crypto/bio/bss_mem.o" =>
                [
                    "crypto/bio/bss_mem.c",
                ],
            "crypto/bio/bss_null.o" =>
                [
                    "crypto/bio/bss_null.c",
                ],
            "crypto/bio/bss_sock.o" =>
                [
                    "crypto/bio/bss_sock.c",
                ],
            "crypto/blake2/blake2b.o" =>
                [
                    "crypto/blake2/blake2b.c",
                ],
            "crypto/blake2/blake2s.o" =>
                [
                    "crypto/blake2/blake2s.c",
                ],
            "crypto/blake2/m_blake2b.o" =>
                [
                    "crypto/blake2/m_blake2b.c",
                ],
            "crypto/blake2/m_blake2s.o" =>
                [
                    "crypto/blake2/m_blake2s.c",
                ],
            "crypto/bn/bn_add.o" =>
                [
                    "crypto/bn/bn_add.c",
                ],
            "crypto/bn/bn_asm.o" =>
                [
                    "crypto/bn/bn_asm.c",
                ],
            "crypto/bn/bn_blind.o" =>
                [
                    "crypto/bn/bn_blind.c",
                ],
            "crypto/bn/bn_const.o" =>
                [
                    "crypto/bn/bn_const.c",
                ],
            "crypto/bn/bn_ctx.o" =>
                [
                    "crypto/bn/bn_ctx.c",
                ],
            "crypto/bn/bn_depr.o" =>
                [
                    "crypto/bn/bn_depr.c",
                ],
            "crypto/bn/bn_dh.o" =>
                [
                    "crypto/bn/bn_dh.c",
                ],
            "crypto/bn/bn_div.o" =>
                [
                    "crypto/bn/bn_div.c",
                ],
            "crypto/bn/bn_err.o" =>
                [
                    "crypto/bn/bn_err.c",
                ],
            "crypto/bn/bn_exp.o" =>
                [
                    "crypto/bn/bn_exp.c",
                ],
            "crypto/bn/bn_exp2.o" =>
                [
                    "crypto/bn/bn_exp2.c",
                ],
            "crypto/bn/bn_gcd.o" =>
                [
                    "crypto/bn/bn_gcd.c",
                ],
            "crypto/bn/bn_gf2m.o" =>
                [
                    "crypto/bn/bn_gf2m.c",
                ],
            "crypto/bn/bn_intern.o" =>
                [
                    "crypto/bn/bn_intern.c",
                ],
            "crypto/bn/bn_kron.o" =>
                [
                    "crypto/bn/bn_kron.c",
                ],
            "crypto/bn/bn_lib.o" =>
                [
                    "crypto/bn/bn_lib.c",
                ],
            "crypto/bn/bn_mod.o" =>
                [
                    "crypto/bn/bn_mod.c",
                ],
            "crypto/bn/bn_mont.o" =>
                [
                    "crypto/bn/bn_mont.c",
                ],
            "crypto/bn/bn_mpi.o" =>
                [
                    "crypto/bn/bn_mpi.c",
                ],
            "crypto/bn/bn_mul.o" =>
                [
                    "crypto/bn/bn_mul.c",
                ],
            "crypto/bn/bn_nist.o" =>
                [
                    "crypto/bn/bn_nist.c",
                ],
            "crypto/bn/bn_prime.o" =>
                [
                    "crypto/bn/bn_prime.c",
                ],
            "crypto/bn/bn_print.o" =>
                [
                    "crypto/bn/bn_print.c",
                ],
            "crypto/bn/bn_rand.o" =>
                [
                    "crypto/bn/bn_rand.c",
                ],
            "crypto/bn/bn_recp.o" =>
                [
                    "crypto/bn/bn_recp.c",
                ],
            "crypto/bn/bn_shift.o" =>
                [
                    "crypto/bn/bn_shift.c",
                ],
            "crypto/bn/bn_sqr.o" =>
                [
                    "crypto/bn/bn_sqr.c",
                ],
            "crypto/bn/bn_sqrt.o" =>
                [
                    "crypto/bn/bn_sqrt.c",
                ],
            "crypto/bn/bn_srp.o" =>
                [
                    "crypto/bn/bn_srp.c",
                ],
            "crypto/bn/bn_word.o" =>
                [
                    "crypto/bn/bn_word.c",
                ],
            "crypto/bn/bn_x931p.o" =>
                [
                    "crypto/bn/bn_x931p.c",
                ],
            "crypto/buffer/buf_err.o" =>
                [
                    "crypto/buffer/buf_err.c",
                ],
            "crypto/buffer/buffer.o" =>
                [
                    "crypto/buffer/buffer.c",
                ],
            "crypto/cast/c_cfb64.o" =>
                [
                    "crypto/cast/c_cfb64.c",
                ],
            "crypto/cast/c_ecb.o" =>
                [
                    "crypto/cast/c_ecb.c",
                ],
            "crypto/cast/c_enc.o" =>
                [
                    "crypto/cast/c_enc.c",
                ],
            "crypto/cast/c_ofb64.o" =>
                [
                    "crypto/cast/c_ofb64.c",
                ],
            "crypto/cast/c_skey.o" =>
                [
                    "crypto/cast/c_skey.c",
                ],
            "crypto/chacha/chacha_enc.o" =>
                [
                    "crypto/chacha/chacha_enc.c",
                ],
            "crypto/cmac/cm_ameth.o" =>
                [
                    "crypto/cmac/cm_ameth.c",
                ],
            "crypto/cmac/cm_pmeth.o" =>
                [
                    "crypto/cmac/cm_pmeth.c",
                ],
            "crypto/cmac/cmac.o" =>
                [
                    "crypto/cmac/cmac.c",
                ],
            "crypto/cms/cms_asn1.o" =>
                [
                    "crypto/cms/cms_asn1.c",
                ],
            "crypto/cms/cms_att.o" =>
                [
                    "crypto/cms/cms_att.c",
                ],
            "crypto/cms/cms_cd.o" =>
                [
                    "crypto/cms/cms_cd.c",
                ],
            "crypto/cms/cms_dd.o" =>
                [
                    "crypto/cms/cms_dd.c",
                ],
            "crypto/cms/cms_enc.o" =>
                [
                    "crypto/cms/cms_enc.c",
                ],
            "crypto/cms/cms_env.o" =>
                [
                    "crypto/cms/cms_env.c",
                ],
            "crypto/cms/cms_err.o" =>
                [
                    "crypto/cms/cms_err.c",
                ],
            "crypto/cms/cms_ess.o" =>
                [
                    "crypto/cms/cms_ess.c",
                ],
            "crypto/cms/cms_io.o" =>
                [
                    "crypto/cms/cms_io.c",
                ],
            "crypto/cms/cms_kari.o" =>
                [
                    "crypto/cms/cms_kari.c",
                ],
            "crypto/cms/cms_lib.o" =>
                [
                    "crypto/cms/cms_lib.c",
                ],
            "crypto/cms/cms_pwri.o" =>
                [
                    "crypto/cms/cms_pwri.c",
                ],
            "crypto/cms/cms_sd.o" =>
                [
                    "crypto/cms/cms_sd.c",
                ],
            "crypto/cms/cms_smime.o" =>
                [
                    "crypto/cms/cms_smime.c",
                ],
            "crypto/comp/c_zlib.o" =>
                [
                    "crypto/comp/c_zlib.c",
                ],
            "crypto/comp/comp_err.o" =>
                [
                    "crypto/comp/comp_err.c",
                ],
            "crypto/comp/comp_lib.o" =>
                [
                    "crypto/comp/comp_lib.c",
                ],
            "crypto/conf/conf_api.o" =>
                [
                    "crypto/conf/conf_api.c",
                ],
            "crypto/conf/conf_def.o" =>
                [
                    "crypto/conf/conf_def.c",
                ],
            "crypto/conf/conf_err.o" =>
                [
                    "crypto/conf/conf_err.c",
                ],
            "crypto/conf/conf_lib.o" =>
                [
                    "crypto/conf/conf_lib.c",
                ],
            "crypto/conf/conf_mall.o" =>
                [
                    "crypto/conf/conf_mall.c",
                ],
            "crypto/conf/conf_mod.o" =>
                [
                    "crypto/conf/conf_mod.c",
                ],
            "crypto/conf/conf_sap.o" =>
                [
                    "crypto/conf/conf_sap.c",
                ],
            "crypto/conf/conf_ssl.o" =>
                [
                    "crypto/conf/conf_ssl.c",
                ],
            "crypto/cpt_err.o" =>
                [
                    "crypto/cpt_err.c",
                ],
            "crypto/cryptlib.o" =>
                [
                    "crypto/cryptlib.c",
                ],
            "crypto/ct/ct_b64.o" =>
                [
                    "crypto/ct/ct_b64.c",
                ],
            "crypto/ct/ct_err.o" =>
                [
                    "crypto/ct/ct_err.c",
                ],
            "crypto/ct/ct_log.o" =>
                [
                    "crypto/ct/ct_log.c",
                ],
            "crypto/ct/ct_oct.o" =>
                [
                    "crypto/ct/ct_oct.c",
                ],
            "crypto/ct/ct_policy.o" =>
                [
                    "crypto/ct/ct_policy.c",
                ],
            "crypto/ct/ct_prn.o" =>
                [
                    "crypto/ct/ct_prn.c",
                ],
            "crypto/ct/ct_sct.o" =>
                [
                    "crypto/ct/ct_sct.c",
                ],
            "crypto/ct/ct_sct_ctx.o" =>
                [
                    "crypto/ct/ct_sct_ctx.c",
                ],
            "crypto/ct/ct_vfy.o" =>
                [
                    "crypto/ct/ct_vfy.c",
                ],
            "crypto/ct/ct_x509v3.o" =>
                [
                    "crypto/ct/ct_x509v3.c",
                ],
            "crypto/ctype.o" =>
                [
                    "crypto/ctype.c",
                ],
            "crypto/cversion.o" =>
                [
                    "crypto/cversion.c",
                ],
            "crypto/des/cbc_cksm.o" =>
                [
                    "crypto/des/cbc_cksm.c",
                ],
            "crypto/des/cbc_enc.o" =>
                [
                    "crypto/des/cbc_enc.c",
                ],
            "crypto/des/cfb64ede.o" =>
                [
                    "crypto/des/cfb64ede.c",
                ],
            "crypto/des/cfb64enc.o" =>
                [
                    "crypto/des/cfb64enc.c",
                ],
            "crypto/des/cfb_enc.o" =>
                [
                    "crypto/des/cfb_enc.c",
                ],
            "crypto/des/des_enc.o" =>
                [
                    "crypto/des/des_enc.c",
                ],
            "crypto/des/ecb3_enc.o" =>
                [
                    "crypto/des/ecb3_enc.c",
                ],
            "crypto/des/ecb_enc.o" =>
                [
                    "crypto/des/ecb_enc.c",
                ],
            "crypto/des/fcrypt.o" =>
                [
                    "crypto/des/fcrypt.c",
                ],
            "crypto/des/fcrypt_b.o" =>
                [
                    "crypto/des/fcrypt_b.c",
                ],
            "crypto/des/ofb64ede.o" =>
                [
                    "crypto/des/ofb64ede.c",
                ],
            "crypto/des/ofb64enc.o" =>
                [
                    "crypto/des/ofb64enc.c",
                ],
            "crypto/des/ofb_enc.o" =>
                [
                    "crypto/des/ofb_enc.c",
                ],
            "crypto/des/pcbc_enc.o" =>
                [
                    "crypto/des/pcbc_enc.c",
                ],
            "crypto/des/qud_cksm.o" =>
                [
                    "crypto/des/qud_cksm.c",
                ],
            "crypto/des/rand_key.o" =>
                [
                    "crypto/des/rand_key.c",
                ],
            "crypto/des/set_key.o" =>
                [
                    "crypto/des/set_key.c",
                ],
            "crypto/des/str2key.o" =>
                [
                    "crypto/des/str2key.c",
                ],
            "crypto/des/xcbc_enc.o" =>
                [
                    "crypto/des/xcbc_enc.c",
                ],
            "crypto/dh/dh_ameth.o" =>
                [
                    "crypto/dh/dh_ameth.c",
                ],
            "crypto/dh/dh_asn1.o" =>
                [
                    "crypto/dh/dh_asn1.c",
                ],
            "crypto/dh/dh_check.o" =>
                [
                    "crypto/dh/dh_check.c",
                ],
            "crypto/dh/dh_depr.o" =>
                [
                    "crypto/dh/dh_depr.c",
                ],
            "crypto/dh/dh_err.o" =>
                [
                    "crypto/dh/dh_err.c",
                ],
            "crypto/dh/dh_gen.o" =>
                [
                    "crypto/dh/dh_gen.c",
                ],
            "crypto/dh/dh_kdf.o" =>
                [
                    "crypto/dh/dh_kdf.c",
                ],
            "crypto/dh/dh_key.o" =>
                [
                    "crypto/dh/dh_key.c",
                ],
            "crypto/dh/dh_lib.o" =>
                [
                    "crypto/dh/dh_lib.c",
                ],
            "crypto/dh/dh_meth.o" =>
                [
                    "crypto/dh/dh_meth.c",
                ],
            "crypto/dh/dh_pmeth.o" =>
                [
                    "crypto/dh/dh_pmeth.c",
                ],
            "crypto/dh/dh_prn.o" =>
                [
                    "crypto/dh/dh_prn.c",
                ],
            "crypto/dh/dh_rfc5114.o" =>
                [
                    "crypto/dh/dh_rfc5114.c",
                ],
            "crypto/dh/dh_rfc7919.o" =>
                [
                    "crypto/dh/dh_rfc7919.c",
                ],
            "crypto/dsa/dsa_ameth.o" =>
                [
                    "crypto/dsa/dsa_ameth.c",
                ],
            "crypto/dsa/dsa_asn1.o" =>
                [
                    "crypto/dsa/dsa_asn1.c",
                ],
            "crypto/dsa/dsa_depr.o" =>
                [
                    "crypto/dsa/dsa_depr.c",
                ],
            "crypto/dsa/dsa_err.o" =>
                [
                    "crypto/dsa/dsa_err.c",
                ],
            "crypto/dsa/dsa_gen.o" =>
                [
                    "crypto/dsa/dsa_gen.c",
                ],
            "crypto/dsa/dsa_key.o" =>
                [
                    "crypto/dsa/dsa_key.c",
                ],
            "crypto/dsa/dsa_lib.o" =>
                [
                    "crypto/dsa/dsa_lib.c",
                ],
            "crypto/dsa/dsa_meth.o" =>
                [
                    "crypto/dsa/dsa_meth.c",
                ],
            "crypto/dsa/dsa_ossl.o" =>
                [
                    "crypto/dsa/dsa_ossl.c",
                ],
            "crypto/dsa/dsa_pmeth.o" =>
                [
                    "crypto/dsa/dsa_pmeth.c",
                ],
            "crypto/dsa/dsa_prn.o" =>
                [
                    "crypto/dsa/dsa_prn.c",
                ],
            "crypto/dsa/dsa_sign.o" =>
                [
                    "crypto/dsa/dsa_sign.c",
                ],
            "crypto/dsa/dsa_vrf.o" =>
                [
                    "crypto/dsa/dsa_vrf.c",
                ],
            "crypto/dso/dso_dl.o" =>
                [
                    "crypto/dso/dso_dl.c",
                ],
            "crypto/dso/dso_dlfcn.o" =>
                [
                    "crypto/dso/dso_dlfcn.c",
                ],
            "crypto/dso/dso_err.o" =>
                [
                    "crypto/dso/dso_err.c",
                ],
            "crypto/dso/dso_lib.o" =>
                [
                    "crypto/dso/dso_lib.c",
                ],
            "crypto/dso/dso_openssl.o" =>
                [
                    "crypto/dso/dso_openssl.c",
                ],
            "crypto/dso/dso_vms.o" =>
                [
                    "crypto/dso/dso_vms.c",
                ],
            "crypto/dso/dso_win32.o" =>
                [
                    "crypto/dso/dso_win32.c",
                ],
            "crypto/ebcdic.o" =>
                [
                    "crypto/ebcdic.c",
                ],
            "crypto/ec/curve25519.o" =>
                [
                    "crypto/ec/curve25519.c",
                ],
            "crypto/ec/curve448/arch_32/f_impl.o" =>
                [
                    "crypto/ec/curve448/arch_32/f_impl.c",
                ],
            "crypto/ec/curve448/curve448.o" =>
                [
                    "crypto/ec/curve448/curve448.c",
                ],
            "crypto/ec/curve448/curve448_tables.o" =>
                [
                    "crypto/ec/curve448/curve448_tables.c",
                ],
            "crypto/ec/curve448/eddsa.o" =>
                [
                    "crypto/ec/curve448/eddsa.c",
                ],
            "crypto/ec/curve448/f_generic.o" =>
                [
                    "crypto/ec/curve448/f_generic.c",
                ],
            "crypto/ec/curve448/scalar.o" =>
                [
                    "crypto/ec/curve448/scalar.c",
                ],
            "crypto/ec/ec2_oct.o" =>
                [
                    "crypto/ec/ec2_oct.c",
                ],
            "crypto/ec/ec2_smpl.o" =>
                [
                    "crypto/ec/ec2_smpl.c",
                ],
            "crypto/ec/ec_ameth.o" =>
                [
                    "crypto/ec/ec_ameth.c",
                ],
            "crypto/ec/ec_asn1.o" =>
                [
                    "crypto/ec/ec_asn1.c",
                ],
            "crypto/ec/ec_check.o" =>
                [
                    "crypto/ec/ec_check.c",
                ],
            "crypto/ec/ec_curve.o" =>
                [
                    "crypto/ec/ec_curve.c",
                ],
            "crypto/ec/ec_cvt.o" =>
                [
                    "crypto/ec/ec_cvt.c",
                ],
            "crypto/ec/ec_err.o" =>
                [
                    "crypto/ec/ec_err.c",
                ],
            "crypto/ec/ec_key.o" =>
                [
                    "crypto/ec/ec_key.c",
                ],
            "crypto/ec/ec_kmeth.o" =>
                [
                    "crypto/ec/ec_kmeth.c",
                ],
            "crypto/ec/ec_lib.o" =>
                [
                    "crypto/ec/ec_lib.c",
                ],
            "crypto/ec/ec_mult.o" =>
                [
                    "crypto/ec/ec_mult.c",
                ],
            "crypto/ec/ec_oct.o" =>
                [
                    "crypto/ec/ec_oct.c",
                ],
            "crypto/ec/ec_pmeth.o" =>
                [
                    "crypto/ec/ec_pmeth.c",
                ],
            "crypto/ec/ec_print.o" =>
                [
                    "crypto/ec/ec_print.c",
                ],
            "crypto/ec/ecdh_kdf.o" =>
                [
                    "crypto/ec/ecdh_kdf.c",
                ],
            "crypto/ec/ecdh_ossl.o" =>
                [
                    "crypto/ec/ecdh_ossl.c",
                ],
            "crypto/ec/ecdsa_ossl.o" =>
                [
                    "crypto/ec/ecdsa_ossl.c",
                ],
            "crypto/ec/ecdsa_sign.o" =>
                [
                    "crypto/ec/ecdsa_sign.c",
                ],
            "crypto/ec/ecdsa_vrf.o" =>
                [
                    "crypto/ec/ecdsa_vrf.c",
                ],
            "crypto/ec/eck_prn.o" =>
                [
                    "crypto/ec/eck_prn.c",
                ],
            "crypto/ec/ecp_mont.o" =>
                [
                    "crypto/ec/ecp_mont.c",
                ],
            "crypto/ec/ecp_nist.o" =>
                [
                    "crypto/ec/ecp_nist.c",
                ],
            "crypto/ec/ecp_nistp224.o" =>
                [
                    "crypto/ec/ecp_nistp224.c",
                ],
            "crypto/ec/ecp_nistp256.o" =>
                [
                    "crypto/ec/ecp_nistp256.c",
                ],
            "crypto/ec/ecp_nistp521.o" =>
                [
                    "crypto/ec/ecp_nistp521.c",
                ],
            "crypto/ec/ecp_nistputil.o" =>
                [
                    "crypto/ec/ecp_nistputil.c",
                ],
            "crypto/ec/ecp_oct.o" =>
                [
                    "crypto/ec/ecp_oct.c",
                ],
            "crypto/ec/ecp_smpl.o" =>
                [
                    "crypto/ec/ecp_smpl.c",
                ],
            "crypto/ec/ecx_meth.o" =>
                [
                    "crypto/ec/ecx_meth.c",
                ],
            "crypto/engine/eng_all.o" =>
                [
                    "crypto/engine/eng_all.c",
                ],
            "crypto/engine/eng_cnf.o" =>
                [
                    "crypto/engine/eng_cnf.c",
                ],
            "crypto/engine/eng_ctrl.o" =>
                [
                    "crypto/engine/eng_ctrl.c",
                ],
            "crypto/engine/eng_dyn.o" =>
                [
                    "crypto/engine/eng_dyn.c",
                ],
            "crypto/engine/eng_err.o" =>
                [
                    "crypto/engine/eng_err.c",
                ],
            "crypto/engine/eng_fat.o" =>
                [
                    "crypto/engine/eng_fat.c",
                ],
            "crypto/engine/eng_init.o" =>
                [
                    "crypto/engine/eng_init.c",
                ],
            "crypto/engine/eng_lib.o" =>
                [
                    "crypto/engine/eng_lib.c",
                ],
            "crypto/engine/eng_list.o" =>
                [
                    "crypto/engine/eng_list.c",
                ],
            "crypto/engine/eng_openssl.o" =>
                [
                    "crypto/engine/eng_openssl.c",
                ],
            "crypto/engine/eng_pkey.o" =>
                [
                    "crypto/engine/eng_pkey.c",
                ],
            "crypto/engine/eng_rdrand.o" =>
                [
                    "crypto/engine/eng_rdrand.c",
                ],
            "crypto/engine/eng_table.o" =>
                [
                    "crypto/engine/eng_table.c",
                ],
            "crypto/engine/tb_asnmth.o" =>
                [
                    "crypto/engine/tb_asnmth.c",
                ],
            "crypto/engine/tb_cipher.o" =>
                [
                    "crypto/engine/tb_cipher.c",
                ],
            "crypto/engine/tb_dh.o" =>
                [
                    "crypto/engine/tb_dh.c",
                ],
            "crypto/engine/tb_digest.o" =>
                [
                    "crypto/engine/tb_digest.c",
                ],
            "crypto/engine/tb_dsa.o" =>
                [
                    "crypto/engine/tb_dsa.c",
                ],
            "crypto/engine/tb_eckey.o" =>
                [
                    "crypto/engine/tb_eckey.c",
                ],
            "crypto/engine/tb_pkmeth.o" =>
                [
                    "crypto/engine/tb_pkmeth.c",
                ],
            "crypto/engine/tb_rand.o" =>
                [
                    "crypto/engine/tb_rand.c",
                ],
            "crypto/engine/tb_rsa.o" =>
                [
                    "crypto/engine/tb_rsa.c",
                ],
            "crypto/err/err.o" =>
                [
                    "crypto/err/err.c",
                ],
            "crypto/err/err_all.o" =>
                [
                    "crypto/err/err_all.c",
                ],
            "crypto/err/err_prn.o" =>
                [
                    "crypto/err/err_prn.c",
                ],
            "crypto/evp/bio_b64.o" =>
                [
                    "crypto/evp/bio_b64.c",
                ],
            "crypto/evp/bio_enc.o" =>
                [
                    "crypto/evp/bio_enc.c",
                ],
            "crypto/evp/bio_md.o" =>
                [
                    "crypto/evp/bio_md.c",
                ],
            "crypto/evp/bio_ok.o" =>
                [
                    "crypto/evp/bio_ok.c",
                ],
            "crypto/evp/c_allc.o" =>
                [
                    "crypto/evp/c_allc.c",
                ],
            "crypto/evp/c_alld.o" =>
                [
                    "crypto/evp/c_alld.c",
                ],
            "crypto/evp/cmeth_lib.o" =>
                [
                    "crypto/evp/cmeth_lib.c",
                ],
            "crypto/evp/digest.o" =>
                [
                    "crypto/evp/digest.c",
                ],
            "crypto/evp/e_aes.o" =>
                [
                    "crypto/evp/e_aes.c",
                ],
            "crypto/evp/e_aes_cbc_hmac_sha1.o" =>
                [
                    "crypto/evp/e_aes_cbc_hmac_sha1.c",
                ],
            "crypto/evp/e_aes_cbc_hmac_sha256.o" =>
                [
                    "crypto/evp/e_aes_cbc_hmac_sha256.c",
                ],
            "crypto/evp/e_aria.o" =>
                [
                    "crypto/evp/e_aria.c",
                ],
            "crypto/evp/e_bf.o" =>
                [
                    "crypto/evp/e_bf.c",
                ],
            "crypto/evp/e_camellia.o" =>
                [
                    "crypto/evp/e_camellia.c",
                ],
            "crypto/evp/e_cast.o" =>
                [
                    "crypto/evp/e_cast.c",
                ],
            "crypto/evp/e_chacha20_poly1305.o" =>
                [
                    "crypto/evp/e_chacha20_poly1305.c",
                ],
            "crypto/evp/e_des.o" =>
                [
                    "crypto/evp/e_des.c",
                ],
            "crypto/evp/e_des3.o" =>
                [
                    "crypto/evp/e_des3.c",
                ],
            "crypto/evp/e_idea.o" =>
                [
                    "crypto/evp/e_idea.c",
                ],
            "crypto/evp/e_null.o" =>
                [
                    "crypto/evp/e_null.c",
                ],
            "crypto/evp/e_old.o" =>
                [
                    "crypto/evp/e_old.c",
                ],
            "crypto/evp/e_rc2.o" =>
                [
                    "crypto/evp/e_rc2.c",
                ],
            "crypto/evp/e_rc4.o" =>
                [
                    "crypto/evp/e_rc4.c",
                ],
            "crypto/evp/e_rc4_hmac_md5.o" =>
                [
                    "crypto/evp/e_rc4_hmac_md5.c",
                ],
            "crypto/evp/e_rc5.o" =>
                [
                    "crypto/evp/e_rc5.c",
                ],
            "crypto/evp/e_seed.o" =>
                [
                    "crypto/evp/e_seed.c",
                ],
            "crypto/evp/e_sm4.o" =>
                [
                    "crypto/evp/e_sm4.c",
                ],
            "crypto/evp/e_xcbc_d.o" =>
                [
                    "crypto/evp/e_xcbc_d.c",
                ],
            "crypto/evp/encode.o" =>
                [
                    "crypto/evp/encode.c",
                ],
            "crypto/evp/evp_cnf.o" =>
                [
                    "crypto/evp/evp_cnf.c",
                ],
            "crypto/evp/evp_enc.o" =>
                [
                    "crypto/evp/evp_enc.c",
                ],
            "crypto/evp/evp_err.o" =>
                [
                    "crypto/evp/evp_err.c",
                ],
            "crypto/evp/evp_key.o" =>
                [
                    "crypto/evp/evp_key.c",
                ],
            "crypto/evp/evp_lib.o" =>
                [
                    "crypto/evp/evp_lib.c",
                ],
            "crypto/evp/evp_pbe.o" =>
                [
                    "crypto/evp/evp_pbe.c",
                ],
            "crypto/evp/evp_pkey.o" =>
                [
                    "crypto/evp/evp_pkey.c",
                ],
            "crypto/evp/m_md2.o" =>
                [
                    "crypto/evp/m_md2.c",
                ],
            "crypto/evp/m_md4.o" =>
                [
                    "crypto/evp/m_md4.c",
                ],
            "crypto/evp/m_md5.o" =>
                [
                    "crypto/evp/m_md5.c",
                ],
            "crypto/evp/m_md5_sha1.o" =>
                [
                    "crypto/evp/m_md5_sha1.c",
                ],
            "crypto/evp/m_mdc2.o" =>
                [
                    "crypto/evp/m_mdc2.c",
                ],
            "crypto/evp/m_null.o" =>
                [
                    "crypto/evp/m_null.c",
                ],
            "crypto/evp/m_ripemd.o" =>
                [
                    "crypto/evp/m_ripemd.c",
                ],
            "crypto/evp/m_sha1.o" =>
                [
                    "crypto/evp/m_sha1.c",
                ],
            "crypto/evp/m_sha3.o" =>
                [
                    "crypto/evp/m_sha3.c",
                ],
            "crypto/evp/m_sigver.o" =>
                [
                    "crypto/evp/m_sigver.c",
                ],
            "crypto/evp/m_wp.o" =>
                [
                    "crypto/evp/m_wp.c",
                ],
            "crypto/evp/names.o" =>
                [
                    "crypto/evp/names.c",
                ],
            "crypto/evp/p5_crpt.o" =>
                [
                    "crypto/evp/p5_crpt.c",
                ],
            "crypto/evp/p5_crpt2.o" =>
                [
                    "crypto/evp/p5_crpt2.c",
                ],
            "crypto/evp/p_dec.o" =>
                [
                    "crypto/evp/p_dec.c",
                ],
            "crypto/evp/p_enc.o" =>
                [
                    "crypto/evp/p_enc.c",
                ],
            "crypto/evp/p_lib.o" =>
                [
                    "crypto/evp/p_lib.c",
                ],
            "crypto/evp/p_open.o" =>
                [
                    "crypto/evp/p_open.c",
                ],
            "crypto/evp/p_seal.o" =>
                [
                    "crypto/evp/p_seal.c",
                ],
            "crypto/evp/p_sign.o" =>
                [
                    "crypto/evp/p_sign.c",
                ],
            "crypto/evp/p_verify.o" =>
                [
                    "crypto/evp/p_verify.c",
                ],
            "crypto/evp/pbe_scrypt.o" =>
                [
                    "crypto/evp/pbe_scrypt.c",
                ],
            "crypto/evp/pmeth_fn.o" =>
                [
                    "crypto/evp/pmeth_fn.c",
                ],
            "crypto/evp/pmeth_gn.o" =>
                [
                    "crypto/evp/pmeth_gn.c",
                ],
            "crypto/evp/pmeth_lib.o" =>
                [
                    "crypto/evp/pmeth_lib.c",
                ],
            "crypto/ex_data.o" =>
                [
                    "crypto/ex_data.c",
                ],
            "crypto/getenv.o" =>
                [
                    "crypto/getenv.c",
                ],
            "crypto/hmac/hm_ameth.o" =>
                [
                    "crypto/hmac/hm_ameth.c",
                ],
            "crypto/hmac/hm_pmeth.o" =>
                [
                    "crypto/hmac/hm_pmeth.c",
                ],
            "crypto/hmac/hmac.o" =>
                [
                    "crypto/hmac/hmac.c",
                ],
            "crypto/init.o" =>
                [
                    "crypto/init.c",
                ],
            "crypto/kdf/hkdf.o" =>
                [
                    "crypto/kdf/hkdf.c",
                ],
            "crypto/kdf/kdf_err.o" =>
                [
                    "crypto/kdf/kdf_err.c",
                ],
            "crypto/kdf/scrypt.o" =>
                [
                    "crypto/kdf/scrypt.c",
                ],
            "crypto/kdf/tls1_prf.o" =>
                [
                    "crypto/kdf/tls1_prf.c",
                ],
            "crypto/lhash/lh_stats.o" =>
                [
                    "crypto/lhash/lh_stats.c",
                ],
            "crypto/lhash/lhash.o" =>
                [
                    "crypto/lhash/lhash.c",
                ],
            "crypto/md4/md4_dgst.o" =>
                [
                    "crypto/md4/md4_dgst.c",
                ],
            "crypto/md4/md4_one.o" =>
                [
                    "crypto/md4/md4_one.c",
                ],
            "crypto/md5/md5_dgst.o" =>
                [
                    "crypto/md5/md5_dgst.c",
                ],
            "crypto/md5/md5_one.o" =>
                [
                    "crypto/md5/md5_one.c",
                ],
            "crypto/mem.o" =>
                [
                    "crypto/mem.c",
                ],
            "crypto/mem_clr.o" =>
                [
                    "crypto/mem_clr.c",
                ],
            "crypto/mem_dbg.o" =>
                [
                    "crypto/mem_dbg.c",
                ],
            "crypto/mem_sec.o" =>
                [
                    "crypto/mem_sec.c",
                ],
            "crypto/modes/cbc128.o" =>
                [
                    "crypto/modes/cbc128.c",
                ],
            "crypto/modes/ccm128.o" =>
                [
                    "crypto/modes/ccm128.c",
                ],
            "crypto/modes/cfb128.o" =>
                [
                    "crypto/modes/cfb128.c",
                ],
            "crypto/modes/ctr128.o" =>
                [
                    "crypto/modes/ctr128.c",
                ],
            "crypto/modes/cts128.o" =>
                [
                    "crypto/modes/cts128.c",
                ],
            "crypto/modes/gcm128.o" =>
                [
                    "crypto/modes/gcm128.c",
                ],
            "crypto/modes/ocb128.o" =>
                [
                    "crypto/modes/ocb128.c",
                ],
            "crypto/modes/ofb128.o" =>
                [
                    "crypto/modes/ofb128.c",
                ],
            "crypto/modes/wrap128.o" =>
                [
                    "crypto/modes/wrap128.c",
                ],
            "crypto/modes/xts128.o" =>
                [
                    "crypto/modes/xts128.c",
                ],
            "crypto/o_dir.o" =>
                [
                    "crypto/o_dir.c",
                ],
            "crypto/o_fips.o" =>
                [
                    "crypto/o_fips.c",
                ],
            "crypto/o_fopen.o" =>
                [
                    "crypto/o_fopen.c",
                ],
            "crypto/o_init.o" =>
                [
                    "crypto/o_init.c",
                ],
            "crypto/o_str.o" =>
                [
                    "crypto/o_str.c",
                ],
            "crypto/o_time.o" =>
                [
                    "crypto/o_time.c",
                ],
            "crypto/objects/o_names.o" =>
                [
                    "crypto/objects/o_names.c",
                ],
            "crypto/objects/obj_dat.o" =>
                [
                    "crypto/objects/obj_dat.c",
                ],
            "crypto/objects/obj_err.o" =>
                [
                    "crypto/objects/obj_err.c",
                ],
            "crypto/objects/obj_lib.o" =>
                [
                    "crypto/objects/obj_lib.c",
                ],
            "crypto/objects/obj_xref.o" =>
                [
                    "crypto/objects/obj_xref.c",
                ],
            "crypto/ocsp/ocsp_asn.o" =>
                [
                    "crypto/ocsp/ocsp_asn.c",
                ],
            "crypto/ocsp/ocsp_cl.o" =>
                [
                    "crypto/ocsp/ocsp_cl.c",
                ],
            "crypto/ocsp/ocsp_err.o" =>
                [
                    "crypto/ocsp/ocsp_err.c",
                ],
            "crypto/ocsp/ocsp_ext.o" =>
                [
                    "crypto/ocsp/ocsp_ext.c",
                ],
            "crypto/ocsp/ocsp_ht.o" =>
                [
                    "crypto/ocsp/ocsp_ht.c",
                ],
            "crypto/ocsp/ocsp_lib.o" =>
                [
                    "crypto/ocsp/ocsp_lib.c",
                ],
            "crypto/ocsp/ocsp_prn.o" =>
                [
                    "crypto/ocsp/ocsp_prn.c",
                ],
            "crypto/ocsp/ocsp_srv.o" =>
                [
                    "crypto/ocsp/ocsp_srv.c",
                ],
            "crypto/ocsp/ocsp_vfy.o" =>
                [
                    "crypto/ocsp/ocsp_vfy.c",
                ],
            "crypto/ocsp/v3_ocsp.o" =>
                [
                    "crypto/ocsp/v3_ocsp.c",
                ],
            "crypto/pem/pem_all.o" =>
                [
                    "crypto/pem/pem_all.c",
                ],
            "crypto/pem/pem_err.o" =>
                [
                    "crypto/pem/pem_err.c",
                ],
            "crypto/pem/pem_info.o" =>
                [
                    "crypto/pem/pem_info.c",
                ],
            "crypto/pem/pem_lib.o" =>
                [
                    "crypto/pem/pem_lib.c",
                ],
            "crypto/pem/pem_oth.o" =>
                [
                    "crypto/pem/pem_oth.c",
                ],
            "crypto/pem/pem_pk8.o" =>
                [
                    "crypto/pem/pem_pk8.c",
                ],
            "crypto/pem/pem_pkey.o" =>
                [
                    "crypto/pem/pem_pkey.c",
                ],
            "crypto/pem/pem_sign.o" =>
                [
                    "crypto/pem/pem_sign.c",
                ],
            "crypto/pem/pem_x509.o" =>
                [
                    "crypto/pem/pem_x509.c",
                ],
            "crypto/pem/pem_xaux.o" =>
                [
                    "crypto/pem/pem_xaux.c",
                ],
            "crypto/pem/pvkfmt.o" =>
                [
                    "crypto/pem/pvkfmt.c",
                ],
            "crypto/pkcs12/p12_add.o" =>
                [
                    "crypto/pkcs12/p12_add.c",
                ],
            "crypto/pkcs12/p12_asn.o" =>
                [
                    "crypto/pkcs12/p12_asn.c",
                ],
            "crypto/pkcs12/p12_attr.o" =>
                [
                    "crypto/pkcs12/p12_attr.c",
                ],
            "crypto/pkcs12/p12_crpt.o" =>
                [
                    "crypto/pkcs12/p12_crpt.c",
                ],
            "crypto/pkcs12/p12_crt.o" =>
                [
                    "crypto/pkcs12/p12_crt.c",
                ],
            "crypto/pkcs12/p12_decr.o" =>
                [
                    "crypto/pkcs12/p12_decr.c",
                ],
            "crypto/pkcs12/p12_init.o" =>
                [
                    "crypto/pkcs12/p12_init.c",
                ],
            "crypto/pkcs12/p12_key.o" =>
                [
                    "crypto/pkcs12/p12_key.c",
                ],
            "crypto/pkcs12/p12_kiss.o" =>
                [
                    "crypto/pkcs12/p12_kiss.c",
                ],
            "crypto/pkcs12/p12_mutl.o" =>
                [
                    "crypto/pkcs12/p12_mutl.c",
                ],
            "crypto/pkcs12/p12_npas.o" =>
                [
                    "crypto/pkcs12/p12_npas.c",
                ],
            "crypto/pkcs12/p12_p8d.o" =>
                [
                    "crypto/pkcs12/p12_p8d.c",
                ],
            "crypto/pkcs12/p12_p8e.o" =>
                [
                    "crypto/pkcs12/p12_p8e.c",
                ],
            "crypto/pkcs12/p12_sbag.o" =>
                [
                    "crypto/pkcs12/p12_sbag.c",
                ],
            "crypto/pkcs12/p12_utl.o" =>
                [
                    "crypto/pkcs12/p12_utl.c",
                ],
            "crypto/pkcs12/pk12err.o" =>
                [
                    "crypto/pkcs12/pk12err.c",
                ],
            "crypto/pkcs7/bio_pk7.o" =>
                [
                    "crypto/pkcs7/bio_pk7.c",
                ],
            "crypto/pkcs7/pk7_asn1.o" =>
                [
                    "crypto/pkcs7/pk7_asn1.c",
                ],
            "crypto/pkcs7/pk7_attr.o" =>
                [
                    "crypto/pkcs7/pk7_attr.c",
                ],
            "crypto/pkcs7/pk7_doit.o" =>
                [
                    "crypto/pkcs7/pk7_doit.c",
                ],
            "crypto/pkcs7/pk7_lib.o" =>
                [
                    "crypto/pkcs7/pk7_lib.c",
                ],
            "crypto/pkcs7/pk7_mime.o" =>
                [
                    "crypto/pkcs7/pk7_mime.c",
                ],
            "crypto/pkcs7/pk7_smime.o" =>
                [
                    "crypto/pkcs7/pk7_smime.c",
                ],
            "crypto/pkcs7/pkcs7err.o" =>
                [
                    "crypto/pkcs7/pkcs7err.c",
                ],
            "crypto/poly1305/poly1305.o" =>
                [
                    "crypto/poly1305/poly1305.c",
                ],
            "crypto/poly1305/poly1305_ameth.o" =>
                [
                    "crypto/poly1305/poly1305_ameth.c",
                ],
            "crypto/poly1305/poly1305_pmeth.o" =>
                [
                    "crypto/poly1305/poly1305_pmeth.c",
                ],
            "crypto/rand/drbg_ctr.o" =>
                [
                    "crypto/rand/drbg_ctr.c",
                ],
            "crypto/rand/drbg_lib.o" =>
                [
                    "crypto/rand/drbg_lib.c",
                ],
            "crypto/rand/rand_egd.o" =>
                [
                    "crypto/rand/rand_egd.c",
                ],
            "crypto/rand/rand_err.o" =>
                [
                    "crypto/rand/rand_err.c",
                ],
            "crypto/rand/rand_lib.o" =>
                [
                    "crypto/rand/rand_lib.c",
                ],
            "crypto/rand/rand_unix.o" =>
                [
                    "crypto/rand/rand_unix.c",
                ],
            "crypto/rand/rand_vms.o" =>
                [
                    "crypto/rand/rand_vms.c",
                ],
            "crypto/rand/rand_win.o" =>
                [
                    "crypto/rand/rand_win.c",
                ],
            "crypto/rand/randfile.o" =>
                [
                    "crypto/rand/randfile.c",
                ],
            "crypto/rc4/rc4_enc.o" =>
                [
                    "crypto/rc4/rc4_enc.c",
                ],
            "crypto/rc4/rc4_skey.o" =>
                [
                    "crypto/rc4/rc4_skey.c",
                ],
            "crypto/ripemd/rmd_dgst.o" =>
                [
                    "crypto/ripemd/rmd_dgst.c",
                ],
            "crypto/ripemd/rmd_one.o" =>
                [
                    "crypto/ripemd/rmd_one.c",
                ],
            "crypto/rsa/rsa_ameth.o" =>
                [
                    "crypto/rsa/rsa_ameth.c",
                ],
            "crypto/rsa/rsa_asn1.o" =>
                [
                    "crypto/rsa/rsa_asn1.c",
                ],
            "crypto/rsa/rsa_chk.o" =>
                [
                    "crypto/rsa/rsa_chk.c",
                ],
            "crypto/rsa/rsa_crpt.o" =>
                [
                    "crypto/rsa/rsa_crpt.c",
                ],
            "crypto/rsa/rsa_depr.o" =>
                [
                    "crypto/rsa/rsa_depr.c",
                ],
            "crypto/rsa/rsa_err.o" =>
                [
                    "crypto/rsa/rsa_err.c",
                ],
            "crypto/rsa/rsa_gen.o" =>
                [
                    "crypto/rsa/rsa_gen.c",
                ],
            "crypto/rsa/rsa_lib.o" =>
                [
                    "crypto/rsa/rsa_lib.c",
                ],
            "crypto/rsa/rsa_meth.o" =>
                [
                    "crypto/rsa/rsa_meth.c",
                ],
            "crypto/rsa/rsa_mp.o" =>
                [
                    "crypto/rsa/rsa_mp.c",
                ],
            "crypto/rsa/rsa_none.o" =>
                [
                    "crypto/rsa/rsa_none.c",
                ],
            "crypto/rsa/rsa_oaep.o" =>
                [
                    "crypto/rsa/rsa_oaep.c",
                ],
            "crypto/rsa/rsa_ossl.o" =>
                [
                    "crypto/rsa/rsa_ossl.c",
                ],
            "crypto/rsa/rsa_pk1.o" =>
                [
                    "crypto/rsa/rsa_pk1.c",
                ],
            "crypto/rsa/rsa_pmeth.o" =>
                [
                    "crypto/rsa/rsa_pmeth.c",
                ],
            "crypto/rsa/rsa_prn.o" =>
                [
                    "crypto/rsa/rsa_prn.c",
                ],
            "crypto/rsa/rsa_pss.o" =>
                [
                    "crypto/rsa/rsa_pss.c",
                ],
            "crypto/rsa/rsa_saos.o" =>
                [
                    "crypto/rsa/rsa_saos.c",
                ],
            "crypto/rsa/rsa_sign.o" =>
                [
                    "crypto/rsa/rsa_sign.c",
                ],
            "crypto/rsa/rsa_ssl.o" =>
                [
                    "crypto/rsa/rsa_ssl.c",
                ],
            "crypto/rsa/rsa_x931.o" =>
                [
                    "crypto/rsa/rsa_x931.c",
                ],
            "crypto/rsa/rsa_x931g.o" =>
                [
                    "crypto/rsa/rsa_x931g.c",
                ],
            "crypto/sha/keccak1600.o" =>
                [
                    "crypto/sha/keccak1600.c",
                ],
            "crypto/sha/sha1_one.o" =>
                [
                    "crypto/sha/sha1_one.c",
                ],
            "crypto/sha/sha1dgst.o" =>
                [
                    "crypto/sha/sha1dgst.c",
                ],
            "crypto/sha/sha256.o" =>
                [
                    "crypto/sha/sha256.c",
                ],
            "crypto/sha/sha512.o" =>
                [
                    "crypto/sha/sha512.c",
                ],
            "crypto/srp/srp_lib.o" =>
                [
                    "crypto/srp/srp_lib.c",
                ],
            "crypto/srp/srp_vfy.o" =>
                [
                    "crypto/srp/srp_vfy.c",
                ],
            "crypto/stack/stack.o" =>
                [
                    "crypto/stack/stack.c",
                ],
            "crypto/store/loader_file.o" =>
                [
                    "crypto/store/loader_file.c",
                ],
            "crypto/store/store_err.o" =>
                [
                    "crypto/store/store_err.c",
                ],
            "crypto/store/store_init.o" =>
                [
                    "crypto/store/store_init.c",
                ],
            "crypto/store/store_lib.o" =>
                [
                    "crypto/store/store_lib.c",
                ],
            "crypto/store/store_register.o" =>
                [
                    "crypto/store/store_register.c",
                ],
            "crypto/store/store_strings.o" =>
                [
                    "crypto/store/store_strings.c",
                ],
            "crypto/threads_none.o" =>
                [
                    "crypto/threads_none.c",
                ],
            "crypto/threads_pthread.o" =>
                [
                    "crypto/threads_pthread.c",
                ],
            "crypto/threads_win.o" =>
                [
                    "crypto/threads_win.c",
                ],
            "crypto/ts/ts_asn1.o" =>
                [
                    "crypto/ts/ts_asn1.c",
                ],
            "crypto/ts/ts_conf.o" =>
                [
                    "crypto/ts/ts_conf.c",
                ],
            "crypto/ts/ts_err.o" =>
                [
                    "crypto/ts/ts_err.c",
                ],
            "crypto/ts/ts_lib.o" =>
                [
                    "crypto/ts/ts_lib.c",
                ],
            "crypto/ts/ts_req_print.o" =>
                [
                    "crypto/ts/ts_req_print.c",
                ],
            "crypto/ts/ts_req_utils.o" =>
                [
                    "crypto/ts/ts_req_utils.c",
                ],
            "crypto/ts/ts_rsp_print.o" =>
                [
                    "crypto/ts/ts_rsp_print.c",
                ],
            "crypto/ts/ts_rsp_sign.o" =>
                [
                    "crypto/ts/ts_rsp_sign.c",
                ],
            "crypto/ts/ts_rsp_utils.o" =>
                [
                    "crypto/ts/ts_rsp_utils.c",
                ],
            "crypto/ts/ts_rsp_verify.o" =>
                [
                    "crypto/ts/ts_rsp_verify.c",
                ],
            "crypto/ts/ts_verify_ctx.o" =>
                [
                    "crypto/ts/ts_verify_ctx.c",
                ],
            "crypto/txt_db/txt_db.o" =>
                [
                    "crypto/txt_db/txt_db.c",
                ],
            "crypto/ui/ui_err.o" =>
                [
                    "crypto/ui/ui_err.c",
                ],
            "crypto/ui/ui_lib.o" =>
                [
                    "crypto/ui/ui_lib.c",
                ],
            "crypto/ui/ui_null.o" =>
                [
                    "crypto/ui/ui_null.c",
                ],
            "crypto/ui/ui_openssl.o" =>
                [
                    "crypto/ui/ui_openssl.c",
                ],
            "crypto/ui/ui_util.o" =>
                [
                    "crypto/ui/ui_util.c",
                ],
            "crypto/uid.o" =>
                [
                    "crypto/uid.c",
                ],
            "crypto/x509/by_dir.o" =>
                [
                    "crypto/x509/by_dir.c",
                ],
            "crypto/x509/by_file.o" =>
                [
                    "crypto/x509/by_file.c",
                ],
            "crypto/x509/t_crl.o" =>
                [
                    "crypto/x509/t_crl.c",
                ],
            "crypto/x509/t_req.o" =>
                [
                    "crypto/x509/t_req.c",
                ],
            "crypto/x509/t_x509.o" =>
                [
                    "crypto/x509/t_x509.c",
                ],
            "crypto/x509/x509_att.o" =>
                [
                    "crypto/x509/x509_att.c",
                ],
            "crypto/x509/x509_cmp.o" =>
                [
                    "crypto/x509/x509_cmp.c",
                ],
            "crypto/x509/x509_d2.o" =>
                [
                    "crypto/x509/x509_d2.c",
                ],
            "crypto/x509/x509_def.o" =>
                [
                    "crypto/x509/x509_def.c",
                ],
            "crypto/x509/x509_err.o" =>
                [
                    "crypto/x509/x509_err.c",
                ],
            "crypto/x509/x509_ext.o" =>
                [
                    "crypto/x509/x509_ext.c",
                ],
            "crypto/x509/x509_lu.o" =>
                [
                    "crypto/x509/x509_lu.c",
                ],
            "crypto/x509/x509_meth.o" =>
                [
                    "crypto/x509/x509_meth.c",
                ],
            "crypto/x509/x509_obj.o" =>
                [
                    "crypto/x509/x509_obj.c",
                ],
            "crypto/x509/x509_r2x.o" =>
                [
                    "crypto/x509/x509_r2x.c",
                ],
            "crypto/x509/x509_req.o" =>
                [
                    "crypto/x509/x509_req.c",
                ],
            "crypto/x509/x509_set.o" =>
                [
                    "crypto/x509/x509_set.c",
                ],
            "crypto/x509/x509_trs.o" =>
                [
                    "crypto/x509/x509_trs.c",
                ],
            "crypto/x509/x509_txt.o" =>
                [
                    "crypto/x509/x509_txt.c",
                ],
            "crypto/x509/x509_v3.o" =>
                [
                    "crypto/x509/x509_v3.c",
                ],
            "crypto/x509/x509_vfy.o" =>
                [
                    "crypto/x509/x509_vfy.c",
                ],
            "crypto/x509/x509_vpm.o" =>
                [
                    "crypto/x509/x509_vpm.c",
                ],
            "crypto/x509/x509cset.o" =>
                [
                    "crypto/x509/x509cset.c",
                ],
            "crypto/x509/x509name.o" =>
                [
                    "crypto/x509/x509name.c",
                ],
            "crypto/x509/x509rset.o" =>
                [
                    "crypto/x509/x509rset.c",
                ],
            "crypto/x509/x509spki.o" =>
                [
                    "crypto/x509/x509spki.c",
                ],
            "crypto/x509/x509type.o" =>
                [
                    "crypto/x509/x509type.c",
                ],
            "crypto/x509/x_all.o" =>
                [
                    "crypto/x509/x_all.c",
                ],
            "crypto/x509/x_attrib.o" =>
                [
                    "crypto/x509/x_attrib.c",
                ],
            "crypto/x509/x_crl.o" =>
                [
                    "crypto/x509/x_crl.c",
                ],
            "crypto/x509/x_exten.o" =>
                [
                    "crypto/x509/x_exten.c",
                ],
            "crypto/x509/x_name.o" =>
                [
                    "crypto/x509/x_name.c",
                ],
            "crypto/x509/x_pubkey.o" =>
                [
                    "crypto/x509/x_pubkey.c",
                ],
            "crypto/x509/x_req.o" =>
                [
                    "crypto/x509/x_req.c",
                ],
            "crypto/x509/x_x509.o" =>
                [
                    "crypto/x509/x_x509.c",
                ],
            "crypto/x509/x_x509a.o" =>
                [
                    "crypto/x509/x_x509a.c",
                ],
            "crypto/x509v3/pcy_cache.o" =>
                [
                    "crypto/x509v3/pcy_cache.c",
                ],
            "crypto/x509v3/pcy_data.o" =>
                [
                    "crypto/x509v3/pcy_data.c",
                ],
            "crypto/x509v3/pcy_lib.o" =>
                [
                    "crypto/x509v3/pcy_lib.c",
                ],
            "crypto/x509v3/pcy_map.o" =>
                [
                    "crypto/x509v3/pcy_map.c",
                ],
            "crypto/x509v3/pcy_node.o" =>
                [
                    "crypto/x509v3/pcy_node.c",
                ],
            "crypto/x509v3/pcy_tree.o" =>
                [
                    "crypto/x509v3/pcy_tree.c",
                ],
            "crypto/x509v3/v3_addr.o" =>
                [
                    "crypto/x509v3/v3_addr.c",
                ],
            "crypto/x509v3/v3_admis.o" =>
                [
                    "crypto/x509v3/v3_admis.c",
                ],
            "crypto/x509v3/v3_akey.o" =>
                [
                    "crypto/x509v3/v3_akey.c",
                ],
            "crypto/x509v3/v3_akeya.o" =>
                [
                    "crypto/x509v3/v3_akeya.c",
                ],
            "crypto/x509v3/v3_alt.o" =>
                [
                    "crypto/x509v3/v3_alt.c",
                ],
            "crypto/x509v3/v3_asid.o" =>
                [
                    "crypto/x509v3/v3_asid.c",
                ],
            "crypto/x509v3/v3_bcons.o" =>
                [
                    "crypto/x509v3/v3_bcons.c",
                ],
            "crypto/x509v3/v3_bitst.o" =>
                [
                    "crypto/x509v3/v3_bitst.c",
                ],
            "crypto/x509v3/v3_conf.o" =>
                [
                    "crypto/x509v3/v3_conf.c",
                ],
            "crypto/x509v3/v3_cpols.o" =>
                [
                    "crypto/x509v3/v3_cpols.c",
                ],
            "crypto/x509v3/v3_crld.o" =>
                [
                    "crypto/x509v3/v3_crld.c",
                ],
            "crypto/x509v3/v3_enum.o" =>
                [
                    "crypto/x509v3/v3_enum.c",
                ],
            "crypto/x509v3/v3_extku.o" =>
                [
                    "crypto/x509v3/v3_extku.c",
                ],
            "crypto/x509v3/v3_genn.o" =>
                [
                    "crypto/x509v3/v3_genn.c",
                ],
            "crypto/x509v3/v3_ia5.o" =>
                [
                    "crypto/x509v3/v3_ia5.c",
                ],
            "crypto/x509v3/v3_info.o" =>
                [
                    "crypto/x509v3/v3_info.c",
                ],
            "crypto/x509v3/v3_int.o" =>
                [
                    "crypto/x509v3/v3_int.c",
                ],
            "crypto/x509v3/v3_lib.o" =>
                [
                    "crypto/x509v3/v3_lib.c",
                ],
            "crypto/x509v3/v3_ncons.o" =>
                [
                    "crypto/x509v3/v3_ncons.c",
                ],
            "crypto/x509v3/v3_pci.o" =>
                [
                    "crypto/x509v3/v3_pci.c",
                ],
            "crypto/x509v3/v3_pcia.o" =>
                [
                    "crypto/x509v3/v3_pcia.c",
                ],
            "crypto/x509v3/v3_pcons.o" =>
                [
                    "crypto/x509v3/v3_pcons.c",
                ],
            "crypto/x509v3/v3_pku.o" =>
                [
                    "crypto/x509v3/v3_pku.c",
                ],
            "crypto/x509v3/v3_pmaps.o" =>
                [
                    "crypto/x509v3/v3_pmaps.c",
                ],
            "crypto/x509v3/v3_prn.o" =>
                [
                    "crypto/x509v3/v3_prn.c",
                ],
            "crypto/x509v3/v3_purp.o" =>
                [
                    "crypto/x509v3/v3_purp.c",
                ],
            "crypto/x509v3/v3_skey.o" =>
                [
                    "crypto/x509v3/v3_skey.c",
                ],
            "crypto/x509v3/v3_sxnet.o" =>
                [
                    "crypto/x509v3/v3_sxnet.c",
                ],
            "crypto/x509v3/v3_tlsf.o" =>
                [
                    "crypto/x509v3/v3_tlsf.c",
                ],
            "crypto/x509v3/v3_utl.o" =>
                [
                    "crypto/x509v3/v3_utl.c",
                ],
            "crypto/x509v3/v3err.o" =>
                [
                    "crypto/x509v3/v3err.c",
                ],
            "engines/afalg" =>
                [
                    "engines/e_afalg.o",
                ],
            "engines/capi" =>
                [
                    "engines/e_capi.o",
                ],
            "engines/dasync" =>
                [
                    "engines/e_dasync.o",
                ],
            "engines/e_afalg.o" =>
                [
                    "engines/e_afalg.c",
                ],
            "engines/e_capi.o" =>
                [
                    "engines/e_capi.c",
                ],
            "engines/e_dasync.o" =>
                [
                    "engines/e_dasync.c",
                ],
            "engines/e_ossltest.o" =>
                [
                    "engines/e_ossltest.c",
                ],
            "engines/e_padlock.o" =>
                [
                    "engines/e_padlock.c",
                ],
            "engines/ossltest" =>
                [
                    "engines/e_ossltest.o",
                ],
            "engines/padlock" =>
                [
                    "engines/e_padlock.o",
                ],
            "libcrypto" =>
                [
                    "crypto/aes/aes_cbc.o",
                    "crypto/aes/aes_cfb.o",
                    "crypto/aes/aes_core.o",
                    "crypto/aes/aes_ecb.o",
                    "crypto/aes/aes_ige.o",
                    "crypto/aes/aes_misc.o",
                    "crypto/aes/aes_ofb.o",
                    "crypto/aes/aes_wrap.o",
                    "crypto/asn1/a_bitstr.o",
                    "crypto/asn1/a_d2i_fp.o",
                    "crypto/asn1/a_digest.o",
                    "crypto/asn1/a_dup.o",
                    "crypto/asn1/a_gentm.o",
                    "crypto/asn1/a_i2d_fp.o",
                    "crypto/asn1/a_int.o",
                    "crypto/asn1/a_mbstr.o",
                    "crypto/asn1/a_object.o",
                    "crypto/asn1/a_octet.o",
                    "crypto/asn1/a_print.o",
                    "crypto/asn1/a_sign.o",
                    "crypto/asn1/a_strex.o",
                    "crypto/asn1/a_strnid.o",
                    "crypto/asn1/a_time.o",
                    "crypto/asn1/a_type.o",
                    "crypto/asn1/a_utctm.o",
                    "crypto/asn1/a_utf8.o",
                    "crypto/asn1/a_verify.o",
                    "crypto/asn1/ameth_lib.o",
                    "crypto/asn1/asn1_err.o",
                    "crypto/asn1/asn1_gen.o",
                    "crypto/asn1/asn1_item_list.o",
                    "crypto/asn1/asn1_lib.o",
                    "crypto/asn1/asn1_par.o",
                    "crypto/asn1/asn_mime.o",
                    "crypto/asn1/asn_moid.o",
                    "crypto/asn1/asn_mstbl.o",
                    "crypto/asn1/asn_pack.o",
                    "crypto/asn1/bio_asn1.o",
                    "crypto/asn1/bio_ndef.o",
                    "crypto/asn1/d2i_pr.o",
                    "crypto/asn1/d2i_pu.o",
                    "crypto/asn1/evp_asn1.o",
                    "crypto/asn1/f_int.o",
                    "crypto/asn1/f_string.o",
                    "crypto/asn1/i2d_pr.o",
                    "crypto/asn1/i2d_pu.o",
                    "crypto/asn1/n_pkey.o",
                    "crypto/asn1/nsseq.o",
                    "crypto/asn1/p5_pbe.o",
                    "crypto/asn1/p5_pbev2.o",
                    "crypto/asn1/p5_scrypt.o",
                    "crypto/asn1/p8_pkey.o",
                    "crypto/asn1/t_bitst.o",
                    "crypto/asn1/t_pkey.o",
                    "crypto/asn1/t_spki.o",
                    "crypto/asn1/tasn_dec.o",
                    "crypto/asn1/tasn_enc.o",
                    "crypto/asn1/tasn_fre.o",
                    "crypto/asn1/tasn_new.o",
                    "crypto/asn1/tasn_prn.o",
                    "crypto/asn1/tasn_scn.o",
                    "crypto/asn1/tasn_typ.o",
                    "crypto/asn1/tasn_utl.o",
                    "crypto/asn1/x_algor.o",
                    "crypto/asn1/x_bignum.o",
                    "crypto/asn1/x_info.o",
                    "crypto/asn1/x_int64.o",
                    "crypto/asn1/x_long.o",
                    "crypto/asn1/x_pkey.o",
                    "crypto/asn1/x_sig.o",
                    "crypto/asn1/x_spki.o",
                    "crypto/asn1/x_val.o",
                    "crypto/async/arch/async_null.o",
                    "crypto/async/arch/async_posix.o",
                    "crypto/async/arch/async_win.o",
                    "crypto/async/async.o",
                    "crypto/async/async_err.o",
                    "crypto/async/async_wait.o",
                    "crypto/bf/bf_cfb64.o",
                    "crypto/bf/bf_ecb.o",
                    "crypto/bf/bf_enc.o",
                    "crypto/bf/bf_ofb64.o",
                    "crypto/bf/bf_skey.o",
                    "crypto/bio/b_addr.o",
                    "crypto/bio/b_dump.o",
                    "crypto/bio/b_print.o",
                    "crypto/bio/b_sock.o",
                    "crypto/bio/b_sock2.o",
                    "crypto/bio/bf_buff.o",
                    "crypto/bio/bf_lbuf.o",
                    "crypto/bio/bf_nbio.o",
                    "crypto/bio/bf_null.o",
                    "crypto/bio/bio_cb.o",
                    "crypto/bio/bio_err.o",
                    "crypto/bio/bio_lib.o",
                    "crypto/bio/bio_meth.o",
                    "crypto/bio/bss_acpt.o",
                    "crypto/bio/bss_bio.o",
                    "crypto/bio/bss_conn.o",
                    "crypto/bio/bss_dgram.o",
                    "crypto/bio/bss_fd.o",
                    "crypto/bio/bss_file.o",
                    "crypto/bio/bss_log.o",
                    "crypto/bio/bss_mem.o",
                    "crypto/bio/bss_null.o",
                    "crypto/bio/bss_sock.o",
                    "crypto/blake2/blake2b.o",
                    "crypto/blake2/blake2s.o",
                    "crypto/blake2/m_blake2b.o",
                    "crypto/blake2/m_blake2s.o",
                    "crypto/bn/bn_add.o",
                    "crypto/bn/bn_asm.o",
                    "crypto/bn/bn_blind.o",
                    "crypto/bn/bn_const.o",
                    "crypto/bn/bn_ctx.o",
                    "crypto/bn/bn_depr.o",
                    "crypto/bn/bn_dh.o",
                    "crypto/bn/bn_div.o",
                    "crypto/bn/bn_err.o",
                    "crypto/bn/bn_exp.o",
                    "crypto/bn/bn_exp2.o",
                    "crypto/bn/bn_gcd.o",
                    "crypto/bn/bn_gf2m.o",
                    "crypto/bn/bn_intern.o",
                    "crypto/bn/bn_kron.o",
                    "crypto/bn/bn_lib.o",
                    "crypto/bn/bn_mod.o",
                    "crypto/bn/bn_mont.o",
                    "crypto/bn/bn_mpi.o",
                    "crypto/bn/bn_mul.o",
                    "crypto/bn/bn_nist.o",
                    "crypto/bn/bn_prime.o",
                    "crypto/bn/bn_print.o",
                    "crypto/bn/bn_rand.o",
                    "crypto/bn/bn_recp.o",
                    "crypto/bn/bn_shift.o",
                    "crypto/bn/bn_sqr.o",
                    "crypto/bn/bn_sqrt.o",
                    "crypto/bn/bn_srp.o",
                    "crypto/bn/bn_word.o",
                    "crypto/bn/bn_x931p.o",
                    "crypto/buffer/buf_err.o",
                    "crypto/buffer/buffer.o",
                    "crypto/cast/c_cfb64.o",
                    "crypto/cast/c_ecb.o",
                    "crypto/cast/c_enc.o",
                    "crypto/cast/c_ofb64.o",
                    "crypto/cast/c_skey.o",
                    "crypto/chacha/chacha_enc.o",
                    "crypto/cmac/cm_ameth.o",
                    "crypto/cmac/cm_pmeth.o",
                    "crypto/cmac/cmac.o",
                    "crypto/cms/cms_asn1.o",
                    "crypto/cms/cms_att.o",
                    "crypto/cms/cms_cd.o",
                    "crypto/cms/cms_dd.o",
                    "crypto/cms/cms_enc.o",
                    "crypto/cms/cms_env.o",
                    "crypto/cms/cms_err.o",
                    "crypto/cms/cms_ess.o",
                    "crypto/cms/cms_io.o",
                    "crypto/cms/cms_kari.o",
                    "crypto/cms/cms_lib.o",
                    "crypto/cms/cms_pwri.o",
                    "crypto/cms/cms_sd.o",
                    "crypto/cms/cms_smime.o",
                    "crypto/comp/c_zlib.o",
                    "crypto/comp/comp_err.o",
                    "crypto/comp/comp_lib.o",
                    "crypto/conf/conf_api.o",
                    "crypto/conf/conf_def.o",
                    "crypto/conf/conf_err.o",
                    "crypto/conf/conf_lib.o",
                    "crypto/conf/conf_mall.o",
                    "crypto/conf/conf_mod.o",
                    "crypto/conf/conf_sap.o",
                    "crypto/conf/conf_ssl.o",
                    "crypto/cpt_err.o",
                    "crypto/cryptlib.o",
                    "crypto/ct/ct_b64.o",
                    "crypto/ct/ct_err.o",
                    "crypto/ct/ct_log.o",
                    "crypto/ct/ct_oct.o",
                    "crypto/ct/ct_policy.o",
                    "crypto/ct/ct_prn.o",
                    "crypto/ct/ct_sct.o",
                    "crypto/ct/ct_sct_ctx.o",
                    "crypto/ct/ct_vfy.o",
                    "crypto/ct/ct_x509v3.o",
                    "crypto/ctype.o",
                    "crypto/cversion.o",
                    "crypto/des/cbc_cksm.o",
                    "crypto/des/cbc_enc.o",
                    "crypto/des/cfb64ede.o",
                    "crypto/des/cfb64enc.o",
                    "crypto/des/cfb_enc.o",
                    "crypto/des/des_enc.o",
                    "crypto/des/ecb3_enc.o",
                    "crypto/des/ecb_enc.o",
                    "crypto/des/fcrypt.o",
                    "crypto/des/fcrypt_b.o",
                    "crypto/des/ofb64ede.o",
                    "crypto/des/ofb64enc.o",
                    "crypto/des/ofb_enc.o",
                    "crypto/des/pcbc_enc.o",
                    "crypto/des/qud_cksm.o",
                    "crypto/des/rand_key.o",
                    "crypto/des/set_key.o",
                    "crypto/des/str2key.o",
                    "crypto/des/xcbc_enc.o",
                    "crypto/dh/dh_ameth.o",
                    "crypto/dh/dh_asn1.o",
                    "crypto/dh/dh_check.o",
                    "crypto/dh/dh_depr.o",
                    "crypto/dh/dh_err.o",
                    "crypto/dh/dh_gen.o",
                    "crypto/dh/dh_kdf.o",
                    "crypto/dh/dh_key.o",
                    "crypto/dh/dh_lib.o",
                    "crypto/dh/dh_meth.o",
                    "crypto/dh/dh_pmeth.o",
                    "crypto/dh/dh_prn.o",
                    "crypto/dh/dh_rfc5114.o",
                    "crypto/dh/dh_rfc7919.o",
                    "crypto/dsa/dsa_ameth.o",
                    "crypto/dsa/dsa_asn1.o",
                    "crypto/dsa/dsa_depr.o",
                    "crypto/dsa/dsa_err.o",
                    "crypto/dsa/dsa_gen.o",
                    "crypto/dsa/dsa_key.o",
                    "crypto/dsa/dsa_lib.o",
                    "crypto/dsa/dsa_meth.o",
                    "crypto/dsa/dsa_ossl.o",
                    "crypto/dsa/dsa_pmeth.o",
                    "crypto/dsa/dsa_prn.o",
                    "crypto/dsa/dsa_sign.o",
                    "crypto/dsa/dsa_vrf.o",
                    "crypto/dso/dso_dl.o",
                    "crypto/dso/dso_dlfcn.o",
                    "crypto/dso/dso_err.o",
                    "crypto/dso/dso_lib.o",
                    "crypto/dso/dso_openssl.o",
                    "crypto/dso/dso_vms.o",
                    "crypto/dso/dso_win32.o",
                    "crypto/ebcdic.o",
                    "crypto/ec/curve25519.o",
                    "crypto/ec/curve448/arch_32/f_impl.o",
                    "crypto/ec/curve448/curve448.o",
                    "crypto/ec/curve448/curve448_tables.o",
                    "crypto/ec/curve448/eddsa.o",
                    "crypto/ec/curve448/f_generic.o",
                    "crypto/ec/curve448/scalar.o",
                    "crypto/ec/ec2_oct.o",
                    "crypto/ec/ec2_smpl.o",
                    "crypto/ec/ec_ameth.o",
                    "crypto/ec/ec_asn1.o",
                    "crypto/ec/ec_check.o",
                    "crypto/ec/ec_curve.o",
                    "crypto/ec/ec_cvt.o",
                    "crypto/ec/ec_err.o",
                    "crypto/ec/ec_key.o",
                    "crypto/ec/ec_kmeth.o",
                    "crypto/ec/ec_lib.o",
                    "crypto/ec/ec_mult.o",
                    "crypto/ec/ec_oct.o",
                    "crypto/ec/ec_pmeth.o",
                    "crypto/ec/ec_print.o",
                    "crypto/ec/ecdh_kdf.o",
                    "crypto/ec/ecdh_ossl.o",
                    "crypto/ec/ecdsa_ossl.o",
                    "crypto/ec/ecdsa_sign.o",
                    "crypto/ec/ecdsa_vrf.o",
                    "crypto/ec/eck_prn.o",
                    "crypto/ec/ecp_mont.o",
                    "crypto/ec/ecp_nist.o",
                    "crypto/ec/ecp_nistp224.o",
                    "crypto/ec/ecp_nistp256.o",
                    "crypto/ec/ecp_nistp521.o",
                    "crypto/ec/ecp_nistputil.o",
                    "crypto/ec/ecp_oct.o",
                    "crypto/ec/ecp_smpl.o",
                    "crypto/ec/ecx_meth.o",
                    "crypto/engine/eng_all.o",
                    "crypto/engine/eng_cnf.o",
                    "crypto/engine/eng_ctrl.o",
                    "crypto/engine/eng_dyn.o",
                    "crypto/engine/eng_err.o",
                    "crypto/engine/eng_fat.o",
                    "crypto/engine/eng_init.o",
                    "crypto/engine/eng_lib.o",
                    "crypto/engine/eng_list.o",
                    "crypto/engine/eng_openssl.o",
                    "crypto/engine/eng_pkey.o",
                    "crypto/engine/eng_rdrand.o",
                    "crypto/engine/eng_table.o",
                    "crypto/engine/tb_asnmth.o",
                    "crypto/engine/tb_cipher.o",
                    "crypto/engine/tb_dh.o",
                    "crypto/engine/tb_digest.o",
                    "crypto/engine/tb_dsa.o",
                    "crypto/engine/tb_eckey.o",
                    "crypto/engine/tb_pkmeth.o",
                    "crypto/engine/tb_rand.o",
                    "crypto/engine/tb_rsa.o",
                    "crypto/err/err.o",
                    "crypto/err/err_all.o",
                    "crypto/err/err_prn.o",
                    "crypto/evp/bio_b64.o",
                    "crypto/evp/bio_enc.o",
                    "crypto/evp/bio_md.o",
                    "crypto/evp/bio_ok.o",
                    "crypto/evp/c_allc.o",
                    "crypto/evp/c_alld.o",
                    "crypto/evp/cmeth_lib.o",
                    "crypto/evp/digest.o",
                    "crypto/evp/e_aes.o",
                    "crypto/evp/e_aes_cbc_hmac_sha1.o",
                    "crypto/evp/e_aes_cbc_hmac_sha256.o",
                    "crypto/evp/e_aria.o",
                    "crypto/evp/e_bf.o",
                    "crypto/evp/e_camellia.o",
                    "crypto/evp/e_cast.o",
                    "crypto/evp/e_chacha20_poly1305.o",
                    "crypto/evp/e_des.o",
                    "crypto/evp/e_des3.o",
                    "crypto/evp/e_idea.o",
                    "crypto/evp/e_null.o",
                    "crypto/evp/e_old.o",
                    "crypto/evp/e_rc2.o",
                    "crypto/evp/e_rc4.o",
                    "crypto/evp/e_rc4_hmac_md5.o",
                    "crypto/evp/e_rc5.o",
                    "crypto/evp/e_seed.o",
                    "crypto/evp/e_sm4.o",
                    "crypto/evp/e_xcbc_d.o",
                    "crypto/evp/encode.o",
                    "crypto/evp/evp_cnf.o",
                    "crypto/evp/evp_enc.o",
                    "crypto/evp/evp_err.o",
                    "crypto/evp/evp_key.o",
                    "crypto/evp/evp_lib.o",
                    "crypto/evp/evp_pbe.o",
                    "crypto/evp/evp_pkey.o",
                    "crypto/evp/m_md2.o",
                    "crypto/evp/m_md4.o",
                    "crypto/evp/m_md5.o",
                    "crypto/evp/m_md5_sha1.o",
                    "crypto/evp/m_mdc2.o",
                    "crypto/evp/m_null.o",
                    "crypto/evp/m_ripemd.o",
                    "crypto/evp/m_sha1.o",
                    "crypto/evp/m_sha3.o",
                    "crypto/evp/m_sigver.o",
                    "crypto/evp/m_wp.o",
                    "crypto/evp/names.o",
                    "crypto/evp/p5_crpt.o",
                    "crypto/evp/p5_crpt2.o",
                    "crypto/evp/p_dec.o",
                    "crypto/evp/p_enc.o",
                    "crypto/evp/p_lib.o",
                    "crypto/evp/p_open.o",
                    "crypto/evp/p_seal.o",
                    "crypto/evp/p_sign.o",
                    "crypto/evp/p_verify.o",
                    "crypto/evp/pbe_scrypt.o",
                    "crypto/evp/pmeth_fn.o",
                    "crypto/evp/pmeth_gn.o",
                    "crypto/evp/pmeth_lib.o",
                    "crypto/ex_data.o",
                    "crypto/getenv.o",
                    "crypto/hmac/hm_ameth.o",
                    "crypto/hmac/hm_pmeth.o",
                    "crypto/hmac/hmac.o",
                    "crypto/init.o",
                    "crypto/kdf/hkdf.o",
                    "crypto/kdf/kdf_err.o",
                    "crypto/kdf/scrypt.o",
                    "crypto/kdf/tls1_prf.o",
                    "crypto/lhash/lh_stats.o",
                    "crypto/lhash/lhash.o",
                    "crypto/md4/md4_dgst.o",
                    "crypto/md4/md4_one.o",
                    "crypto/md5/md5_dgst.o",
                    "crypto/md5/md5_one.o",
                    "crypto/mem.o",
                    "crypto/mem_clr.o",
                    "crypto/mem_dbg.o",
                    "crypto/mem_sec.o",
                    "crypto/modes/cbc128.o",
                    "crypto/modes/ccm128.o",
                    "crypto/modes/cfb128.o",
                    "crypto/modes/ctr128.o",
                    "crypto/modes/cts128.o",
                    "crypto/modes/gcm128.o",
                    "crypto/modes/ocb128.o",
                    "crypto/modes/ofb128.o",
                    "crypto/modes/wrap128.o",
                    "crypto/modes/xts128.o",
                    "crypto/o_dir.o",
                    "crypto/o_fips.o",
                    "crypto/o_fopen.o",
                    "crypto/o_init.o",
                    "crypto/o_str.o",
                    "crypto/o_time.o",
                    "crypto/objects/o_names.o",
                    "crypto/objects/obj_dat.o",
                    "crypto/objects/obj_err.o",
                    "crypto/objects/obj_lib.o",
                    "crypto/objects/obj_xref.o",
                    "crypto/ocsp/ocsp_asn.o",
                    "crypto/ocsp/ocsp_cl.o",
                    "crypto/ocsp/ocsp_err.o",
                    "crypto/ocsp/ocsp_ext.o",
                    "crypto/ocsp/ocsp_ht.o",
                    "crypto/ocsp/ocsp_lib.o",
                    "crypto/ocsp/ocsp_prn.o",
                    "crypto/ocsp/ocsp_srv.o",
                    "crypto/ocsp/ocsp_vfy.o",
                    "crypto/ocsp/v3_ocsp.o",
                    "crypto/pem/pem_all.o",
                    "crypto/pem/pem_err.o",
                    "crypto/pem/pem_info.o",
                    "crypto/pem/pem_lib.o",
                    "crypto/pem/pem_oth.o",
                    "crypto/pem/pem_pk8.o",
                    "crypto/pem/pem_pkey.o",
                    "crypto/pem/pem_sign.o",
                    "crypto/pem/pem_x509.o",
                    "crypto/pem/pem_xaux.o",
                    "crypto/pem/pvkfmt.o",
                    "crypto/pkcs12/p12_add.o",
                    "crypto/pkcs12/p12_asn.o",
                    "crypto/pkcs12/p12_attr.o",
                    "crypto/pkcs12/p12_crpt.o",
                    "crypto/pkcs12/p12_crt.o",
                    "crypto/pkcs12/p12_decr.o",
                    "crypto/pkcs12/p12_init.o",
                    "crypto/pkcs12/p12_key.o",
                    "crypto/pkcs12/p12_kiss.o",
                    "crypto/pkcs12/p12_mutl.o",
                    "crypto/pkcs12/p12_npas.o",
                    "crypto/pkcs12/p12_p8d.o",
                    "crypto/pkcs12/p12_p8e.o",
                    "crypto/pkcs12/p12_sbag.o",
                    "crypto/pkcs12/p12_utl.o",
                    "crypto/pkcs12/pk12err.o",
                    "crypto/pkcs7/bio_pk7.o",
                    "crypto/pkcs7/pk7_asn1.o",
                    "crypto/pkcs7/pk7_attr.o",
                    "crypto/pkcs7/pk7_doit.o",
                    "crypto/pkcs7/pk7_lib.o",
                    "crypto/pkcs7/pk7_mime.o",
                    "crypto/pkcs7/pk7_smime.o",
                    "crypto/pkcs7/pkcs7err.o",
                    "crypto/poly1305/poly1305.o",
                    "crypto/poly1305/poly1305_ameth.o",
                    "crypto/poly1305/poly1305_pmeth.o",
                    "crypto/rand/drbg_ctr.o",
                    "crypto/rand/drbg_lib.o",
                    "crypto/rand/rand_egd.o",
                    "crypto/rand/rand_err.o",
                    "crypto/rand/rand_lib.o",
                    "crypto/rand/rand_unix.o",
                    "crypto/rand/rand_vms.o",
                    "crypto/rand/rand_win.o",
                    "crypto/rand/randfile.o",
                    "crypto/rc4/rc4_enc.o",
                    "crypto/rc4/rc4_skey.o",
                    "crypto/ripemd/rmd_dgst.o",
                    "crypto/ripemd/rmd_one.o",
                    "crypto/rsa/rsa_ameth.o",
                    "crypto/rsa/rsa_asn1.o",
                    "crypto/rsa/rsa_chk.o",
                    "crypto/rsa/rsa_crpt.o",
                    "crypto/rsa/rsa_depr.o",
                    "crypto/rsa/rsa_err.o",
                    "crypto/rsa/rsa_gen.o",
                    "crypto/rsa/rsa_lib.o",
                    "crypto/rsa/rsa_meth.o",
                    "crypto/rsa/rsa_mp.o",
                    "crypto/rsa/rsa_none.o",
                    "crypto/rsa/rsa_oaep.o",
                    "crypto/rsa/rsa_ossl.o",
                    "crypto/rsa/rsa_pk1.o",
                    "crypto/rsa/rsa_pmeth.o",
                    "crypto/rsa/rsa_prn.o",
                    "crypto/rsa/rsa_pss.o",
                    "crypto/rsa/rsa_saos.o",
                    "crypto/rsa/rsa_sign.o",
                    "crypto/rsa/rsa_ssl.o",
                    "crypto/rsa/rsa_x931.o",
                    "crypto/rsa/rsa_x931g.o",
                    "crypto/sha/keccak1600.o",
                    "crypto/sha/sha1_one.o",
                    "crypto/sha/sha1dgst.o",
                    "crypto/sha/sha256.o",
                    "crypto/sha/sha512.o",
                    "crypto/srp/srp_lib.o",
                    "crypto/srp/srp_vfy.o",
                    "crypto/stack/stack.o",
                    "crypto/store/loader_file.o",
                    "crypto/store/store_err.o",
                    "crypto/store/store_init.o",
                    "crypto/store/store_lib.o",
                    "crypto/store/store_register.o",
                    "crypto/store/store_strings.o",
                    "crypto/threads_none.o",
                    "crypto/threads_pthread.o",
                    "crypto/threads_win.o",
                    "crypto/ts/ts_asn1.o",
                    "crypto/ts/ts_conf.o",
                    "crypto/ts/ts_err.o",
                    "crypto/ts/ts_lib.o",
                    "crypto/ts/ts_req_print.o",
                    "crypto/ts/ts_req_utils.o",
                    "crypto/ts/ts_rsp_print.o",
                    "crypto/ts/ts_rsp_sign.o",
                    "crypto/ts/ts_rsp_utils.o",
                    "crypto/ts/ts_rsp_verify.o",
                    "crypto/ts/ts_verify_ctx.o",
                    "crypto/txt_db/txt_db.o",
                    "crypto/ui/ui_err.o",
                    "crypto/ui/ui_lib.o",
                    "crypto/ui/ui_null.o",
                    "crypto/ui/ui_openssl.o",
                    "crypto/ui/ui_util.o",
                    "crypto/uid.o",
                    "crypto/x509/by_dir.o",
                    "crypto/x509/by_file.o",
                    "crypto/x509/t_crl.o",
                    "crypto/x509/t_req.o",
                    "crypto/x509/t_x509.o",
                    "crypto/x509/x509_att.o",
                    "crypto/x509/x509_cmp.o",
                    "crypto/x509/x509_d2.o",
                    "crypto/x509/x509_def.o",
                    "crypto/x509/x509_err.o",
                    "crypto/x509/x509_ext.o",
                    "crypto/x509/x509_lu.o",
                    "crypto/x509/x509_meth.o",
                    "crypto/x509/x509_obj.o",
                    "crypto/x509/x509_r2x.o",
                    "crypto/x509/x509_req.o",
                    "crypto/x509/x509_set.o",
                    "crypto/x509/x509_trs.o",
                    "crypto/x509/x509_txt.o",
                    "crypto/x509/x509_v3.o",
                    "crypto/x509/x509_vfy.o",
                    "crypto/x509/x509_vpm.o",
                    "crypto/x509/x509cset.o",
                    "crypto/x509/x509name.o",
                    "crypto/x509/x509rset.o",
                    "crypto/x509/x509spki.o",
                    "crypto/x509/x509type.o",
                    "crypto/x509/x_all.o",
                    "crypto/x509/x_attrib.o",
                    "crypto/x509/x_crl.o",
                    "crypto/x509/x_exten.o",
                    "crypto/x509/x_name.o",
                    "crypto/x509/x_pubkey.o",
                    "crypto/x509/x_req.o",
                    "crypto/x509/x_x509.o",
                    "crypto/x509/x_x509a.o",
                    "crypto/x509v3/pcy_cache.o",
                    "crypto/x509v3/pcy_data.o",
                    "crypto/x509v3/pcy_lib.o",
                    "crypto/x509v3/pcy_map.o",
                    "crypto/x509v3/pcy_node.o",
                    "crypto/x509v3/pcy_tree.o",
                    "crypto/x509v3/v3_addr.o",
                    "crypto/x509v3/v3_admis.o",
                    "crypto/x509v3/v3_akey.o",
                    "crypto/x509v3/v3_akeya.o",
                    "crypto/x509v3/v3_alt.o",
                    "crypto/x509v3/v3_asid.o",
                    "crypto/x509v3/v3_bcons.o",
                    "crypto/x509v3/v3_bitst.o",
                    "crypto/x509v3/v3_conf.o",
                    "crypto/x509v3/v3_cpols.o",
                    "crypto/x509v3/v3_crld.o",
                    "crypto/x509v3/v3_enum.o",
                    "crypto/x509v3/v3_extku.o",
                    "crypto/x509v3/v3_genn.o",
                    "crypto/x509v3/v3_ia5.o",
                    "crypto/x509v3/v3_info.o",
                    "crypto/x509v3/v3_int.o",
                    "crypto/x509v3/v3_lib.o",
                    "crypto/x509v3/v3_ncons.o",
                    "crypto/x509v3/v3_pci.o",
                    "crypto/x509v3/v3_pcia.o",
                    "crypto/x509v3/v3_pcons.o",
                    "crypto/x509v3/v3_pku.o",
                    "crypto/x509v3/v3_pmaps.o",
                    "crypto/x509v3/v3_prn.o",
                    "crypto/x509v3/v3_purp.o",
                    "crypto/x509v3/v3_skey.o",
                    "crypto/x509v3/v3_sxnet.o",
                    "crypto/x509v3/v3_tlsf.o",
                    "crypto/x509v3/v3_utl.o",
                    "crypto/x509v3/v3err.o",
                ],
            "libssl" =>
                [
                    "ssl/bio_ssl.o",
                    "ssl/d1_lib.o",
                    "ssl/d1_msg.o",
                    "ssl/d1_srtp.o",
                    "ssl/methods.o",
                    "ssl/packet.o",
                    "ssl/pqueue.o",
                    "ssl/record/dtls1_bitmap.o",
                    "ssl/record/rec_layer_d1.o",
                    "ssl/record/rec_layer_s3.o",
                    "ssl/record/ssl3_buffer.o",
                    "ssl/record/ssl3_record.o",
                    "ssl/record/ssl3_record_tls13.o",
                    "ssl/s3_cbc.o",
                    "ssl/s3_enc.o",
                    "ssl/s3_lib.o",
                    "ssl/s3_msg.o",
                    "ssl/ssl_asn1.o",
                    "ssl/ssl_cert.o",
                    "ssl/ssl_ciph.o",
                    "ssl/ssl_conf.o",
                    "ssl/ssl_err.o",
                    "ssl/ssl_init.o",
                    "ssl/ssl_lib.o",
                    "ssl/ssl_mcnf.o",
                    "ssl/ssl_rsa.o",
                    "ssl/ssl_sess.o",
                    "ssl/ssl_stat.o",
                    "ssl/ssl_txt.o",
                    "ssl/ssl_utst.o",
                    "ssl/statem/extensions.o",
                    "ssl/statem/extensions_clnt.o",
                    "ssl/statem/extensions_cust.o",
                    "ssl/statem/extensions_srvr.o",
                    "ssl/statem/statem.o",
                    "ssl/statem/statem_clnt.o",
                    "ssl/statem/statem_dtls.o",
                    "ssl/statem/statem_lib.o",
                    "ssl/statem/statem_srvr.o",
                    "ssl/t1_enc.o",
                    "ssl/t1_lib.o",
                    "ssl/t1_trce.o",
                    "ssl/tls13_enc.o",
                    "ssl/tls_srp.o",
                ],
            "ssl/bio_ssl.o" =>
                [
                    "ssl/bio_ssl.c",
                ],
            "ssl/d1_lib.o" =>
                [
                    "ssl/d1_lib.c",
                ],
            "ssl/d1_msg.o" =>
                [
                    "ssl/d1_msg.c",
                ],
            "ssl/d1_srtp.o" =>
                [
                    "ssl/d1_srtp.c",
                ],
            "ssl/methods.o" =>
                [
                    "ssl/methods.c",
                ],
            "ssl/packet.o" =>
                [
                    "ssl/packet.c",
                ],
            "ssl/pqueue.o" =>
                [
                    "ssl/pqueue.c",
                ],
            "ssl/record/dtls1_bitmap.o" =>
                [
                    "ssl/record/dtls1_bitmap.c",
                ],
            "ssl/record/rec_layer_d1.o" =>
                [
                    "ssl/record/rec_layer_d1.c",
                ],
            "ssl/record/rec_layer_s3.o" =>
                [
                    "ssl/record/rec_layer_s3.c",
                ],
            "ssl/record/ssl3_buffer.o" =>
                [
                    "ssl/record/ssl3_buffer.c",
                ],
            "ssl/record/ssl3_record.o" =>
                [
                    "ssl/record/ssl3_record.c",
                ],
            "ssl/record/ssl3_record_tls13.o" =>
                [
                    "ssl/record/ssl3_record_tls13.c",
                ],
            "ssl/s3_cbc.o" =>
                [
                    "ssl/s3_cbc.c",
                ],
            "ssl/s3_enc.o" =>
                [
                    "ssl/s3_enc.c",
                ],
            "ssl/s3_lib.o" =>
                [
                    "ssl/s3_lib.c",
                ],
            "ssl/s3_msg.o" =>
                [
                    "ssl/s3_msg.c",
                ],
            "ssl/ssl_asn1.o" =>
                [
                    "ssl/ssl_asn1.c",
                ],
            "ssl/ssl_cert.o" =>
                [
                    "ssl/ssl_cert.c",
                ],
            "ssl/ssl_ciph.o" =>
                [
                    "ssl/ssl_ciph.c",
                ],
            "ssl/ssl_conf.o" =>
                [
                    "ssl/ssl_conf.c",
                ],
            "ssl/ssl_err.o" =>
                [
                    "ssl/ssl_err.c",
                ],
            "ssl/ssl_init.o" =>
                [
                    "ssl/ssl_init.c",
                ],
            "ssl/ssl_lib.o" =>
                [
                    "ssl/ssl_lib.c",
                ],
            "ssl/ssl_mcnf.o" =>
                [
                    "ssl/ssl_mcnf.c",
                ],
            "ssl/ssl_rsa.o" =>
                [
                    "ssl/ssl_rsa.c",
                ],
            "ssl/ssl_sess.o" =>
                [
                    "ssl/ssl_sess.c",
                ],
            "ssl/ssl_stat.o" =>
                [
                    "ssl/ssl_stat.c",
                ],
            "ssl/ssl_txt.o" =>
                [
                    "ssl/ssl_txt.c",
                ],
            "ssl/ssl_utst.o" =>
                [
                    "ssl/ssl_utst.c",
                ],
            "ssl/statem/extensions.o" =>
                [
                    "ssl/statem/extensions.c",
                ],
            "ssl/statem/extensions_clnt.o" =>
                [
                    "ssl/statem/extensions_clnt.c",
                ],
            "ssl/statem/extensions_cust.o" =>
                [
                    "ssl/statem/extensions_cust.c",
                ],
            "ssl/statem/extensions_srvr.o" =>
                [
                    "ssl/statem/extensions_srvr.c",
                ],
            "ssl/statem/statem.o" =>
                [
                    "ssl/statem/statem.c",
                ],
            "ssl/statem/statem_clnt.o" =>
                [
                    "ssl/statem/statem_clnt.c",
                ],
            "ssl/statem/statem_dtls.o" =>
                [
                    "ssl/statem/statem_dtls.c",
                ],
            "ssl/statem/statem_lib.o" =>
                [
                    "ssl/statem/statem_lib.c",
                ],
            "ssl/statem/statem_srvr.o" =>
                [
                    "ssl/statem/statem_srvr.c",
                ],
            "ssl/t1_enc.o" =>
                [
                    "ssl/t1_enc.c",
                ],
            "ssl/t1_lib.o" =>
                [
                    "ssl/t1_lib.c",
                ],
            "ssl/t1_trce.o" =>
                [
                    "ssl/t1_trce.c",
                ],
            "ssl/tls13_enc.o" =>
                [
                    "ssl/tls13_enc.c",
                ],
            "ssl/tls_srp.o" =>
                [
                    "ssl/tls_srp.c",
                ],
            "tools/c_rehash" =>
                [
                    "tools/c_rehash.in",
                ],
            "util/shlib_wrap.sh" =>
                [
                    "util/shlib_wrap.sh.in",
                ],
        },
);

# The following data is only used when this files is use as a script
my @makevars = (
    'AR',
    'ARFLAGS',
    'AS',
    'ASFLAGS',
    'CC',
    'CFLAGS',
    'CPP',
    'CPPDEFINES',
    'CPPFLAGS',
    'CPPINCLUDES',
    'CROSS_COMPILE',
    'CXX',
    'CXXFLAGS',
    'HASHBANGPERL',
    'LD',
    'LDFLAGS',
    'LDLIBS',
    'MT',
    'MTFLAGS',
    'PERL',
    'RANLIB',
    'RC',
    'RCFLAGS',
    'RM',
);
my %disabled_info = (
    'aria' => {
        macro => 'OPENSSL_NO_ARIA',
        skipped => [ 'crypto/aria' ],
    },
    'asan' => {
        macro => 'OPENSSL_NO_ASAN',
    },
    'camellia' => {
        macro => 'OPENSSL_NO_CAMELLIA',
        skipped => [ 'crypto/camellia' ],
    },
    'crypto-mdebug' => {
        macro => 'OPENSSL_NO_CRYPTO_MDEBUG',
    },
    'crypto-mdebug-backtrace' => {
        macro => 'OPENSSL_NO_CRYPTO_MDEBUG_BACKTRACE',
    },
    'devcryptoeng' => {
        macro => 'OPENSSL_NO_DEVCRYPTOENG',
    },
    'ec2m' => {
        macro => 'OPENSSL_NO_EC2M',
    },
    'ec_nistp_64_gcc_128' => {
        macro => 'OPENSSL_NO_EC_NISTP_64_GCC_128',
    },
    'egd' => {
        macro => 'OPENSSL_NO_EGD',
    },
    'err' => {
        macro => 'OPENSSL_NO_ERR',
    },
    'external-tests' => {
        macro => 'OPENSSL_NO_EXTERNAL_TESTS',
    },
    'fuzz-afl' => {
        macro => 'OPENSSL_NO_FUZZ_AFL',
    },
    'fuzz-libfuzzer' => {
        macro => 'OPENSSL_NO_FUZZ_LIBFUZZER',
    },
    'gost' => {
        macro => 'OPENSSL_NO_GOST',
    },
    'heartbeats' => {
        macro => 'OPENSSL_NO_HEARTBEATS',
    },
    'idea' => {
        macro => 'OPENSSL_NO_IDEA',
        skipped => [ 'crypto/idea' ],
    },
    'md2' => {
        macro => 'OPENSSL_NO_MD2',
        skipped => [ 'crypto/md2' ],
    },
    'mdc2' => {
        macro => 'OPENSSL_NO_MDC2',
        skipped => [ 'crypto/mdc2' ],
    },
    'msan' => {
        macro => 'OPENSSL_NO_MSAN',
    },
    'rc2' => {
        macro => 'OPENSSL_NO_RC2',
        skipped => [ 'crypto/rc2' ],
    },
    'rc5' => {
        macro => 'OPENSSL_NO_RC5',
        skipped => [ 'crypto/rc5' ],
    },
    'sctp' => {
        macro => 'OPENSSL_NO_SCTP',
    },
    'seed' => {
        macro => 'OPENSSL_NO_SEED',
        skipped => [ 'crypto/seed' ],
    },
    'siphash' => {
        macro => 'OPENSSL_NO_SIPHASH',
        skipped => [ 'crypto/siphash' ],
    },
    'sm2' => {
        macro => 'OPENSSL_NO_SM2',
        skipped => [ 'crypto/sm2' ],
    },
    'sm3' => {
        macro => 'OPENSSL_NO_SM3',
        skipped => [ 'crypto/sm3' ],
    },
    'sm4' => {
        macro => 'OPENSSL_NO_SM4',
        skipped => [ 'crypto/sm4' ],
    },
    'ssl-trace' => {
        macro => 'OPENSSL_NO_SSL_TRACE',
    },
    'ssl3' => {
        macro => 'OPENSSL_NO_SSL3',
    },
    'tests' => {
        macro => 'OPENSSL_NO_TESTS',
    },
    'ubsan' => {
        macro => 'OPENSSL_NO_UBSAN',
    },
    'unit-test' => {
        macro => 'OPENSSL_NO_UNIT_TEST',
    },
    'weak-ssl-ciphers' => {
        macro => 'OPENSSL_NO_WEAK_SSL_CIPHERS',
    },
    'whrlpool' => {
        macro => 'OPENSSL_NO_WHIRLPOOL',
        skipped => [ 'crypto/whrlpool' ],
    },
);
my @user_crossable = qw( AR AS CC CXX CPP LD MT RANLIB RC );
# If run directly, we can give some answers, and even reconfigure
unless (caller) {
    use Getopt::Long;
    use File::Spec::Functions;
    use File::Basename;
    use Pod::Usage;

    my $here = dirname($0);

    my $dump = undef;
    my $cmdline = undef;
    my $options = undef;
    my $target = undef;
    my $envvars = undef;
    my $makevars = undef;
    my $buildparams = undef;
    my $reconf = undef;
    my $verbose = undef;
    my $help = undef;
    my $man = undef;
    GetOptions('dump|d'                 => \$dump,
               'command-line|c'         => \$cmdline,
               'options|o'              => \$options,
               'target|t'               => \$target,
               'environment|e'          => \$envvars,
               'make-variables|m'       => \$makevars,
               'build-parameters|b'     => \$buildparams,
               'reconfigure|reconf|r'   => \$reconf,
               'verbose|v'              => \$verbose,
               'help'                   => \$help,
               'man'                    => \$man)
        or die "Errors in command line arguments\n";

    unless ($dump || $cmdline || $options || $target || $envvars || $makevars
            || $buildparams || $reconf || $verbose || $help || $man) {
        print STDERR <<"_____";
You must give at least one option.
For more information, do '$0 --help'
_____
        exit(2);
    }

    if ($help) {
        pod2usage(-exitval => 0,
                  -verbose => 1);
    }
    if ($man) {
        pod2usage(-exitval => 0,
                  -verbose => 2);
    }
    if ($dump || $cmdline) {
        print "\nCommand line (with current working directory = $here):\n\n";
        print '    ',join(' ',
                          $config{PERL},
                          catfile($config{sourcedir}, 'Configure'),
                          @{$config{perlargv}}), "\n";
        print "\nPerl information:\n\n";
        print '    ',$config{perl_cmd},"\n";
        print '    ',$config{perl_version},' for ',$config{perl_archname},"\n";
    }
    if ($dump || $options) {
        my $longest = 0;
        my $longest2 = 0;
        foreach my $what (@disablables) {
            $longest = length($what) if $longest < length($what);
            $longest2 = length($disabled{$what})
                if $disabled{$what} && $longest2 < length($disabled{$what});
        }
        print "\nEnabled features:\n\n";
        foreach my $what (@disablables) {
            print "    $what\n"
                unless grep { $_ =~ /^${what}$/ } keys %disabled;
        }
        print "\nDisabled features:\n\n";
        foreach my $what (@disablables) {
            my @what2 = grep { $_ =~ /^${what}$/ } keys %disabled;
            my $what3 = $what2[0];
            if ($what3) {
                print "    $what3", ' ' x ($longest - length($what3) + 1),
                    "[$disabled{$what3}]", ' ' x ($longest2 - length($disabled{$what3}) + 1);
                print $disabled_info{$what3}->{macro}
                    if $disabled_info{$what3}->{macro};
                print ' (skip ',
                    join(', ', @{$disabled_info{$what3}->{skipped}}),
                    ')'
                    if $disabled_info{$what3}->{skipped};
                print "\n";
            }
        }
    }
    if ($dump || $target) {
        print "\nConfig target attributes:\n\n";
        foreach (sort keys %target) {
            next if $_ =~ m|^_| || $_ eq 'template';
            my $quotify = sub {
                map { (my $x = $_) =~ s|([\\\$\@"])|\\$1|g; "\"$x\""} @_;
            };
            print '    ', $_, ' => ';
            if (ref($target{$_}) eq "ARRAY") {
                print '[ ', join(', ', $quotify->(@{$target{$_}})), " ],\n";
            } else {
                print $quotify->($target{$_}), ",\n"
            }
        }
    }
    if ($dump || $envvars) {
        print "\nRecorded environment:\n\n";
        foreach (sort keys %{$config{perlenv}}) {
            print '    ',$_,' = ',($config{perlenv}->{$_} || ''),"\n";
        }
    }
    if ($dump || $makevars) {
        print "\nMakevars:\n\n";
        foreach my $var (@makevars) {
            my $prefix = '';
            $prefix = $config{CROSS_COMPILE}
                if grep { $var eq $_ } @user_crossable;
            $prefix //= '';
            print '    ',$var,' ' x (16 - length $var),'= ',
                (ref $config{$var} eq 'ARRAY'
                 ? join(' ', @{$config{$var}})
                 : $prefix.$config{$var}),
                "\n"
                if defined $config{$var};
        }

        my @buildfile = ($config{builddir}, $config{build_file});
        unshift @buildfile, $here
            unless file_name_is_absolute($config{builddir});
        my $buildfile = canonpath(catdir(@buildfile));
        print <<"_____";

NOTE: These variables only represent the configuration view.  The build file
template may have processed these variables further, please have a look at the
build file for more exact data:
    $buildfile
_____
    }
    if ($dump || $buildparams) {
        my @buildfile = ($config{builddir}, $config{build_file});
        unshift @buildfile, $here
            unless file_name_is_absolute($config{builddir});
        print "\nbuild file:\n\n";
        print "    ", canonpath(catfile(@buildfile)),"\n";

        print "\nbuild file templates:\n\n";
        foreach (@{$config{build_file_templates}}) {
            my @tmpl = ($_);
            unshift @tmpl, $here
                unless file_name_is_absolute($config{sourcedir});
            print '    ',canonpath(catfile(@tmpl)),"\n";
        }
    }
    if ($reconf) {
        if ($verbose) {
            print 'Reconfiguring with: ', join(' ',@{$config{perlargv}}), "\n";
            foreach (sort keys %{$config{perlenv}}) {
                print '    ',$_,' = ',($config{perlenv}->{$_} || ""),"\n";
            }
        }

        chdir $here;
        exec $^X,catfile($config{sourcedir}, 'Configure'),'reconf';
    }
}

1;

__END__

=head1 NAME

configdata.pm - configuration data for OpenSSL builds

=head1 SYNOPSIS

Interactive:

  perl configdata.pm [options]

As data bank module:

  use configdata;

=head1 DESCRIPTION

This module can be used in two modes, interactively and as a module containing
all the data recorded by OpenSSL's Configure script.

When used interactively, simply run it as any perl script, with at least one
option, and you will get the information you ask for.  See L</OPTIONS> below.

When loaded as a module, you get a few databanks with useful information to
perform build related tasks.  The databanks are:

    %config             Configured things.
    %target             The OpenSSL config target with all inheritances
                        resolved.
    %disabled           The features that are disabled.
    @disablables        The list of features that can be disabled.
    %withargs           All data given through --with-THING options.
    %unified_info       All information that was computed from the build.info
                        files.

=head1 OPTIONS

=over 4

=item B<--help>

Print a brief help message and exit.

=item B<--man>

Print the manual page and exit.

=item B<--dump> | B<-d>

Print all relevant configuration data.  This is equivalent to B<--command-line>
B<--options> B<--target> B<--environment> B<--make-variables>
B<--build-parameters>.

=item B<--command-line> | B<-c>

Print the current configuration command line.

=item B<--options> | B<-o>

Print the features, both enabled and disabled, and display defined macro and
skipped directories where applicable.

=item B<--target> | B<-t>

Print the config attributes for this config target.

=item B<--environment> | B<-e>

Print the environment variables and their values at the time of configuration.

=item B<--make-variables> | B<-m>

Print the main make variables generated in the current configuration

=item B<--build-parameters> | B<-b>

Print the build parameters, i.e. build file and build file templates.

=item B<--reconfigure> | B<--reconf> | B<-r>

Redo the configuration.

=item B<--verbose> | B<-v>

Verbose output.

=back

=cut

