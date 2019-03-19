import org.cesnet.*;
import org.cesnet.Module;
import static org.cesnet.LYS_INFORMAT.LYS_IN_YANG;
import static org.cesnet.LYS_INFORMAT.LYS_IN_YIN;
import static org.cesnet.LYS_OUTFORMAT.LYS_OUT_TREE;
import static org.cesnet.LYS_OUTFORMAT.LYS_OUT_YANG;
import static org.cesnet.LYS_OUTFORMAT.LYS_OUT_YIN;

import java.io.FileDescriptor;
import java.io.RandomAccessFile;
import java.lang.reflect.Field;
import org.junit.Test;
import static org.junit.Assert.*;

public class TreeSchemaTest {

    static {
        System.loadLibrary("yangJava");
    }

    String lys_module_a = "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n" +
            "\n" +
            "<module xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\" xmlns:a=\"urn:a\" name=\"a\">  \n" +
            "  <namespace uri=\"urn:a\"/>  \n" +
            "  <prefix value=\"a_mod\"/>  \n" +
            "  <include module=\"asub\"/>  \n" +
            "  <include module=\"atop\"/>  \n" +
            "  <revision date=\"2015-01-01\"> \n" +
            "    <description> \n" +
            "      <text>version 1</text> \n" +
            "    </description>  \n" +
            "    <reference> \n" +
            "      <text>RFC XXXX</text> \n" +
            "    </reference> \n" +
            "  </revision>  \n" +
            "  <feature name=\"foo\"/>  \n" +
            "  <grouping name=\"gg\"> \n" +
            "    <leaf name=\"bar-gggg\"> \n" +
            "      <type name=\"string\"/> \n" +
            "    </leaf> \n" +
            "  </grouping>  \n" +
            "  <container name=\"x\"> \n" +
            "    <leaf name=\"bar-leaf\"> \n" +
            "      <if-feature name=\"bar\"/>  \n" +
            "      <type name=\"string\"/> \n" +
            "    </leaf>  \n" +
            "    <uses name=\"gg\"> \n" +
            "      <if-feature name=\"bar\"/> \n" +
            "    </uses>  \n" +
            "    <leaf name=\"baz\"> \n" +
            "      <if-feature name=\"foo\"/>  \n" +
            "      <type name=\"string\"/> \n" +
            "    </leaf>  \n" +
            "    <leaf name=\"bubba\"> \n" +
            "      <type name=\"string\"/> \n" +
            "    </leaf> \n" +
            "  </container>  \n" +
            "  <augment target-node=\"/x\"> \n" +
            "    <if-feature name=\"bar\"/>  \n" +
            "    <container name=\"bar-y\"> \n" +
            "      <leaf name=\"ll\"> \n" +
            "        <type name=\"string\"/> \n" +
            "      </leaf> \n" +
            "    </container> \n" +
            "  </augment>  \n" +
            "  <rpc name=\"bar-rpc\"> \n" +
            "    <if-feature name=\"bar\"/> \n" +
            "  </rpc>  \n" +
            "  <rpc name=\"foo-rpc\"> \n" +
            "    <if-feature name=\"foo\"/> \n" +
            "  </rpc> \n" +
            "</module>\n";

    String lys_module_b = "module b {\n" +
            "    namespace \"urn:b\";\n" +
            "    prefix b_mod;\n" +
            "    include bsub;\n" +
            "    include btop;\n" +
            "    feature foo;\n" +
            "    grouping gg {\n" +
            "        leaf bar-gggg {\n" +
            "            type string;\n" +
            "        }\n" +
            "    }\n" +
            "    container x {\n" +
            "        leaf bar-leaf {\n" +
            "            if-feature \"bar\";\n" +
            "            type string;\n" +
            "        }\n" +
            "        uses gg {\n" +
            "            if-feature \"bar\";\n" +
            "        }\n" +
            "        leaf baz {\n" +
            "            if-feature \"foo\";\n" +
            "            type string;\n" +
            "        }\n" +
            "        leaf bubba {\n" +
            "            type string;\n" +
            "        }\n" +
            "    }\n" +
            "    augment \"/x\" {\n" +
            "            if-feature \"bar\";\n" +
            "            container bar-y;\n" +
            "    }\n" +
            "    rpc bar-rpc {\n" +
            "        if-feature \"bar\";\n" +
            "    }\n" +
            "    rpc foo-rpc {\n" +
            "        if-feature \"foo\";\n" +
            "    }\n" +
            "}";

