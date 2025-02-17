# SPDX-FileCopyrightText: 2022 Endless OS Foundation, LLC
# SPDX-License-Identifier: LGPL-2.1-or-later

project('gvdb', 'c',
    version: '0.0',
    meson_version: '>=1.1',
)

libgvdb_sources = files(
    'gvdb/gvdb-builder.c',
    'gvdb/gvdb-reader.c',
)

gvdb_dep = declare_dependency(
  sources: libgvdb_sources,
  include_directories: include_directories('.'),
)
meson.override_dependency('gvdb', gvdb_dep)

if get_option('tests')
  subdir('gvdb/tests')
endif
