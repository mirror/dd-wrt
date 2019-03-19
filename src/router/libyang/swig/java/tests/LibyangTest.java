import org.cesnet.*;
import org.cesnet.Module;
import static org.cesnet.LYD_FORMAT.LYD_XML;
import static org.cesnet.LYS_INFORMAT.LYS_IN_YANG;
import static org.cesnet.LYS_INFORMAT.LYS_IN_YIN;
import static org.cesnet.yangConstants.LYD_OPT_CONFIG;
import static org.cesnet.yangConstants.LYD_OPT_STRICT;
import static org.cesnet.yangConstants.LYD_VAL_OK;

import org.junit.Test;
import static org.junit.Assert.*;

public class LibyangTest {

    static {
        System.loadLibrary("yangJava");
    }

    @Test
    public void test_ly_ctx_new(){
        String yang_folder1 = Constants.TESTS_DIR + "/data/files";
        String yang_folder2 = Constants.TESTS_DIR + "/data:" + Constants.TESTS_DIR + "/data/files";

        try {
            Context ctx = new Context(yang_folder1);
            assertNotNull(ctx);
            vector_String list = ctx.get_searchdirs();
            assertEquals(1, list.size());

            ctx = new Context(yang_folder2);
            assertNotNull(ctx);
            list = ctx.get_searchdirs();
            assertEquals(2, list.size());
        } catch(Exception e) {
            fail(e.getMessage());
        }
    }

    @Test
    public void test_ly_ctx_new_invalid(){
        String yang_folder = "INVALID_PATH";

        try {
            Context ctx = new Context(yang_folder);
            throw new Exception("exception not thrown");
        } catch(Exception e) {
            assertTrue(e.getMessage().contains("No Context"));
        }
    }

    @Test
    public void test_ly_ctx_get_searchdirs() {
        String yang_folder = Constants.TESTS_DIR + "/data/files";

        try {
            Context ctx = new Context(yang_folder);
            assertNotNull(ctx);

            vector_String list = ctx.get_searchdirs();
            assertEquals(1, list.size());
            assertEquals(yang_folder, list.get(0));
        } catch(Exception e) {
            fail(e.getMessage());
        }
    }

    @Test
    public void  test_ly_ctx_set_searchdir() {
        String yang_folder = Constants.TESTS_DIR + "/data/files";
        String new_yang_folder = Constants.TESTS_DIR + "/schema/yin";

        try {
            Context ctx = new Context(yang_folder);
            assertNotNull(ctx);

            vector_String list = ctx.get_searchdirs();
            assertEquals(1, list.size());
            assertEquals(yang_folder, list.get(0));

            ctx.set_searchdir(new_yang_folder);
            list = ctx.get_searchdirs();
            assertEquals(2, list.size());
            assertEquals(new_yang_folder, list.get(1));

            ctx.unset_searchdirs(0);
            list = ctx.get_searchdirs();
            assertEquals(1, list.size());
            assertEquals(new_yang_folder, list.get(0));
        } catch(Exception e ) {
            fail(e.getMessage());
        }
    }

    @Test
    public void test_ly_ctx_set_searchdir_invalid() {
        String yang_folder = Constants.TESTS_DIR + "/data/files";
        String new_yang_folder = Constants.TESTS_DIR + "INVALID_PATH";

        try {
            Context ctx = new Context(yang_folder);
            assertNotNull(ctx);

            ctx.set_searchdir(new_yang_folder);
            throw new Exception("exception not thrown");
        } catch(Exception e) {
            assertTrue(e.getMessage().contains(new_yang_folder));
        }
    }

    @Test
    public void test_ly_ctx_info(){
        String yang_folder = Constants.TESTS_DIR + "/api/files";
        try {
            Context ctx = new Context(yang_folder);
            assertNotNull(ctx);

            Data_Node info = ctx.info();
            assertNotNull(info);
            assertEquals(LYD_VAL_OK, info.validity());
        } catch(Exception e ) {
            fail(e.getMessage());
        }
    }

    @Test
    public void test_ly_ctx_load_module_invalid() {
        String yang_folder = Constants.TESTS_DIR + "/api/files";
        try {
            Context ctx = new Context(yang_folder);
            assertNotNull(ctx);

            Module module = ctx.load_module("invalid", null);
            throw new Exception("exception not thrown");
        } catch(Exception e ) {
            assertTrue(e.getMessage().contains("invalid"));
        }
    }

