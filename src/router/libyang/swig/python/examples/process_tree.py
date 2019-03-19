__author__ = "Mislav Novakovic <mislav.novakovic@sartura.hr>"
__copyright__ = "Copyright 2017, Deutsche Telekom AG"
__license__ = "BSD 3-Clause"

import yang as ly

ctx = None
try:
    ctx = ly.Context("/etc/sysrepo/yang")
except Exception as e:
    print(e)
    errors = ly.get_ly_errors(ctx)
    for err in errors:
        print("err: %d" % err.err())
        print("vecode: %d" % err.vecode())
        print("errmsg: " + err.errmsg())
        print("errpath:" + err.errpath())
        print("errapptag:" + err.errapptag())

module = ctx.get_module("turing-machine")
if module is not None:
    print(module.name())
else:
    module = ctx.load_module("turing-machine")

node = None
try:
    if node is None : node = ctx.parse_data_path("/etc/sysrepo/data/turing-machine.startup", ly.LYD_LYB, ly.LYD_OPT_CONFIG)
except Exception as e:
    print(e)
try:
    if node is None : node = ctx.parse_data_path("/etc/sysrepo/data/turing-machine.startup", ly.LYD_XML, ly.LYD_OPT_CONFIG)
except Exception as e:
    print(e)
try:
    if node is None : node = ctx.parse_data_path("/etc/sysrepo/data/turing-machine.startup", ly.LYD_JSON, ly.LYD_OPT_CONFIG)
except Exception as e:
    print(e)

if node is None:
    sys.exit()

if node is None:
    print("parse_data_path did not return any nodes")
else:
    print("tree_dfs\n")
    data_list = node.tree_dfs()
    for elem in data_list:
        schema = elem.schema()
        print("name: " + schema.name() + " type: " + str(schema.nodetype()))
        if (ly.LYS_LEAF == schema.nodetype() or ly.LYS_LEAFLIST == schema.nodetype()):
            casted = elem.subtype()
            if casted is None:
                continue
            print("node " + casted.schema().name() + " has value " + casted.value_str())

    print("\nChild of " + node.schema().name() + " is " + node.child().schema().name() + " \n ")

    print("tree_for\n")
    data_list = node.child().child().tree_for()
    for elem in data_list:
        print("child of " + node.child().schema().name() + " is: " + elem.schema().name() + " type: " + str(elem.schema().nodetype()))

    print("\nschema tree_dfs\n")
    schema_list = node.schema().tree_dfs()
    for elem in schema_list:
        print("schema name: " + elem.name() + " type: " + str(elem.nodetype()))
