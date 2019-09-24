#! /bin/sh

set -e

cd /workdir

apk --update upgrade
apk add build-base
apk add coreutils
apk add ldns-dev
apk add libsodium-dev

./configure --disable-dependency-tracking --enable-debug
make clean > /dev/null
make -j$(nproc) check
make -j$(nproc) install

/usr/local/sbin/dnscrypt-proxy -t 60 -R random \
  --plugin=libdcplugin_example.so \
  --plugin=libdcplugin_example_cache.so \
  --plugin=/usr/local/lib/dnscrypt-proxy/libdcplugin_example_ldns_aaaa_blocking.so

echo 'ResolverName random' > /tmp/dnscrypt-proxy.conf
echo 'LocalCache yes' >> /tmp/dnscrypt-proxy.conf
echo 'BlockIPv6 yes' >> /tmp/dnscrypt-proxy.conf
echo 'Plugin libdcplugin_example.so' >> /tmp/dnscrypt-proxy.conf
echo 'Test 60' >> /tmp/dnscrypt-proxy.conf

/usr/local/sbin/dnscrypt-proxy /tmp/dnscrypt-proxy.conf

make uninstall > /dev/null
make distclean > /dev/null
