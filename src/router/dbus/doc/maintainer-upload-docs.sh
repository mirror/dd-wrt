#!/usr/bin/env bash
# Copyright © 2019-2020 Salamandar <felix@piedallu.me>
# SPDX-License-Identifier: MIT
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

set -eux

: "${DOC_SERVER:=dbus.freedesktop.org}"
: "${DOC_WWW_DIR:=/srv/dbus.freedesktop.org/www}"

: "${SPECIFICATION_SERVER:=specifications.freedesktop.org}"
: "${SPECIFICATION_PATH:=/srv/specifications.freedesktop.org/www/dbus/1.0}"

if [ -n "${MESON_BUILD_ROOT-}" ]; then
    cd "${MESON_BUILD_ROOT}"
fi

if [ -n "${MESON_SOURCE_ROOT-}" ]; then
    top_srcdir="${MESON_SOURCE_ROOT}"
else
    # assume build directory is inside source directory
    top_srcdir=".."
fi

TMPDIR=$(mktemp -d)

mkdir -p "$TMPDIR/api"
cp -r doc/api/html "$TMPDIR/api"
cp -r "$@" "$TMPDIR"
mv "$TMPDIR" dbus-docs
tar --xz -c -f dbus-docs.tar.xz dbus-docs

scp dbus-docs.tar.xz "$DOC_SERVER:$DOC_WWW_DIR/"
rsync -rpvzP --chmod=Dg+s,ug+rwX,o=rX dbus-docs/ "$DOC_SERVER:$DOC_WWW_DIR/doc/"

scp -p "$top_srcdir"/doc/*.dtd "$SPECIFICATION_SERVER:$SPECIFICATION_PATH/"
