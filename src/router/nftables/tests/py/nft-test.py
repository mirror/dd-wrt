#!/usr/bin/env python
#
# (C) 2014 by Ana Rey Botello <anarey@gmail.com>
#
# Based on iptables-test.py:
# (C) 2012 by Pablo Neira Ayuso <pablo@netfilter.org>"
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# Thanks to the Outreach Program for Women (OPW) for sponsoring this test
# infrastructure.

from __future__ import print_function
import sys
import os
import io
import argparse
import signal
import json
import traceback
import tempfile

TESTS_PATH = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(TESTS_PATH, '../../py/src'))
os.environ['TZ'] = 'UTC-2'

from nftables import Nftables

TESTS_DIRECTORY = ["any", "arp", "bridge", "inet", "ip", "ip6", "netdev"]
LOGFILE = "/tmp/nftables-test.log"
log_file = None
table_list = []
chain_list = []
all_set = dict()
obj_list = []
signal_received = 0


class Colors:
    if sys.stdout.isatty() and sys.stderr.isatty():
        HEADER = '\033[95m'
        GREEN = '\033[92m'
        YELLOW = '\033[93m'
        RED = '\033[91m'
        ENDC = '\033[0m'
    else:
        HEADER = ''
        GREEN = ''
        YELLOW = ''
        RED = ''
        ENDC = ''


class Chain:
    """Class that represents a chain"""

    def __init__(self, name, config, lineno):
        self.name = name
        self.config = config
        self.lineno = lineno

    def __eq__(self, other):
        return self.__dict__ == other.__dict__

    def __str__(self):
        return "%s" % self.name


class Table:
    """Class that represents a table"""

    def __init__(self, family, name, chains):
        self.family = family
        self.name = name
        self.chains = chains

    def __eq__(self, other):
        return self.__dict__ == other.__dict__

    def __str__(self):
        return "%s %s" % (self.family, self.name)


class Set:
    """Class that represents a set"""

    def __init__(self, family, table, name, type, data, timeout, flags):
        self.family = family
        self.table = table
        self.name = name
        self.type = type
        self.data = data
        self.timeout = timeout
        self.flags = flags

    def __eq__(self, other):
        return self.__dict__ == other.__dict__


class Obj:
    """Class that represents an object"""

    def __init__(self, table, family, name, type, spcf):
        self.table = table
        self.family = family
        self.name = name
        self.type = type
        self.spcf = spcf

    def __eq__(self, other):
        return self.__dict__ == other.__dict__


def print_msg(reason, errstr, filename=None, lineno=None, color=None):
    '''
    Prints a message with nice colors, indicating file and line number.
    '''
    color_errstr = "%s%s%s" % (color, errstr, Colors.ENDC)
    if filename and lineno:
        sys.stderr.write("%s: %s line %d: %s\n" %
                         (filename, color_errstr, lineno + 1, reason))
    else:
        sys.stderr.write("%s %s\n" % (color_errstr, reason))
    sys.stderr.flush() # So that the message stay in the right place.


def print_error(reason, filename=None, lineno=None):
    print_msg(reason, "ERROR:", filename, lineno, Colors.RED)


def print_warning(reason, filename=None, lineno=None):
    print_msg(reason, "WARNING:", filename, lineno, Colors.YELLOW)

def print_info(reason, filename=None, lineno=None):
    print_msg(reason, "INFO:", filename, lineno, Colors.GREEN)

def color_differences(rule, other, color):
    rlen = len(rule)
    olen = len(other)
    out = ""
    i = 0

    # find equal part at start
    for i in range(rlen):
        if i >= olen or rule[i] != other[i]:
            break
    if i > 0:
        out += rule[:i]
        rule = rule[i:]
        other = other[i:]
        rlen = len(rule)
        olen = len(other)

    # find equal part at end
    for i in range(1, rlen + 1):
        if i > olen or rule[rlen - i] != other[olen - i]:
            i -= 1
            break
    if rlen > i:
        out += color + rule[:rlen - i] + Colors.ENDC
        rule = rule[rlen - i:]

    out += rule
    return out

def print_differences_warning(filename, lineno, rule1, rule2, cmd):
    colored_rule1 = color_differences(rule1, rule2, Colors.YELLOW)
    colored_rule2 = color_differences(rule2, rule1, Colors.YELLOW)
    reason = "'%s': '%s' mismatches '%s'" % (cmd, colored_rule1, colored_rule2)
    print_warning(reason, filename, lineno)


def print_differences_error(filename, lineno, cmd):
    reason = "'%s': Listing is broken." % cmd
    print_error(reason, filename, lineno)


def table_exist(table, filename, lineno):
    '''
    Exists a table.
    '''
    cmd = "list table %s" % table
    ret = execute_cmd(cmd, filename, lineno)

    return True if (ret == 0) else False


def table_flush(table, filename, lineno):
    '''
    Flush a table.
    '''
    cmd = "flush table %s" % table
    execute_cmd(cmd, filename, lineno)

    return cmd


def table_create(table, filename, lineno):
    '''
    Adds a table.
    '''
    # We check if table exists.
    if table_exist(table, filename, lineno):
        reason = "Table %s already exists" % table
        print_error(reason, filename, lineno)
        return -1

    table_list.append(table)

    # We add a new table
    cmd = "add table %s" % table
    ret = execute_cmd(cmd, filename, lineno)

    if ret != 0:
        reason = "Cannot " + cmd
        print_error(reason, filename, lineno)
        table_list.remove(table)
        return -1

    # We check if table was added correctly.
    if not table_exist(table, filename, lineno):
        table_list.remove(table)
        reason = "I have just added the table %s " \
                 "but it does not exist. Giving up!" % table
        print_error(reason, filename, lineno)
        return -1

    for table_chain in table.chains:
        chain = chain_get_by_name(table_chain)
        if chain is None:
            reason = "The chain %s requested by table %s " \
                     "does not exist." % (table_chain, table)
            print_error(reason, filename, lineno)
        else:
            chain_create(chain, table, filename)

    return 0


