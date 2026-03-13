#!/usr/bin/env python3
#
# Parallel VM test case executor
# Copyright (c) 2014-2024, Jouni Malinen <j@w1.fi>
#
# This software may be distributed under the terms of the BSD license.
# See README for more details.

from __future__ import print_function
import curses
import fcntl
import logging
import multiprocessing
import os
import selectors
import shutil
import signal
import subprocess
import sys
import time
import errno

logger = logging.getLogger()

# Test cases that take significantly longer time to execute than average.
long_tests = ["ap_roam_open",
              "hostapd_oom_wpa2_eap_connect_1",
              "hostapd_oom_wpa2_eap_connect_2",
              "hostapd_oom_wpa2_eap_connect_3",
              "hostapd_oom_wpa2_eap_connect_4",
              "hostapd_oom_wpa2_eap_connect_5",
              "ap_wpa2_eap_eke_many",
              "wpas_mesh_password_mismatch_retry",
              "wpas_mesh_password_mismatch",
              "hostapd_oom_wpa2_psk_connect",
              "ap_hs20_fetch_osu_stop",
              "ap_roam_wpa2_psk",
              "ibss_wpa_none_ccmp",
              "nfc_wps_er_handover_pk_hash_mismatch_sta",
              "go_neg_peers_force_diff_freq",
              "p2p_cli_invite",
              "sta_ap_scan_2b",
              "ap_pmf_sta_unprot_deauth_burst",
              "ap_bss_add_remove_during_ht_scan",
              "wext_scan_hidden",
              "autoscan_exponential",
              "nfc_p2p_client",
              "wnm_bss_keep_alive",
              "ap_inactivity_disconnect",
              "scan_bss_expiration_age",
              "autoscan_periodic",
              "discovery_group_client",
              "concurrent_p2pcli",
              "ap_bss_add_remove",
              "wpas_ap_wps",
              "wext_pmksa_cache",
              "ibss_wpa_none",
              "ap_ht_40mhz_intolerant_ap",
              "ibss_rsn",
              "discovery_pd_retries",
              "ap_wps_setup_locked_timeout",
              "ap_vht160",
              'he160',
              'he160b',
              "dfs_radar",
              "dfs",
              "dfs_ht40_minus",
              "dfs_etsi",
              "dfs_radar_vht80_downgrade",
              "ap_acs_dfs",
              "grpform_cred_ready_timeout",
              "wpas_ap_dfs",
              "autogo_many",
              "hostapd_oom_wpa2_eap",
              "ibss_open",
              "proxyarp_open_ebtables",
              "proxyarp_open_ebtables_ipv6",
              "radius_failover",
              "obss_scan_40_intolerant",
              "dbus_connect_oom",
              "proxyarp_open",
              "proxyarp_open_ipv6",
              "ap_wps_iteration",
              "ap_wps_iteration_error",
              "ap_wps_pbc_timeout",
              "ap_wps_pbc_ap_timeout",
              "ap_wps_pin_ap_timeout",
              "ap_wps_http_timeout",
              "p2p_go_move_reg_change",
              "p2p_go_move_active",
              "p2p_go_move_scm",
              "p2p_go_move_scm_peer_supports",
              "p2p_go_move_scm_peer_does_not_support",
              "p2p_go_move_scm_multi"]

# Test cases that have common, but random, issues with UML.
uml_issue_tests = ["eht_connect_invalid_link",
                   "eht_mld_connect_probes",
                   "gas_anqp_address3_ap_forced",
                   "gas_anqp_address3_ap_non_compliant",
                   "gas_anqp_address3_not_assoc",
                   "gas_anqp_address3_assoc",
                   "p2p_channel_random_social_with_op_class_change",
                   "ap_open_ps_mc_buf",
                   "wmediumd_path_rann",
                   "mbo_assoc_disallow",
                   "ieee8021x_reauth_wep"]

# Test cases that depend on dumping full process memory
memory_read_tests = ["ap_wpa2_eap_fast_pac_lifetime",
                     "wpa2_eap_ttls_pap_key_lifetime_in_memory",
                     "wpa2_eap_peap_gtc_key_lifetime_in_memory",
                     "ft_psk_key_lifetime_in_memory",
                     "wpa2_psk_key_lifetime_in_memory",
                     "ap_wpa2_tdls_long_lifetime",
                     "ap_wpa2_tdls_wrong_lifetime_resp",
                     "erp_key_lifetime_in_memory",
                     "sae_key_lifetime_in_memory",
                     "sae_okc_pmk_lifetime",
                     "sae_pmk_lifetime",
                     "wpas_ap_lifetime_in_memory",
                     "wpas_ap_lifetime_in_memory2"]

