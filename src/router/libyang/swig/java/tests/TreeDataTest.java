import org.cesnet.*;
import org.cesnet.Module;
import static org.cesnet.LYD_ANYDATA_VALUETYPE.LYD_ANYDATA_CONSTSTRING;
import static org.cesnet.LYD_FORMAT.LYD_JSON;
import static org.cesnet.LYD_FORMAT.LYD_XML;
import static org.cesnet.LYS_INFORMAT.LYS_IN_YIN;
import static org.cesnet.yangConstants.*;

import java.io.FileDescriptor;
import java.io.RandomAccessFile;
import java.lang.reflect.Field;
import org.junit.Test;
import static org.junit.Assert.*;

public class TreeDataTest {

    static {
        System.loadLibrary("yangJava");
    }

    String lys_module_a = "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n" +
            "<module xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\" xmlns:md=\"urn:ietf:params:xml:ns:yang:ietf-yang-metadata\" xmlns:a=\"urn:a\" name=\"a\">  \n" +
            "  <namespace uri=\"urn:a\"/>  \n" +
            "  <prefix value=\"a_mod\"/>  \n" +
            "  <include module=\"asub\"/>  \n" +
            "  <include module=\"atop\"/>  \n" +
            "  <import module=\"ietf-yang-metadata\"> \n" +
            "    <prefix value=\"md\"/> \n" +
            "  </import>  \n" +
            "  <feature name=\"foo\"/>  \n" +
            "  <grouping name=\"gg\"> \n" +
            "    <leaf name=\"bar-gggg\"> \n" +
            "      <type name=\"string\"/> \n" +
            "    </leaf> \n" +
            "  </grouping>  \n" +
            "  <md:annotation name=\"test\"> \n" +
            "    <type name=\"string\"/> \n" +
            "  </md:annotation>  \n" +
            "  <container name=\"x\"> \n" +
            "    <leaf name=\"bar-leaf\"> \n" +
            "      <type name=\"string\"/> \n" +
            "    </leaf>  \n" +
            "    <uses name=\"gg\"></uses>  \n" +
            "    <leaf name=\"baz\"> \n" +
            "      <type name=\"string\"/> \n" +
            "    </leaf>  \n" +
            "    <leaf name=\"bubba\"> \n" +
            "      <type name=\"string\"/> \n" +
            "    </leaf>  \n" +
            "    <leaf name=\"number32\"> \n" +
            "      <type name=\"int32\"/> \n" +
            "    </leaf>  \n" +
            "    <leaf name=\"number64\"> \n" +
            "      <type name=\"int64\"/> \n" +
            "    </leaf>  \n" +
            "    <leaf name=\"def-leaf\"> \n" +
            "      <type name=\"string\"/>  \n" +
            "      <default value=\"def\"/> \n" +
            "    </leaf> \n" +
            "  </container>  \n" +
            "  <leaf name=\"y\">\n" +
            "    <type name=\"string\"/>\n" +
            "  </leaf>  \n" +
            "  <anyxml name=\"any\"/>  \n" +
            "  <augment target-node=\"/x\"> \n" +
            "    <container name=\"bar-y\"/> \n" +
            "  </augment>  \n" +
            "  <rpc name=\"bar-rpc\"></rpc>  \n" +
            "  <rpc name=\"foo-rpc\"></rpc>  \n" +
            "  <rpc name=\"rpc1\"> \n" +
            "    <input> \n" +
            "      <leaf name=\"input-leaf1\"> \n" +
            "        <type name=\"string\"/> \n" +
            "      </leaf>  \n" +
            "      <container name=\"x\"> \n" +
            "        <leaf name=\"input-leaf2\"> \n" +
            "          <type name=\"string\"/> \n" +
            "        </leaf> \n" +
            "      </container> \n" +
            "    </input>  \n" +
            "    <output> \n" +
            "      <leaf name=\"output-leaf1\"> \n" +
            "        <type name=\"string\"/> \n" +
            "      </leaf>  \n" +
            "      <leaf name=\"output-leaf2\"> \n" +
            "        <type name=\"string\"/> \n" +
            "      </leaf>  \n" +
            "      <container name=\"rpc-container\"> \n" +
            "        <leaf name=\"output-leaf3\"> \n" +
            "          <type name=\"string\"/> \n" +
            "        </leaf> \n" +
            "      </container> \n" +
            "    </output> \n" +
            "  </rpc>  \n" +
            "  <list name=\"l\"> \n" +
            "    <key value=\"key1 key2\"/>  \n" +
            "    <leaf name=\"key1\"> \n" +
            "      <type name=\"uint8\"/> \n" +
            "    </leaf>  \n" +
            "    <leaf name=\"key2\"> \n" +
            "      <type name=\"uint8\"/> \n" +
            "    </leaf>  \n" +
            "    <leaf name=\"value\"> \n" +
            "      <type name=\"string\"/> \n" +
            "    </leaf> \n" +
            "  </list> \n" +
            "</module>";

