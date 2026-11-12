#!/bin/sh
# Regression test: CA -> CSR -> cert chain, verified with OpenSSL.
#   tests/check.sh ./eccert            (native)
#   tests/check.sh "qemu-mipsel bin/eccert-mipsel"
E="$1"; T=$(mktemp -d); fail=0
ok()  { echo "  ok   $1"; }
bad() { echo "  FAIL $1"; fail=1; }
t()   { d="$1"; shift; if "$@" >/dev/null 2>&1; then ok "$d"; else bad "$d"; fi; }
o()   { openssl "$@"; }

t "genkey CA"               $E genkey -out $T/ca.key
t "key valid (openssl)"     o ec -in $T/ca.key -check -noout
t "CA cert"                 $E cert -key $T/ca.key -ca -pathlen 0 -subj "/C=DE/O=Test/CN=Test CA" -days 7300 -out $T/ca.pem
t "genkey server"           $E genkey -out $T/s.key
t "CSR with SAN"            $E req -key $T/s.key -subj /CN=router.lan -san "DNS:router.lan,IP:192.168.1.1,IP:fd00::1" -out $T/s.csr
t "CSR verifies (openssl)"  o req -in $T/s.csr -verify -noout
$E req -key $T/s.key -subj /CN=router.lan -san "DNS:router.lan,IP:192.168.1.1,IP:fd00::1" -out $T/s2.csr
t "CSR deterministic (RFC 6979)" cmp -s $T/s.csr $T/s2.csr
t "issue from CSR"          $E cert -csr $T/s.csr -cakey $T/ca.key -cacert $T/ca.pem -days 825 -out $T/s.pem
t "chain strict + sslserver" o verify -x509_strict -purpose sslserver -CAfile $T/ca.pem $T/s.pem
o x509 -in $T/s.pem -noout -ext subjectAltName 2>/dev/null | grep -q "FD00:0:0:0:0:0:0:1" && ok "SAN copied from CSR" || bad "SAN copied from CSR"
$E pub -key $T/s.key > $T/pub1 2>/dev/null; o ec -in $T/s.key -pubout 2>/dev/null > $T/pub2
t "pub matches openssl"     cmp -s $T/pub1 $T/pub2
t "self-signed, auto SAN"   $E cert -key $T/s.key -subj /CN=10.0.0.1 -out $T/ss.pem
t "self-signed verifies"    o verify -x509_strict -purpose sslserver -CAfile $T/ss.pem $T/ss.pem
o genpkey -algorithm EC -pkeyopt ec_paramgen_curve:P-256 -out $T/o.key 2>/dev/null
o req -new -key $T/o.key -subj "/O=Foo/CN=bar" -addext "subjectAltName=DNS:bar.example" -out $T/o.csr 2>/dev/null
t "openssl PKCS#8 key in"   $E cert -key $T/o.key -subj /CN=p8 -out $T/p8.pem
t "openssl CSR signed"      $E cert -csr $T/o.csr -cakey $T/ca.key -cacert $T/ca.pem -out $T/o.pem
t "openssl CSR chain"       o verify -CAfile $T/ca.pem $T/o.pem
python3 - "$T/s.csr" "$T/bad.csr" <<'EOF'
import base64, sys
t = open(sys.argv[1]).read().split('\n'); d = bytearray(base64.b64decode(''.join(t[1:-2]))); d[-5] ^= 1
open(sys.argv[2], 'w').write('-----BEGIN CERTIFICATE REQUEST-----\n' + base64.b64encode(d).decode() + '\n-----END CERTIFICATE REQUEST-----\n')
EOF
if $E cert -csr $T/bad.csr -cakey $T/ca.key -cacert $T/ca.pem >/dev/null 2>&1; then bad "tampered CSR rejected"; else ok "tampered CSR rejected"; fi
if $E cert -key $T/s.key -cakey $T/s.key -cacert $T/ca.pem >/dev/null 2>&1; then bad "wrong CA key rejected"; else ok "wrong CA key rejected"; fi
o x509 -in "$( $E cert -key $T/s.key -days 0 > $T/n.pem; echo $T/n.pem )" -noout -enddate | grep -q 9999 && ok "-days 0 = 9999" || bad "-days 0 = 9999"
rm -rf $T
[ $fail = 0 ] && echo "ALL OK" || { echo "FAILURES"; exit 1; }
