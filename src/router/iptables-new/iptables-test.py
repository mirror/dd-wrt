#!/usr/bin/env python
#
# (C) 2012-2013 by Pablo Neira Ayuso <pablo@netfilter.org>
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This software has been sponsored by Sophos Astaro <http://www.sophos.com>
#

from __future__ import print_function
import sys
import os
import subprocess
import argparse

IPTABLES = "iptables"
IP6TABLES = "ip6tables"
ARPTABLES = "arptables"
EBTABLES = "ebtables"

IPTABLES_SAVE = "iptables-save"
IP6TABLES_SAVE = "ip6tables-save"
ARPTABLES_SAVE = "arptables-save"
EBTABLES_SAVE = "ebtables-save"
#IPTABLES_SAVE = ['xtables-save','-4']
#IP6TABLES_SAVE = ['xtables-save','-6']

EXTENSIONS_PATH = "extensions"
LOGFILE="/tmp/iptables-test.log"
log_file = None


class Colors:
    HEADER = '\033[95m'
    BLUE = '\033[94m'
    GREEN = '\033[92m'
    YELLOW = '\033[93m'
    RED = '\033[91m'
    ENDC = '\033[0m'


def print_error(reason, filename=None, lineno=None):
    '''
    Prints an error with nice colors, indicating file and line number.
    '''
    print(filename + ": " + Colors.RED + "ERROR" +
        Colors.ENDC + ": line %d (%s)" % (lineno, reason))


def delete_rule(iptables, rule, filename, lineno):
    '''
    Removes an iptables rule
    '''
    cmd = iptables + " -D " + rule
    ret = execute_cmd(cmd, filename, lineno)
    if ret == 1:
        reason = "cannot delete: " + iptables + " -I " + rule
        print_error(reason, filename, lineno)
        return -1

    return 0