    String result_xml = "<x xmlns=\"urn:a\"><bubba>test</bubba></x>";
    String result_xml_format =
            "<x xmlns=\"urn:a\">\n" +
                    "  <bubba>test</bubba>\n" +
                    "</x>\n";
    String result_json = "{\n" +
            "  \"a:x\": {\n" +
            "    \"bubba\": \"test\"\n" +
            "  }\n" +
            "}\n";

    @Test
    public void test_ly_ctx_parse_data_mem() {
        String a_data_xml = "<x xmlns=\"urn:a\">\n" +
                "<bubba>test</bubba>\n" +
                "</x>";

        String yang_folder = Constants.TESTS_DIR + "/api/files";
        String yin_file = Constants.TESTS_DIR + "/api/files/a.yin";

        try {
            Context ctx = new Context(yang_folder);
            assertNotNull(ctx);
            ctx.parse_module_path(yin_file, LYS_IN_YIN);

            Data_Node root = ctx.parse_data_mem(a_data_xml, LYD_XML, LYD_OPT_NOSIBLINGS | LYD_OPT_STRICT);
            assertNotNull(root);
            assertEquals("x", root.schema().name());
        } catch (Exception e) {
            fail(e.getMessage());
        }
    }

    @Test
    public void test_ly_ctx_parse_data_fd() {
        String yang_folder = Constants.TESTS_DIR + "/api/files";
        String yin_file = Constants.TESTS_DIR + "/api/files/a.yin";
        String config_file = Constants.TESTS_DIR + "/api/files/a.xml";

        try {
            Context ctx = new Context(yang_folder);
            assertNotNull(ctx);
            ctx.parse_module_path(yin_file, LYS_IN_YIN);
            RandomAccessFile f = new RandomAccessFile(config_file, "r");
            FileDescriptor fileno = f.getFD();
            Field _fileno = FileDescriptor.class.getDeclaredField("fd");
            _fileno.setAccessible(true);
            Integer fd = (Integer) _fileno.get(fileno);

            Data_Node root = ctx.parse_data_fd(fd, LYD_XML, LYD_OPT_NOSIBLINGS | LYD_OPT_STRICT);
            assertNotNull(root);
            assertEquals("x", root.schema().name());
            f.close();
        } catch (Exception e) {
            fail(e.getMessage());
        }
    }

    @Test
    public void test_ly_ctx_parse_data_path() {
        String yang_folder = Constants.TESTS_DIR + "/api/files";
        String yin_file = Constants.TESTS_DIR + "/api/files/a.yin";
        String config_file = Constants.TESTS_DIR + "/api/files/a.xml";
        String module_name = "a";
        String schema_name = "x";

        try {
            Context ctx = new Context(yang_folder);
            assertNotNull(ctx);
            Module module = ctx.parse_module_path(yin_file, LYS_IN_YIN);
            assertNotNull(module);
            assertEquals(module_name, module.name());

            Data_Node root = ctx.parse_data_path(config_file, LYD_XML, LYD_OPT_CONFIG | LYD_OPT_STRICT);
            assertNotNull(root);
            assertEquals(schema_name, root.schema().name());
        } catch (Exception e) {
            fail(e.getMessage());
        }
    }