def table_delete(table, filename=None, lineno=None):
    '''
    Deletes a table.
    '''
    if not table_exist(table, filename, lineno):
        reason = "Table %s does not exist but I added it before." % table
        print_error(reason, filename, lineno)
        return -1

    cmd = "delete table %s" % table
    ret = execute_cmd(cmd, filename, lineno)
    if ret != 0:
        reason = "%s: I cannot delete table %s. Giving up!" % (cmd, table)
        print_error(reason, filename, lineno)
        return -1

    if table_exist(table, filename, lineno):
        reason = "I have just deleted the table %s " \
                 "but it still exists." % table
        print_error(reason, filename, lineno)
        return -1

    return 0


def chain_exist(chain, table, filename):
    '''
    Checks a chain
    '''
    cmd = "list chain %s %s" % (table, chain)
    ret = execute_cmd(cmd, filename, chain.lineno)

    return True if (ret == 0) else False


def chain_create(chain, table, filename):
    '''
    Adds a chain
    '''
    if chain_exist(chain, table, filename):
        reason = "This chain '%s' exists in %s. I cannot create " \
                 "two chains with same name." % (chain, table)
        print_error(reason, filename, chain.lineno)
        return -1

    cmd = "add chain %s %s" % (table, chain)
    if chain.config:
        cmd += " { %s; }" % chain.config

    ret = execute_cmd(cmd, filename, chain.lineno)
    if ret != 0:
        reason = "I cannot create the chain '%s'" % chain
        print_error(reason, filename, chain.lineno)
        return -1

    if not chain_exist(chain, table, filename):
        reason = "I have added the chain '%s' " \
                 "but it does not exist in %s" % (chain, table)
        print_error(reason, filename, chain.lineno)
        return -1

    return 0


def chain_delete(chain, table, filename=None, lineno=None):
    '''
    Flushes and deletes a chain.
    '''
    if not chain_exist(chain, table, filename):
        reason = "The chain %s does not exist in %s. " \
                 "I cannot delete it." % (chain, table)
        print_error(reason, filename, lineno)
        return -1

    cmd = "flush chain %s %s" % (table, chain)
    ret = execute_cmd(cmd, filename, lineno)
    if ret != 0:
        reason = "I cannot " + cmd
        print_error(reason, filename, lineno)
        return -1

    cmd = "delete chain %s %s" % (table, chain)
    ret = execute_cmd(cmd, filename, lineno)
    if ret != 0:
        reason = "I cannot " + cmd
        print_error(reason, filename, lineno)
        return -1

    if chain_exist(chain, table, filename):
        reason = "The chain %s exists in %s. " \
                 "I cannot delete this chain" % (chain, table)
        print_error(reason, filename, lineno)
        return -1

    return 0


def chain_get_by_name(name):
    for chain in chain_list:
        if chain.name == name:
            break
    else:
        chain = None

    return chain


def set_add(s, test_result, filename, lineno):
    '''
    Adds a set.
    '''
    if not table_list:
        reason = "Missing table to add rule"
        print_error(reason, filename, lineno)
        return -1

    for table in table_list:
        s.table = table.name
        s.family = table.family
        if _set_exist(s, filename, lineno):
            reason = "Set %s already exists in %s" % (s.name, table)
            print_error(reason, filename, lineno)
            return -1

        flags = s.flags
        if flags != "":
            flags = "flags %s; " % flags

        if s.data == "":
                cmd = "add set %s %s { %s;%s %s}" % (table, s.name, s.type, s.timeout, flags)
        else:
                cmd = "add map %s %s { %s : %s;%s %s}" % (table, s.name, s.type, s.data, s.timeout, flags)

        ret = execute_cmd(cmd, filename, lineno)

        if (ret == 0 and test_result == "fail") or \
                (ret != 0 and test_result == "ok"):
            reason = "%s: I cannot add the set %s" % (cmd, s.name)
            print_error(reason, filename, lineno)
            return -1

        if not _set_exist(s, filename, lineno):
            reason = "I have just added the set %s to " \
                     "the table %s but it does not exist" % (s.name, table)
            print_error(reason, filename, lineno)
            return -1

    return 0


def map_add(s, test_result, filename, lineno):
    '''
    Adds a map
    '''
    if not table_list:
        reason = "Missing table to add rule"
        print_error(reason, filename, lineno)
        return -1

    for table in table_list:
        s.table = table.name
        s.family = table.family
        if _map_exist(s, filename, lineno):
            reason = "Map %s already exists in %s" % (s.name, table)
            print_error(reason, filename, lineno)
            return -1

        flags = s.flags
        if flags != "":
            flags = "flags %s; " % flags

        cmd = "add map %s %s { %s : %s;%s %s}" % (table, s.name, s.type, s.data, s.timeout, flags)

        ret = execute_cmd(cmd, filename, lineno)

        if (ret == 0 and test_result == "fail") or \
                (ret != 0 and test_result == "ok"):
            reason = "%s: I cannot add the set %s" % (cmd, s.name)
            print_error(reason, filename, lineno)
            return -1

        if not _map_exist(s, filename, lineno):
            reason = "I have just added the set %s to " \
                     "the table %s but it does not exist" % (s.name, table)
            print_error(reason, filename, lineno)
            return -1


