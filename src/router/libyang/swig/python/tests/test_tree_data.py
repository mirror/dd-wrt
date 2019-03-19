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

lys_module_a = \
"<?xml version=\"1.0\" encoding=\"UTF-8\"?>           \
<module name=\"a\"                                    \
        xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\"   \
        xmlns:md=\"urn:ietf:params:xml:ns:yang:ietf-yang-metadata\"\
        xmlns:a=\"urn:a\">                            \
  <namespace uri=\"urn:a\"/>                          \
  <prefix value=\"a_mod\"/>                           \
  <include module=\"asub\"/>                          \
  <include module=\"atop\"/>                          \
  <import module=\"ietf-yang-metadata\">              \
    <prefix value=\"md\"/>                            \
  </import>                                           \
  <feature name=\"foo\"/>                             \
  <grouping name=\"gg\">                              \
    <leaf name=\"bar-gggg\">                          \
      <type name=\"string\"/>                         \
    </leaf>                                           \
  </grouping>                                         \
  <md:annotation name=\"test\">                       \
    <type name=\"string\"/>                           \
  </md:annotation>                                    \
  <container name=\"x\">                              \
    <leaf name=\"bar-leaf\">                          \
      <type name=\"string\"/>                         \
    </leaf>                                           \
    <uses name=\"gg\">                                \
    </uses>                                           \
    <leaf name=\"baz\">                               \
      <type name=\"string\"/>                         \
    </leaf>                                           \
    <leaf name=\"bubba\">                             \
      <type name=\"string\"/>                         \
    </leaf>                                           \
    <leaf name=\"number32\">                          \
      <type name=\"int32\"/>                          \
    </leaf>                                           \
    <leaf name=\"number64\">                          \
      <type name=\"int64\"/>                          \
    </leaf>                                           \
    <leaf name=\"def-leaf\">                          \
      <type name=\"string\"/>                         \
      <default value=\"def\"/>                        \
    </leaf>                                           \
    <anydata name=\"any-data\"/>                      \
  </container>                                        \
  <leaf name=\"y\"><type name=\"string\"/></leaf>     \
  <anyxml name=\"any\"/>                              \
  <augment target-node=\"/x\">                        \
    <container name=\"bar-y\"/>                       \
  </augment>                                          \
  <rpc name=\"bar-rpc\">                              \
  </rpc>                                              \
  <rpc name=\"foo-rpc\">                              \
  </rpc>                                              \
  <rpc name=\"rpc1\">                                 \
    <input>                                           \
      <leaf name=\"input-leaf1\">                     \
        <type name=\"string\"/>                       \
      </leaf>                                         \
      <container name=\"x\">                          \
        <leaf name=\"input-leaf2\">                   \
          <type name=\"string\"/>                     \
        </leaf>                                       \
      </container>                                    \
    </input>                                          \
    <output>                                          \
      <leaf name=\"output-leaf1\">                    \
        <type name=\"string\"/>                       \
      </leaf>                                         \
      <leaf name=\"output-leaf2\">                    \
        <type name=\"string\"/>                       \
      </leaf>                                         \
      <container name=\"rpc-container\">              \
        <leaf name=\"output-leaf3\">                  \
          <type name=\"string\"/>                     \
        </leaf>                                       \
      </container>                                    \
    </output>                                         \
  </rpc>                                              \
  <list name=\"l\">                                   \
    <key value=\"key1 key2\"/>                        \
    <leaf name=\"key1\">                              \
      <type name=\"uint8\"/>                          \
    </leaf>                                           \
    <leaf name=\"key2\">                              \
      <type name=\"uint8\"/>                          \
    </leaf>                                           \
    <leaf name=\"value\">                             \
      <type name=\"string\"/>                         \
    </leaf>                                           \
  </list>                                             \
</module>                                             \
"

a_data_xml = "\
<x xmlns=\"urn:a\">\n\
  <bubba>test</bubba>\n\
  </x>\n"

result_xml = "<x xmlns=\"urn:a\"><bubba>test</bubba></x>"

result_xml_format ="\
<x xmlns=\"urn:a\">\n\
  <bubba>test</bubba>\n\
</x>\n\
"