    @Test
    public void test_ly_ctx_parse_data_path_invalid() {
        String yang_folder = Constants.TESTS_DIR + "/api/files";

        try {
            Context ctx = new Context(yang_folder);
            assertNotNull(ctx);

            Data_Node root = ctx.parse_data_path("INVALID_PATH", LYD_XML, LYD_OPT_CONFIG | LYD_OPT_STRICT);
            throw new Exception("exception not thrown");
        } catch (Exception e) {
            assertTrue(e.getMessage().contains("INVALID_PATH"));
        }
    }

    @Test
    public void test_ly_data_node() {
        String yang_folder = Constants.TESTS_DIR + "/api/files";
        String config_file = Constants.TESTS_DIR + "/api/files/a.xml";

        try {
            Context ctx = new Context(yang_folder);
            assertNotNull(ctx);
            ctx.parse_module_mem(lys_module_a, LYS_IN_YIN);
            Data_Node root = ctx.parse_data_path(config_file, LYD_XML, LYD_OPT_CONFIG | LYD_OPT_STRICT);
            assertNotNull(root);

            Data_Node new_node = new Data_Node(root, root.child().schema().module(), "bar-y");
            assertNotNull(new_node);
            new_node = new Data_Node(root, root.schema().module(), "number32", "100");
            assertNotNull(new_node);
            Data_Node dup_node = new_node.dup(0);
            assertNotNull(dup_node);
        } catch (Exception e) {
            fail(e.getMessage());
        }
    }

    @Test
    public void test_ly_data_node_new_path() {
        String yang_folder = Constants.TESTS_DIR + "/api/files";
        String config_file = Constants.TESTS_DIR + "/api/files/a.xml";

        try {
            Context ctx = new Context(yang_folder);
            assertNotNull(ctx);
            ctx.parse_module_mem(lys_module_a, LYS_IN_YIN);
            ctx.parse_data_path(config_file, LYD_XML, LYD_OPT_CONFIG | LYD_OPT_STRICT);
            Module mod = ctx.get_module("a", null, 1);
            assertNotNull(mod);

            Data_Node root = new Data_Node(ctx, "/a:x/bar-gggg", "a", LYD_ANYDATA_CONSTSTRING, 0);
            assertNotNull(root);
            assertEquals("x", root.schema().name());
            assertEquals("bar-gggg", root.child().schema().name());

            Data_Node node = root.new_path(ctx, "def-leaf", "def", LYD_ANYDATA_CONSTSTRING, LYD_PATH_OPT_DFLT);
            assertNotNull(node);
            assertEquals("def-leaf", node.schema().name());
            assertEquals(1, node.dflt());

            node = root.new_path(ctx, "def-leaf", "def", LYD_ANYDATA_CONSTSTRING, 0);
            assertNotNull(node);
            assertEquals("def-leaf", node.schema().name());
            assertEquals(0, node.dflt());

            node = root.new_path(ctx, "bubba", "b", LYD_ANYDATA_CONSTSTRING, 0);
            assertNotNull(node);
            assertEquals("bubba", node.schema().name());

            node = root.new_path(ctx, "/a:x/number32", "3", LYD_ANYDATA_CONSTSTRING, 0);
            assertNotNull(node);
            assertEquals("number32", node.schema().name());

            node = root.new_path(ctx, "/a:l[key1='1'][key2='2']/value", null, LYD_ANYDATA_CONSTSTRING, 0);
            assertNotNull(node);
            assertEquals("l", node.schema().name());
            assertEquals("key1", node.child().schema().name());
            assertEquals("key2", node.child().next().schema().name());
            assertEquals("value", node.child().next().next().schema().name());
        } catch (Exception e) {
            fail(e.getMessage());
        }
    }

