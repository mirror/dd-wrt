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
        xmlns:a=\"urn:a\">                            \
  <namespace uri=\"urn:a\"/>                          \
  <prefix value=\"a_mod\"/>                           \
  <include module=\"asub\"/>                          \
  <include module=\"atop\"/>                          \
  <revision date=\"2015-01-01\">                      \
    <description>                                     \
      <text>version 1</text>                          \
    </description>                                    \
    <reference>                                       \
      <text>RFC XXXX</text>                           \
    </reference>                                      \
  </revision>                                         \
  <feature name=\"foo\"/>                             \
  <grouping name=\"gg\">                              \
    <leaf name=\"bar-gggg\">                          \
      <type name=\"string\"/>                         \
    </leaf>                                           \
  </grouping>                                         \
  <container name=\"x\">                              \
    <leaf name=\"bar-leaf\">                          \
      <if-feature name=\"bar\"/>                      \
      <type name=\"string\"/>                         \
    </leaf>                                           \
    <uses name=\"gg\">                                \
      <if-feature name=\"bar\"/>                      \
    </uses>                                           \
    <leaf name=\"baz\">                               \
      <if-feature name=\"foo\"/>                      \
      <type name=\"string\"/>                         \
    </leaf>                                           \
    <leaf name=\"bubba\">                             \
      <type name=\"string\"/>                         \
    </leaf>                                           \
  </container>                                        \
  <augment target-node=\"/x\">                        \
    <if-feature name=\"bar\"/>                        \
    <container name=\"bar-y\">                        \
      <leaf name=\"ll\">                              \
        <type name=\"string\"/>                       \
      </leaf>                                         \
    </container>                                      \
  </augment>                                          \
  <rpc name=\"bar-rpc\">                              \
    <if-feature name=\"bar\"/>                        \
  </rpc>                                              \
  <rpc name=\"foo-rpc\">                              \
    <if-feature name=\"foo\"/>                        \
  </rpc>                                              \
</module>                                             \
"

lys_module_b = \
"module b {\
    namespace \"urn:b\";\
    prefix b_mod;\
    include bsub;\
    include btop;\
    feature foo;\
    grouping gg {\
        leaf bar-gggg {\
            type string;\
        }\
    }\
    container x {\
        leaf bar-leaf {\
            if-feature \"bar\";\
            type string;\
        }\
        uses gg {\
            if-feature \"bar\";\
        }\
        leaf baz {\
            if-feature \"foo\";\
            type string;\
        }\
        leaf bubba {\
            type string;\
        }\
    }\
    augment \"/x\" {\
        if-feature \"bar\";\
        container bar-y;\
    }\
    rpc bar-rpc {\
        if-feature \"bar\";\
    }\
    rpc foo-rpc {\
        if-feature \"foo\";\
    }\
}"

lys_module_a_with_typo = \
"<?xml version=\"1.0\" encoding=\"UTF-8\"?>           \
<module_typo name=\"a\"                                    \
        xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\"   \
        xmlns:a=\"urn:a\">                            \
  <namespace uri=\"urn:a\"/>                          \
  <prefix value=\"a_mod\"/>                           \
  <include module=\"asub\"/>                          \
  <include module=\"atop\"/>                          \
  <feature name=\"foo\"/>                             \
  <grouping name=\"gg\">                              \
    <leaf name=\"bar-gggg\">                          \
      <type name=\"string\"/>                         \
    </leaf>                                           \
  </grouping>                                         \
  <container name=\"x\">                              \
    <leaf name=\"bar-leaf\">                          \
      <if-feature name=\"bar\"/>                      \
      <type name=\"string\"/>                         \
    </leaf>                                           \
    <uses name=\"gg\">                                \
      <if-feature name=\"bar\"/>                      \
    </uses>                                           \
    <leaf name=\"baz\">                               \
      <if-feature name=\"foo\"/>                      \
      <type name=\"string\"/>                         \
    </leaf>                                           \
    <leaf name=\"bubba\">                             \
      <type name=\"string\"/>                         \
    </leaf>                                           \
  </container>                                        \
  <augment target-node=\"/x\">                        \
    <if-feature name=\"bar\"/>                        \
    <container name=\"bar-y\">                        \
      <leaf name=\"ll\">                              \
        <type name=\"string\"/>                       \
      </leaf>                                         \
    </container>                                      \
  </augment>                                          \
  <rpc name=\"bar-rpc\">                              \
    <if-feature name=\"bar\"/>                        \
  </rpc>                                              \
  <rpc name=\"foo-rpc\">                              \
    <if-feature name=\"foo\"/>                        \
  </rpc>                                              \
</module>                                             \
"