def get_failed(vm):
    failed = []
    for i in range(num_servers):
        failed += vm[i]['failed']
    return failed

def vm_read_stdout(vm, test_queue):
    global total_started, total_passed, total_failed, total_skipped
    global rerun_failures
    global first_run_failures
    global all_failed

    ready = False
    try:
        out = vm['proc'].stdout.read()
        if out == None:
            return False
        out = out.decode()
    except IOError as e:
        if e.errno == errno.EAGAIN:
            return False
        raise
    vm['last_stdout'] = time.time()
    logger.debug("VM[%d] stdout.read[%s]" % (vm['idx'], out.rstrip()))
    pending = vm['pending'] + out
    lines = []
    while True:
        pos = pending.find('\n')
        if pos < 0:
            break
        line = pending[0:pos].rstrip()
        pending = pending[(pos + 1):]
        logger.debug("VM[%d] stdout full line[%s]" % (vm['idx'], line))
        if line.startswith("READY"):
            vm['starting'] = False
            vm['started'] = True
            ready = True
        elif line.startswith("PASS"):
            ready = True
            total_passed += 1
            vm['current_name'] = None
        elif line.startswith("FAIL"):
            ready = True
            total_failed += 1
            vals = line.split(' ')
            if len(vals) < 2:
                logger.info("VM[%d] incomplete FAIL line: %s" % (vm['idx'],
                                                                 line))
                name = line
            else:
                name = vals[1]
            logger.debug("VM[%d] test case failed: %s" % (vm['idx'], name))
            vm['failed'].append(name)
            all_failed.append(name)
            if name != vm['current_name']:
                logger.info("VM[%d] test result mismatch: %s (expected %s)" % (vm['idx'], name, vm['current_name']))
            else:
                vm['current_name'] = None
                count = vm['current_count']
                if count == 0:
                    first_run_failures.append(name)
                if rerun_failures and count < 1:
                    logger.debug("Requeue test case %s" % name)
                    test_queue.append((name, vm['current_count'] + 1))
        elif line.startswith("NOT-FOUND"):
            ready = True
            total_failed += 1
            logger.info("VM[%d] test case not found" % vm['idx'])
        elif line.startswith("SKIP"):
            ready = True
            total_skipped += 1
            vm['current_name'] = None
        elif line.startswith("REASON"):
            vm['skip_reason'].append(line[7:])
        elif line.startswith("START"):
            total_started += 1
            if len(vm['failed']) == 0:
                vals = line.split(' ')
                if len(vals) >= 2:
                    vm['fail_seq'].append(vals[1])
        vm['out'] += line + '\n'
        lines.append(line)
    vm['pending'] = pending
    return ready

def start_vm(vm, sel):
    logger.info("VM[%d] starting up" % (vm['idx'] + 1))
    vm['starting'] = True
    vm['proc'] = subprocess.Popen(vm['cmd'],
                                  stdin=subprocess.PIPE,
                                  stdout=subprocess.PIPE,
                                  stderr=subprocess.PIPE)
    vm['cmd'] = None
    for stream in [vm['proc'].stdout, vm['proc'].stderr]:
        fd = stream.fileno()
        fl = fcntl.fcntl(fd, fcntl.F_GETFL)
        fcntl.fcntl(fd, fcntl.F_SETFL, fl | os.O_NONBLOCK)
        sel.register(stream, selectors.EVENT_READ, vm)

def num_vm_starting():
    count = 0
    for i in range(num_servers):
        if vm[i]['starting']:
            count += 1
    return count

def vm_read_stderr(vm):
    try:
        err = vm['proc'].stderr.read()
        if err != None:
            err = err.decode()
            if len(err) > 0:
                vm['err'] += err
                logger.info("VM[%d] stderr.read[%s]" % (vm['idx'], err))
    except IOError as e:
        if e.errno != errno.EAGAIN:
            raise