    String lys_module_a_with_typo = "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n" +
            "<module_typo xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\" xmlns:a=\"urn:a\" name=\"a\">  \n" +
            "  <namespace uri=\"urn:a\"/>  \n" +
            "  <prefix value=\"a_mod\"/>  \n" +
            "  <include module=\"asub\"/>  \n" +
            "  <include module=\"atop\"/>  \n" +
            "  <feature name=\"foo\"/>  \n" +
            "  <grouping name=\"gg\"> \n" +
            "    <leaf name=\"bar-gggg\"> \n" +
            "      <type name=\"string\"/> \n" +
            "    </leaf> \n" +
            "  </grouping>  \n" +
            "  <container name=\"x\"> \n" +
            "    <leaf name=\"bar-leaf\"> \n" +
            "      <if-feature name=\"bar\"/>  \n" +
            "      <type name=\"string\"/> \n" +
            "    </leaf>  \n" +
            "    <uses name=\"gg\"> \n" +
            "      <if-feature name=\"bar\"/> \n" +
            "    </uses>  \n" +
            "    <leaf name=\"baz\"> \n" +
            "      <if-feature name=\"foo\"/>  \n" +
            "      <type name=\"string\"/> \n" +
            "    </leaf>  \n" +
            "    <leaf name=\"bubba\"> \n" +
            "      <type name=\"string\"/> \n" +
            "    </leaf> \n" +
            "  </container>  \n" +
            "  <augment target-node=\"/x\"> \n" +
            "    <if-feature name=\"bar\"/>  \n" +
            "    <container name=\"bar-y\"> \n" +
            "      <leaf name=\"ll\"> \n" +
            "        <type name=\"string\"/> \n" +
            "      </leaf> \n" +
            "    </container> \n" +
            "  </augment>  \n" +
            "  <rpc name=\"bar-rpc\"> \n" +
            "    <if-feature name=\"bar\"/> \n" +
            "  </rpc>  \n" +
            "  <rpc name=\"foo-rpc\"> \n" +
            "    <if-feature name=\"foo\"/> \n" +
            "  </rpc> \n" +
            "</module>\n";

    String result_tree = "module: a\n" +
            "  +--rw top\n" +
            "  |  +--rw bar-sub2\n" +
            "  +--rw x\n" +
            "     +--rw bubba?      string\n";

    String result_yang = "module a {\n" +
            "  namespace \"urn:a\";\n" +
            "  prefix a_mod;\n" +
            "\n" +
            "  include \"asub\";\n" +
            "\n" +
            "  include \"atop\";\n" +
            "\n" +
            "  revision 2015-01-01 {\n" +
            "    description\n" +
            "      \"version 1\";\n" +
            "    reference\n" +
            "      \"RFC XXXX\";\n" +
            "  }\n" +
            "\n" +
            "  feature foo;\n" +
            "\n" +
            "  grouping gg {\n" +
            "    leaf bar-gggg {\n" +
            "      type string;\n" +
            "    }\n" +
            "  }\n" +
            "\n" +
            "  container x {\n" +
            "    leaf bar-leaf {\n" +
            "      if-feature \"bar\";\n" +
            "      type string;\n" +
            "    }\n" +
            "\n" +
            "    uses gg {\n" +
            "      if-feature \"bar\";\n" +
            "    }\n" +
            "\n" +
            "    leaf baz {\n" +
            "      if-feature \"foo\";\n" +
            "      type string;\n" +
            "    }\n" +
            "\n" +
            "    leaf bubba {\n" +
            "      type string;\n" +
            "    }\n" +
            "  }\n" +
            "\n" +
            "  augment \"/x\" {\n" +
            "    if-feature \"bar\";\n" +
            "    container bar-y {\n" +
            "      leaf ll {\n" +
            "        type string;\n" +
            "      }\n" +
            "    }\n" +
            "  }\n" +
            "\n" +
            "  rpc bar-rpc {\n" +
            "    if-feature \"bar\";\n" +
            "  }\n" +
            "\n" +
            "  rpc foo-rpc {\n" +
            "    if-feature \"foo\";\n" +
            "  }\n" +
            "}\n";