def set_add_elements(set_element, set_name, state, filename, lineno):
    '''
    Adds elements to the set.
    '''
    if not table_list:
        reason = "Missing table to add rules"
        print_error(reason, filename, lineno)
        return -1

    for table in table_list:
        # Check if set exists.
        if (not set_exist(set_name, table, filename, lineno) or
                    set_name not in all_set) and state == "ok":
            reason = "I cannot add an element to the set %s " \
                     "since it does not exist." % set_name
            print_error(reason, filename, lineno)
            return -1

        element = ", ".join(set_element)
        cmd = "add element %s %s { %s }" % (table, set_name, element)
        ret = execute_cmd(cmd, filename, lineno)

        if (state == "fail" and ret == 0) or (state == "ok" and ret != 0):
            if state == "fail":
                    test_state = "This rule should have failed."
            else:
                    test_state = "This rule should not have failed."

            reason = cmd + ": " + test_state
            print_error(reason, filename, lineno)
            return -1

        # Add element into all_set.
        if ret == 0 and state == "ok":
            for e in set_element:
                all_set[set_name].add(e)

    return 0


def set_delete_elements(set_element, set_name, table, filename=None,
                        lineno=None):
    '''
    Deletes elements in a set.
    '''
    for element in set_element:
        cmd = "delete element %s %s { %s }" % (table, set_name, element)
        ret = execute_cmd(cmd, filename, lineno)
        if ret != 0:
            reason = "I cannot delete element %s " \
                     "from the set %s" % (element, set_name)
            print_error(reason, filename, lineno)
            return -1

    return 0


def set_delete(table, filename=None, lineno=None):
    '''
    Deletes set and its content.
    '''
    for set_name in all_set.keys():
        # Check if exists the set
        if not set_exist(set_name, table, filename, lineno):
            reason = "The set %s does not exist, " \
                     "I cannot delete it" % set_name
            print_error(reason, filename, lineno)
            return -1

        # We delete all elements in the set
        set_delete_elements(all_set[set_name], set_name, table, filename,
                            lineno)

        # We delete the set.
        cmd = "delete set %s %s" % (table, set_name)
        ret = execute_cmd(cmd, filename, lineno)

        # Check if the set still exists after I deleted it.
        if ret != 0 or set_exist(set_name, table, filename, lineno):
            reason = "Cannot remove the set " + set_name
            print_error(reason, filename, lineno)
            return -1

    return 0


def set_exist(set_name, table, filename, lineno):
    '''
    Check if the set exists.
    '''
    cmd = "list set %s %s" % (table, set_name)
    ret = execute_cmd(cmd, filename, lineno)

    return True if (ret == 0) else False


def _set_exist(s, filename, lineno):
    '''
    Check if the set exists.
    '''
    cmd = "list set %s %s %s" % (s.family, s.table, s.name)
    ret = execute_cmd(cmd, filename, lineno)

    return True if (ret == 0) else False


def _map_exist(s, filename, lineno):
    '''
    Check if the map exists.
    '''
    cmd = "list map %s %s %s" % (s.family, s.table, s.name)
    ret = execute_cmd(cmd, filename, lineno)

    return True if (ret == 0) else False


def set_check_element(rule1, rule2):
    '''
    Check if element exists in anonymous sets.
    '''
    pos1 = rule1.find("{")
    pos2 = rule2.find("{")

    if (rule1[:pos1] != rule2[:pos2]):
        return False

    end1 = rule1.find("}")
    end2 = rule2.find("}")

    if (pos1 != -1) and (pos2 != -1) and (end1 != -1) and (end2 != -1):
        list1 = (rule1[pos1 + 1:end1].replace(" ", "")).split(",")
        list2 = (rule2[pos2 + 1:end2].replace(" ", "")).split(",")
        list1.sort()
        list2.sort()
        if list1 != list2:
            return False

        return rule1[end1:] == rule2[end2:]

    return False


def obj_add(o, test_result, filename, lineno):
    '''
    Adds an object.
    '''
    if not table_list:
        reason = "Missing table to add rule"
        print_error(reason, filename, lineno)
        return -1

    for table in table_list:
        o.table = table.name
        o.family = table.family
        obj_handle = o.type + " " + o.name
        if _obj_exist(o, filename, lineno):
            reason = "The %s already exists in %s" % (obj_handle, table)
            print_error(reason, filename, lineno)
            return -1

        cmd = "add %s %s %s %s" % (o.type, table, o.name, o.spcf)
        ret = execute_cmd(cmd, filename, lineno)

        if (ret == 0 and test_result == "fail") or \
                (ret != 0 and test_result == "ok"):
            reason = "%s: I cannot add the %s" % (cmd, obj_handle)
            print_error(reason, filename, lineno)
            return -1

        exist = _obj_exist(o, filename, lineno)

        if exist:
            if test_result == "ok":
                 return 0
            reason = "I added the %s to the table %s " \
                     "but it should have failed" % (obj_handle, table)
            print_error(reason, filename, lineno)
            return -1

        if test_result == "fail":
            return 0

        reason = "I have just added the %s to " \
                 "the table %s but it does not exist" % (obj_handle, table)
        print_error(reason, filename, lineno)
        return -1

def obj_delete(table, filename=None, lineno=None):
    '''
    Deletes object.
    '''
    for o in obj_list:
        obj_handle = o.type + " " + o.name
        # Check if exists the obj
        if not obj_exist(o, table, filename, lineno):
            reason = "The %s does not exist, I cannot delete it" % obj_handle
            print_error(reason, filename, lineno)
            return -1

        # We delete the object.
        cmd = "delete %s %s %s" % (o.type, table, o.name)
        ret = execute_cmd(cmd, filename, lineno)

        # Check if the object still exists after I deleted it.
        if ret != 0 or obj_exist(o, table, filename, lineno):
            reason = "Cannot remove the " + obj_handle
            print_error(reason, filename, lineno)
            return -1

    return 0


