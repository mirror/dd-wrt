#! /bin/sh

export XCODEDIR=$(xcode-select -p)
export BASEDIR="${XCODEDIR}/Platforms/iPhoneOS.platform/Developer"
export PATH="${BASEDIR}/usr/bin:$BASEDIR/usr/sbin:$PATH"
export SDK="${BASEDIR}/SDKs/iPhoneOS.sdk"
export IPHONEOS_VERSION_MIN="6.0.0"
export CFLAGS="-Oz -mthumb -arch armv7 -isysroot ${SDK} -miphoneos-version-min=${IPHONEOS_VERSION_MIN}"
export LDFLAGS="-mthumb -arch armv7 -isysroot ${SDK} -miphoneos-version-min=${IPHONEOS_VERSION_MIN}"
export PREFIX="$(pwd)/dnscrypt-proxy-ios"

export SODIUM_IOS_PREFIX="/tmp/libsodium-ios"
export CPPFLAGS="$CPPFLAGS -I${SODIUM_IOS_PREFIX}/include"
export LDFLAGS="$LDFLAGS -L${SODIUM_IOS_PREFIX}/lib"

./configure \
    --datadir="${PREFIX}/etc" \
    --disable-plugins \
    --disable-shared \
    --enable-relaxed-plugins-permissions \
    --host=arm-apple-darwin10 \
    --prefix="${PREFIX}" \
    --sysconfdir="${PREFIX}/etc/dnscrypt-proxy" && \
make clean && \
make -j3 install && \
rm -fr "${PREFIX}/include" "${PREFIX}/share" "${PREFIX}/man" && \
install -m 644 org.dnscrypt.osx.DNSCryptProxy.plist "${PREFIX}/org.dnscrypt.osx.DNSCryptProxy.plist" && \
cp README-iOS.markdown "${PREFIX}/" && \
echo "dnscrypt-proxy has been installed into ${PREFIX}" && \
echo 'Now, using codesign(1) to sign dnscrypt-proxy'
