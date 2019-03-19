/**
 * @file process_notif_tree.cpp
 * @author Enbo Zhang <akbbxbxnmn@qq.com>
 * @brief Example of the libyang C++ bindings
 *
 * Copyright (c) 2017 Deutsche Telekom AG.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#include <iostream>
#include <memory>
#include <stdexcept>
#include <vector>

#include <Libyang.hpp>
#include <Tree_Data.hpp>
#include <Tree_Schema.hpp>

int main() {

    S_Context ctx = nullptr;
    try {
        ctx = S_Context(new Context("/etc/sysrepo/yang"));
    } catch( const std::exception& e ) {
        std::cout << e.what() << std::endl;
        auto errors = std::shared_ptr<std::vector<S_Error>>(get_ly_errors(ctx));
        for(auto error = errors->begin() ; error != errors->end() ; ++error) {
            std::cout << "err: " << (*error)->err() << std::endl;
            std::cout << "vecode: " << (*error)->vecode() << std::endl;
            std::cout << "errmsg: " << (*error)->errmsg() << std::endl;
            std::cout << "errpath: " << (*error)->errpath() << std::endl;
            std::cout << "errapptag: " << (*error)->errapptag() << std::endl;
        }
        return -1;
    }

    const char *schema_xml = "module cpu-notif {\n"
                             "\n"
                             "  namespace \"cpu-notif\";\n"
                             "\n"
                             "  prefix \"cn\";\n"
                             "\n"
                             "  description\n"
                             "    \"CPU nontification model.\";\n"
                             "\n"
                             "  revision 2018-06-26 {\n"
                             "    description\n"
                             "      \"Initial revision.\";\n"
                             "  }\n"
                             "\n"
                             "  /* Notifications */\n"
                             "\n"
                             "  notification busy {\n"
                             "    description\n"
                             "      \"The cpu is busy.\";\n"
                             "    leaf usage {\n"
                             "      type uint32;\n"
                             "      mandatory \"true\";\n"
                             "      description\n"
                             "        \"The usage of current usage.\";\n"
                             "    }\n"
                             "  }\n"
                             "}";
    ctx->parse_module_mem(schema_xml, LYS_IN_YANG);
    S_Data_Node node = nullptr;
    const char *data_xml = "<busy xmlns=\"cpu-notif\">\n"
                           "    <usage>80</usage>\n"
                           "</busy>";
    try {
        node = ctx->parse_data_mem(data_xml, LYD_XML, LYD_OPT_NOTIF);
    } catch( const std::exception& e ) {
        std::cout << e.what() << std::endl;
    }

    if (!node) {
        std::cout << "parse_path did not return any nodes" << std::endl;
    } else {
        std::cout << "\n data tree_dfs\n" << std::endl;
        auto data_list = std::shared_ptr<std::vector<S_Data_Node>>(node->tree_dfs());
        for(auto elem = data_list->begin() ; elem != data_list->end() ; ++elem) {
            std::cout << "name: " << (*elem)->schema()->name() << " type: " << (*elem)->schema()->nodetype() << std::endl;
        }

        std::cout << "\nChild of " << node->schema()->name() << " is: " << node->child()->schema()->name() << "\n" << std::endl;

        std::cout << "data child tree_for\n" << std::endl;

        data_list = std::shared_ptr<std::vector<S_Data_Node>>(node->child()->tree_dfs());
        for(auto elem = data_list->begin() ; elem != data_list->end() ; ++elem) {
            std::cout << "child of " << node->schema()->name() << " is: " << (*elem)->schema()->name() << " type: " << (*elem)->schema()->nodetype() << std::endl;
        }

        std::cout << "\n schema tree_dfs\n" << std::endl;
        auto schema_list = std::shared_ptr<std::vector<S_Schema_Node>>(node->schema()->tree_dfs());
        for(auto elem = schema_list->begin() ; elem != schema_list->end() ; ++elem) {
            std::cout << "schema name " << (*elem)->name() << " type " << (*elem)->nodetype() << std::endl;
        }
    }

    return 0;
}
