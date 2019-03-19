/**
 * @file process_tree.cpp
 * @author Mislav Novakovic <mislav.novakovic@sartura.hr>
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

    libyang::S_Context ctx;
    try {
        ctx = std::make_shared<libyang::Context>("/etc/sysrepo/yang");
    } catch( const std::exception& e ) {
        std::cout << e.what() << std::endl;
        auto errors = get_ly_errors(ctx);
        for(auto error = errors.begin() ; error != errors.end() ; ++error) {
            std::cout << "err: " << (*error)->err() << std::endl;
            std::cout << "vecode: " << (*error)->vecode() << std::endl;
            std::cout << "errmsg: " << (*error)->errmsg() << std::endl;
            std::cout << "errpath: " << (*error)->errpath() << std::endl;
            std::cout << "errapptag: " << (*error)->errapptag() << std::endl;
        }
        return -1;
    }

    auto module = ctx->get_module("turing-machine");
    if (module) {
        std::cout << module->name() << std::endl;
    } else {
        module = ctx->load_module("turing-machine");
    }

    libyang::S_Data_Node node;
    try {
        node = ctx->parse_data_path("/etc/sysrepo/data/turing-machine.startup", LYD_XML, LYD_OPT_CONFIG);
    } catch( const std::exception& e ) {
        std::cout << e.what() << std::endl;
    }

    if (!node) {
        std::cout << "parse_path did not return any nodes" << std::endl;
    } else {
        std::cout << "tree_dfs\n" << std::endl;
        auto data_list = node->tree_dfs();
        for(auto elem = data_list.begin() ; elem != data_list.end() ; ++elem) {
            std::cout << "name: " << (*elem)->schema()->name() << " type: " << (*elem)->schema()->nodetype() << std::endl;
        }

        std::cout << "\nChild of " << node->schema()->name() << " is: " << node->child()->schema()->name() << "\n" << std::endl;

        std::cout << "tree_for\n" << std::endl;

        data_list = node->child()->child()->tree_dfs();
        for(auto elem = data_list.begin() ; elem != data_list.end() ; ++elem) {
            std::cout << "child of " << node->child()->schema()->name() << " is: " << (*elem)->schema()->name() << " type: " << (*elem)->schema()->nodetype() << std::endl;
        }

        std::cout << "\n schema tree_dfs\n" << std::endl;
        auto schema_list = node->schema()->tree_dfs();
        for(auto elem = schema_list.begin() ; elem != schema_list.end() ; ++elem) {
            std::cout << "schema name " << (*elem)->name() << " type " << (*elem)->nodetype() << std::endl;
            if  (LYS_LEAF == (*elem)->nodetype()) {
                auto leaf = libyang::Schema_Node_Leaf(*elem);
                auto list = leaf.is_key();
                if (list) {
                    std::cout << "leaf " << leaf.name() << " is a key for the list " << list->name() << std::endl;
                }
            }
        }
    }

    return 0;
}