def vm_next_step(_vm, scr, test_queue):
    max_y, max_x = scr.getmaxyx()
    status_line = num_servers
    if status_line >= max_y:
        status_line = max_y - 1
    if _vm['idx'] < status_line:
        scr.move(_vm['idx'], 10)
        scr.clrtoeol()
    if not test_queue:
        _vm['proc'].stdin.write(b'\n')
        _vm['proc'].stdin.flush()
        if _vm['idx'] < status_line:
            scr.addstr("shutting down")
        logger.info("VM[%d] shutting down" % _vm['idx'])
        return
    (name, count) = test_queue.pop(0)
    _vm['current_name'] = name
    _vm['current_count'] = count
    _vm['proc'].stdin.write(name.encode() + b'\n')
    _vm['proc'].stdin.flush()
    if _vm['idx'] < status_line:
        scr.addstr(name)
    logger.debug("VM[%d] start test %s" % (_vm['idx'], name))

def check_vm_start(scr, sel, test_queue):
    running = False
    max_y, max_x = scr.getmaxyx()
    status_line = num_servers
    if status_line >= max_y:
        status_line = max_y - 1
    for i in range(num_servers):
        if vm[i]['proc']:
            running = True
            continue

        # Either not yet started or already stopped VM
        max_start = multiprocessing.cpu_count()
        if max_start > 4:
            max_start /= 2
        num_starting = num_vm_starting()
        if vm[i]['cmd'] and len(test_queue) > num_starting and \
           num_starting < max_start:
            if i < status_line:
                scr.move(i, 10)
                scr.clrtoeol()
                scr.addstr(i, 10, "starting VM")
            start_vm(vm[i], sel)
            return True, True

    return running, False

def vm_terminated(_vm, scr, sel, test_queue):
    logger.debug("VM[%s] terminated" % _vm['idx'])
    updated = False
    for stream in [_vm['proc'].stdout, _vm['proc'].stderr]:
        sel.unregister(stream)
    _vm['proc'] = None
    max_y, max_x = scr.getmaxyx()
    status_line = num_servers
    if status_line >= max_y:
        status_line = max_y - 1
    if _vm['idx'] < status_line:
        scr.move(_vm['idx'], 10)
        scr.clrtoeol()
    log = '{}/{}.srv.{}/console'.format(dir, timestamp, _vm['idx'] + 1)
    with open(log, 'r') as f:
        if "Kernel panic" in f.read():
            if _vm['idx'] < status_line:
                scr.addstr("kernel panic")
            logger.info("VM[%d] kernel panic" % _vm['idx'])
            updated = True
    if test_queue:
        num_vm = 0
        for i in range(num_servers):
            if vm[i]['proc']:
                num_vm += 1
        if len(test_queue) > num_vm:
            if _vm['idx'] < status_line:
                scr.addstr("unexpected exit")
            logger.info("VM[%d] unexpected exit" % _vm['idx'])
            updated = True

    if _vm['current_name']:
        global total_failed, all_failed, first_run_failures, rerun_failures
        name = _vm['current_name']
        if _vm['idx'] < status_line:
            if updated:
                scr.addstr(" - ")
            scr.addstr(name + " - did not complete")
        logger.info("VM[%d] did not complete test %s" % (_vm['idx'], name))
        total_failed += 1
        _vm['failed'].append(name)
        all_failed.append(name)

        count = _vm['current_count']
        if count == 0:
            first_run_failures.append(name)
        if rerun_failures and count < 1:
            logger.debug("Requeue test case %s" % name)
            test_queue.append((name, count + 1))
        updated = True

    return updated

def update_screen(scr, total_tests):
    max_y, max_x = scr.getmaxyx()
    status_line = num_servers
    if status_line >= max_y:
        status_line = max_y - 1
    scr.move(status_line, 10)
    scr.clrtoeol()
    scr.addstr("{} %".format(int(100.0 * (total_passed + total_failed + total_skipped) / total_tests)))
    scr.addstr(status_line, 20,
               "TOTAL={} STARTED={} PASS={} FAIL={} SKIP={}".format(total_tests, total_started, total_passed, total_failed, total_skipped))
    global all_failed
    max_y, max_x = scr.getmaxyx()
    max_lines = max_y - num_servers - 3
    if len(all_failed) > 0 and max_lines > 0 and num_servers + 1 < max_y - 1:
        scr.move(num_servers + 1, 0)
        scr.addstr("Last failed test cases:")
        if max_lines >= len(all_failed):
            max_lines = len(all_failed)
        count = 0
        for i in range(len(all_failed) - max_lines, len(all_failed)):
            count += 1
            if num_servers + 2 + count >= max_y:
                break
            scr.move(num_servers + 1 + count, 0)
            scr.addstr(all_failed[i])
            scr.clrtoeol()
    scr.refresh()

