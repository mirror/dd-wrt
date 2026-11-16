#!/usr/bin/python3

from __future__ import print_function
import sys
import os
import json
import argparse

if os.getuid() != 0:
    print("You need to be root to run this, sorry")
    sys.exit(77)

TESTS_PATH = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(TESTS_PATH, '../../py/'))

from nftables import Nftables

# Change working directory to repository root
os.chdir(TESTS_PATH + "/../..")

parser = argparse.ArgumentParser(description='Run JSON echo tests')
parser.add_argument('-H', '--host', action='store_true',
                    help='Run tests against installed libnftables.so.1')
parser.add_argument('-l', '--library', default=None,
                    help='Path to libntables.so, overrides --host')
args = parser.parse_args()

check_lib_path = True
if args.library is None:
    if args.host:
        args.library = 'libnftables.so.1'
        check_lib_path = False
    else:
        args.library = 'src/.libs/libnftables.so.1'

if check_lib_path and not os.path.exists(args.library):
    print("Library not found at '%s'." % args.library)
    sys.exit(1)

nftables = Nftables(sofile = args.library)
nftables.set_echo_output(True)

# various commands to work with

flush_ruleset = { "flush": { "ruleset": None } }

list_ruleset = { "list": { "ruleset": None } }

add_table = { "add": {
    "table": {
        "family": "inet",
        "name": "t",
    }
}}

add_chain = { "add": {
    "chain": {
        "family": "inet",
        "table": "t",
        "name": "c"
    }
}}

add_set = { "add": {
    "set": {
        "family": "inet",
        "table": "t",
        "name": "s",
        "type": "inet_service"
    }
}}

add_rule = { "add": {
    "rule": {
        "family": "inet",
        "table": "t",
        "chain": "c",
        "expr": [ { "accept": None } ]
    }
}}

add_counter = { "add": {
    "counter": {
        "family": "inet",
        "table": "t",
        "name": "c"
    }
}}

add_quota = { "add": {
    "quota": {
        "family": "inet",
        "table": "t",
        "name": "q",
        "bytes": 65536
    }
}}

# helper functions

def exit_err(msg):
    print("Error: %s" %msg, file=sys.stderr)
    sys.exit(1)

def exit_dump(e, obj):
    msg = "{}\n".format(e)
    msg += "Output was:\n"
    msg += json.dumps(obj, sort_keys = True, indent = 4, separators = (',', ': '))
    exit_err(msg)

def do_flush():
    rc, out, err = nftables.json_cmd({ "nftables": [flush_ruleset] })
    if rc != 0:
        exit_err("flush ruleset failed: {}".format(err))

def do_command(cmd):
    if not type(cmd) is list:
        cmd = [cmd]
    rc, out, err = nftables.json_cmd({ "nftables": cmd })
    if rc != 0:
        exit_err("command failed: {}".format(err))
    return out

def do_list_ruleset():
    echo = nftables.get_echo_output()
    nftables.set_echo_output(False)
    out = do_command(list_ruleset)
    nftables.set_echo_output(echo)
    return out

def get_handle(output, search):
    try:
        for item in output["nftables"]:
            if "add" in item:
                data = item["add"]
            elif "insert" in item:
                data = item["insert"]
            else:
                data = item

            k = list(search.keys())[0]

            if not k in data:
                continue
            found = True
            for key in list(search[k].keys()):
                if key == "handle":
                    continue
                if not key in data[k] or search[k][key] != data[k][key]:
                    found = False
                    break
            if not found:
                continue

            return data[k]["handle"]
    except Exception as e:
        exit_dump(e, output)

# single commands first

do_flush()

print("Adding table t")
out = do_command(add_table)
handle = get_handle(out, add_table["add"])

out = do_list_ruleset()
handle_cmp = get_handle(out, add_table["add"])

if handle != handle_cmp:
    exit_err("handle mismatch!")

add_table["add"]["table"]["handle"] = handle

print("Adding chain c")
out = do_command(add_chain)
handle = get_handle(out, add_chain["add"])

out = do_list_ruleset()
handle_cmp = get_handle(out, add_chain["add"])