result_tree = "\
module: a\n\
  +--rw top\n\
  |  +--rw bar-sub2\n\
  +--rw x\n\
     +--rw bubba?      string\n"

result_yang = "\
module a {\n\
  namespace \"urn:a\";\n\
  prefix a_mod;\n\
\n\
  include \"asub\";\n\
\n\
  include \"atop\";\n\
\n\
  revision 2015-01-01 {\n\
    description\n\
      \"version 1\";\n\
    reference\n\
      \"RFC XXXX\";\n\
  }\n\
\n\
  feature foo;\n\
\n\
  grouping gg {\n\
    leaf bar-gggg {\n\
      type string;\n\
    }\n\
  }\n\
\n\
  container x {\n\
    leaf bar-leaf {\n\
      if-feature \"bar\";\n\
      type string;\n\
    }\n\n\
    uses gg {\n\
      if-feature \"bar\";\n\
    }\n\n\
    leaf baz {\n\
      if-feature \"foo\";\n\
      type string;\n\
    }\n\n\
    leaf bubba {\n\
      type string;\n\
    }\n\
  }\n\
\n\
  augment \"/x\" {\n\
    if-feature \"bar\";\n\
    container bar-y {\n\
      leaf ll {\n\
        type string;\n\
      }\n\
    }\n\
  }\n\
\n\
  rpc bar-rpc {\n\
    if-feature \"bar\";\n\
  }\n\
\n\
  rpc foo-rpc {\n\
    if-feature \"foo\";\n\
  }\n\
}\n"

result_yin = "\
<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n\
<module name=\"a\"\n\
        xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\"\n\
        xmlns:a_mod=\"urn:a\">\n\
  <namespace uri=\"urn:a\"/>\n\
  <prefix value=\"a_mod\"/>\n\
  <include module=\"asub\"/>\n\
  <include module=\"atop\"/>\n\
  <revision date=\"2015-01-01\">\n\
    <description>\n\
      <text>version 1</text>\n\
    </description>\n\
    <reference>\n\
      <text>RFC XXXX</text>\n\
    </reference>\n\
  </revision>\n\
  <feature name=\"foo\"/>\n\
  <grouping name=\"gg\">\n\
    <leaf name=\"bar-gggg\">\n\
      <type name=\"string\"/>\n\
    </leaf>\n\
  </grouping>\n\
  <container name=\"x\">\n\
    <leaf name=\"bar-leaf\">\n\
      <if-feature name=\"bar\"/>\n\
      <type name=\"string\"/>\n\
    </leaf>\n\
    <uses name=\"gg\">\n\
      <if-feature name=\"bar\"/>\n\
    </uses>\n\
    <leaf name=\"baz\">\n\
      <if-feature name=\"foo\"/>\n\
      <type name=\"string\"/>\n\
    </leaf>\n\
    <leaf name=\"bubba\">\n\
      <type name=\"string\"/>\n\
    </leaf>\n\
  </container>\n\
  <augment target-node=\"/x\">\n\
    <if-feature name=\"bar\"/>\n\
    <container name=\"bar-y\">\n\
      <leaf name=\"ll\">\n\
        <type name=\"string\"/>\n\
      </leaf>\n\
    </container>\n\
  </augment>\n\
  <rpc name=\"bar-rpc\">\n\
    <if-feature name=\"bar\"/>\n\
  </rpc>\n\
  <rpc name=\"foo-rpc\">\n\
    <if-feature name=\"foo\"/>\n\
  </rpc>\n\
</module>\n"

result_info ="\
Feature:   foo\n\
Module:    a\n\
Desc:      \n\
Reference: \n\
Status:    current\n\
Enabled:   no\n\
If-feats:  \n"

class UnexpectedError(Exception):
    """Exception raised for errors that are not expected.

    Attributes:
        message -- explanation of the error
    """

    def __init__(self, message):
        self.message = message

