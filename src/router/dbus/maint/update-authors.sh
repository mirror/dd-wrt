#!/bin/sh
# Copyright 2010 Lennart Poettering
# Copyright 2022 Simon McVittie
# SPDX-License-Identifier: MIT

set -eux

if [ -n "${MESON_SOURCE_ROOT-}" ]; then
    cd "${MESON_SOURCE_ROOT}"
fi

git shortlog -s -e | cut -c 8- | sort > AUTHORS
