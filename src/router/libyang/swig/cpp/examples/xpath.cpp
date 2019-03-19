/**
 * @file xpath.cpp
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
        auto module = ctx->load_module("turing-machine", nullptr);

        auto node = ctx->parse_data_path("/etc/sysrepo/data/turing-machine.startup", LYD_XML, LYD_OPT_CONFIG);

        auto node_set = node->find_path("/turing-machine:turing-machine/transition--function/delta[label='left summand']/*");\
        if (!node_set) {
            std::cout << "could not find data for xpath" << std::endl;
            return -1;
        }

        auto list = node_set->data();
        for(auto data_set = list.begin() ; data_set != list.end() ; ++data_set) {
            std::cout << "name: " << (*data_set)->schema()->name() << " type: " << (*data_set)->schema()->nodetype() << " path: " << (*data_set)->path() << std::endl;
        }
    } catch( const std::exception& e ) {
        std::cout << "test" << std::endl;
        std::cout << e.what() << std::endl;
        auto errors = libyang::get_ly_errors(ctx);
        for(auto error = errors.begin() ; error != errors.end() ; ++error) {
            std::cout << "err: " << (*error)->err() << std::endl;
            std::cout << "vecode: " << (*error)->vecode() << std::endl;
            std::cout << "errmsg: " << (*error)->errmsg() << std::endl;
            std::cout << "errpath: " << (*error)->errpath() << std::endl;
            std::cout << "errapptag: " << (*error)->errapptag() << std::endl;
        }
        return -1;
    }

    return 0;
}