def obj_exist(o, table, filename, lineno):
    '''
    Check if the object exists.
    '''
    cmd = "list %s %s %s" % (o.type, table, o.name)
    ret = execute_cmd(cmd, filename, lineno)

    return True if (ret == 0) else False


def _obj_exist(o, filename, lineno):
    '''
    Check if the object exists.
    '''
    cmd = "list %s %s %s %s" % (o.type, o.family, o.table, o.name)
    ret = execute_cmd(cmd, filename, lineno)

    return True if (ret == 0) else False


def output_clean(pre_output, chain):
    pos_chain = pre_output.find(chain.name)
    if pos_chain == -1:
        return ""
    output_intermediate = pre_output[pos_chain:]
    brace_start = output_intermediate.find("{")
    brace_end = output_intermediate.find("}")
    pre_rule = output_intermediate[brace_start:brace_end]
    if pre_rule[1:].find("{") > -1:  # this rule has a set.
        set = pre_rule[1:].replace("\t", "").replace("\n", "").strip()
        set = set.split(";")[2].strip() + "}"
        remainder = output_clean(chain.name + " {;;" + output_intermediate[brace_end+1:], chain)
        if len(remainder) <= 0:
            return set
        return set + " " + remainder
    else:
        rule = pre_rule.split(";")[2].replace("\t", "").replace("\n", "").\
            strip()
    if len(rule) < 0:
        return ""
    return rule


def payload_check_elems_to_set(elems):
    newset = set()

    for n, line in enumerate(elems.split("element")):
        e = line.strip()
        if e in newset:
            print_error("duplicate", e, n)
            return newset

        newset.add(e)

    return newset


def payload_check_set_elems(want, got):
    if not want.strip().startswith("element") or \
       not got.strip().startswith("element"):
        return False

    set_want = payload_check_elems_to_set(want)
    set_got = payload_check_elems_to_set(got)

    return set_want == set_got

def payload_line_relevant(line):
    return line.startswith('  [ ') or line.strip().startswith("element")

def payload_check(payload_buffer, file, cmd):
    file.seek(0, 0)
    i = 0

    if not payload_buffer:
        return False

    for lineno, want_line in enumerate(payload_buffer):
        # skip irreleant parts, such as "ip test-ipv4 output"
        if not payload_line_relevant(want_line):
            continue

        line = file.readline()
        while not payload_line_relevant(line):
            line = file.readline()
            if line == "":
                break

        if want_line == line:
            i += 1
            continue

        if payload_check_set_elems(want_line, line):
            continue

        print_differences_warning(file.name, lineno, want_line.strip(),
                                  line.strip(), cmd)
        return 0

    return i > 0


def payload_record(path, rule, payload, desc="payload"):
    '''
    Record payload for @rule in file at @path

    - @payload may be a file handle, a string or an array of strings
    - Avoid duplicate entries by searching for a match first
    - Separate entries by a single empty line, so check for trailing newlines
      before writing
    - @return False if already existing, True otherwise
    '''
    try:
        with open(path, 'r') as f:
            lines = f.readlines()
    except:
        lines = []

    plines = []
    if isinstance(payload, io.TextIOWrapper):
        payload.seek(0, 0)
        while True:
            line = payload.readline()
            if line.startswith("family "):
                continue
            if line == "":
                break
            plines.append(line)
    elif isinstance(payload, str):
        plines = [l + "\n" for l in payload.split("\n")]
    elif isinstance(payload, list):
        plines = payload
    else:
        raise Exception

    found = False
    for i in range(len(lines)):
        if lines[i] == rule + "\n":
            found = True
            for pline in plines:
                i += 1
                if lines[i] != pline:
                    found = False
                    break
            if found:
                return False

    try:
        with open(path, 'a') as f:
            if len(lines) > 0 and lines[-1] != "\n":
                f.write("\n")
            f.write("# %s\n" % rule)
            f.writelines(plines)
    except:
        warnfmt = "Failed to write %s for rule %s"
    else:
        warnfmt = "Wrote %s for rule %s"

    print_warning(warnfmt % (desc, rule[0]), os.path.basename(path), 1)
    return True


def json_dump_normalize(json_string, human_readable = False):
    json_obj = json.loads(json_string)

    if human_readable:
        return json.dumps(json_obj, sort_keys = True,
                          indent = 4, separators = (',', ': '))
    else:
        return json.dumps(json_obj, sort_keys = True)

def json_validate(json_string):
    json_obj = json.loads(json_string)
    try:
        nftables.json_validate(json_obj)
    except Exception:
        print_error("schema validation failed for input '%s'" % json_string)
        print_error(traceback.format_exc())