class TestUM(unittest.TestCase):
    def test_ly_ctx_parse_module_mem(self):
        yang_folder = config.TESTS_DIR + "/api/files"

        try:
            # Setup
            ctx = ly.Context(yang_folder)
            self.assertIsNotNone(ctx)

            # Tests
            module = ctx.parse_module_mem(lys_module_a, ly.LYS_IN_YIN)
            self.assertIsNotNone(module)
            self.assertEqual("a", module.name())

            module = ctx.parse_module_mem(lys_module_b, ly.LYS_IN_YANG)
            self.assertIsNotNone(module)
            self.assertEqual("b", module.name())

        except Exception as e:
            self.fail(e)

    def test_ly_ctx_parse_module_mem_invalid(self):
        yang_folder = config.TESTS_DIR + "/api/files"

        try:
            # Setup
            ctx = ly.Context(yang_folder)
            self.assertIsNotNone(ctx)

            # Tests
            ctx.parse_module_mem(lys_module_a_with_typo, ly.LYS_IN_YIN)
            raise UnexpectedError("Exception not thrown")

        except UnexpectedError as e:
            self.fail(e)

        except RuntimeError as e:
            return

        except Exception as e:
            self.fail(e)

    def test_ly_ctx_parse_module_fd(self):
        yang_folder = config.TESTS_DIR + "/api/files"
        yin_file = config.TESTS_DIR + "/api/files/a.yin"
        yang_file = config.TESTS_DIR + "/api/files/b.yang"

        try:
            # Setup
            ctx = ly.Context(yang_folder)
            self.assertIsNotNone(ctx)

            # Tests
            f = open(yin_file, 'r')
            fd = f.fileno()
            module = ctx.parse_module_fd(fd, ly.LYS_IN_YIN)
            self.assertIsNotNone(module)
            self.assertEqual("a", module.name())

            f.close()
            f = open(yang_file, 'r')
            fd = f.fileno()
            module = ctx.parse_module_fd(fd, ly.LYS_IN_YANG)
            self.assertIsNotNone(module)
            self.assertEqual("b", module.name())

        except Exception as e:
            self.fail(e)

        finally:
            f.close()

    def test_ly_ctx_parse_module_fd_invalid(self):
        yang_folder = config.TESTS_DIR + "/api/files"
        yin_file = config.TESTS_DIR + "/api/files/a.yin"

        try:
            # Setup
            ctx = ly.Context(yang_folder)
            self.assertIsNotNone(ctx)

            # Tests
            f = open(yin_file, 'r')
            fd = f.fileno()
            # parsing with wrong format should raise runtime exception
            module = ctx.parse_module_fd(fd, ly.LYS_IN_YANG)
            raise UnexpectedError("Exception not thrown")

        except UnexpectedError as e:
            self.fail(e)

        except RuntimeError as e:
            return

        except Exception as e:
            self.fail(e)

        finally:
            f.close()

    def test_ly_ctx_parse_module_path(self):
        yang_folder = config.TESTS_DIR + "/api/files"
        yin_file = config.TESTS_DIR + "/api/files/a.yin"
        yang_file = config.TESTS_DIR + "/api/files/b.yang"

        try:
            # Setup
            ctx = ly.Context(yang_folder)
            self.assertIsNotNone(ctx)

            # Tests
            module = ctx.parse_module_path(yin_file, ly.LYS_IN_YIN)
            self.assertIsNotNone(module)
            self.assertEqual("a", module.name())

            module = ctx.parse_module_path(yang_file, ly.LYS_IN_YANG)
            self.assertIsNotNone(module)
            self.assertEqual("b", module.name())

        except Exception as e:
            self.fail(e)

    def test_ly_ctx_parse_module_path_invalid(self):
        yang_folder = config.TESTS_DIR + "/api/files"
        yin_file = config.TESTS_DIR + "/api/files/a.yin"

        try:
            # Setup
            ctx = ly.Context(yang_folder)
            self.assertIsNotNone(ctx)

            # Tests
            # parsing with wrong format should raise runtime exception
            module = ctx.parse_module_path(yin_file, ly.LYS_IN_YANG)
            raise UnexpectedError("Exception not thrown")

        except UnexpectedError as e:
            self.fail(e)

        except RuntimeError as e:
            return

        except Exception as e:
            self.fail(e)

    def test_ly_module_print_mem_tree(self):
        yang_folder = config.TESTS_DIR + "/api/files"

        try:
            # Setup
            ctx = ly.Context(yang_folder)
            self.assertIsNotNone(ctx)
            module = ctx.parse_module_mem(lys_module_a, ly.LYS_IN_YIN)
            self.assertIsNotNone(module)

            # Tests
            result = module.print_mem(ly.LYS_OUT_TREE, 0)
            self.assertEqual(result_tree, result)

        except Exception as e:
            self.fail(e)

    def test_ly_module_print_mem_yang(self):
        yang_folder = config.TESTS_DIR + "/api/files"

        try:
            # Setup
            ctx = ly.Context(yang_folder)
            self.assertIsNotNone(ctx)
            module = ctx.parse_module_mem(lys_module_a, ly.LYS_IN_YIN)
            self.assertIsNotNone(module)

            # Tests
            result = module.print_mem(ly.LYS_OUT_YANG, 0)
            self.assertEqual(result_yang, result)

        except Exception as e:
            self.fail(e)

    def test_ly_module_print_mem_yin(self):
        yang_folder = config.TESTS_DIR + "/api/files"

        try:
            # Setup
            ctx = ly.Context(yang_folder)
            self.assertIsNotNone(ctx)
            module = ctx.parse_module_mem(lys_module_a, ly.LYS_IN_YIN)
            self.assertIsNotNone(module)

            # Tests
            result = module.print_mem(ly.LYS_OUT_YIN, 0)
            self.assertEqual(result_yin, result)

        except Exception as e:
            self.fail(e)

    def test_ly_schema_node_find_path(self):
        yang_folder = config.TESTS_DIR + "/api/files"

        try:
            # Setup
            ctx = ly.Context(yang_folder)
            self.assertIsNotNone(ctx)
            module = ctx.parse_module_mem(lys_module_a, ly.LYS_IN_YIN)
            self.assertIsNotNone(module)
            schema_node = module.data()
            self.assertIsNotNone(schema_node)

            # Tests
            set = schema_node.find_path("/a:x/*")
            self.assertIsNotNone(set)
            self.assertEqual(5, set.number())
            set = schema_node.find_path("/a:x//*")
            self.assertIsNotNone(set)
            self.assertEqual(6, set.number())
            set = schema_node.find_path("/a:x//.")
            self.assertIsNotNone(set)
            self.assertEqual(7, set.number())

        except Exception as e:
            self.fail(e)

    def test_ly_schema_node_path(self):
        yang_folder = config.TESTS_DIR + "/api/files"

        try:
            # Setup
            ctx = ly.Context(yang_folder)
            self.assertIsNotNone(ctx)
            module = ctx.parse_module_mem(lys_module_a, ly.LYS_IN_YIN)
            self.assertIsNotNone(module)
            schema_node = module.data()
            self.assertIsNotNone(schema_node)

            # Tests
            template = "/a:x/a:bar-gggg"
            set = schema_node.find_path(template)
            self.assertIsNotNone(set)
            schema = set.schema()[0]
            path = schema.path(0)
            self.assertEqual(template, path)

        except Exception as e:
            self.fail(e)

    def test_ly_module_data_instatiables(self):
        yang_folder = config.TESTS_DIR + "/api/files"
        module_name = "b"

        try:
            ctx = ly.Context(yang_folder)
            self.assertIsNotNone(ctx)

            module = ctx.load_module(module_name)
            self.assertIsNotNone(module)
            self.assertEqual(module_name, module.name())

            instantiables = module.data_instantiables(0)
            self.assertIsNotNone(instantiables)
            self.assertEqual(1, len(instantiables))

        except Exception as e:
            self.fail(e)

    def test_ly_schema_child_instatiables(self):
        yang_folder = config.TESTS_DIR + "/api/files"
        module_name = "b"

        try:
            ctx = ly.Context(yang_folder)
            self.assertIsNotNone(ctx)

            module = ctx.load_module(module_name)
            self.assertIsNotNone(module)
            self.assertEqual(module_name, module.name())

            instantiables = module.data_instantiables(0)
            self.assertIsNotNone(instantiables)
            self.assertEqual(1, len(instantiables))
            child_instantiables = instantiables[0].child_instantiables(0)
            self.assertIsNotNone(child_instantiables)
            self.assertEqual(3, len(child_instantiables))

        except Exception as e:
            self.fail(e)

if __name__ == '__main__':
    unittest.main()
