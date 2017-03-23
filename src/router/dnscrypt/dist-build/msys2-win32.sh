#! /bin/sh

export CFLAGS="-Os -m32 -march=pentium3 -mtune=core2"
export LDFLAGS="-march=pentium3"
export PREFIX="$(pwd)/dnscrypt-proxy-win32"
export MINGW_PREFIX='/mingw32'
export SODIUM_PREFIX='/tmp/libsodium-win32'

export CPPFLAGS="${CPPFLAGS} -I${SODIUM_PREFIX}/include"
export LDFLAGS="${LDFLAGS} -L${SODIUM_PREFIX}/lib"

if [ ! -d "$SODIUM_PREFIX" ]; then
  echo "WARNING: [$SODIUM_PREFIX] not found, compiling against system libsodium" >&2
  sleep 10
fi

./configure \
  --host=i686-w64-mingw32 \
  --with-included-ltdl \
  --bindir="$PREFIX" \
  --datarootdir="$PREFIX" \
  --exec-prefix="$PREFIX" \
  --prefix="$PREFIX" \
  --sbindir="$PREFIX" \
  --sysconfdir="$PREFIX" && \
make install

rm -fr "${PREFIX}/man"
rm -fr "${PREFIX}/share"
rm -fr "${PREFIX}/lib/pkgconfig"
mv "${PREFIX}/lib/dnscrypt-proxy/"*.dll "${PREFIX}/"
mv "${PREFIX}/dnscrypt-proxy/"* "${PREFIX}/"
rmdir "${PREFIX}/dnscrypt-proxy"
rm -fr "${PREFIX}/lib"
cp "${MINGW_PREFIX}/bin/libwinpthread-1.dll" "${PREFIX}/"
cp "${MINGW_PREFIX}/bin/libgcc_s_dw2-1.dll" "${PREFIX}/"
cp "${MINGW_PREFIX}/bin/libldns-1.dll" "${PREFIX}/"
cp "${SODIUM_PREFIX}/bin/libsodium-18.dll" "${PREFIX}/"

nm "${MINGW_PREFIX}/bin/libldns-1.dll" | \
  fgrep -i libeay > /dev/null &&
  cp "${MINGW_PREFIX}/bin/"libeay*.dll "${PREFIX}/"