def rule_add(rule, filename, lineno, force_all_family_option, filename_path):
    '''
    Adds a rule
    '''
    # TODO Check if a rule is added correctly.
    ret = warning = error = unit_tests = 0

    if not table_list or not chain_list:
        reason = "Missing table or chain to add rule."
        print_error(reason, filename, lineno)
        return [-1, warning, error, unit_tests]

    if rule[1].strip() == "ok":
        payload_expected = None
        payload_path = "%s.payload" % filename_path
        try:
            payload_log = open(payload_path)
            payload_expected = payload_find_expected(payload_log, rule[0])
        except:
            payload_log = None

        if enable_json_option:
            try:
                json_log = open("%s.json" % filename_path)
                json_input = json_find_expected(json_log, rule[0])
            except:
                json_input = None

            if not json_input:
                print_error("did not find JSON equivalent for rule '%s'"
                            % rule[0])
            else:
                try:
                    json_input = json_dump_normalize(json_input)
                except ValueError:
                    reason = "Invalid JSON syntax in rule: %s" % json_input
                    print_error(reason)
                    return [-1, warning, error, unit_tests]

            try:
                json_log = open("%s.json.output" % filename_path)
                json_expected = json_find_expected(json_log, rule[0])
            except:
                # will use json_input for comparison
                json_expected = None

            if json_expected:
                try:
                    json_expected = json_dump_normalize(json_expected)
                except ValueError:
                    reason = "Invalid JSON syntax in expected output: %s" % json_expected
                    print_error(reason)
                    return [-1, warning, error, unit_tests]
                if json_expected == json_input:
                    print_warning("Recorded JSON output matches input for: %s" % rule[0])

    for table in table_list:
        if rule[1].strip() == "ok":
            table_payload_expected = None
            table_payload_path = payload_path
            try:
                payload_log = open("%s.payload.%s" % (filename_path, table.family))
                table_payload_path = payload_log.name
                table_payload_expected = payload_find_expected(payload_log, rule[0])
            except:
                if not payload_log:
                    print_error("did not find any payload information",
                                filename_path)
                elif not payload_expected:
                    print_error("did not find payload information for "
                                "rule '%s'" % rule[0], payload_log.name, 1)
            if not table_payload_expected:
                table_payload_expected = payload_expected

        for table_chain in table.chains:
            chain = chain_get_by_name(table_chain)
            unit_tests += 1
            table_flush(table, filename, lineno)

            payload_log = tempfile.TemporaryFile(mode="w+")

            # Add rule and check return code
            cmd = "add rule %s %s %s" % (table, chain, rule[0])
            ret = execute_cmd(cmd, filename, lineno, payload_log, debug="netlink")

            state = rule[1].rstrip()
            if (ret in [0,134] and state == "fail") or (ret != 0 and state == "ok"):
                if state == "fail":
                    test_state = "This rule should have failed."
                else:
                    test_state = "This rule should not have failed."
                reason = cmd + ": " + test_state
                print_error(reason, filename, lineno)
                ret = -1
                error += 1
                if not force_all_family_option:
                    return [ret, warning, error, unit_tests]

            if state == "fail" and ret != 0:
                ret = 0
                continue

            if ret != 0:
                continue

            # Check for matching payload
            if state == "ok" and not payload_check(table_payload_expected,
                                                   payload_log, cmd):
                error += 1
                payload_record("%s.got" % table_payload_path,
                               rule[0], payload_log)

            # Check for matching ruleset listing
            numeric_proto_old = nftables.set_numeric_proto_output(True)
            stateless_old = nftables.set_stateless_output(True)
            list_cmd = 'list table %s' % table
            rc, pre_output, err = nftables.cmd(list_cmd)
            nftables.set_numeric_proto_output(numeric_proto_old)
            nftables.set_stateless_output(stateless_old)

            output = pre_output.split(";")
            if len(output) < 2:
                reason = cmd + ": Listing is broken."
                print_error(reason, filename, lineno)
                ret = -1
                error += 1
                if not force_all_family_option:
                    return [ret, warning, error, unit_tests]
                continue

            rule_output = output_clean(pre_output, chain)
            retest_output = False
            if len(rule) == 3:
                teoric_exit = rule[2]
                retest_output = True
            else:
                teoric_exit = rule[0]

            if rule_output.rstrip() != teoric_exit.rstrip():
                if rule[0].find("{") != -1:  # anonymous sets
                    if not set_check_element(teoric_exit.rstrip(),
                                         rule_output.rstrip()):
                        warning += 1
                        retest_output = True
                        print_differences_warning(filename, lineno,
                                                  teoric_exit.rstrip(),
                                                  rule_output, cmd)
                        if not force_all_family_option:
                            return [ret, warning, error, unit_tests]
                else:
                    if len(rule_output) <= 0:
                        error += 1
                        print_differences_error(filename, lineno, cmd)
                        if not force_all_family_option:
                            return [ret, warning, error, unit_tests]

                    warning += 1
                    retest_output = True
                    print_differences_warning(filename, lineno,
                                              teoric_exit.rstrip(),
                                              rule_output, cmd)

                    if not force_all_family_option:
                        return [ret, warning, error, unit_tests]

            if retest_output:
                table_flush(table, filename, lineno)

                # Add rule and check return code
                cmd = "add rule %s %s %s" % (table, chain, rule_output.rstrip())
                ret = execute_cmd(cmd, filename, lineno, payload_log, debug="netlink")

                if ret != 0:
                    test_state = "Replaying rule failed."
                    reason = cmd + ": " + test_state
                    print_warning(reason, filename, lineno)
                    ret = -1
                    error += 1
                    if not force_all_family_option:
                        return [ret, warning, error, unit_tests]
                # Check for matching payload
                elif not payload_check(table_payload_expected,
                                       payload_log, cmd):
                    error += 1

            if not enable_json_option:
                continue

            # Generate JSON equivalent for rule if not found
            if not json_input:
                json_old = nftables.set_json_output(True)
                rc, json_output, err = nftables.cmd(list_cmd)
                nftables.set_json_output(json_old)

                json_output = json.loads(json_output)
                for item in json_output["nftables"]:
                    if "rule" in item:
                        del(item["rule"]["handle"])
                        json_output = item["rule"]
                        break
                json_input = json.dumps(json_output["expr"], sort_keys = True)
                payload_record("%s.json.got" % filename_path, rule[0],
                               json_dump_normalize(json_input, True),
                               "JSON equivalent")

            table_flush(table, filename, lineno)
            payload_log = tempfile.TemporaryFile(mode="w+")

            # Add rule in JSON format
            cmd = json.dumps({ "nftables": [{ "add": { "rule": {
                    "family": table.family,
                    "table": table.name,
                    "chain": chain.name,
                    "expr": json.loads(json_input),
            }}}]})

            if enable_json_schema:
                json_validate(cmd)

            json_old = nftables.set_json_output(True)
            ret = execute_cmd(cmd, filename, lineno, payload_log, debug="netlink")
            nftables.set_json_output(json_old)

            if ret != 0:
                reason = "Failed to add JSON equivalent rule"
                print_error(reason, filename, lineno)
                continue

            # Check for matching payload
            if not payload_check(table_payload_expected, payload_log, cmd):
                error += 1
                payload_record("%s.json.payload.got" % filename_path,
                               rule[0], payload_log, "JSON payload")

            # Check for matching ruleset listing
            numeric_proto_old = nftables.set_numeric_proto_output(True)
            stateless_old = nftables.set_stateless_output(True)
            json_old = nftables.set_json_output(True)
            rc, json_output, err = nftables.cmd(list_cmd)
            nftables.set_json_output(json_old)
            nftables.set_numeric_proto_output(numeric_proto_old)
            nftables.set_stateless_output(stateless_old)

            if enable_json_schema:
                json_validate(json_output)

            json_output = json.loads(json_output)
            for item in json_output["nftables"]:
                if "rule" in item:
                    del(item["rule"]["handle"])
                    json_output = item["rule"]
                    break
            json_output = json.dumps(json_output["expr"], sort_keys = True)

            if not json_expected and json_output != json_input:
                print_differences_warning(filename, lineno,
                                          json_input, json_output, cmd)
                error += 1
                payload_record("%s.json.output.got" % filename_path, rule[0],
                               json_dump_normalize(json_output, True),
                               "JSON output")
                # prevent further warnings and .got file updates
                json_expected = json_output
            elif json_expected and json_output != json_expected:
                print_differences_warning(filename, lineno,
                                          json_expected, json_output, cmd)
                error += 1

    return [ret, warning, error, unit_tests]