def run_test(iptables, rule, rule_save, res, filename, lineno, netns):
    '''
    Executes an unit test. Returns the output of delete_rule().

    Parameters:
    :param  iptables: string with the iptables command to execute
    :param rule: string with iptables arguments for the rule to test
    :param rule_save: string to find the rule in the output of iptables -save
    :param res: expected result of the rule. Valid values: "OK", "FAIL"
    :param filename: name of the file tested (used for print_error purposes)
    :param lineno: line number being tested (used for print_error purposes)
    '''
    ret = 0

    cmd = iptables + " -A " + rule
    if netns:
            cmd = "ip netns exec ____iptables-container-test " + EXECUTEABLE + " " + cmd

    ret = execute_cmd(cmd, filename, lineno)

    #
    # report failed test
    #
    if ret:
        if res == "OK":
            reason = "cannot load: " + cmd
            print_error(reason, filename, lineno)
            return -1
        else:
            # do not report this error
            return 0
    else:
        if res == "FAIL":
            reason = "should fail: " + cmd
            print_error(reason, filename, lineno)
            delete_rule(iptables, rule, filename, lineno)
            return -1

    matching = 0
    splitted = iptables.split(" ")
    if len(splitted) == 2:
        if splitted[1] == '-4':
            command = IPTABLES_SAVE
        elif splitted[1] == '-6':
            command = IP6TABLES_SAVE
    elif len(splitted) == 1:
        if splitted[0] == IPTABLES:
            command = IPTABLES_SAVE
        elif splitted[0] == IP6TABLES:
            command = IP6TABLES_SAVE
        elif splitted[0] == ARPTABLES:
            command = ARPTABLES_SAVE
        elif splitted[0] == EBTABLES:
            command = EBTABLES_SAVE

    command = EXECUTEABLE + " " + command

    if netns:
            command = "ip netns exec ____iptables-container-test " + command

    args = splitted[1:]
    proc = subprocess.Popen(command, shell=True,
                            stdin=subprocess.PIPE,
                            stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    out, err = proc.communicate()

    #
    # check for segfaults
    #
    if proc.returncode == -11:
        reason = "iptables-save segfaults: " + cmd
        print_error(reason, filename, lineno)
        delete_rule(iptables, rule, filename, lineno)
        return -1

    # find the rule
    matching = out.find(rule_save.encode('utf-8'))
    if matching < 0:
        reason = "cannot find: " + iptables + " -I " + rule
        print_error(reason, filename, lineno)
        delete_rule(iptables, rule, filename, lineno)
        return -1

    # Test "ip netns del NETNS" path with rules in place
    if netns:
        return 0

    return delete_rule(iptables, rule, filename, lineno)

def execute_cmd(cmd, filename, lineno):
    '''
    Executes a command, checking for segfaults and returning the command exit
    code.

    :param cmd: string with the command to be executed
    :param filename: name of the file tested (used for print_error purposes)
    :param lineno: line number being tested (used for print_error purposes)
    '''
    global log_file
    if cmd.startswith('iptables ') or cmd.startswith('ip6tables ') or cmd.startswith('ebtables ') or cmd.startswith('arptables '):
        cmd = EXECUTEABLE + " " + cmd

    print("command: {}".format(cmd), file=log_file)
    ret = subprocess.call(cmd, shell=True, universal_newlines=True,
        stderr=subprocess.STDOUT, stdout=log_file)
    log_file.flush()

    # generic check for segfaults
    if ret  == -11:
        reason = "command segfaults: " + cmd
        print_error(reason, filename, lineno)
    return ret


def run_test_file(filename, netns):
    '''
    Runs a test file

    :param filename: name of the file with the test rules
    '''
    #
    # if this is not a test file, skip.
    #
    if not filename.endswith(".t"):
        return 0, 0

    if "libipt_" in filename:
        iptables = IPTABLES
    elif "libip6t_" in filename:
        iptables = IP6TABLES
    elif "libxt_"  in filename:
        iptables = IPTABLES
    elif "libarpt_" in filename:
        # only supported with nf_tables backend
        if EXECUTEABLE != "xtables-nft-multi":
           return 0, 0
        iptables = ARPTABLES
    elif "libebt_" in filename:
        # only supported with nf_tables backend
        if EXECUTEABLE != "xtables-nft-multi":
           return 0, 0
        iptables = EBTABLES
    else:
        # default to iptables if not known prefix
        iptables = IPTABLES

    f = open(filename)

    tests = 0
    passed = 0
    table = ""
    total_test_passed = True

    if netns:
        execute_cmd("ip netns add ____iptables-container-test", filename, 0)

    for lineno, line in enumerate(f):
        if line[0] == "#" or len(line.strip()) == 0:
            continue

        if line[0] == ":":
            chain_array = line.rstrip()[1:].split(",")
            continue

        # external non-iptables invocation, executed as is.
        if line[0] == "@":
            external_cmd = line.rstrip()[1:]
            if netns:
                external_cmd = "ip netns exec ____iptables-container-test " + external_cmd
            execute_cmd(external_cmd, filename, lineno)
            continue

        # external iptables invocation, executed as is.
        if line[0] == "%":
            external_cmd = line.rstrip()[1:]
            if netns:
                external_cmd = "ip netns exec ____iptables-container-test " + EXECUTEABLE + " " + external_cmd
            execute_cmd(external_cmd, filename, lineno)
            continue

        if line[0] == "*":
            table = line.rstrip()[1:]
            continue

        if len(chain_array) == 0:
            print("broken test, missing chain, leaving")
            sys.exit()

        test_passed = True
        tests += 1

        for chain in chain_array:
            item = line.split(";")
            if table == "":
                rule = chain + " " + item[0]
            else:
                rule = chain + " -t " + table + " " + item[0]

            if item[1] == "=":
                rule_save = chain + " " + item[0]
            else:
                rule_save = chain + " " + item[1]

            res = item[2].rstrip()
            ret = run_test(iptables, rule, rule_save,
                           res, filename, lineno + 1, netns)

            if ret < 0:
                test_passed = False
                total_test_passed = False
                break

        if test_passed:
            passed += 1

    if netns:
        execute_cmd("ip netns del ____iptables-container-test", filename, 0)
    if total_test_passed:
        print(filename + ": " + Colors.GREEN + "OK" + Colors.ENDC)

    f.close()
    return tests, passed


def show_missing():
    '''
    Show the list of missing test files
    '''
    file_list = os.listdir(EXTENSIONS_PATH)
    testfiles = [i for i in file_list if i.endswith('.t')]
    libfiles = [i for i in file_list
                if i.startswith('lib') and i.endswith('.c')]

    def test_name(x):
        return x[0:-2] + '.t'
    missing = [test_name(i) for i in libfiles
               if not test_name(i) in testfiles]

    print('\n'.join(missing))


#
# main
#
def main():
    parser = argparse.ArgumentParser(description='Run iptables tests')
    parser.add_argument('filename', nargs='?',
                        metavar='path/to/file.t',
                        help='Run only this test')
    parser.add_argument('-H', '--host', action='store_true',
                        help='Run tests against installed binaries')
    parser.add_argument('-l', '--legacy', action='store_true',
                        help='Test iptables-legacy')
    parser.add_argument('-m', '--missing', action='store_true',
                        help='Check for missing tests')
    parser.add_argument('-n', '--nftables', action='store_true',
                        help='Test iptables-over-nftables')
    parser.add_argument('-N', '--netns', action='store_true',
                        help='Test netnamespace path')
    args = parser.parse_args()

    #
    # show list of missing test files
    #
    if args.missing:
        show_missing()
        return

    global EXECUTEABLE
    EXECUTEABLE = "xtables-legacy-multi"
    if args.nftables:
        EXECUTEABLE = "xtables-nft-multi"

    if os.getuid() != 0:
        print("You need to be root to run this, sorry")
        return

    if not args.host:
        os.putenv("XTABLES_LIBDIR", os.path.abspath(EXTENSIONS_PATH))
        os.putenv("PATH", "%s/iptables:%s" % (os.path.abspath(os.path.curdir),
                                              os.getenv("PATH")))

    test_files = 0
    tests = 0
    passed = 0

    # setup global var log file
    global log_file
    try:
        log_file = open(LOGFILE, 'w')
    except IOError:
        print("Couldn't open log file %s" % LOGFILE)
        return

    if args.filename:
        file_list = [args.filename]
    else:
        file_list = [os.path.join(EXTENSIONS_PATH, i)
                     for i in os.listdir(EXTENSIONS_PATH)
                     if i.endswith('.t')]
        file_list.sort()

    for filename in file_list:
        file_tests, file_passed = run_test_file(filename, args.netns)
        if file_tests:
            tests += file_tests
            passed += file_passed
            test_files += 1

    print("%d test files, %d unit tests, %d passed" % (test_files, tests, passed))


if __name__ == '__main__':
    main()