    @Test
    public void test_ly_data_node_insert() {
        String yang_folder = Constants.TESTS_DIR + "/api/files";
        String config_file = Constants.TESTS_DIR + "/api/files/a.xml";

        try {
            Context ctx = new Context(yang_folder);
            assertNotNull(ctx);
            ctx.parse_module_mem(lys_module_a, LYS_IN_YIN);
            Data_Node root = ctx.parse_data_path(config_file, LYD_XML, LYD_OPT_CONFIG | LYD_OPT_STRICT);
            assertNotNull(root);
            Data_Node new_node = new Data_Node(root, root.schema().module(), "number32", "200");
            assertNotNull(new_node);
            int rc = root.insert(new_node);
            assertEquals(0, rc);
            assertEquals("number32", root.child().prev().schema().name());
        } catch (Exception e) {
            fail(e.getMessage());
        }
    }

    @Test
    public void test_ly_data_node_insert_sibling() {
        String yang_folder = Constants.TESTS_DIR + "/api/files";
        String config_file = Constants.TESTS_DIR + "/api/files/a.xml";

        try {
            Context ctx = new Context(yang_folder);
            assertNotNull(ctx);
            ctx.parse_module_mem(lys_module_a, LYS_IN_YIN);
            Data_Node root = ctx.parse_data_path(config_file, LYD_XML, LYD_OPT_CONFIG | LYD_OPT_STRICT);
            assertNotNull(root);

            Data_Node last = root.prev();
            Data_Node new_node = new Data_Node(null, root.schema().module(), "y", "test");
            assertNotNull(new_node);
            int rc = root.insert_sibling(new_node);
            assertEquals(0, rc);
            assertNotEquals(last.schema().name(), root.prev().schema().name());
            assertEquals("y", root.prev().schema().name());
        } catch (Exception e) {
            fail(e.getMessage());
        }
    }

    @Test
    public void test_ly_data_node_insert_before() {
        String yang_folder = Constants.TESTS_DIR + "/api/files";
        String config_file = Constants.TESTS_DIR + "/api/files/a.xml";

        try {
            Context ctx = new Context(yang_folder);
            assertNotNull(ctx);
            ctx.parse_module_mem(lys_module_a, LYS_IN_YIN);
            Data_Node root = ctx.parse_data_path(config_file, LYD_XML, LYD_OPT_CONFIG | LYD_OPT_STRICT);
            assertNotNull(root);

            Data_Node last = root.prev();
            Data_Node new_node = new Data_Node(null, root.schema().module(), "y", "test");
            assertNotNull(new_node);
            int rc = root.insert_before(new_node);
            assertEquals(0, rc);
            assertNotEquals(last.schema().name(), root.prev().schema().name());
            assertEquals("y", root.prev().schema().name());
        } catch (Exception e) {
            fail(e.getMessage());
        }
    }

    @Test
    public void test_ly_data_node_insert_after() {
        String yang_folder = Constants.TESTS_DIR + "/api/files";
        String config_file = Constants.TESTS_DIR + "/api/files/a.xml";

        try {
            Context ctx = new Context(yang_folder);
            assertNotNull(ctx);
            ctx.parse_module_mem(lys_module_a, LYS_IN_YIN);
            Data_Node root = ctx.parse_data_path(config_file, LYD_XML, LYD_OPT_CONFIG | LYD_OPT_STRICT);
            assertNotNull(root);

            Data_Node last = root.next();
            Data_Node new_node = new Data_Node(null, root.schema().module(), "y", "test");
            assertNotNull(new_node);
            int rc = root.insert_after(new_node);
            assertEquals(0, rc);
            assertNotEquals(last.schema().name(), root.next().schema().name());
            assertEquals("y", root.next().schema().name());
        } catch (Exception e) {
            fail(e.getMessage());
        }
    }