def cleanup_on_exit():
    for table in table_list:
        for table_chain in table.chains:
            chain = chain_get_by_name(table_chain)
            chain_delete(chain, table, "", "")
        if all_set:
            set_delete(table)
        if obj_list:
            obj_delete(table)
        table_delete(table)


def signal_handler(signal, frame):
    global signal_received
    signal_received = 1


def execute_cmd(cmd, filename, lineno, stdout_log=False, debug=False):
    '''
    Executes a command, checks for segfaults and returns the command exit
    code.

    :param cmd: string with the command to be executed
    :param filename: name of the file tested (used for print_error purposes)
    :param lineno: line number being tested (used for print_error purposes)
    :param stdout_log: redirect stdout to this file instead of global log_file
    :param debug: temporarily set these debug flags
    '''
    global log_file
    print("command: {}".format(cmd), file=log_file)
    if debug_option:
        print(cmd)

    log_file.flush()

    if debug:
        debug_old = nftables.get_debug()
        nftables.set_debug(debug)

    ret, out, err = nftables.cmd(cmd)

    if not stdout_log:
        stdout_log = log_file

    stdout_log.write(out)
    stdout_log.flush()
    log_file.write(err)
    log_file.flush()

    if debug:
        nftables.set_debug(debug_old)

    return ret


def print_result(filename, tests, warning, error):
    return str(filename) + ": " + str(tests) + " unit tests, " + str(error) + \
           " error, " + str(warning) + " warning"


def print_result_all(filename, tests, warning, error, unit_tests):
    return str(filename) + ": " + str(tests) + " unit tests, " + \
           str(unit_tests) + " total test executed, " + str(error) + \
           " error, " + str(warning) + " warning"


def table_process(table_line, filename, lineno):
    table_info = table_line.split(";")
    table = Table(table_info[0], table_info[1], table_info[2].split(","))

    return table_create(table, filename, lineno)


def chain_process(chain_line, lineno):
    chain_info = chain_line.split(";")
    chain_list.append(Chain(chain_info[0], chain_info[1], lineno))

    return 0


def set_process(set_line, filename, lineno):
    test_result = set_line[1]
    timeout=""

    tokens = set_line[0].split(" ")
    set_name = tokens[0]
    parse_typeof = tokens[1] == "typeof"
    set_type = tokens[1] + " " + tokens[2]
    set_data = ""
    set_flags = ""

    i = 3
    if parse_typeof and tokens[i] == "id":
        set_type += " " + tokens[i]
        i += 1;

    while len(tokens) > i and tokens[i] == ".":
        set_type += " . " + tokens[i+1]
        i += 2

    while len(tokens) > i and tokens[i] == ":":
        set_data = tokens[i+1]
        i += 2

    while len(tokens) > i and tokens[i] == ".":
        set_data += " . " + tokens[i+1]
        i += 2

    if parse_typeof and tokens[i] == "mark":
        set_data += " " + tokens[i]
        i += 1;

    if len(tokens) == i+2 and tokens[i] == "timeout":
        timeout = "timeout " + tokens[i+1] + ";"
        i += 2

    if len(tokens) == i+2 and tokens[i] == "flags":
        set_flags = tokens[i+1]
    elif len(tokens) != i:
        print_error(set_name + " bad flag: " + tokens[i], filename, lineno)

    s = Set("", "", set_name, set_type, set_data, timeout, set_flags)

    if set_data == "":
        ret = set_add(s, test_result, filename, lineno)
    else:
        ret = map_add(s, test_result, filename, lineno)

    if ret == 0:
        all_set[set_name] = set()

    return ret


