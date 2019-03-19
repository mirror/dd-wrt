/**
 * @file context.cpp
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
        ctx = std::make_shared<libyang::Context>("/etc/sysrepo2/yang");
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
    }

    try {
        ctx = std::make_shared<libyang::Context>("/etc/sysrepo/yang");
    } catch( const std::exception& e ) {
        std::cout << e.what() << std::endl;
    }

    if (!ctx) {
        std::cerr << "Modify this example so that it can find some YANG dirs" << std::endl;
        return 1;
    }

    auto folders = ctx->get_searchdirs();
    for(auto elem = folders.begin() ; elem != folders.end() ; ++elem) {
        std::cout << (*elem) << std::endl;
    }
    std::cout << std::endl;

    auto module = ctx->get_module("ietf-interfaces");
    if (module) {
        std::cout << module->name() << std::endl;
    } else {
        module = ctx->load_module("ietf-interfaces");
        if (module) {
            std::cout << module->name() << std::endl;
        }
    }

    auto modules = ctx->get_module_iter();
    for(auto mod = modules.begin() ; mod != modules.end() ; ++mod) {
        std::cout << "module " << (*mod)->name() << " prefix " << (*mod)->prefix() << " type " << (*mod)->type() << std::endl;
    }

    return 0;
}