    @Test
    public void test_ly_data_node_schema_sort() {
        String yang_folder = Constants.TESTS_DIR + "/api/files";
        String config_file = Constants.TESTS_DIR + "/api/files/a.xml";

        try {
            Context ctx = new Context(yang_folder);
            assertNotNull(ctx);
            ctx.parse_module_mem(lys_module_a, LYS_IN_YIN);
            ctx.parse_data_path(config_file, LYD_XML, LYD_OPT_CONFIG | LYD_OPT_STRICT);
            Module mod = ctx.get_module("a", null, 1);
            assertNotNull(mod);

            Data_Node root = new Data_Node(null, mod, "l");
            assertNotNull(root);
            Data_Node node = new Data_Node(root, mod, "key1", "1");
            assertNotNull(node);
            node = new Data_Node(root, mod, "key2", "2");
            assertNotNull(node);

            node = new Data_Node(null, mod, "x");
            assertNotNull(node);
            int rc = root.insert_after(node);
            assertEquals(0, rc);
            node = root.next();

            Data_Node node2 = new Data_Node(node, mod, "bubba", "a");
            assertNotNull(node2);
            node2 = new Data_Node(node, mod, "bar-gggg", "b");
            assertNotNull(node2);
            node2 = new Data_Node(node, mod, "number64", "64");
            assertNotNull(node2);
            node2 = new Data_Node(node, mod, "number32", "32");
            assertNotNull(node2);

            rc = root.schema_sort(1);
            assertEquals(0, rc);

            root = node;
            assertEquals("x", root.schema().name());
            assertEquals("l", root.next().schema().name());

            assertEquals("bar-gggg", root.child().schema().name());
            assertEquals("bubba", root.child().next().schema().name());
            assertEquals("number32", root.child().next().next().schema().name());
            assertEquals("number64", root.child().next().next().next().schema().name());
        } catch (Exception e) {
            fail(e.getMessage());
        }
    }

    @Test
    public void test_ly_data_node_find_path() {
        String yang_folder = Constants.TESTS_DIR + "/api/files";
        String config_file = Constants.TESTS_DIR + "/api/files/a.xml";

        try {
            Context ctx = new Context(yang_folder);
            assertNotNull(ctx);
            ctx.parse_module_mem(lys_module_a, LYS_IN_YIN);
            Data_Node root = ctx.parse_data_path(config_file, LYD_XML, LYD_OPT_CONFIG | LYD_OPT_STRICT);
            assertNotNull(root);

            Data_Node node = root.child();
            assertNotNull(node);
            Set set = node.find_path("/a:x/bubba");
            assertNotNull(set);
            assertEquals(1, set.number());
        } catch (Exception e) {
            fail(e.getMessage());
        }
    }

    @Test
    public void test_ly_data_node_find_instance() {
        String yang_folder = Constants.TESTS_DIR + "/api/files";
        String config_file = Constants.TESTS_DIR + "/api/files/a.xml";

        try {
            Context ctx = new Context(yang_folder);
            assertNotNull(ctx);
            ctx.parse_module_mem(lys_module_a, LYS_IN_YIN);
            Data_Node root = ctx.parse_data_path(config_file, LYD_XML, LYD_OPT_CONFIG | LYD_OPT_STRICT);
            assertNotNull(root);

            Data_Node node = root.child();
            assertNotNull(node);
            Set set = node.find_instance(node.schema());
            assertNotNull(set);
            assertEquals(1, set.number());
        } catch (Exception e) {
            fail(e.getMessage());
        }
    }

    @Test
    public void test_ly_data_node_validate() {
        String yang_folder = Constants.TESTS_DIR + "/api/files";
        String config_file = Constants.TESTS_DIR + "/api/files/a.xml";

        try {
            Context ctx = new Context(yang_folder);
            assertNotNull(ctx);
            ctx.parse_module_mem(lys_module_a, LYS_IN_YIN);
            Data_Node root = ctx.parse_data_path(config_file, LYD_XML, LYD_OPT_CONFIG | LYD_OPT_STRICT);
            assertNotNull(root);

            int rc = root.validate(LYD_OPT_CONFIG, ctx);
            assertEquals(0, rc);
            Data_Node new_node = new Data_Node(root, root.schema().module(), "number32", "1");
            assertNotNull(new_node);
            rc = root.insert(new_node);
            assertEquals(0, rc);
            rc = root.validate(LYD_OPT_CONFIG, ctx);
            assertEquals(0, rc);
        } catch (Exception e) {
            fail(e.getMessage());
        }
    }