if handle != handle_cmp:
    exit_err("handle mismatch!")

add_chain["add"]["chain"]["handle"] = handle

print("Adding set s")
out = do_command(add_set)
handle = get_handle(out, add_set["add"])

out = do_list_ruleset()
handle_cmp = get_handle(out, add_set["add"])

if handle != handle_cmp:
    exit_err("handle mismatch!")

add_set["add"]["set"]["handle"] = handle

print("Adding rule")
out = do_command(add_rule)
handle = get_handle(out, add_rule["add"])

out = do_list_ruleset()
handle_cmp = get_handle(out, add_rule["add"])

if handle != handle_cmp:
    exit_err("handle mismatch!")

add_rule["add"]["rule"]["handle"] = handle

print("Adding counter")
out = do_command(add_counter)
handle = get_handle(out, add_counter["add"])

out = do_list_ruleset()
handle_cmp = get_handle(out, add_counter["add"])

if handle != handle_cmp:
    exit_err("handle mismatch!")

add_counter["add"]["counter"]["handle"] = handle

print("Adding quota")
out = do_command(add_quota)
handle = get_handle(out, add_quota["add"])

out = do_list_ruleset()
handle_cmp = get_handle(out, add_quota["add"])

if handle != handle_cmp:
    exit_err("handle mismatch!")

add_quota["add"]["quota"]["handle"] = handle

# adjust names and add items again
# Note: Handles are per-table, hence add renamed objects to first table
#       to make sure assigned handle differs from first run.

add_table["add"]["table"]["name"] = "t2"
add_chain["add"]["chain"]["name"] = "c2"
add_set["add"]["set"]["name"] = "s2"
add_counter["add"]["counter"]["name"] = "c2"
add_quota["add"]["quota"]["name"] = "q2"

print("Adding table t2")
out = do_command(add_table)
handle = get_handle(out, add_table["add"])
if handle == add_table["add"]["table"]["handle"]:
   exit_err("handle not changed in re-added table!")

print("Adding chain c2")
out = do_command(add_chain)
handle = get_handle(out, add_chain["add"])
if handle == add_chain["add"]["chain"]["handle"]:
   exit_err("handle not changed in re-added chain!")

print("Adding set s2")
out = do_command(add_set)
handle = get_handle(out, add_set["add"])
if handle == add_set["add"]["set"]["handle"]:
   exit_err("handle not changed in re-added set!")

print("Adding rule again")
out = do_command(add_rule)
handle = get_handle(out, add_rule["add"])
if handle == add_rule["add"]["rule"]["handle"]:
   exit_err("handle not changed in re-added rule!")

print("Adding counter c2")
out = do_command(add_counter)
handle = get_handle(out, add_counter["add"])
if handle == add_counter["add"]["counter"]["handle"]:
   exit_err("handle not changed in re-added counter!")

print("Adding quota q2")
out = do_command(add_quota)
handle = get_handle(out, add_quota["add"])
if handle == add_quota["add"]["quota"]["handle"]:
   exit_err("handle not changed in re-added quota!")

# now multiple commands

# reset name changes again
add_table["add"]["table"]["name"] = "t"
add_chain["add"]["chain"]["name"] = "c"
add_set["add"]["set"]["name"] = "s"
add_counter["add"]["counter"]["name"] = "c"
add_quota["add"]["quota"]["name"] = "q"

do_flush()

print("doing multi add")
del(add_rule["add"]["rule"]["handle"])
add_multi = [ add_table, add_chain, add_set, add_rule ]
out = do_command(add_multi)

thandle = get_handle(out, add_table["add"])
chandle = get_handle(out, add_chain["add"])
shandle = get_handle(out, add_set["add"])
rhandle = get_handle(out, add_rule["add"])

out = do_list_ruleset()

if thandle != get_handle(out, add_table["add"]):
    exit_err("table handle mismatch!")

if chandle != get_handle(out, add_chain["add"]):
    exit_err("chain handle mismatch!")

if shandle != get_handle(out, add_set["add"]):
    exit_err("set handle mismatch!")

if rhandle != get_handle(out, add_rule["add"]):
    exit_err("rule handle mismatch!")
