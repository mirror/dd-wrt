GVDB
====

GVDB (GVariant Database) is a simple database file format that stores a
mapping from strings to GVariant values in a way that is extremely
efficient for lookups.

The code is intended to be pulled into projects as a submodule/subproject,
and it is not shipped as a separately compiled library. It has no API
guarantees.

A GVDB database table is a single file. It is designed to be memory mapped
by one or more clients, with accesses to the stored data being fast. The
storage format has low size overheads, assuming the GVariant formats for
values do not require much padding or alignment.

Modifying a GVDB table requires writing out the whole file. This is
relatively slow. `gvdb_table_write_contents()` does this by writing out
the new file and atomically renaming it over the old one. This means
that any clients who have memory mapped the old file will need to reload
their memory mapping.

This means that if multiple clients are using a GVDB table, an external
process is needed to synchronise writes and to notify clients to reload
the table. `dconf-service` is an example of such a process.
