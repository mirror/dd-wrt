project('gvdb', 'c',
    version: '0.0',
    meson_version: '>=0.54.0',
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
