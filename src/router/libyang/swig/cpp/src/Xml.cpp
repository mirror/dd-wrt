/**
 * @file Xml.cpp
 * @author Mislav Novakovic <mislav.novakovic@sartura.hr>
 * @brief Implementation of header Xml.hpp
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

#include "Internal.hpp"
#include "Xml.hpp"

extern "C" {
#include "libyang.h"
#include "xml.h"
}

namespace libyang {

Xml_Ns::Xml_Ns(const struct lyxml_ns *ns, S_Deleter deleter):
    ns((struct lyxml_ns *) ns),
    deleter(deleter)
{}
Xml_Ns::~Xml_Ns() {}
S_Xml_Ns Xml_Ns::next() LY_NEW(ns, next, Xml_Ns);

Xml_Attr::Xml_Attr(struct lyxml_attr *attr, S_Deleter deleter):
    attr(attr),
    deleter(deleter)
{}
Xml_Attr::~Xml_Attr() {}
S_Xml_Attr Xml_Attr::next() LY_NEW(attr, next, Xml_Attr);
S_Xml_Ns Xml_Attr::ns() LY_NEW(attr, ns, Xml_Ns);

Xml_Elem::Xml_Elem(S_Context context, struct lyxml_elem *elem, S_Deleter deleter):
    context(context),
    elem(elem),
    deleter(deleter)
{}
Xml_Elem::~Xml_Elem() {}
S_Xml_Elem Xml_Elem::parent() {return elem->parent ? std::make_shared<Xml_Elem>(context, elem->parent, deleter) : nullptr;}
S_Xml_Attr Xml_Elem::attr() LY_NEW(elem, attr, Xml_Attr);
S_Xml_Elem Xml_Elem::child() {return elem->child ? std::make_shared<Xml_Elem>(context, elem->child, deleter) : nullptr;}
S_Xml_Elem Xml_Elem::next() {return elem->next ? std::make_shared<Xml_Elem>(context, elem->next, deleter) : nullptr;}
S_Xml_Elem Xml_Elem::prev() {return elem->prev ? std::make_shared<Xml_Elem>(context, elem->prev, deleter) : nullptr;}
S_Xml_Ns Xml_Elem::ns() LY_NEW(elem, ns, Xml_Ns);
const char *Xml_Elem::get_attr(const char *name, const char *ns) {
    return lyxml_get_attr(elem, name, ns);
}
S_Xml_Ns Xml_Elem::get_ns(const char *prefix) {
    const struct lyxml_ns *ns = lyxml_get_ns(elem, prefix);
    return elem->ns ? std::make_shared<Xml_Ns>((struct lyxml_ns *)ns, deleter) : nullptr;
}
std::string Xml_Elem::print_mem(int options) {
    char *data = nullptr;

    lyxml_print_mem(&data, (const struct lyxml_elem *) elem, options);
    if (!data) {
        return nullptr;
    }

    std::string s_data = data;
    free(data);
    return s_data;
}

std::vector<S_Xml_Elem> Xml_Elem::tree_for() {
    std::vector<S_Xml_Elem> s_vector;

    struct lyxml_elem *elem = nullptr;
    LY_TREE_FOR(elem, elem) {
        s_vector.push_back(std::make_shared<Xml_Elem>(context, elem, deleter));
    }

    return s_vector;
}
std::vector<S_Xml_Elem> Xml_Elem::tree_dfs() {
    std::vector<S_Xml_Elem> s_vector;

    struct lyxml_elem *elem = nullptr, *next = nullptr;
    LY_TREE_DFS_BEGIN(elem, next, elem) {
        s_vector.push_back(std::make_shared<Xml_Elem>(context, elem, deleter));
        LY_TREE_DFS_END(elem, next, elem)
    }

    return s_vector;
}

}