    String result_yin = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n" +
            "<module name=\"a\"\n" +
            "        xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\"\n" +
            "        xmlns:a_mod=\"urn:a\">\n" +
            "  <namespace uri=\"urn:a\"/>\n" +
            "  <prefix value=\"a_mod\"/>\n" +
            "  <include module=\"asub\"/>\n" +
            "  <include module=\"atop\"/>\n" +
            "  <revision date=\"2015-01-01\">\n" +
            "    <description>\n" +
            "      <text>version 1</text>\n" +
            "    </description>\n" +
            "    <reference>\n" +
            "      <text>RFC XXXX</text>\n" +
            "    </reference>\n" +
            "  </revision>\n" +
            "  <feature name=\"foo\"/>\n" +
            "  <grouping name=\"gg\">\n" +
            "    <leaf name=\"bar-gggg\">\n" +
            "      <type name=\"string\"/>\n" +
            "    </leaf>\n" +
            "  </grouping>\n" +
            "  <container name=\"x\">\n" +
            "    <leaf name=\"bar-leaf\">\n" +
            "      <if-feature name=\"bar\"/>\n" +
            "      <type name=\"string\"/>\n" +
            "    </leaf>\n" +
            "    <uses name=\"gg\">\n" +
            "      <if-feature name=\"bar\"/>\n" +
            "    </uses>\n" +
            "    <leaf name=\"baz\">\n" +
            "      <if-feature name=\"foo\"/>\n" +
            "      <type name=\"string\"/>\n" +
            "    </leaf>\n" +
            "    <leaf name=\"bubba\">\n" +
            "      <type name=\"string\"/>\n" +
            "    </leaf>\n" +
            "  </container>\n" +
            "  <augment target-node=\"/x\">\n" +
            "    <if-feature name=\"bar\"/>\n" +
            "    <container name=\"bar-y\">\n" +
            "      <leaf name=\"ll\">\n" +
            "        <type name=\"string\"/>\n" +
            "      </leaf>\n" +
            "    </container>\n" +
            "  </augment>\n" +
            "  <rpc name=\"bar-rpc\">\n" +
            "    <if-feature name=\"bar\"/>\n" +
            "  </rpc>\n" +
            "  <rpc name=\"foo-rpc\">\n" +
            "    <if-feature name=\"foo\"/>\n" +
            "  </rpc>\n" +
            "</module>\n";

    @Test
    public void test_ly_ctx_parse_module_mem() {
        String yang_folder = Constants.TESTS_DIR + "/api/files";

        try {
            Context ctx = new Context(yang_folder);
            assertNotNull(ctx);

            Module module = ctx.parse_module_mem(lys_module_a, LYS_IN_YIN);
            assertNotNull(module);
            assertEquals("a", module.name());

            module = ctx.parse_module_mem(lys_module_b, LYS_IN_YANG);
            assertNotNull(module);
            assertEquals("b", module.name());
        } catch (Exception e) {
            fail(e.getMessage());
        }
    }

    @Test
    public void test_ly_ctx_parse_module_mem_invalid() {
        String yang_folder = Constants.TESTS_DIR + "/api/files";

        try {
            Context ctx = new Context(yang_folder);
            assertNotNull(ctx);

            ctx.parse_module_mem(lys_module_a_with_typo, LYS_IN_YIN);
            throw new Exception("exception not thrown");
        } catch (Exception e) {
            assertEquals("Module parsing failed.", e.getMessage());
        }
    }

    @Test
    public void test_ly_ctx_parse_module_fd() {
        String yang_folder = Constants.TESTS_DIR + "/api/files";
        String yin_file = Constants.TESTS_DIR + "/api/files/a.yin";
        String yang_file = Constants.TESTS_DIR + "/api/files/b.yang";

        try {
            Context ctx = new Context(yang_folder);
            assertNotNull(ctx);

            RandomAccessFile f = new RandomAccessFile(yin_file, "r");
            FileDescriptor fileno = f.getFD();
            Field _fileno = FileDescriptor.class.getDeclaredField("fd");
            _fileno.setAccessible(true);
            Integer fd = (Integer) _fileno.get(fileno);


            Module module = ctx.parse_module_fd(fd, LYS_IN_YIN);
            assertNotNull(module);
            assertEquals("a", module.name());

            f.close();

            f = new RandomAccessFile(yang_file, "r");
            fileno = f.getFD();
            _fileno = FileDescriptor.class.getDeclaredField("fd");
            _fileno.setAccessible(true);
            fd = (Integer) _fileno.get(fileno);


            module = ctx.parse_module_fd(fd, LYS_IN_YANG);
            assertNotNull(module);
            assertEquals("b", module.name());
            f.close();
        } catch (Exception e) {
            fail(e.getMessage());
        }
    }

    @Test
    public void test_ly_ctx_parse_module_fd_invalid() {
        String yang_folder = Constants.TESTS_DIR + "/api/files";
        String yin_file = Constants.TESTS_DIR + "/api/files/a.yin";

        try {
            Context ctx = new Context(yang_folder);
            assertNotNull(ctx);

            RandomAccessFile f = new RandomAccessFile(yin_file, "r");
            FileDescriptor fileno = f.getFD();
            Field _fileno = FileDescriptor.class.getDeclaredField("fd");
            _fileno.setAccessible(true);
            Integer fd = (Integer) _fileno.get(fileno);


            Module module = ctx.parse_module_fd(fd, LYS_IN_YANG);

            throw new Exception("exception not thrown");
        } catch (Exception e) {
            assertEquals("Module parsing failed.", e.getMessage());
        }
    }