    @Test
    public void test_ly_ctx_load_get_module()
    {
        String yang_folder = Constants.TESTS_DIR + "/api/files";
        String name1 = "a";
        String name2 = "b";
        String revision = "2016-03-01";

        try {
            Context ctx = new Context(yang_folder);
            assertNotNull(ctx);

            Module module = ctx.get_module("invalid");
            assertNull(module);

            module = ctx.get_module(name1);
            assertNull(module);

            module = ctx.load_module(name1);
            assertNotNull(module);
            assertEquals(name1, module.name());

            module = ctx.load_module(name2, revision);
            assertNotNull(module);
            assertEquals(name2, module.name());
            assertEquals(revision, module.rev().date());

            module = ctx.get_module(name2, "INVALID_REVISION");
            assertNull(module);

            module = ctx.get_module(name1);
            assertNotNull(module);
            assertEquals(name1, module.name());

            module = ctx.get_module(name2, revision);
            assertNotNull(module);
            assertEquals(name2, module.name());
            assertEquals(revision, module.rev().date());
        } catch(Exception e) {
            fail(e.getMessage());
        }
    }

    @Test
    public void test_ly_ctx_get_module_older()
    {
        String yang_folder = Constants.TESTS_DIR + "/api/files";
        String name = "b";
        String revision = "2016-03-01";
        String revision_older = "2015-01-01";

        try {
            Context ctx = new Context(yang_folder);
            assertNotNull(ctx);

            Module module = ctx.load_module("c");
            assertNotNull(module);
            assertEquals("c", module.name());

            module = ctx.load_module(name, revision);
            assertNotNull(module);
            assertEquals(name, module.name());
            assertEquals(revision, module.rev().date());

            Module module_older = ctx.get_module_older(module);
            assertNotNull(module_older);
            assertEquals(name, module_older.name());
            assertEquals(revision_older, module_older.rev().date());
        } catch(Exception e) {
            fail(e.getMessage());
        }
    }

    @Test
    public void test_ly_ctx_get_module_by_ns()
    {
        String yang_folder = Constants.TESTS_DIR + "/api/files";
        String module_name = "a";
        String ns = "urn:a";

        try {
            Context ctx = new Context(yang_folder);
            assertNotNull(ctx);

            Module module = ctx.load_module(module_name);
            assertNotNull(module);
            assertEquals(module_name, module.name());

            module = ctx.get_module_by_ns(ns);
            assertNotNull(module);
            assertEquals(module_name, module.name());
        } catch(Exception e) {
            fail(e.getMessage());
        }
    }

    @Test
    public void test_ly_ctx_clean()
    {
        String yang_folder = Constants.TESTS_DIR + "/api/files";
        String module_name = "a";

        try {
            Context ctx = new Context(yang_folder);
            assertNotNull(ctx);

            Module module = ctx.load_module(module_name);
            assertNotNull(module);
            assertEquals(module_name, module.name());

            module = ctx.get_module(module_name);
            assertNotNull(module);
            assertEquals(module_name, module.name());

            ctx.clean();

            module = ctx.get_module(module_name);
            assertNull(module);
        } catch(Exception e) {
            fail(e.getMessage());
        }
    }

    @Test
    public void test_ly_ctx_parse_module_path()
    {
        String yang_folder = Constants.TESTS_DIR + "/api/files";
        String yin_file = Constants.TESTS_DIR + "/api/files/a.yin";
        String yang_file = Constants.TESTS_DIR + "/api/files/b.yang";
        String module_name1 = "a";
        String module_name2 = "b";

        try {
            Context ctx = new Context(yang_folder);
            assertNotNull(ctx);

            Module module = ctx.parse_module_path(yin_file, LYS_IN_YIN);
            assertNotNull(module);
            assertEquals(module_name1, module.name());

            module = ctx.parse_module_path(yang_file, LYS_IN_YANG);
            assertNotNull(module);
            assertEquals(module_name2, module.name());
        } catch(Exception e) {
            fail(e.getMessage());
        }
    }

