import org.cesnet.Module;
import org.cesnet.vectorModules;
import org.cesnet.vector_String;

public class Context {

    public static void main(String[] args) {
        System.loadLibrary("yangJava");
        org.cesnet.Context ctx = new org.cesnet.Context("/etc/sysrepo/yang");
        vector_String vs = ctx.get_searchdirs();
        for (int i = 0; i < vs.size(); i++) {
            System.out.println(vs.get(i));
        }
        Module module = ctx.get_module("ietf-interfaces");
        if (module != null){
            System.out.println(module.name());
        }else {
            module = ctx.load_module("ietf-interfaces");
            if (module != null){
                System.out.println(module.name());
            }
        }
        vectorModules vModules= ctx.get_module_iter();
        for (int i = 0; i < vModules.size(); i++) {
            Module mod = vModules.get(i);
            System.out.println("module " + mod.name() + " prefix " + mod.prefix() + " type " + + mod.type());
        }

    }

}