result_json = "\
{\n\
  \"a:x\": {\n\
    \"bubba\": \"test\"\n\
  }\n\
}\n\
"

class UnexpectedError(Exception):
    """Exception raised for errors that are not expected.

    Attributes:
        message -- explanation of the error
    """

    def __init__(self, message):
        self.message = message

class TestUM(unittest.TestCase):
    def test_ly_ctx_parse_data_mem(self):
        yang_folder = config.TESTS_DIR + "/api/files"
        yin_file = config.TESTS_DIR + "/api/files/a.yin"

        try:
            # Setup
            ctx = ly.Context(yang_folder)
            self.assertIsNotNone(ctx)
            ctx.parse_module_path(yin_file, ly.LYS_IN_YIN)

            # Tests
            root = ctx.parse_data_mem(a_data_xml, ly.LYD_XML, ly.LYD_OPT_NOSIBLINGS | ly.LYD_OPT_STRICT)
            self.assertIsNotNone(root)
            self.assertEqual("x", root.schema().name())

        except Exception as e:
            self.fail(e)

    def test_ly_ctx_parse_data_fd(self):
        yang_folder = config.TESTS_DIR + "/api/files"
        yin_file = config.TESTS_DIR + "/api/files/a.yin"
        config_file = config.TESTS_DIR + "/api/files/a.xml"
        try:
            # Setup
            ctx = ly.Context(yang_folder)
            self.assertIsNotNone(ctx)
            ctx.parse_module_path(yin_file, ly.LYS_IN_YIN)

            # Tests
            f = open(config_file, 'r')
            fd = f.fileno()
            root = ctx.parse_data_fd(fd, ly.LYD_XML, ly.LYD_OPT_NOSIBLINGS | ly.LYD_OPT_STRICT)
            self.assertIsNotNone(root)
            self.assertEqual("x", root.schema().name())

        except Exception as e:
            self.fail(e)

        finally:
            f.close()

    def test_ly_ctx_parse_data_path(self):
        yang_folder = config.TESTS_DIR + "/api/files"
        yin_file = config.TESTS_DIR + "/api/files/a.yin"
        config_file = config.TESTS_DIR + "/api/files/a.xml"
        module_name = "a"
        schema_name = "x"
        try:
            # Setup
            ctx = ly.Context(yang_folder)
            self.assertIsNotNone(ctx)
            module = ctx.parse_module_path(yin_file, ly.LYS_IN_YIN)
            self.assertIsNotNone(module)
            self.assertEqual(module_name, module.name(), "Module names don't match")

            # Tests
            root = ctx.parse_data_path(config_file, ly.LYD_XML, ly.LYD_OPT_CONFIG | ly.LYD_OPT_STRICT)
            self.assertIsNotNone(root)
            self.assertEqual(schema_name, root.schema().name())

        except Exception as e:
            self.fail(e)

    def test_ly_ctx_parse_data_path_invalid(self):
        yang_folder = config.TESTS_DIR + "/api/files"

        try:
            # Setup
            ctx = ly.Context(yang_folder)
            self.assertIsNotNone(ctx)

            # Tests
            root = ctx.parse_data_path("INVALID_PATH", ly.LYD_XML, ly.LYD_OPT_CONFIG | ly.LYD_OPT_STRICT)
            raise UnexpectedError("exception not thrown")

        except UnexpectedError as e:
            self.fail(e)

        except RuntimeError as e:
            return

        except Exception as e:
            self.fail(e)

    def test_ly_data_node(self):
        yang_folder = config.TESTS_DIR + "/api/files"
        config_file = config.TESTS_DIR + "/api/files/a.xml"
        try:
            # Setup
            ctx = ly.Context(yang_folder)
            self.assertIsNotNone(ctx)
            ctx.parse_module_mem(lys_module_a, ly.LYS_IN_YIN)
            root = ctx.parse_data_path(config_file, ly.LYD_XML, ly.LYD_OPT_CONFIG | ly.LYD_OPT_STRICT)
            self.assertIsNotNone(root)

            # Tests
            new_node = ly.Data_Node(root, root.schema().module(), "number32", "100")
            self.assertIsNotNone(new_node)

        except Exception as e:
            self.fail(e)

    def test_ly_data_node_new_path(self):
        yang_folder = config.TESTS_DIR + "/api/files"
        config_file = config.TESTS_DIR + "/api/files/a.xml"
        try:
            # Setup
            ctx = ly.Context(yang_folder)
            self.assertIsNotNone(ctx)
            ctx.parse_module_mem(lys_module_a, ly.LYS_IN_YIN)
            ctx.parse_data_path(config_file, ly.LYD_XML, ly.LYD_OPT_CONFIG | ly.LYD_OPT_STRICT)
            mod = ctx.get_module("a", None, 1)
            self.assertIsNotNone(mod)

            # Tests
            root = ly.Data_Node(ctx, "/a:x/bar-gggg", "a", 0, 0)
            self.assertIsNotNone(root)
            self.assertEqual("x", root.schema().name())
            self.assertEqual("bar-gggg", root.child().schema().name())

            node = root.new_path(ctx, "def-leaf", "def", 0, ly.LYD_PATH_OPT_DFLT)
            self.assertIsNotNone(node)
            self.assertEqual("def-leaf", node.schema().name())
            self.assertEqual(1, node.dflt())

            node = root.new_path(ctx, "def-leaf", "def", 0, 0)
            self.assertIsNotNone(node)
            self.assertEqual("def-leaf", node.schema().name())
            self.assertEqual(0, node.dflt())

            node = root.new_path(ctx, "bubba", "b", 0, 0)
            self.assertIsNotNone(node)
            self.assertEqual("bubba", node.schema().name())

            node = root.new_path(ctx, "/a:x/number32", "3", 0, 0)
            self.assertIsNotNone(node)
            self.assertEqual("number32", node.schema().name())

            node = root.new_path(ctx, "/a:l[key1='1'][key2='2']/value", None, 0, 0)
            self.assertIsNotNone(node)
            self.assertEqual("l", node.schema().name())
            self.assertEqual("key1", node.child().schema().name())
            self.assertEqual("key2", node.child().next().schema().name())
            self.assertEqual("value", node.child().next().next().schema().name())

        except Exception as e:
            self.fail(e)

    def test_ly_data_node_insert(self):
        yang_folder = config.TESTS_DIR + "/api/files"
        config_file = config.TESTS_DIR + "/api/files/a.xml"

        try:
            # Setup
            ctx = ly.Context(yang_folder)
            self.assertIsNotNone(ctx)
            ctx.parse_module_mem(lys_module_a, ly.LYS_IN_YIN)
            root = ctx.parse_data_path(config_file, ly.LYD_XML, ly.LYD_OPT_CONFIG | ly.LYD_OPT_STRICT)
            self.assertIsNotNone(root)

            # Tests
            new_node = ly.Data_Node(root, root.schema().module(), "number32", "200")
            self.assertIsNotNone(new_node)
            rc = root.insert(new_node)
            self.assertEqual(0, rc)
            self.assertEqual("number32", root.child().prev().schema().name())


        except Exception as e:
            self.fail(e)

    def test_ly_data_node_insert_sibling(self):
        yang_folder = config.TESTS_DIR + "/api/files"
        config_file = config.TESTS_DIR + "/api/files/a.xml"

        try:
            # Setup
            ctx = ly.Context(yang_folder)
            self.assertIsNotNone(ctx)
            ctx.parse_module_mem(lys_module_a, ly.LYS_IN_YIN)
            root = ctx.parse_data_path(config_file, ly.LYD_XML, ly.LYD_OPT_CONFIG | ly.LYD_OPT_STRICT)
            self.assertIsNotNone(root)

            # Tests
            last = root.prev()
            new_node = ly.Data_Node(None, root.schema().module(), "y", "test")
            self.assertIsNotNone(new_node)
            rc = root.insert_sibling(new_node)
            self.assertEqual(0, rc)
            self.assertNotEqual(last.schema().name(), root.prev().schema().name())
            self.assertEqual("y", root.prev().schema().name())

        except Exception as e:
            self.fail(e)

    def test_ly_data_node_insert_before(self):
        yang_folder = config.TESTS_DIR + "/api/files"
        config_file = config.TESTS_DIR + "/api/files/a.xml"

        try:
            # Setup
            ctx = ly.Context(yang_folder)
            self.assertIsNotNone(ctx)
            ctx.parse_module_mem(lys_module_a, ly.LYS_IN_YIN)
            root = ctx.parse_data_path(config_file, ly.LYD_XML, ly.LYD_OPT_CONFIG | ly.LYD_OPT_STRICT)
            self.assertIsNotNone(root)

            # Tests
            last = root.prev()
            new_node = ly.Data_Node(None, root.schema().module(), "y", "test")
            self.assertIsNotNone(new_node)
            rc = root.insert_before(new_node)
            self.assertEqual(0, rc)
            self.assertNotEqual(last.schema().name(), root.prev().schema().name())
            self.assertEqual("y", root.prev().schema().name())

        except Exception as e:
            self.fail(e)

    def test_ly_data_node_insert_after(self):
        yang_folder = config.TESTS_DIR + "/api/files"
        config_file = config.TESTS_DIR + "/api/files/a.xml"

        try:
            # Setup
            ctx = ly.Context(yang_folder)
            self.assertIsNotNone(ctx)
            ctx.parse_module_mem(lys_module_a, ly.LYS_IN_YIN)
            root = ctx.parse_data_path(config_file, ly.LYD_XML, ly.LYD_OPT_CONFIG | ly.LYD_OPT_STRICT)
            self.assertIsNotNone(root)

            # Tests
            last = root.next()
            new_node = ly.Data_Node(None, root.schema().module(), "y", "test")
            self.assertIsNotNone(new_node)
            rc = root.insert_after(new_node)
            self.assertEqual(0, rc)
            self.assertNotEqual(last.schema().name(), root.next().schema().name())
            self.assertEqual("y", root.next().schema().name())

        except Exception as e:
            self.fail(e)

    def test_ly_data_node_schema_sort(self):
        yang_folder = config.TESTS_DIR + "/api/files"
        config_file = config.TESTS_DIR + "/api/files/a.xml"

        try:
            # Setup
            ctx = ly.Context(yang_folder)
            self.assertIsNotNone(ctx)
            ctx.parse_module_mem(lys_module_a, ly.LYS_IN_YIN)
            ctx.parse_data_path(config_file, ly.LYD_XML, ly.LYD_OPT_CONFIG | ly.LYD_OPT_STRICT)
            mod = ctx.get_module("a", None, 1)
            self.assertIsNotNone(mod)

            # Tests
            root = ly.Data_Node(None, mod, "l")
            self.assertIsNotNone(root)
            node = ly.Data_Node(root, mod, "key1", "1")
            self.assertIsNotNone(node)
            node = ly.Data_Node(root, mod, "key2", "2")
            self.assertIsNotNone(node)

            node = ly.Data_Node(None, mod, "x")
            self.assertIsNotNone(node)
            rc = root.insert_after(node)
            self.assertEqual(0, rc)
            node = root.next()

            node2 = ly.Data_Node(node, mod, "bubba", "a")
            self.assertIsNotNone(node2)
            node2 = ly.Data_Node(node, mod, "bar-gggg", "b")
            self.assertIsNotNone(node2)
            node2 = ly.Data_Node(node, mod, "number64", "64")
            self.assertIsNotNone(node2)
            node2 = ly.Data_Node(node, mod, "number32", "32")
            self.assertIsNotNone(node2)

            rc = root.schema_sort(1)
            self.assertEqual(0, rc)

            root = node
            self.assertEqual("x", root.schema().name())
            self.assertEqual("l", root.next().schema().name())

            self.assertEqual("bar-gggg", root.child().schema().name())
            self.assertEqual("bubba", root.child().next().schema().name())
            self.assertEqual("number32", root.child().next().next().schema().name())
            self.assertEqual("number64", root.child().next().next().next().schema().name())

        except Exception as e:
            self.fail(e)

    def test_ly_data_node_find_path(self):
        yang_folder = config.TESTS_DIR + "/api/files"
        config_file = config.TESTS_DIR + "/api/files/a.xml"
        try:
            # Setup
            ctx = ly.Context(yang_folder)
            self.assertIsNotNone(ctx)
            ctx.parse_module_mem(lys_module_a, ly.LYS_IN_YIN)
            root = ctx.parse_data_path(config_file, ly.LYD_XML, ly.LYD_OPT_CONFIG | ly.LYD_OPT_STRICT)
            self.assertIsNotNone(root)

            # Tests
            node = root.child()
            self.assertIsNotNone(node)
            set = node.find_path("/a:x/bubba")
            self.assertIsNotNone(set)
            self.assertEqual(1, set.number())

        except Exception as e:
            self.fail(e)

    def test_ly_data_node_find_instance(self):
        yang_folder = config.TESTS_DIR + "/api/files"
        config_file = config.TESTS_DIR + "/api/files/a.xml"
        try:
            # Setup
            ctx = ly.Context(yang_folder)
            self.assertIsNotNone(ctx)
            ctx.parse_module_mem(lys_module_a, ly.LYS_IN_YIN)
            root = ctx.parse_data_path(config_file, ly.LYD_XML, ly.LYD_OPT_CONFIG | ly.LYD_OPT_STRICT)
            self.assertIsNotNone(root)

            # Tests
            node = root.child()
            self.assertIsNotNone(node)
            set = node.find_instance(node.schema())
            self.assertIsNotNone(set)
            self.assertEqual(1, set.number())

        except Exception as e:
            self.fail(e)

    def test_ly_data_node_validate(self):
        yang_folder = config.TESTS_DIR + "/api/files"
        config_file = config.TESTS_DIR + "/api/files/a.xml"
        try:
            # Setup
            ctx = ly.Context(yang_folder)
            self.assertIsNotNone(ctx)
            ctx.parse_module_mem(lys_module_a, ly.LYS_IN_YIN)
            root = ctx.parse_data_path(config_file, ly.LYD_XML, ly.LYD_OPT_CONFIG | ly.LYD_OPT_STRICT)
            self.assertIsNotNone(root)

            # Tests
            rc = root.validate(ly.LYD_OPT_CONFIG, ctx)
            self.assertEqual(0, rc)
            new = ly.Data_Node(root, root.schema().module(), "number32", "1")
            self.assertIsNotNone(new)
            rc = root.insert(new)
            self.assertEqual(0, rc)
            rc = root.validate(ly.LYD_OPT_CONFIG, ctx)
            self.assertEqual(0, rc)

        except Exception as e:
            self.fail(e)

    def test_ly_data_node_unlink(self):
        yang_folder = config.TESTS_DIR + "/api/files"
        config_file = config.TESTS_DIR + "/api/files/a.xml"
        try:
            # Setup
            ctx = ly.Context(yang_folder)
            self.assertIsNotNone(ctx)
            ctx.parse_module_mem(lys_module_a, ly.LYS_IN_YIN)
            root = ctx.parse_data_path(config_file, ly.LYD_XML, ly.LYD_OPT_CONFIG | ly.LYD_OPT_STRICT)
            self.assertIsNotNone(root)

            # Tests
            node = root.child()
            new = ly.Data_Node(root, node.schema().module(), "number32", "1")
            self.assertIsNotNone(new)
            rc = root.insert(new)
            self.assertEqual(0, rc)

            schema = node.prev().schema()
            if (ly.LYS_LEAF == schema.nodetype() or ly.LYS_LEAFLIST == schema.nodetype()):
                casted = node.prev().subtype()
                self.assertEqual("1", casted.value_str())
            else:
                self.fail()

            rc = node.prev().unlink()
            self.assertEqual(0, rc)
            schema = node.prev().schema()
            if (ly.LYS_LEAF == schema.nodetype() or ly.LYS_LEAFLIST == schema.nodetype()):
                self.fail()
            else:
                return

        except Exception as e:
            self.fail(e)

    def test_ly_data_node_print_mem_xml(self):
        yang_folder = config.TESTS_DIR + "/api/files"
        config_file = config.TESTS_DIR + "/api/files/a.xml"
        try:
            # Setup
            ctx = ly.Context(yang_folder)
            self.assertIsNotNone(ctx)
            ctx.parse_module_mem(lys_module_a, ly.LYS_IN_YIN)
            root = ctx.parse_data_path(config_file, ly.LYD_XML, ly.LYD_OPT_CONFIG | ly.LYD_OPT_STRICT)
            self.assertIsNotNone(root)

            # Tests
            result = root.print_mem(ly.LYD_XML, 0)
            self.assertEqual(result_xml, result)

        except Exception as e:
            self.fail(e)

    def test_ly_data_node_print_mem_xml_format(self):
        yang_folder = config.TESTS_DIR + "/api/files"
        config_file = config.TESTS_DIR + "/api/files/a.xml"
        try:
            # Setup
            ctx = ly.Context(yang_folder)
            self.assertIsNotNone(ctx)
            ctx.parse_module_mem(lys_module_a, ly.LYS_IN_YIN)
            root = ctx.parse_data_path(config_file, ly.LYD_XML, ly.LYD_OPT_CONFIG | ly.LYD_OPT_STRICT)
            self.assertIsNotNone(root)

            # Tests
            result = root.print_mem(ly.LYD_XML, ly.LYP_FORMAT)
            self.assertEqual(result_xml_format, result)


        except Exception as e:
            self.fail(e)

    def test_ly_data_node_print_mem_json(self):
        yang_folder = config.TESTS_DIR + "/api/files"
        config_file = config.TESTS_DIR + "/api/files/a.xml"
        try:
            # Setup
            ctx = ly.Context(yang_folder)
            self.assertIsNotNone(ctx)
            ctx.parse_module_mem(lys_module_a, ly.LYS_IN_YIN)
            root = ctx.parse_data_path(config_file, ly.LYD_XML, ly.LYD_OPT_CONFIG | ly.LYD_OPT_STRICT)
            self.assertIsNotNone(root)

            # Tests
            result = root.print_mem(ly.LYD_JSON, ly.LYP_FORMAT)
            self.assertEqual(result_json, result)

        except Exception as e:
            self.fail(e)

    def test_ly_data_node_path(self):
        yang_folder = config.TESTS_DIR + "/api/files"
        config_file = config.TESTS_DIR + "/api/files/a.xml"
        try:
            # Setup
            ctx = ly.Context(yang_folder)
            self.assertIsNotNone(ctx)
            ctx.parse_module_mem(lys_module_a, ly.LYS_IN_YIN)
            root = ctx.parse_data_path(config_file, ly.LYD_XML, ly.LYD_OPT_CONFIG | ly.LYD_OPT_STRICT)
            self.assertIsNotNone(root)

            # Tests
            str = root.path()
            self.assertIsNotNone(str)
            self.assertEqual("/a:x", str)
            str = root.child().path()
            self.assertIsNotNone(str)
            self.assertEqual("/a:x/bubba", str)

        except Exception as e:
            self.fail(e)

    def test_ly_data_node_leaf(self):
        yang_folder = config.TESTS_DIR + "/api/files"
        config_file = config.TESTS_DIR + "/api/files/a.xml"

        try:
            ctx = ly.Context(yang_folder)
            self.assertIsNotNone(ctx)
            ctx.parse_module_mem(lys_module_a, ly.LYS_IN_YIN)
            root = ctx.parse_data_path(config_file, ly.LYD_XML, ly.LYD_OPT_CONFIG | ly.LYD_OPT_STRICT)
            self.assertIsNotNone(root)

            new_node = ly.Data_Node(root, root.schema().module(), "number32", "100")
            self.assertIsNotNone(new_node)

        except Exception as e:
            self.fail(e)

    def test_ly_data_node_anydata(self):
        yang_folder = config.TESTS_DIR + "/api/files"
        config_file = config.TESTS_DIR + "/api/files/a.xml"

        try:
            ctx = ly.Context(yang_folder)
            self.assertIsNotNone(ctx)
            ctx.parse_module_mem(lys_module_a, ly.LYS_IN_YIN)
            root = ctx.parse_data_path(config_file, ly.LYD_XML, ly.LYD_OPT_CONFIG | ly.LYD_OPT_STRICT)
            self.assertIsNotNone(root)
            mod = ctx.get_module("a", None, 1)

            new_node = ly.Data_Node(root, mod, "any-data", "100", ly.LYD_ANYDATA_CONSTSTRING)
            self.assertIsNotNone(new_node)

        except Exception as e:
            self.fail(e)

    def test_ly_data_node_dup(self):
        yang_folder = config.TESTS_DIR + "/api/files";
        config_file = config.TESTS_DIR + "/api/files/a.xml";

        try:
            ctx = ly.Context(yang_folder)
            self.assertIsNotNone(ctx)
            ctx.parse_module_mem(lys_module_a, ly.LYS_IN_YIN)
            root = ctx.parse_data_path(config_file, ly.LYD_XML, ly.LYD_OPT_CONFIG | ly.LYD_OPT_STRICT)
            self.assertIsNotNone(root)

            new_node = ly.Data_Node(root, root.child().schema().module(), "bar-y")
            self.assertIsNotNone(new_node)
            dup_node = new_node.dup(0);
            self.assertIsNotNone(dup_node)

        except Exception as e:
            self.fail(e)


    def test_ly_data_node_dup_to_ctx(self):
        sch = "module x {\
              namespace urn:x;\
              prefix x;\
              leaf x { type string; }}"
        data = "<x xmlns=\"urn:x\">hello</x>"

        try:
            ctx1 = ly.Context(None)
            self.assertIsNotNone(ctx1);
            ctx1.parse_module_mem(sch, ly.LYS_IN_YANG)
            data1 = ctx1.parse_data_mem(data, ly.LYD_XML, ly.LYD_OPT_CONFIG | ly.LYD_OPT_STRICT)
            self.assertIsNotNone(data1)

            ctx2 = ly.Context(None)
            self.assertIsNotNone(ctx2)
            # we expect NULL due to missing schema in the second ctx
            dup_node = data1.dup_to_ctx(1, ctx2)
            self.assertIsNone(dup_node)

            ctx2.parse_module_mem(sch, ly.LYS_IN_YANG)
            # now we expect success due to schema being added to the second ctx
            dup_node = data1.dup_to_ctx(1, ctx2)
            self.assertIsNotNone(dup_node)

        except Exception as e:
            self.fail(e)

    def test_ly_data_node_validate_node(self):
        yang_folder = config.TESTS_DIR + "/api/files";
        config_file = config.TESTS_DIR + "/api/files/a.xml";

        try:
            ctx = ly.Context(yang_folder)
            self.assertIsNotNone(ctx)
            ctx.parse_module_mem(lys_module_a, ly.LYS_IN_YIN)
            root = ctx.parse_data_path(config_file, ly.LYD_XML, ly.LYD_OPT_CONFIG | ly.LYD_OPT_STRICT)
            self.assertIsNotNone(root)

            rc = root.validate(ly.LYD_OPT_CONFIG, ctx)
            self.assertEqual(0, rc)
            new_node = ly.Data_Node(root, root.schema().module(), "number32", "1")
            self.assertIsNotNone(new_node)
            rc = root.validate(ly.LYD_OPT_CONFIG, new_node)
            self.assertEqual(0, rc)

        except Exception as e:
            self.fail(e)

    def test_ly_data_node_validate_value(self):
        yang_folder = config.TESTS_DIR + "/api/files";
        config_file = config.TESTS_DIR + "/api/files/a.xml";

        try:
            ctx = ly.Context(yang_folder)
            self.assertIsNotNone(ctx)
            ctx.parse_module_mem(lys_module_a, ly.LYS_IN_YIN)
            root = ctx.parse_data_path(config_file, ly.LYD_XML, ly.LYD_OPT_CONFIG | ly.LYD_OPT_STRICT)
            self.assertIsNotNone(root)

            rc = root.validate(ly.LYD_OPT_CONFIG, ctx)
            self.assertEqual(0, rc)
            new_node = ly.Data_Node(root, root.schema().module(), "number32", "1")
            self.assertIsNotNone(new_node)
            self.assertEqual(0, new_node.validate_value("1"))
            self.assertEqual(0, new_node.validate_value("100"))
            self.assertEqual(0, new_node.validate_value("110000000"))

        except Exception as e:
            self.fail(e)

if __name__ == '__main__':
    unittest.main()
