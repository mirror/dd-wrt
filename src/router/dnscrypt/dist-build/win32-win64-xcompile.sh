#! /bin/sh

set -x
cd $(dirname $(readlink -f "$0"))

setup() {

  pacman -Syu --noconfirm
  pacman -Sy --noconfirm \
    base-devel git libtool autoconf automake \
    mingw-w64-toolchain \
    p7zip

  rm -fr tmp

  mkdir -p tmp && cd tmp
  BASE_DIR=$(pwd)

  SRCDEPS_DIR=$BASE_DIR/srcdeps
  mkdir -p $SRCDEPS_DIR

}

fetch_src() {

  cd $SRCDEPS_DIR
  (
    git clone --recursive https://github.com/libressl-portable/portable libressl --branch=v2.5.4
    cd libressl
    ./autogen.sh
  )
  (
    git clone --recursive https://git.nlnetlabs.nl/ldns
    cd ldns
    libtoolize -if
    autoreconf -if
  )
  (
    git clone --recursive https://github.com/jedisct1/libsodium
    cd libsodium
    ./autogen.sh
  )
  (
    git clone --recursive https://github.com/jedisct1/dnscrypt-proxy
    cd dnscrypt-proxy
    ./autogen.sh
  )
    
}

compile() {
    
  ARCH_DIR=${BASE_DIR}/${TARGET}
  mkdir -p $ARCH_DIR

  DEPS_DIR=$ARCH_DIR/deps
  mkdir -p $DEPS_DIR/share/man \
    $DEPS_DIR/bin $DEPS_DIR/lib $DEPS_DIR/sbin $DEPS_DIR/etc

  BUILD_DIR=$ARCH_DIR/build
  mkdir -p $BUILD_DIR

  RELEASE_DIR=$ARCH_DIR/release
  rm -fr $RELEASE_DIR
  mkdir -p $RELEASE_DIR

  PACKAGE_DIR=$ARCH_DIR/package
  rm -fr $PACKAGE_DIR
  mkdir -p $PACKAGE_DIR

  export CPPFLAGS="$CPPFLAGS -I${DEPS_DIR}/include"
  export LDFLAGS="$LDFLAGS -L${DEPS_DIR}/lib"

  cd $BUILD_DIR
  (
    mkdir libressl
    cd libressl
    $SRCDEPS_DIR/libressl/configure --disable-dependency-tracking \
      --host=$TARGET --prefix="$DEPS_DIR"
    make clean
    $SRCDEPS_DIR/libressl/configure --disable-dependency-tracking \
      --host=$TARGET --prefix="$DEPS_DIR"
    make install
  )

  cd $BUILD_DIR
  (
    mkdir ldns
    cd ldns
    $SRCDEPS_DIR/ldns/configure \
      --host=$TARGET --prefix="$DEPS_DIR" \
      --disable-dane --with-ssl=$DEPS_DIR --with-trust-anchor=root.key
    make clean
    make -i install
  )

  cd $BUILD_DIR
  (
    mkdir libsodium
    cd libsodium
    $SRCDEPS_DIR/libsodium/configure --disable-dependency-tracking \
      --host=$TARGET --prefix="$DEPS_DIR" --without-pthreads
    make clean
    make install
  )

  cd $BUILD_DIR
  (
    rm -fr dnscrypt-proxy
    cp -a $SRCDEPS_DIR/dnscrypt-proxy .
    cd dnscrypt-proxy
    ./configure --disable-dependency-tracking \
      --host=$TARGET \
      --bindir="$RELEASE_DIR" \
      --datarootdir="$RELEASE_DIR" \
      --docdir="${RELEASE_DIR}/doc" \
      --exec-prefix="$RELEASE_DIR" \
      --prefix="$RELEASE_DIR" \
      --sbindir="$RELEASE_DIR" \
      --sysconfdir="$RELEASE_DIR"
    make clean
    make install
    mv ${RELEASE_DIR}/lib/dnscrypt-proxy/*.dll $RELEASE_DIR
    mv ${RELEASE_DIR}/dnscrypt-proxy/* $RELEASE_DIR
    rmdir ${RELEASE_DIR}/dnscrypt-proxy
    rm -fr ${RELEASE_DIR}/lib
    rm -fr ${RELEASE_DIR}/man
    rm -fr ${RELEASE_DIR}/pkgconfig
    cp README-WINDOWS.markdown "${RELEASE_DIR}/doc"
    rm -f "${RELEASE_DIR}/doc/dnscrypt-proxy.conf"
    cp ${DEPS_DIR}/bin/*.dll $RELEASE_DIR
    rm ${RELEASE_DIR}/libtls-*.dll
    rm ${RELEASE_DIR}/libssl-*.dll
    cp /usr/${TARGET}/bin/libwinpthread-*.dll $RELEASE_DIR
    cp /usr/${TARGET}/bin/libgcc_s_*.dll $RELEASE_DIR
    strip ${RELEASE_DIR}/*.exe
  )

  (
    cd $PACKAGE_DIR
    mv $RELEASE_DIR $1
    7z a -tzip -mx=9 -r ${1}.zip $1
  )
}

setup

fetch_src

export CFLAGS="-Os -m64 -mtune=westmere"
export LDFLAGS="-mtune=westmere -static-libgcc -Wl,--dynamicbase -Wl,--high-entropy-va -Wl,--nxcompat -static-libgcc"
export TARGET=x86_64-w64-mingw32
compile dnscrypt-proxy-win64

export CFLAGS="-Os -m32 -march=pentium3 -mtune=core2"
export LDFLAGS="-march=pentium3 -static-libgcc -Wl,--dynamicbase -Wl,--nxcompat -static-libgcc"
export TARGET=i686-w64-mingw32
compile dnscrypt-proxy-win32
