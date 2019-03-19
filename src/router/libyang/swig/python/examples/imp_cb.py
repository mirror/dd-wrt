__author__ = "Mislav Novakovic <mislav.novakovic@sartura.hr>"
__copyright__ = "Copyright 2017, Deutsche Telekom AG"
__license__ = "BSD 3-Clause"

import yang as ly

mod_a = "module a {namespace urn:a; prefix a; import b { prefix b; } leaf a { type b:mytype; }}"
mod_b = "module b {namespace urn:b; prefix b; typedef mytype { type string; }}"

def module_imp_clb(mod_name, mod_rev, submod_name, sub_rev, user_data):
    print("module_imp_cb called")
    print("mod_name: ", mod_name, " mod_rev: ", mod_rev, " submod_name: ", submod_name, " sub_rev: ", sub_rev)
    return (ly.LYS_IN_YANG, mod_b)


ctx = None
try:
    ctx = ly.Context()
except Exception as e:
    print(e)

ctx.set_module_imp_clb(module_imp_clb, None)
module = ctx.parse_module_mem(mod_a, ly.LYS_IN_YANG)
print(module.print_mem(ly.LYS_OUT_TREE, 0))
