import org.cesnet.Data_Node;
import org.cesnet.Module;
import org.cesnet.Set;
import org.cesnet.vectorData_Node;

import static org.cesnet.LYD_FORMAT.LYD_XML;
import static org.cesnet.yangConstants.LYD_OPT_CONFIG;

public class Xpath {

    public static void main(String[] args) {
        System.loadLibrary("yangJava");
        org.cesnet.Context ctx = new org.cesnet.Context("/etc/sysrepo/yang");

        Module module = ctx.get_module("turing-machine");
        if (module != null){
            System.out.println(module.name());
        }else {
            ctx.load_module("turing-machine");
        }

        Data_Node node = ctx.parse_data_path("/etc/sysrepo/data/turing-machine.startup", LYD_XML, LYD_OPT_CONFIG);

        Set node_set = node.find_path("/turing-machine:turing-machine/transition--function/delta[label='left summand']/*");
        if (node_set == null) {
            System.out.println("could not find data for xpath");
            return ;
        }

        vectorData_Node list = node_set.data();
        for (int i = 0; i < list.size(); i++) {
            Data_Node data_set = list.get(i);
            System.out.println("name: " + data_set.schema().name() + " type: " + data_set.schema().nodetype() + " path: " + data_set.path());
        }
    }

}