def set_element_process(element_line, filename, lineno):
    rule_state = element_line[1]
    element_line = element_line[0]
    space = element_line.find(" ")
    set_name = element_line[:space]
    set_element = element_line[space:].split(",")

    return set_add_elements(set_element, set_name, rule_state, filename, lineno)


def obj_process(obj_line, filename, lineno):
    test_result = obj_line[1]

    tokens = obj_line[0].split(" ")
    obj_name = tokens[0]
    obj_type = tokens[2]
    obj_spcf = ""

    if obj_type == "ct" and tokens[3] == "helper":
       obj_type = "ct helper"
       tokens[3] = ""

    if obj_type == "ct" and tokens[3] == "timeout":
       obj_type = "ct timeout"
       tokens[3] = ""

    if obj_type == "ct" and tokens[3] == "expectation":
       obj_type = "ct expectation"
       tokens[3] = ""

    if len(tokens) > 3:
        obj_spcf = " ".join(tokens[3:])

    o = Obj("", "", obj_name, obj_type, obj_spcf)

    ret = obj_add(o, test_result, filename, lineno)
    if ret == 0:
        obj_list.append(o)

    return ret


def payload_find_expected(payload_log, rule):
    '''
    Find the netlink payload that should be generated by given rule in
    payload_log

    :param payload_log: open file handle of the payload data
    :param rule: nft rule we are going to add
    '''
    found = 0
    payload_buffer = []

    while True:
        line = payload_log.readline()
        if not line:
            break

        if line[0] == "#":  # rule start
            rule_line = line.strip()[2:]

            if rule_line == rule.strip():
                found = 1
                continue

        if found == 1:
            payload_buffer.append(line)
            if line.isspace():
                return payload_buffer

    payload_log.seek(0, 0)
    return payload_buffer


def json_find_expected(json_log, rule):
    '''
    Find the corresponding JSON for given rule

    :param json_log: open file handle of the json data
    :param rule: nft rule we are going to add
    '''
    found = 0
    json_buffer = ""

    while True:
        line = json_log.readline()
        if not line:
            break

        if line[0] == "#":  # rule start
            rule_line = line.strip()[2:]

            if rule_line == rule.strip():
                found = 1
                continue

        if found == 1:
            json_buffer += line.rstrip("\n").strip()
            if line.isspace():
                return json_buffer

    json_log.seek(0, 0)
    return json_buffer


def run_test_file(filename, force_all_family_option, specific_file):
    '''
    Runs a test file

    :param filename: name of the file with the test rules
    '''
    filename_path = os.path.join(TESTS_PATH, filename)
    f = open(filename_path)
    tests = passed = total_unit_run = total_warning = total_error = 0

    for lineno, line in enumerate(f):
        sys.stdout.flush()

        if signal_received == 1:
            print("\nSignal received. Cleaning up and Exitting...")
            cleanup_on_exit()
            sys.exit(0)

        if line.isspace():
            continue

        if line[0] == "#":  # Command-line
            continue

        if line[0] == '*':  # Table
            table_line = line.rstrip()[1:]
            ret = table_process(table_line, filename, lineno)
            if ret != 0:
                break
            continue

        if line[0] == ":":  # Chain
            chain_line = line.rstrip()[1:]
            ret = chain_process(chain_line, lineno)
            if ret != 0:
                break
            continue

        if line[0] == "!":  # Adds this set
            set_line = line.rstrip()[1:].split(";")
            ret = set_process(set_line, filename, lineno)
            tests += 1
            if ret == -1:
                continue
            passed += 1
            continue

        if line[0] == "?":  # Adds elements in a set
            element_line = line.rstrip()[1:].split(";")
            ret = set_element_process(element_line, filename, lineno)
            tests += 1
            if ret == -1:
                continue

            passed += 1
            continue

        if line[0] == "%":  # Adds this object
            brace = line.rfind("}")
            if brace < 0:
                obj_line = line.rstrip()[1:].split(";")
            else:
                obj_line = (line[1:brace+1], line[brace+2:].rstrip())

            ret = obj_process(obj_line, filename, lineno)
            tests += 1
            if ret == -1:
                continue
            passed += 1
            continue

        # Rule
        rule = line.split(';')  # rule[1] Ok or FAIL
        if len(rule) == 1 or len(rule) > 3 or rule[1].rstrip() \
                not in {"ok", "fail"}:
            reason = "Skipping malformed rule test. (%s)" % line.rstrip('\n')
            print_warning(reason, filename, lineno)
            continue

        if line[0] == "-":  # Run omitted lines
            if need_fix_option:
                rule[0] = rule[0].rstrip()[1:].strip()
            else:
                continue
        elif need_fix_option:
            continue

        result = rule_add(rule, filename, lineno, force_all_family_option,
                          filename_path)
        tests += 1
        ret = result[0]
        warning = result[1]
        total_warning += warning
        total_error += result[2]
        total_unit_run += result[3]

        if ret != 0:
            continue

        if warning == 0:  # All ok.
            passed += 1

    # Delete rules, sets, chains and tables
    for table in table_list:
        # We delete chains
        for table_chain in table.chains:
            chain = chain_get_by_name(table_chain)
            chain_delete(chain, table, filename, lineno)

        # We delete sets.
        if all_set:
            ret = set_delete(table, filename, lineno)
            if ret != 0:
                reason = "There is a problem when we delete a set"
                print_error(reason, filename, lineno)

        # We delete tables.
        table_delete(table, filename, lineno)

    if specific_file:
        if force_all_family_option:
            print(print_result_all(filename, tests, total_warning, total_error,
                                   total_unit_run))
        else:
            print(print_result(filename, tests, total_warning, total_error))
    else:
        if tests == passed and tests > 0:
            print(filename + ": " + Colors.GREEN + "OK" + Colors.ENDC)

    f.close()
    del table_list[:]
    del chain_list[:]
    all_set.clear()

    return [tests, passed, total_warning, total_error, total_unit_run]