    @Test
    public void test_ly_ctx_parse_module_path() {
        String yang_folder = Constants.TESTS_DIR + "/api/files";
        String yin_file = Constants.TESTS_DIR + "/api/files/a.yin";
        String yang_file = Constants.TESTS_DIR + "/api/files/b.yang";

        try {
            Context ctx = new Context(yang_folder);
            assertNotNull(ctx);

            Module module = ctx.parse_module_path(yin_file, LYS_IN_YIN);
            assertNotNull(module);
            assertEquals("a", module.name());

            module = ctx.parse_module_path(yang_file, LYS_IN_YANG);
            assertNotNull(module);
            assertEquals("b", module.name());
        } catch (Exception e) {
            fail(e.getMessage());
        }
    }

    @Test
    public void test_ly_ctx_parse_module_path_invalid() {
        String yang_folder = Constants.TESTS_DIR + "/api/files";
        String yin_file = Constants.TESTS_DIR + "/api/files/a.yin";

        try {
            Context ctx = new Context(yang_folder);
            assertNotNull(ctx);

            Module module = ctx.parse_module_path(yin_file, LYS_IN_YANG);
            throw new Exception("exception not thrown");
        } catch (Exception e) {
            assertEquals("Module parsing failed.", e.getMessage());
        }
    }

    @Test
    public void test_ly_module_print_mem_tree() {
        String yang_folder = Constants.TESTS_DIR + "/api/files";

        try {
            Context ctx = new Context(yang_folder);
            assertNotNull(ctx);
            Module module = ctx.parse_module_mem(lys_module_a, LYS_IN_YIN);
            assertNotNull(module);

            String result = module.print_mem(LYS_OUT_TREE, 0);
            assertEquals(result_tree, result);
        } catch (Exception e) {
            fail(e.getMessage());
        }
    }

    @Test
    public void test_ly_module_print_mem_yang() {
        String yang_folder = Constants.TESTS_DIR + "/api/files";

        try {
            Context ctx = new Context(yang_folder);
            assertNotNull(ctx);
            Module module = ctx.parse_module_mem(lys_module_a, LYS_IN_YIN);
            assertNotNull(module);

            String result = module.print_mem(LYS_OUT_YANG, 0);
            assertEquals(result_yang, result);
        } catch (Exception e) {
            fail(e.getMessage());
        }
    }

    @Test
    public void test_ly_module_print_mem_yin() {
        String yang_folder = Constants.TESTS_DIR + "/api/files";

        try {
            Context ctx = new Context(yang_folder);
            assertNotNull(ctx);
            Module module = ctx.parse_module_mem(lys_module_a, LYS_IN_YIN);
            assertNotNull(module);

            String result = module.print_mem(LYS_OUT_YIN, 0);
            assertEquals(result_yin, result);
        } catch (Exception e) {
            fail(e.getMessage());
        }
    }

    @Test
    public void test_ly_schema_node_find_path() {
        String yang_folder = Constants.TESTS_DIR + "/api/files";

        try {
            Context ctx = new Context(yang_folder);
            assertNotNull(ctx);
            Module module = ctx.parse_module_mem(lys_module_a, LYS_IN_YIN);
            assertNotNull(module);
            Schema_Node schema_node = module.data();
            assertNotNull(schema_node);

            Set set = schema_node.find_path("/a:x/*");
            assertNotNull(set);
            assertEquals(5, set.number());
            set = schema_node.find_path("/a:x//*");
            assertNotNull(set);
            assertEquals(6, set.number());
            set = schema_node.find_path("/a:x//.");
            assertNotNull(set);
            assertEquals(7, set.number());
        } catch (Exception e) {
            fail(e.getMessage());
        }
    }

    @Test
    public void test_ly_schema_node_path() {
        String yang_folder = Constants.TESTS_DIR + "/api/files";

        try {
            Context ctx = new Context(yang_folder);
            assertNotNull(ctx);
            Module module = ctx.parse_module_mem(lys_module_a, LYS_IN_YIN);
            assertNotNull(module);
            Schema_Node schema_node = module.data();
            assertNotNull(schema_node);

            String path_template = "/a:x/a:bar-gggg";
            Set set = schema_node.find_path(path_template);
            assertNotNull(set);

            vectorSchema_Node schemas = set.schema();
            Schema_Node schema = schemas.get(0);
            String path = schema.path(0);
            assertEquals(path_template, path);
        } catch (Exception e) {
            fail(e.getMessage());
        }
    }

    public static void main(String[] args) {
        new TreeSchemaTest();
    }
}
