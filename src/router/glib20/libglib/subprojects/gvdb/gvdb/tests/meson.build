# SPDX-FileCopyrightText: 2024 Carlos Garnacho
# SPDX-License-Identifier: LGPL-2.1-or-later

gvdb_test_exec = executable(
  'gvdb',
  files('gvdb.c'),
  dependencies: [dependency('gio-2.0'), gvdb_dep],
)

test(
  'gvdb',
  gvdb_test_exec,
)