def spawn_netns():
    # prefer unshare module
    try:
        import unshare
        unshare.unshare(unshare.CLONE_NEWNET)
        return True
    except:
        pass

    # sledgehammer style:
    # - call ourselves prefixed by 'unshare -n' if found
    # - pass extra --no-netns parameter to avoid another recursion
    try:
        import shutil

        unshare = shutil.which("unshare")
        if unshare is None:
            return False

        sys.argv.append("--no-netns")
        if debug_option:
            print("calling: ", [unshare, "-n", sys.executable] + sys.argv)
        os.execv(unshare, [unshare, "-n", sys.executable] + sys.argv)
    except:
        pass

    return False

def main():
    parser = argparse.ArgumentParser(description='Run nft tests')

    parser.add_argument('filenames', nargs='*', metavar='path/to/file.t',
                        help='Run only these tests')

    parser.add_argument('-d', '--debug', action='store_true', dest='debug',
                        help='enable debugging mode')

    parser.add_argument('-e', '--need-fix', action='store_true',
                        dest='need_fix_line', help='run rules that need a fix')

    parser.add_argument('-f', '--force-family', action='store_true',
                        dest='force_all_family',
                        help='keep testing all families on error')

    parser.add_argument('-H', '--host', action='store_true',
                        help='run tests against installed libnftables.so.1')

    parser.add_argument('-j', '--enable-json', action='store_true',
                        dest='enable_json',
                        help='test JSON functionality as well (default)')

    parser.add_argument('-J', '--disable-json', action='store_true',
                        dest='disable_json',
                        help='Do not test JSON functionality as well')

    parser.add_argument('-l', '--library', default=None,
                        help='path to libntables.so.1, overrides --host')

    parser.add_argument('-N', '--no-netns', action='store_true',
                        dest='no_netns',
                        help='Do not run in own network namespace')

    parser.add_argument('-s', '--schema', action='store_true',
                        dest='enable_schema',
                        help='verify json input/output against schema (default)')

    parser.add_argument('-S', '--no-schema', action='store_true',
                        dest='disable_schema',
                        help='Do not verify json input/output against schema')

    parser.add_argument('-v', '--version', action='version',
                        version='1.0',
                        help='Print the version information')

    args = parser.parse_args()
    global debug_option, need_fix_option, enable_json_option, enable_json_schema
    debug_option = args.debug
    need_fix_option = args.need_fix_line
    force_all_family_option = args.force_all_family
    enable_json_option = not args.disable_json
    enable_json_schema = not args.disable_json and not args.disable_schema
    specific_file = False

    signal.signal(signal.SIGINT, signal_handler)
    signal.signal(signal.SIGTERM, signal_handler)

    if os.getuid() != 0:
        print("You need to be root to run this, sorry")
        return 77

    if not args.no_netns and not spawn_netns():
        print_warning("cannot run in own namespace, connectivity might break")

    # Change working directory to repository root
    os.chdir(TESTS_PATH + "/../..")

    check_lib_path = True
    if args.library is None:
        if args.host:
            args.library = 'libnftables.so.1'
            check_lib_path = False
        else:
            args.library = 'src/.libs/libnftables.so.1'

    if check_lib_path and not os.path.exists(args.library):
        print("The nftables library at '%s' does not exist. "
              "You need to build the project." % args.library)
        return 99

    if args.enable_schema and not args.enable_json:
        print_error("Option --schema requires option --json")
        return 99

    global nftables
    nftables = Nftables(sofile = args.library)

    test_files = files_ok = run_total = 0
    tests = passed = warnings = errors = 0
    global log_file
    try:
        log_file = open(LOGFILE, 'w')
        print_info("Log will be available at %s" % LOGFILE)
    except IOError:
        print_error("Cannot open log file %s" % LOGFILE)
        return 99

    file_list = []
    if args.filenames:
        file_list = args.filenames
        if len(args.filenames) == 1:
            specific_file = True
    else:
        for directory in TESTS_DIRECTORY:
            path = os.path.join(TESTS_PATH, directory)
            for root, dirs, files in os.walk(path):
                for f in files:
                    if f.endswith(".t"):
                        file_list.append(os.path.join(directory, f))

    for filename in file_list:
        result = run_test_file(filename, force_all_family_option, specific_file)
        file_tests = result[0]
        file_passed = result[1]
        file_warnings = result[2]
        file_errors = result[3]
        file_unit_run = result[4]

        test_files += 1

        if file_warnings == 0 and file_tests == file_passed:
            files_ok += 1
        if file_tests:
            tests += file_tests
            passed += file_passed
            errors += file_errors
            warnings += file_warnings
        if force_all_family_option:
            run_total += file_unit_run

    if test_files == 0:
        print("No test files to run")
    else:
        if not specific_file:
            if force_all_family_option:
                print("%d test files, %d files passed, %d unit tests, " % (test_files, files_ok, tests))
                print("%d total executed, %d error, %d warning" % (run_total, errors,warnings))
            else:
                print("%d test files, %d files passed, %d unit tests, " % (test_files, files_ok, tests))
                print("%d error, %d warning" % (errors, warnings))

    return errors != 0

if __name__ == '__main__':
    sys.exit(main())