    @Test
    public void test_ly_ctx_parse_module_path_invalid()
    {
        String yang_folder = Constants.TESTS_DIR + "/api/files";

        try {
            Context ctx = new Context(yang_folder);
            assertNotNull(ctx);

            Module module = ctx.parse_module_path("INVALID_YANG_FILE", LYS_IN_YANG);
            throw new Exception("exception not thrown");
        } catch(Exception e) {
            assertTrue(e.getMessage().contains("INVALID_YANG_FILE"));
        }
    }

    @Test
    public void test_ly_ctx_get_submodule()
    {
        String yang_folder = Constants.TESTS_DIR + "/api/files";
        String yin_file = Constants.TESTS_DIR + "/api/files/a.yin";
        String module_name = "a";
        String sub_name = "asub";

        try {
            Context ctx = new Context(yang_folder);
            assertNotNull(ctx);
            ctx.parse_module_path(yin_file, LYS_IN_YIN);

            Submodule submodule = ctx.get_submodule(module_name, null, sub_name, null);
            assertNotNull(submodule);
            assertEquals(sub_name, submodule.name());
        } catch(Exception e) {
            fail(e.getMessage());
        }
    }

    @Test
    public void test_ly_ctx_get_submodule2()
    {
        String yang_folder = Constants.TESTS_DIR + "/api/files";
        String yin_file = Constants.TESTS_DIR + "/api/files/a.yin";
        String config_file = Constants.TESTS_DIR + "/api/files/a.xml";
        String sub_name = "asub";

        try {
            Context ctx = new Context(yang_folder);
            assertNotNull(ctx);
            ctx.parse_module_path(yin_file, LYS_IN_YIN);

            Data_Node root = ctx.parse_data_path(config_file, LYD_XML, LYD_OPT_CONFIG | LYD_OPT_STRICT);
            assertNotNull(root);
            assertNotNull(root.schema().module());

            Submodule submodule = ctx.get_submodule2(root.schema().module(), sub_name);
            assertNotNull(submodule);
            assertEquals(sub_name, submodule.name());
        } catch(Exception e) {
            fail(e.getMessage());
        return;
    }
    }

    @Test
    public void test_ly_ctx_find_path()
    {
        String yang_folder = Constants.TESTS_DIR + "/api/files";
        String yin_file = Constants.TESTS_DIR + "/api/files/a.yin";
        String yang_file = Constants.TESTS_DIR + "/api/files/b.yang";
        String schema_path1 = "/b:x/b:bubba";
        String schema_path2 = "/a:x/a:bubba";

        try {
            Context ctx = new Context(yang_folder);
            assertNotNull(ctx);

            ctx.parse_module_path(yang_file, LYS_IN_YANG);
            Set set = ctx.find_path(schema_path1);
            assertNotNull(set);

            ctx.parse_module_path(yin_file, LYS_IN_YIN);
            set = ctx.find_path(schema_path2);
            assertNotNull(set);
            new Set();
        } catch(Exception e) {
            fail(e.getMessage());
        }
    }

    @Test
    public void test_ly_set() {
        String yang_folder = Constants.TESTS_DIR + "/api/files";
        String yin_file = Constants.TESTS_DIR + "/api/files/a.yin";
        String config_file = Constants.TESTS_DIR + "/api/files/a.xml";

        try {
            Context ctx = new Context(yang_folder);
            assertNotNull(ctx);
            ctx.parse_module_path(yin_file, LYS_IN_YIN);
            Data_Node root = ctx.parse_data_path(config_file, LYD_XML, LYD_OPT_CONFIG | LYD_OPT_STRICT);
            assertNotNull(root);

            Set set = new Set();
            assertNotNull(set);
            assertEquals(0, set.number());

            set.add(root.child().schema());
            assertEquals(1, set.number());

            set.add(root.schema());
            assertEquals(2, set.number());

            set.rm(root.schema());
            assertEquals(1, set.number());

            set.add(root.schema());
            assertEquals(2, set.number());

            set.rm_index(1);
            assertEquals(1, set.number());

            set.clean();
            assertEquals(0, set.number());
        } catch(Exception e) {
            fail(e.getMessage());
        }
    }

    public static void main(String[] args) {
        new LibyangTest();
    }

}
