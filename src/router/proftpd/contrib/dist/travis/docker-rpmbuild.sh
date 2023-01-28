#!/bin/bash

BRANCH=${TRAVIS_BRANCH:-master}
VERSION=${PACKAGE_VERSION:-1.3.8rc1}

# Make sure that the necessary packages/tools are installed
yum install -y \
  epel-release \
  gcc \
  git \
  imake \
  make \
  rpm-build

# These are for the basic proftpd build
yum install -y \
  gettext \
  libacl-devel \
  libcap-devel \
  ncurses-devel \
  pam-devel \
  pkgconfig \
  zlib-devel

# And these are for --with everything
yum install -y \
  geoip-devel \
  hiredis-devel \
  libmemcached-devel \
  libsodium-devel \
  mysql-devel \
  openldap-devel \
  openssl-devel \
  pcre-devel \
  postgresql-devel \
  sqlite-devel \
  tcp_wrappers-devel

# Build the RPM
rm -fr rpm/
mkdir rpm/
cd rpm/
git clone -q -b "${BRANCH}" --depth 10 https://github.com/proftpd/proftpd.git "proftpd-${VERSION}"
cd "proftpd-${VERSION}/"
./configure
make dist
cd ..
tar zcf "proftpd-${VERSION}.tar.gz" "proftpd-${VERSION}"
rpmbuild -ta "proftpd-${VERSION}.tar.gz" --with everything