    @Test
    public void test_ly_data_node_unlink() {
        String yang_folder = Constants.TESTS_DIR + "/api/files";
        String config_file = Constants.TESTS_DIR + "/api/files/a.xml";

        try {
            Context ctx = new Context(yang_folder);
            assertNotNull(ctx);
            ctx.parse_module_mem(lys_module_a, LYS_IN_YIN);
            Data_Node root = ctx.parse_data_path(config_file, LYD_XML, LYD_OPT_CONFIG | LYD_OPT_STRICT);
            assertNotNull(root);

            Data_Node node = root.child();
            Data_Node new_node = new Data_Node(root, root.schema().module(), "number32", "1");
            assertNotNull(new_node);
            int rc = root.insert(new_node);
            assertEquals(0, rc);

            assertEquals("number32", node.prev().schema().name());

            rc = node.prev().unlink();
            assertEquals(0, rc);

            assertNotEquals("number32", node.prev().schema().name());
        } catch (Exception e) {
            fail(e.getMessage());
        }
    }

    @Test
    public void test_ly_data_node_print_mem_xml() {
        String yang_folder = Constants.TESTS_DIR + "/api/files";
        String config_file = Constants.TESTS_DIR + "/api/files/a.xml";

        try {
            Context ctx = new Context(yang_folder);
            assertNotNull(ctx);
            ctx.parse_module_mem(lys_module_a, LYS_IN_YIN);
            Data_Node root = ctx.parse_data_path(config_file, LYD_XML, LYD_OPT_CONFIG | LYD_OPT_STRICT);
            assertNotNull(root);

            String result = root.print_mem(LYD_XML, 0);
            assertEquals(result_xml, result);
        } catch (Exception e) {
            fail(e.getMessage());
        }
    }

    @Test
    public void test_ly_data_node_print_mem_xml_format() {
        String yang_folder = Constants.TESTS_DIR + "/api/files";
        String config_file = Constants.TESTS_DIR + "/api/files/a.xml";

        try {
            Context ctx = new Context(yang_folder);
            assertNotNull(ctx);
            ctx.parse_module_mem(lys_module_a, LYS_IN_YIN);
            Data_Node root = ctx.parse_data_path(config_file, LYD_XML, LYD_OPT_CONFIG | LYD_OPT_STRICT);
            assertNotNull(root);

            String result = root.print_mem(LYD_XML, LYP_FORMAT);
            assertEquals(result_xml_format, result);
        } catch (Exception e) {
            fail(e.getMessage());
        }
    }

    @Test
    public void test_ly_data_node_print_mem_json() {
        String yang_folder = Constants.TESTS_DIR + "/api/files";
        String config_file = Constants.TESTS_DIR + "/api/files/a.xml";

        try {
            Context ctx = new Context(yang_folder);
            assertNotNull(ctx);
            ctx.parse_module_mem(lys_module_a, LYS_IN_YIN);
            Data_Node root = ctx.parse_data_path(config_file, LYD_XML, LYD_OPT_CONFIG | LYD_OPT_STRICT);
            assertNotNull(root);

            String result = root.print_mem(LYD_JSON, LYP_FORMAT);
            assertEquals(result_json, result);
        } catch (Exception e) {
            fail(e.getMessage());
        }
    }

    @Test
    public void test_ly_data_node_path() {
        String yang_folder = Constants.TESTS_DIR + "/api/files";
        String config_file = Constants.TESTS_DIR + "/api/files/a.xml";

        try {
            Context ctx = new Context(yang_folder);
            assertNotNull(ctx);
            ctx.parse_module_mem(lys_module_a, LYS_IN_YIN);
            Data_Node root = ctx.parse_data_path(config_file, LYD_XML, LYD_OPT_CONFIG | LYD_OPT_STRICT);
            assertNotNull(root);

            String path = root.path();
            assertNotNull(path);
            assertEquals("/a:x", path);
            path = root.child().path();
            assertNotNull(path);
            assertEquals("/a:x/bubba", path);
        } catch (Exception e) {
            fail(e.getMessage());
        }
    }

    public static void main(String[] args) {
        new TreeDataTest();
    }

}
