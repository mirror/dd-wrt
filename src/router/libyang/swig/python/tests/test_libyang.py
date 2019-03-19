#!/usr/bin/env python
from __future__ import print_function

__author__ = "Matija Amidzic <matija.amidzic@sartura.hr>"
__copyright__ = "Copyright 2018, Deutsche Telekom AG"
__license__ = "BSD 3-Clause"

# This source code is licensed under BSD 3-Clause License (the "License").
# You may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     https://opensource.org/licenses/BSD-3-Clause

import yang as ly
import unittest
import sys

import config

class UnexpectedError(Exception):
    """Exception raised for errors that are not expected.

    Attributes:
        message -- explanation of the error
    """

    def __init__(self, message):
        self.message = message

class TestUM(unittest.TestCase):
    def test_ly_ctx_new(self):
        yang_folder1 = config.TESTS_DIR + "/data/files"
        yang_folder2 = config.TESTS_DIR + "/data:" + config.TESTS_DIR + "/data/files"

        try:
            # Tests
            ctx = ly.Context(yang_folder1)
            self.assertIsNotNone(ctx)
            list = ctx.get_searchdirs()
            self.assertIsNotNone(list)
            self.assertEqual(1, len(list))

            ctx = ly.Context(yang_folder2)
            self.assertIsNotNone(ctx)
            list = ctx.get_searchdirs()
            self.assertIsNotNone(list)
            self.assertEqual(2, len(list))

        except Exception as e:
            self.fail(e)

    def test_ly_ctx_new_invalid(self):
        yang_folder = "INVALID_PATH"

        try:
            ctx = ly.Context(yang_folder)
            raise UnexpectedError("exception not thrown")

        except UnexpectedError as e:
            self.fail(e)

        except RuntimeError as e:
            return

        except Exception as e:
            self.fail(e)

    def test_ly_ctx_get_searchdirs(self):
        yang_folder = config.TESTS_DIR + "/data/files"

        try:
            # Setup
            ctx = ly.Context(yang_folder)
            self.assertIsNotNone(ctx)

            # Tests
            list = ctx.get_searchdirs()
            self.assertEqual(1, len(list))
            self.assertEqual(yang_folder, list[0])

        except Exception as e:
            self.fail(e)

    def test_ly_ctx_set_searchdir(self):
        yang_folder = config.TESTS_DIR + "/data/files"
        new_yang_folder = config.TESTS_DIR + "/schema/yin"

        try:
            # Setup
            ctx = ly.Context(yang_folder)
            self.assertIsNotNone(ctx)

            # Tests
            list = ctx.get_searchdirs()
            self.assertEqual(1, len(list))
            self.assertEqual(yang_folder, list[0])

            ctx.set_searchdir(new_yang_folder)
            list = ctx.get_searchdirs()
            self.assertEqual(2, len(list))
            self.assertEqual(new_yang_folder, list[1])

            ctx.unset_searchdirs(0)
            list = ctx.get_searchdirs()
            self.assertEqual(1, len(list))
            self.assertEqual(new_yang_folder, list[0])

        except Exception as e:
            self.fail(e)

    def test_ly_ctx_set_searchdir_invalid(self):
        yang_folder = config.TESTS_DIR + "/data/files"
        new_yang_folder = "INVALID_PATH"

        try:
            # Setup
            ctx = ly.Context(yang_folder)
            self.assertIsNotNone(ctx)

            # Tests
            ctx.set_searchdir(new_yang_folder)
            raise UnexpectedError("exception not thrown")

        except UnexpectedError as e:
            self.fail(e)

        except RuntimeError as e:
            return

        except Exception as e:
            self.fail(e)

    def test_ly_ctx_info(self):
        yang_folder = config.TESTS_DIR + "/api/files"

        try:
            # Setup
            ctx = ly.Context(yang_folder)
            self.assertIsNotNone(ctx)

            # Tests
            info = ctx.info()
            self.assertIsNotNone(info)
            self.assertEqual(ly.LYD_VAL_OK, info.validity())

        except Exception as e:
            self.fail(e)

    def test_ly_ctx_load_module_invalid(self):
        yang_folder = config.TESTS_DIR + "/api/files"

        try:
            # Setup
            ctx = ly.Context(yang_folder)
            self.assertIsNotNone(ctx)

            # Tests
            module = ctx.load_module("invalid", None)
            raise UnexpectedError("exception not thrown")

        except UnexpectedError as e:
            self.fail(e)

        except RuntimeError as e:
            return

        except Exception as e:
            self.fail(e)

    def test_ly_ctx_load_get_module(self):
        yang_folder = config.TESTS_DIR + "/api/files"
        name1 = "a"
        name2 = "b"
        revision = "2016-03-01"

        try:
            # Setup
            ctx = ly.Context(yang_folder)
            self.assertIsNotNone(ctx)

            # Tests
            module = ctx.get_module("invalid")
            self.assertIsNone(module)

            # module needs to be loaded first
            module = ctx.get_module(name1)
            self.assertIsNone(module)

            module = ctx.load_module(name1)
            self.assertIsNotNone(module)
            self.assertEqual(name1, module.name(), "Module names don't match")

            module = ctx.load_module(name2, revision)
            self.assertIsNotNone(module)
            self.assertEqual(name2, module.name(), "Module names don't match")
            self.assertEqual(revision, module.rev().date(), "Module revisions don't match")

            module = ctx.get_module(name2, "INVALID_REVISION")
            self.assertIsNone(module)

            module = ctx.get_module(name1)
            self.assertIsNotNone(module)
            self.assertEqual(name1, module.name(), "Module names don't match")

            module = ctx.get_module(name2, revision)
            self.assertIsNotNone(module)
            self.assertEqual(name2, module.name(), "Module names don't match")
            self.assertEqual(revision, module.rev().date(), "Module revisions don't match")

        except Exception as e:
            self.fail(e)

    def test_ly_ctx_get_module_older(self):
        yang_folder = config.TESTS_DIR + "/api/files"
        name = "b"
        revision = "2016-03-01"
        revision_older = "2015-01-01"

        try:
            # Setup
            ctx = ly.Context(yang_folder)
            self.assertIsNotNone(ctx)

            # Tests
            module = ctx.load_module("c")
            self.assertIsNotNone(module)
            self.assertEqual("c", module.name(), "Module names don't match")

            module = ctx.load_module(name, revision)
            self.assertIsNotNone(module)
            self.assertEqual(name, module.name(), "Module names don't match")
            self.assertEqual(revision, module.rev().date(), "Module revisions don't match")

            module_older = ctx.get_module_older(module)
            self.assertIsNotNone(module_older)
            self.assertEqual(name, module_older.name(), "Module names don't match")
            self.assertEqual(revision_older, module_older.rev().date(), "Module revisions don't match")

        except Exception as e:
            self.fail(e)

    def test_ly_ctx_get_module_by_ns(self):
        yang_folder = config.TESTS_DIR + "/api/files"
        module_name = "a"
        ns = "urn:a"

        try:
            # Setup
            ctx = ly.Context(yang_folder)
            self.assertIsNotNone(ctx)
            module = ctx.load_module(module_name)
            self.assertIsNotNone(module)
            self.assertEqual(module_name, module.name(), "Module names don't match")

            # Tests
            module = ctx.get_module_by_ns(ns)
            self.assertIsNotNone(module)
            self.assertEqual(module_name, module.name(), "Module names don't match")

        except Exception as e:
            self.fail(e)

    def test_ly_ctx_clean(self):
        yang_folder = config.TESTS_DIR + "/api/files"
        module_name = "a"

        try:
            # Setup
            ctx = ly.Context(yang_folder)
            self.assertIsNotNone(ctx)
            module = ctx.load_module(module_name)
            self.assertIsNotNone(module)
            self.assertEqual(module_name, module.name(), "Module names don't match")

            # Tests
            # confirm module is loaded
            module = ctx.get_module(module_name)
            self.assertIsNotNone(module)
            self.assertEqual(module_name, module.name(), "Module names don't match")

            ctx.clean()

            # confirm ctx is cleaned
            module = ctx.get_module(module_name)
            self.assertIsNone(module)

        except Exception as e:
            self.fail(e)

    def test_ly_ctx_parse_module_path(self):
        yang_folder = config.TESTS_DIR + "/api/files"
        yin_file = config.TESTS_DIR + "/api/files/a.yin"
        yang_file = config.TESTS_DIR + "/api/files/b.yang"
        module_name1 = "a"
        module_name2 = "b"

        try:
            # Setup
            ctx = ly.Context(yang_folder)
            self.assertIsNotNone(ctx)

            # Tests
            module = ctx.parse_module_path(yin_file, ly.LYS_IN_YIN)
            self.assertIsNotNone(module)
            self.assertEqual(module_name1, module.name(), "Module names don't match")

            module = ctx.parse_module_path(yang_file, ly.LYS_IN_YANG)
            self.assertIsNotNone(module)
            self.assertEqual(module_name2, module.name(), "Module names don't match")

        except Exception as e:
            self.fail(e)

    def test_ly_ctx_parse_module_path_invalid(self):
        yang_folder = config.TESTS_DIR + "/api/files"

        try:
            # Setup
            ctx = ly.Context(yang_folder)
            self.assertIsNotNone(ctx)

            # Tests
            module = ctx.parse_module_path("INVALID_YANG_FILE", ly.LYS_IN_YANG)
            raise UnexpectedError("exception not thrown")

        except UnexpectedError as e:
            self.fail(e)

        except RuntimeError as e:
            return

        except Exception as e:
            self.fail(e)

    def test_ly_ctx_get_submodule(self):
        yang_folder = config.TESTS_DIR + "/api/files"
        yin_file = config.TESTS_DIR + "/api/files/a.yin"
        module_name = "a"
        sub_name = "asub"

        try:
            # Setup
            ctx = ly.Context(yang_folder)
            self.assertIsNotNone(ctx)
            ctx.parse_module_path(yin_file, ly.LYS_IN_YIN)

            # Tests
            submodule = ctx.get_submodule(module_name, None, sub_name, None)
            self.assertIsNotNone(submodule)
            self.assertEqual(sub_name, submodule.name(), "Module names don't match")

        except Exception as e:
            self.fail(e)

    def test_ly_ctx_get_submodule2(self):
        yang_folder = config.TESTS_DIR + "/api/files"
        yin_file = config.TESTS_DIR + "/api/files/a.yin"
        config_file = config.TESTS_DIR + "/api/files/a.xml"
        sub_name = "asub"

        try:
            # Setup
            ctx = ly.Context(yang_folder)
            self.assertIsNotNone(ctx)
            ctx.parse_module_path(yin_file, ly.LYS_IN_YIN)
            root = ctx.parse_data_path(config_file, ly.LYD_XML, ly.LYD_OPT_CONFIG | ly.LYD_OPT_STRICT)
            self.assertIsNotNone(root)
            self.assertIsNotNone(root.schema().module())

            # Tests
            submodule = ctx.get_submodule2(root.schema().module(), sub_name)
            self.assertIsNotNone(submodule)
            self.assertEqual(sub_name, submodule.name(), "Module names don't match")

        except Exception as e:
            self.fail(e)

    def test_ly_ctx_find_path(self):
        yang_folder = config.TESTS_DIR + "/api/files"
        yin_file = config.TESTS_DIR + "/api/files/a.yin"
        yang_file = config.TESTS_DIR + "/api/files/b.yang"
        schema_path1 = "/b:x/b:bubba"
        schema_path2 = "/a:x/a:bubba"

        try:
            # Setup
            ctx = ly.Context(yang_folder)
            self.assertIsNotNone(ctx)

            # Tests
            ctx.parse_module_path(yang_file, ly.LYS_IN_YANG)
            set = ctx.find_path(schema_path1)
            self.assertIsNotNone(set)

            ctx.parse_module_path(yin_file, ly.LYS_IN_YIN)
            set = ctx.find_path(schema_path2)
            self.assertIsNotNone(set)
            ly.Set()

        except Exception as e:
            self.fail(e)

    def test_ly_set(self):
        yang_folder = config.TESTS_DIR + "/api/files"
        yin_file = config.TESTS_DIR + "/api/files/a.yin"
        config_file = config.TESTS_DIR + "/api/files/a.xml"

        try:
            # Setup
            ctx = ly.Context(yang_folder)
            self.assertIsNotNone(ctx)
            ctx.parse_module_path(yin_file, ly.LYS_IN_YIN)
            root = ctx.parse_data_path(config_file, ly.LYD_XML, ly.LYD_OPT_CONFIG | ly.LYD_OPT_STRICT)
            self.assertIsNotNone(root)

            # Tests
            set = ly.Set()
            self.assertIsNotNone(set)
            self.assertEqual(0, set.number())

            set.add(root.child().schema())
            self.assertEqual(1, set.number())

            set.add(root.schema())
            self.assertEqual(2, set.number())

            set.rm(root.schema())
            self.assertEqual(1, set.number())

            set.add(root.schema())
            self.assertEqual(2, set.number())

            set.rm_index(1)
            self.assertEqual(1, set.number())

            set.clean()
            self.assertEqual(0, set.number())

        except Exception as e:
            self.fail(e)

    def test_ly_ctx_new_ylpath(self):
        yang_folder = config.TESTS_DIR + "/api/files"
        path = config.TESTS_DIR + "/api/files/ylpath.xml"

        try:
            ctx = ly.Context(yang_folder, path, ly.LYD_XML, 0)
            self.assertIsNotNone(ctx)
            list = ctx.get_searchdirs()
            self.assertIsNotNone(list)
            self.assertEqual(1, len(list))

        except Exception as e:
            self.fail(e)

    def test_ly_ctx_new_ylmem(self):
        yang_folder = config.TESTS_DIR + "/api/files"

        try:
            ctx = ly.Context(yang_folder)
            self.assertIsNotNone(ctx)
            info = ctx.info()
            self.assertIsNotNone(info)
            self.assertEqual(ly.LYD_VAL_OK, info.validity())

            mem = info.print_mem(ly.LYD_XML, 0)
            self.assertIsNotNone(mem)

            new_ctx = ly.Context(yang_folder, ly.LYD_XML, mem, 0)
            self.assertIsNotNone(new_ctx)
            list = ctx.get_searchdirs()
            self.assertIsNotNone(list)
            self.assertEqual(1, len(list))

        except Exception as e:
            self.fail(e)

    def test_ly_ctx_get_module_iter(self):
        yang_folder = config.TESTS_DIR + "/api/files"
        module_name = "a"

        try:
            ctx = ly.Context(yang_folder)
            self.assertIsNotNone(ctx)

            module = ctx.load_module(module_name)
            self.assertIsNotNone(module)
            self.assertEqual(module_name, module.name())
            itr = ctx.get_module_iter()
            self.assertIsNotNone(itr)
            self.assertEqual(7, len(itr))

        except Exception as e:
            self.fail(e)


    def test_ly_ctx_get_disabled_module_iter(self):
        yang_folder = config.TESTS_DIR +  "/api/files"
        module_name = "x"

        try:
            ctx = ly.Context(yang_folder)
            self.assertIsNotNone(ctx)

            module = ctx.load_module(module_name)
            self.assertIsNotNone(module)
            self.assertEqual(module_name, module.name())
            # FIXME no way to disable module from here

            itr = ctx.get_disabled_module_iter()
            self.assertIsNotNone(itr)
            self.assertEqual(0, len(itr))

        except Exception as e:
            self.fail(e)

    def test_ly_ctx_data_instantiables(self):
        yang_folder = config.TESTS_DIR + "/api/files"
        module_name = "b"

        try:
            ctx = ly.Context(yang_folder)
            self.assertIsNotNone(ctx)

            module = ctx.load_module(module_name)
            self.assertIsNotNone(module)
            self.assertEqual(module_name, module.name())

            instantiables = ctx.data_instantiables(0)
            self.assertIsNotNone(instantiables)
            self.assertEqual(5, len(instantiables))

        except Exception as e:
            self.fail(e)

    def test_ly_ctx_get_node(self):
        yang_folder = config.TESTS_DIR + "/api/files"
        module_name = "b"

        try:
            ctx = ly.Context(yang_folder)
            self.assertIsNotNone(ctx)

            module = ctx.load_module(module_name)
            self.assertIsNotNone(module)
            self.assertEqual(module_name, module.name())

            instantiables = ctx.data_instantiables(0)
            self.assertIsNotNone(instantiables)
            self.assertEqual(5, len(instantiables))

            schema = instantiables[0];
            self.assertIsNotNone(schema)
            node = ctx.get_node(schema, "/b:x/b:bubba", 0)
            self.assertIsNotNone(node)

        except Exception as e:
            self.fail(e)

if __name__ == '__main__':
    unittest.main()