def has_uml_mconsole(_vm):
    if not shutil.which('uml_mconsole'):
        return False
    dir = os.path.join(os.path.expanduser('~/.uml'), 'hwsim-' + _vm['DATE'])
    return os.path.exists(dir)

def kill_uml_vm(_vm):
    fname = os.path.join(os.path.expanduser('~/.uml'), 'hwsim-' + _vm['DATE'],
                         'pid')
    with open(fname, 'r') as f:
        pid = int(f.read())
    logger.info("Kill hung VM[%d] PID[%d]" % (_vm['idx'] + 1, pid))
    try:
        subprocess.call(['uml_mconsole', 'hwsim-' + _vm['DATE'],
                         'halt'], stdout=open('/dev/null', 'w'),
                        timeout=5)
    except subprocess.TimeoutExpired:
        logger.info("uml_console timed out - kill UML process")
        try:
            os.kill(pid, signal.SIGTERM)
        except Exception as e:
            logger.info("os.kill() failed: " + str(e))

def show_progress(scr):
    global num_servers
    global vm
    global dir
    global timestamp
    global tests
    global first_run_failures
    global total_started, total_passed, total_failed, total_skipped
    global rerun_failures

    sel = selectors.DefaultSelector()
    total_tests = len(tests)
    logger.info("Total tests: %d" % total_tests)
    test_queue = [(t, 0) for t in tests]
    start_vm(vm[0], sel)

    scr.leaveok(1)
    max_y, max_x = scr.getmaxyx()
    status_line = num_servers
    if status_line > max_y:
        status_line = max_y
    for i in range(0, num_servers):
        if i < status_line:
            scr.addstr(i, 0, "VM %d:" % (i + 1), curses.A_BOLD)
            status = "starting VM" if vm[i]['proc'] else "not yet started"
            scr.addstr(i, 10, status)
    scr.addstr(status_line, 0, "Total:", curses.A_BOLD)
    scr.addstr(status_line, 20, "TOTAL={} STARTED=0 PASS=0 FAIL=0 SKIP=0".format(total_tests))
    scr.refresh()

    while True:
        updated = False
        events = sel.select(timeout=1)
        for key, mask in events:
            _vm = key.data
            if not _vm['proc']:
                continue
            vm_read_stderr(_vm)
            if vm_read_stdout(_vm, test_queue):
                vm_next_step(_vm, scr, test_queue)
                updated = True
            vm_read_stderr(_vm)
            if _vm['proc'].poll() is not None:
                vm_terminated(_vm, scr, sel, test_queue)
                updated = True

        running, run_update = check_vm_start(scr, sel, test_queue)
        if updated or run_update:
            update_screen(scr, total_tests)
        if not running:
            break

        status_line = num_servers
        if status_line >= max_y:
            status_line = max_y - 1
        max_y, max_x = scr.getmaxyx()
        updated = False

        now = time.time()
        for i in range(num_servers):
            _vm = vm[i]
            if not _vm['proc']:
                continue
            last = _vm['last_stdout']
            if last and now - last > 10:
                if _vm['idx'] < status_line:
                    scr.move(_vm['idx'], max_x - 25)
                    scr.clrtoeol()
                    scr.addstr("(no update in %d s)" % (now - last))
                    updated = True
            if (not _vm['killed']) and has_uml_mconsole(_vm) and \
               last and now - last > 120:
                if _vm['idx'] < status_line:
                    scr.move(_vm['idx'], 10)
                    scr.clrtoeol()
                    scr.addstr("terminating due to no updates received")
                _vm['killed'] = True
                kill_uml_vm(_vm)
        if updated:
            scr.refresh()

    sel.close()

    for i in range(num_servers):
        if not vm[i]['proc']:
            continue
        vm[i]['proc'] = None
        scr.move(i, 10)
        scr.clrtoeol()
        scr.addstr("still running")
        logger.info("VM[%d] still running" % i)

    scr.refresh()
    time.sleep(0.3)

