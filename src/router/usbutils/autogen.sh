#!/bin/sh -e
# SPDX-License-Identifier: GPL-2.0-only
# Copyright (c) 2009,2010 Greg Kroah-Hartman <gregkh@suse.de>
# Copyright (c) 2024 Greg Kroah-Hartman <gregkh@linuxfoundation.org>

meson setup build
meson compile -C build
