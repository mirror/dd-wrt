/* SPDX-FileCopyrightText: 2011 Nokia
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <unistd.h>

#include "gvdb/gvdb-builder.h"
#include "gvdb/gvdb-reader.h"

static void
remove_file (const gchar *filename)
{
  g_assert_no_errno (unlink (filename));
}

static void
test_gvdb_nested_keys (void)
{
  GHashTable *root_table, *ns_table;
  GvdbItem *root, *item;
  GvdbTable *root_level, *ns_level;
  char **keys;
  GVariant *value;
  guint item_id;
  char *key;
  char *db_file = g_build_filename (g_get_tmp_dir (), "test_nested_keys.gvdb", NULL);
  GError *local_error = NULL;
  gboolean retval;

  root_table = gvdb_hash_table_new (NULL, NULL);

  ns_table = gvdb_hash_table_new (root_table, "namespaces");
  root = gvdb_hash_table_insert (ns_table, "");
  for (item_id = 0; item_id < 3; item_id++)
    {
      key = g_strdup_printf ("ns%d", item_id);
      item = gvdb_hash_table_insert (ns_table, key);
      gvdb_item_set_parent (item, root);
      gvdb_item_set_value (item, g_variant_new_string ("http://some.cool.ns"));
      g_free (key);
    }

  retval = gvdb_table_write_contents (root_table, db_file, FALSE, &local_error);
  g_assert_no_error (local_error);
  g_assert_true (retval);

  g_hash_table_unref (ns_table);
  g_hash_table_unref (root_table);

  root_level = gvdb_table_new (db_file, TRUE, &local_error);
  g_assert_no_error (local_error);
  g_assert_nonnull (root_level);

  ns_level = gvdb_table_get_table (root_level, "namespaces");
  g_assert_nonnull (ns_level);

  keys = gvdb_table_list (ns_level, "");
  g_assert_nonnull (keys);
  g_assert_cmpint (g_strv_length (keys), ==, 3);
  for (item_id = 0; item_id < 3; item_id++)
    {
      key = g_strdup_printf ("ns%d", item_id);
      g_assert_true (gvdb_table_has_value (ns_level, key));
      value = gvdb_table_get_raw_value (ns_level, key);
      g_assert_cmpstr (g_variant_get_string (value, NULL), ==, "http://some.cool.ns");
      g_free (key);
      g_variant_unref (value);
    }
  g_strfreev (keys);

  gvdb_table_free (root_level);
  gvdb_table_free (ns_level);
  remove_file (db_file);
  g_free (db_file);
}

static void
simple_test (const gchar *filename,
             gboolean     use_byteswap)
{
  GHashTable *table;
  GvdbTable  *read;
  GVariant   *value;
  GVariant   *expected;
  GError *local_error = NULL;
  gboolean retval;

  table = gvdb_hash_table_new (NULL, "level1");
  gvdb_hash_table_insert_string (table, "key1", "here just a flat string");
  retval = gvdb_table_write_contents (table, filename, use_byteswap, &local_error);
  g_assert_no_error (local_error);
  g_assert_true (retval);
  g_hash_table_unref (table);

  read = gvdb_table_new (filename, TRUE, &local_error);
  g_assert_no_error (local_error);
  g_assert_nonnull (read);
  g_assert_true (gvdb_table_is_valid (read));

  g_assert_true (gvdb_table_has_value (read, "key1"));
  value = gvdb_table_get_value (read, "key1");
  expected = g_variant_new_string ("here just a flat string");
  g_assert_cmpvariant (value, expected);

  g_variant_unref (expected);
  g_variant_unref (value);

  gvdb_table_free (read);
}

static void
test_gvdb_byteswapped (void)
{
  char *db_file = g_build_filename (g_get_tmp_dir (), "test_byteswapped.gvdb", NULL);

  simple_test (db_file, TRUE);

  remove_file (db_file);
  g_free (db_file);
}

static void
test_gvdb_flat_strings (void)
{
  char *db_file = g_build_filename (g_get_tmp_dir (), "test_flat_strings.gvdb", NULL);

  simple_test (db_file, FALSE);

  remove_file (db_file);
  g_free (db_file);
}

static void
test_gvdb_corrupted_file (void)
{
  char *db_file = g_build_filename (g_get_tmp_dir (), "test_invalid.gvdb", NULL);
  GError *local_error = NULL;

  g_file_set_contents (db_file,
                       "Just a bunch of rubbish to fill a text file and try to open it"
                       "as a gvdb and check the error is correctly reported",
                       -1, &local_error);
  g_assert_no_error (local_error);

  gvdb_table_new (db_file, TRUE, &local_error);
  g_assert_error (local_error, G_FILE_ERROR, G_FILE_ERROR_INVAL);
  g_clear_error (&local_error);

  remove_file (db_file);
  g_free (db_file);
}


int
main (int    argc,
      char **argv)
{
  g_test_init (&argc, &argv, G_TEST_OPTION_ISOLATE_DIRS, NULL);

  g_test_add_func ("/gvdb/flat_strings", test_gvdb_flat_strings);
  g_test_add_func ("/gvdb/nested_keys", test_gvdb_nested_keys);
  g_test_add_func ("/gvdb/byteswapped", test_gvdb_byteswapped);
  g_test_add_func ("/gvdb/corrupted_file", test_gvdb_corrupted_file);

  return g_test_run ();
}