def known_output(tests, line):
    if not line:
        return True
    if line in tests:
        return True
    known = ["START ", "PASS ", "FAIL ", "SKIP ", "REASON ", "ALL-PASSED",
             "READY",
             "  ", "Exception: ", "Traceback (most recent call last):",
             "./run-all.sh: running",
             "./run-all.sh: passing",
             "Test run completed", "Logfiles are at", "Starting test run",
             "passed all", "skipped ", "failed tests:"]
    for k in known:
        if line.startswith(k):
            return True
    return False

def main():
    import argparse
    import os
    global num_servers
    global vm
    global all_failed
    global dir
    global timestamp
    global tests
    global first_run_failures
    global total_started, total_passed, total_failed, total_skipped
    global rerun_failures

    total_started = 0
    total_passed = 0
    total_failed = 0
    total_skipped = 0

    debug_level = logging.INFO
    rerun_failures = True
    start_time = time.time()
    timestamp = int(start_time)

    scriptsdir = os.path.dirname(os.path.realpath(sys.argv[0]))

    p = argparse.ArgumentParser(description='run multiple testing VMs in parallel')
    p.add_argument('num_servers', metavar='number of VMs', type=int, choices=range(1, 100),
                   help="number of VMs to start")
    p.add_argument('-f', dest='testmodules', metavar='<test module>',
                   help='execute only tests from these test modules',
                   type=str, nargs='+')
    p.add_argument('-1', dest='no_retry', action='store_const', const=True, default=False,
                   help="don't retry failed tests automatically")
    p.add_argument('--debug', dest='debug', action='store_const', const=True, default=False,
                   help="enable debug logging")
    p.add_argument('--codecov', dest='codecov', action='store_const', const=True, default=False,
                   help="enable code coverage collection")
    p.add_argument('--shuffle-tests', dest='shuffle', action='store_const', const=True, default=False,
                   help="shuffle test cases to randomize order")
    p.add_argument('--short', dest='short', action='store_const', const=True,
                   default=False,
                   help="only run short-duration test cases")
    p.add_argument('--long', dest='long', action='store_const', const=True,
                   default=False,
                   help="include long-duration test cases")
    p.add_argument('--valgrind', dest='valgrind', action='store_const',
                   const=True, default=False,
                   help="run tests under valgrind")
    p.add_argument('--telnet', dest='telnet', metavar='<baseport>', type=int,
                   help="enable telnet server inside VMs, specify the base port here")
    p.add_argument('--nocurses', dest='nocurses', action='store_const',
                   const=True, default=False, help="Don't use curses for output")
    p.add_argument('params', nargs='*')
    args = p.parse_args()

    dir = os.environ.get('HWSIM_TEST_LOG_DIR', '/tmp/hwsim-test-logs')
    try:
        os.makedirs(dir)
    except OSError as e:
        if e.errno != errno.EEXIST:
            raise

    num_servers = args.num_servers
    rerun_failures = not args.no_retry
    if args.debug:
        debug_level = logging.DEBUG
    extra_args = []
    if args.valgrind:
        extra_args += ['--valgrind']
    if args.long:
        extra_args += ['--long']
    if args.codecov:
        print("Code coverage - build separate binaries")
        logdir = os.path.join(dir, str(timestamp))
        os.makedirs(logdir)
        subprocess.check_call([os.path.join(scriptsdir, 'build-codecov.sh'),
                               logdir])
        codecov_args = ['--codecov_dir', logdir]
        codecov = True
    else:
        codecov_args = []
        codecov = False

    first_run_failures = []
    if args.params:
        tests = args.params
    else:
        tests = []
        cmd = [os.path.join(os.path.dirname(scriptsdir), 'run-tests.py'), '-L']
        if args.testmodules:
            cmd += ["-f"]
            cmd += args.testmodules
        lst = subprocess.Popen(cmd, stdout=subprocess.PIPE)
        for l in lst.stdout.readlines():
            name = l.decode().split(' ')[0]
            tests.append(name)
    if len(tests) == 0:
        sys.exit("No test cases selected")

    if args.shuffle:
        from random import shuffle
        shuffle(tests)
    elif num_servers > 2 and len(tests) > 100:
        # Move test cases with long duration to the beginning as an
        # optimization to avoid last part of the test execution running a long
        # duration test case on a single VM while all other VMs have already
        # completed their work.
        for l in long_tests:
            if l in tests:
                tests.remove(l)
                tests.insert(0, l)

        # Move test cases that have shown frequent, but random, issues UML
        # to the beginning of the run to minimize risk of false failures.
        for l in uml_issue_tests:
            if l in tests:
                tests.remove(l)
                tests.insert(0, l)

        # Test cases that read full wpa_supplicant or hostapd process memory
        # can apparently cause resources issues at least with newer python3
        # versions, so run them first before possible memory fragmentation has
        # made this need more resources.
        for l in memory_read_tests:
            if l in tests:
                tests.remove(l)
                tests.insert(0, l)
    if args.short:
        tests = [t for t in tests if t not in long_tests]

    logger.setLevel(debug_level)
    if not args.nocurses:
        log_handler = logging.FileHandler('parallel-vm.log')
    else:
        log_handler = logging.StreamHandler(sys.stdout)
    log_handler.setLevel(debug_level)
    fmt = "%(asctime)s %(levelname)s %(message)s"
    log_formatter = logging.Formatter(fmt)
    log_handler.setFormatter(log_formatter)
    logger.addHandler(log_handler)

    all_failed = []
    vm = {}
    for i in range(0, num_servers):
        cmd = [os.path.join(scriptsdir, 'vm-run.sh'),
               '--timestamp', str(timestamp),
               '--ext', 'srv.%d' % (i + 1),
               '-i'] + codecov_args + extra_args
        if args.telnet:
            cmd += ['--telnet', str(args.telnet + i)]
        vm[i] = {}
        vm[i]['idx'] = i
        vm[i]['DATE'] = str(timestamp) + '.srv.' + str(i + 1)
        vm[i]['starting'] = False
        vm[i]['started'] = False
        vm[i]['cmd'] = cmd
        vm[i]['proc'] = None
        vm[i]['out'] = ""
        vm[i]['pending'] = ""
        vm[i]['err'] = ""
        vm[i]['failed'] = []
        vm[i]['fail_seq'] = []
        vm[i]['skip_reason'] = []
        vm[i]['current_name'] = None
        vm[i]['last_stdout'] = None
        vm[i]['killed'] = False
    print('')

    if not args.nocurses:
        curses.wrapper(show_progress)
    else:
        class FakeScreen:
            def leaveok(self, n):
                pass
            def refresh(self):
                pass
            def addstr(self, *args, **kw):
                pass
            def move(self, x, y):
                pass
            def clrtoeol(self):
                pass
            def getmaxyx(self):
                return (25, 80)

        show_progress(FakeScreen())

    with open('{}/{}-parallel.log'.format(dir, timestamp), 'w') as f:
        for i in range(0, num_servers):
            f.write('VM {}\n{}\n{}\n'.format(i + 1, vm[i]['out'], vm[i]['err']))
        first = True
        for i in range(0, num_servers):
            for line in vm[i]['out'].splitlines():
                if line.startswith("FAIL "):
                    if first:
                        first = False
                        print("Logs for failed test cases:")
                        f.write("Logs for failed test cases:\n")
                    fname = "%s/%d.srv.%d/%s.log" % (dir, timestamp, i + 1,
                                                     line.split(' ')[1])
                    print(fname)
                    f.write("%s\n" % fname)

    failed = get_failed(vm)

    if first_run_failures:
        print("To re-run same failure sequence(s):")
        for i in range(0, num_servers):
            if len(vm[i]['failed']) == 0:
                continue
            print("./vm-run.sh", end=' ')
            if args.long:
                print("--long", end=' ')
            skip = len(vm[i]['fail_seq'])
            skip -= min(skip, 30)
            for t in vm[i]['fail_seq']:
                if skip > 0:
                    skip -= 1
                    continue
                print(t, end=' ')
            logger.info("Failure sequence: " + " ".join(vm[i]['fail_seq']))
            print('')
        print("Failed test cases:")
        for f in first_run_failures:
            print(f, end=' ')
            logger.info("Failed: " + f)
        print('')
    double_failed = []
    for name in failed:
        double_failed.append(name)
    for test in first_run_failures:
        double_failed.remove(test)
    if not rerun_failures:
        pass
    elif failed and not double_failed:
        print("All failed cases passed on retry")
        logger.info("All failed cases passed on retry")
    elif double_failed:
        print("Failed even on retry:")
        for f in double_failed:
            print(f, end=' ')
            logger.info("Failed on retry: " + f)
        print('')
    res = "TOTAL={} PASS={} FAIL={} SKIP={}".format(total_started,
                                                    total_passed,
                                                    total_failed,
                                                    total_skipped)
    print(res)
    logger.info(res)
    print("Logs: " + dir + '/' + str(timestamp))
    logger.info("Logs: " + dir + '/' + str(timestamp))

    skip_reason = []
    for i in range(num_servers):
        if not vm[i]['started']:
            continue
        skip_reason += vm[i]['skip_reason']
        if len(vm[i]['pending']) > 0:
            logger.info("Unprocessed stdout from VM[%d]: '%s'" %
                        (i, vm[i]['pending']))
        log = '{}/{}.srv.{}/console'.format(dir, timestamp, i + 1)
        with open(log, 'r') as f:
            if "Kernel panic" in f.read():
                print("Kernel panic in " + log)
                logger.info("Kernel panic in " + log)
    missing = {}
    missing['OCV not supported'] = 'OCV'
    missing['sigma_dut not available'] = 'sigma_dut'
    missing['Skip test case with long duration due to --long not specified'] = 'long'
    missing['TEST_ALLOC_FAIL not supported' ] = 'TEST_FAIL'
    missing['TEST_ALLOC_FAIL not supported in the build'] = 'TEST_FAIL'
    missing['TEST_FAIL not supported' ] = 'TEST_FAIL'
    missing['veth not supported (kernel CONFIG_VETH)'] = 'KERNEL:CONFIG_VETH'
    missing['WPA-EAP-SUITE-B-192 not supported'] = 'CONFIG_SUITEB192'
    missing['WPA-EAP-SUITE-B not supported'] = 'CONFIG_SUITEB'
    missing['wmediumd not available'] = 'wmediumd'
    missing['DPP not supported'] = 'CONFIG_DPP'
    missing['DPP version 2 not supported'] = 'CONFIG_DPP2'
    missing['EAP method PWD not supported in the build'] = 'CONFIG_EAP_PWD'
    missing['EAP method TEAP not supported in the build'] = 'CONFIG_EAP_TEAP'
    missing['FILS not supported'] = 'CONFIG_FILS'
    missing['FILS-SK-PFS not supported'] = 'CONFIG_FILS_SK_PFS'
    missing['OWE not supported'] = 'CONFIG_OWE'
    missing['SAE not supported'] = 'CONFIG_SAE'
    missing['Not using OpenSSL'] = 'CONFIG_TLS=openssl'
    missing['wpa_supplicant TLS library is not OpenSSL: internal'] = 'CONFIG_TLS=openssl'
    missing_items = []
    other_reasons = []
    for reason in sorted(set(skip_reason)):
        if reason in missing:
            missing_items.append(missing[reason])
        elif reason.startswith('OCSP-multi not supported with this TLS library'):
            missing_items.append('OCSP-MULTI')
        else:
            other_reasons.append(reason)
    if missing_items:
        print("Missing items (SKIP):", missing_items)
    if other_reasons:
        print("Other skip reasons:", other_reasons)

    for i in range(num_servers):
        unknown = ""
        for line in vm[i]['out'].splitlines():
            if not known_output(tests, line):
                unknown += line + "\n"
        if unknown:
            print("\nVM %d - unexpected stdout output:\n%s" % (i + 1, unknown))
        if vm[i]['err']:
            print("\nVM %d - unexpected stderr output:\n%s\n" % (i + 1, vm[i]['err']))

    if codecov:
        print("Code coverage - preparing report")
        for i in range(num_servers):
            subprocess.check_call([os.path.join(scriptsdir,
                                                'process-codecov.sh'),
                                   logdir + ".srv.%d" % (i + 1),
                                   str(i)])
        subprocess.check_call([os.path.join(scriptsdir, 'combine-codecov.sh'),
                               logdir])
        print("file://%s/index.html" % logdir)
        logger.info("Code coverage report: file://%s/index.html" % logdir)

    end_time = time.time()
    try:
        cmd = subprocess.Popen(['git', 'describe'], stdout=subprocess.PIPE)
        ver = cmd.stdout.read().decode().strip()
    except:
        ver = "unknown"
        pass
    logger.info("Tests run: {}  Tests failed: {}  Total time: {}  Version: {}".format(total_started, total_failed, end_time - start_time, ver))

    if double_failed or (failed and not rerun_failures):
        logger.info("Test run complete - failures found")
        sys.exit(2)
    if failed:
        logger.info("Test run complete - failures found on first run; passed on retry")
        sys.exit(1)
    logger.info("Test run complete - no failures")
    sys.exit(0)

if __name__ == "__main__":
    main()
