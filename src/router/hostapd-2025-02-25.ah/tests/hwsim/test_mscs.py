# Test cases for MSCS
# Copyright (c) 2021, Jouni Malinen <j@w1.fi>
#
# This software may be distributed under the terms of the BSD license.
# See README for more details.

import struct
import time

import hostapd
from utils import *

def register_mcsc_req(hapd):
    type = 0x00d0
    match = "1304"
    if "OK" not in hapd.request("REGISTER_FRAME %04x %s" % (type, match)):
        raise Exception("Could not register frame reception for Robust AV Streaming")

def fill_change_params(dialog_token, status_code, params):
    req_type = params['req_type'] if 'req_type' in params else 2
    return struct.pack('<BBBHBBBBBBI', 19, 5, dialog_token, status_code,
                       255, 8, 88, req_type, params['up_bitmap'],
                       params['up_limit'], params['stream_timeout'])

def handle_mscs_req(hapd, wrong_dialog=False, status_code=0,
                    change_params=None):
    msg = hapd.mgmt_rx()
    if msg['subtype'] != 13:
        logger.info("RX:" + str(msg))
        raise Exception("Received unexpected Management frame")
    categ, act, dialog_token = struct.unpack('BBB', msg['payload'][0:3])
    if categ != 19 or act != 4:
        logger.info("RX:" + str(msg))
        raise Exception("Received unexpected Action frame")

    if wrong_dialog:
        dialog_token = (dialog_token + 1) % 256
    msg['da'] = msg['sa']
    msg['sa'] = hapd.own_addr()

    if change_params is not None:
        msg['payload'] = fill_change_params(dialog_token, status_code,
                                            change_params)
    else:
        msg['payload'] = struct.pack('<BBBH', 19, 5, dialog_token, status_code)

    hapd.mgmt_tx(msg)
    ev = hapd.wait_event(["MGMT-TX-STATUS"], timeout=5)
    if ev is None or "stype=13 ok=1" not in ev:
        raise Exception("No TX status reported")

def wait_mscs_result(dev, expect_status=0, change_params=None):
    ev = dev.wait_event(["CTRL-EVENT-MSCS-RESULT"], timeout=1)
    if ev is None:
        raise Exception("No MSCS result reported")
    if "status_code=%d" % expect_status not in ev:
        raise Exception("Unexpected MSCS result: " + ev)
    if change_params is None and 'change' in ev:
        raise Exception("Unexpected 'change' in MSCS result: " + ev)
    if change_params is not None and \
        ("change" not in ev or
         "up_limit=%d" % change_params['up_limit'] not in ev or
         "up_bitmap=%d" % change_params['up_bitmap'] not in ev or
         "stream_timeout=%d" % change_params['stream_timeout'] not in ev):
        raise Exception("Unexpected MSCS result: " + ev)

def add_mscs_ap(apdev, reg_mscs_req=True, mscs_supported=True,
                assocresp_elements=None):
    params = {"ssid": "mscs"}

    if mscs_supported:
        params["ext_capa"] = 10*"00" + "20"
    else:
        params["ext_capa_mask"] = 10*"00" + "20"

    if assocresp_elements is not None:
        params['assocresp_elements'] = assocresp_elements

    hapd = hostapd.add_ap(apdev, params)

    if reg_mscs_req:
        register_mcsc_req(hapd)

    return hapd

def test_mscs_invalid_params(dev, apdev):
    """MSCS command invalid parameters"""
    tests = ["",
             "add Xp_bitmap=F0 up_limit=7 stream_timeout=12345 frame_classifier=045F",
             "add up_bitmap=F0 Xp_limit=7 stream_timeout=12345 frame_classifier=045F",
             "add up_bitmap=F0 up_limit=7 Xtream_timeout=12345 frame_classifier=045F",
             "add up_bitmap=F0 up_limit=7 stream_timeout=12345 Xrame_classifier=045F",
             "add up_bitmap=X0 up_limit=7 stream_timeout=12345 frame_classifier=045F",
             "add up_bitmap=F0 up_limit=7 stream_timeout=0 frame_classifier=045F",
             "add up_bitmap=F0 up_limit=7 stream_timeout=12345 frame_classifier=X45F",
             "change "]
    for t in tests:
        if "FAIL" not in dev[0].request("MSCS " + t):
            raise Exception("Invalid MSCS parameters accepted: " + t)

def mscs_run(dev, apdev, func):
    try:
        func(dev, apdev)
    finally:
        dev[0].request("MSCS remove")

def test_mscs_without_ap_support(dev, apdev):
    """MSCS without AP support"""
    mscs_run(dev, apdev, run_mscs_without_ap_support)

def run_mscs_without_ap_support(dev, apdev):
    add_mscs_ap(apdev[0], reg_mscs_req=False, mscs_supported=False)

    cmd = "MSCS add up_bitmap=F0 up_limit=7 stream_timeout=12345 frame_classifier=045F"
    if "OK" not in dev[0].request(cmd):
        raise Exception("Failed to configure MSCS")

    dev[0].connect("mscs", key_mgmt="NONE", scan_freq="2412")

    cmd = "MSCS change up_bitmap=F0 up_limit=7 stream_timeout=12345 frame_classifier=045F"
    if "FAIL" not in dev[0].request(cmd):
        raise Exception("MSCS change accepted unexpectedly")

    cmd = "MSCS add up_bitmap=F0 up_limit=7 stream_timeout=12345 frame_classifier=045F"
    if "FAIL" not in dev[0].request(cmd):
        raise Exception("MSCS add accepted unexpectedly")

def test_mscs_post_assoc(dev, apdev):
    """MSCS configuration post-association"""
    mscs_run(dev, apdev, run_mscs_post_assoc)

def run_mscs_post_assoc(dev, apdev):
    hapd = add_mscs_ap(apdev[0])

    dev[0].connect("mscs", key_mgmt="NONE", scan_freq="2412")

    hapd.dump_monitor()
    hapd.set("ext_mgmt_frame_handling", "1")

    cmd = "MSCS change up_bitmap=F0 up_limit=7 stream_timeout=12345 frame_classifier=045F"
    if "FAIL" not in dev[0].request(cmd):
        raise Exception("MSCS change accepted unexpectedly")

    cmd = "MSCS add up_bitmap=F0 up_limit=7 stream_timeout=12345 frame_classifier=045F"
    if "OK" not in dev[0].request(cmd):
        raise Exception("MSCS add failed")

    handle_mscs_req(hapd)
    wait_mscs_result(dev[0])

    cmd = "MSCS change up_bitmap=F0 up_limit=7 stream_timeout=12345 frame_classifier=045F"
    if "OK" not in dev[0].request(cmd):
        raise Exception("MSCS change failed")

    handle_mscs_req(hapd)
    wait_mscs_result(dev[0])

    cmd = "MSCS change up_bitmap=F0 up_limit=7 stream_timeout=12345 frame_classifier=045F"
    if "OK" not in dev[0].request(cmd):
        raise Exception("MSCS change failed")

    handle_mscs_req(hapd, status_code=23456)
    wait_mscs_result(dev[0], expect_status=23456)

def test_mscs_pre_assoc(dev, apdev):
    """MSCS configuration pre-association"""
    mscs_run(dev, apdev, run_mscs_pre_assoc)

def run_mscs_pre_assoc(dev, apdev):
    ies = "ff0c5800000000000000" + "01020000"
    hapd = add_mscs_ap(apdev[0], assocresp_elements=ies)

    cmd = "MSCS add up_bitmap=F0 up_limit=7 stream_timeout=12345 frame_classifier=045F"
    if "OK" not in dev[0].request(cmd):
        raise Exception("MSCS add failed")

    dev[0].connect("mscs", key_mgmt="NONE", scan_freq="2412",
                   wait_connect=False)
    wait_mscs_result(dev[0])
    dev[0].wait_connected()

    hapd.set("ext_mgmt_frame_handling", "1")

    cmd = "MSCS change up_bitmap=F0 up_limit=7 stream_timeout=12345 frame_classifier=045F"
    if "OK" not in dev[0].request(cmd):
        raise Exception("MSCS change failed")

    handle_mscs_req(hapd)
    wait_mscs_result(dev[0])

    cmd = "MSCS change up_bitmap=F0 up_limit=7 stream_timeout=12345 frame_classifier=045F"
    if "OK" not in dev[0].request(cmd):
        raise Exception("MSCS change failed")

    handle_mscs_req(hapd, wrong_dialog=True)

    ev = dev[0].wait_event(["CTRL-EVENT-MSCS-RESULT"], timeout=1)
    if ev is not None:
        raise Exception("Unexpected MSCS result reported")

def test_mscs_assoc_failure(dev, apdev):
    """MSCS configuration failure during association exchange"""
    mscs_run(dev, apdev, run_mscs_assoc_failure)

def run_mscs_assoc_failure(dev, apdev):
    ies = "ff0c5800000000000000" + "01020001"
    hapd = add_mscs_ap(apdev[0], assocresp_elements=ies)

    cmd = "MSCS add up_bitmap=F0 up_limit=7 stream_timeout=12345 frame_classifier=045F"
    if "OK" not in dev[0].request(cmd):
        raise Exception("MSCS add failed")

    dev[0].connect("mscs", key_mgmt="NONE", scan_freq="2412",
                   wait_connect=False)
    wait_mscs_result(dev[0], expect_status=256)
    dev[0].wait_connected()
    dev[0].request("REMOVE_NETWORK all")
    dev[0].wait_disconnected()

    hapd.dump_monitor()
    # No MSCS Status subelement
    hapd.set("assocresp_elements", "ff085800000000000000")
    dev[0].connect("mscs", key_mgmt="NONE", scan_freq="2412",
                   wait_connect=False)
    ev = dev[0].wait_event(["CTRL-EVENT-CONNECTED", "CTRL-EVENT-MSCS-RESULT"],
                           timeout=10)
    if ev is None:
        raise Exception("No connection event")
    if "CTRL-EVENT-MSCS-RESULT" in ev:
        raise Exception("Unexpected MSCS result")

def test_mscs_local_errors(dev, apdev):
    """MSCS configuration local errors"""
    mscs_run(dev, apdev, run_mscs_local_errors)

def run_mscs_local_errors(dev, apdev):
    hapd = add_mscs_ap(apdev[0])
    dev[0].connect("mscs", key_mgmt="NONE", scan_freq="2412")

    hapd.dump_monitor()
    hapd.set("ext_mgmt_frame_handling", "1")

    for count in range(1, 3):
        with alloc_fail(dev[0], count, "wpas_send_mscs_req"):
            cmd = "MSCS add up_bitmap=F0 up_limit=7 stream_timeout=12345 frame_classifier=045F"
            if "FAIL" not in dev[0].request(cmd):
                raise Exception("MSCS add succeeded in error case")

def test_mscs_assoc_change_response(dev, apdev):
    """MSCS configuration failure during assoc - AP response with change request"""
    mscs_run(dev, apdev, run_mscs_assoc_change_response)

def run_mscs_assoc_change_response(dev, apdev):
    ies = "ff0c5802060701000000" + "01020001"
    hapd = add_mscs_ap(apdev[0], assocresp_elements=ies)

    cmd = "MSCS add up_bitmap=F0 up_limit=7 stream_timeout=12345 frame_classifier=045F"
    if "OK" not in dev[0].request(cmd):
        raise Exception("MSCS add failed")

    dev[0].connect("mscs", key_mgmt="NONE", scan_freq="2412",
                   wait_connect=False)

    # Based on the Association Response elements above
    change_params = {
        'up_bitmap': 6,
        'up_limit': 7,
        'stream_timeout': 1
    }
    wait_mscs_result(dev[0], expect_status=256, change_params=change_params)
    dev[0].wait_connected()

    hapd.dump_monitor()
    hapd.set("ext_mgmt_frame_handling", "1")

    # Verify we're able to send a new MSCS Request frame.
    cmd = "MSCS add up_bitmap=F0 up_limit=7 stream_timeout=12345` frame_classifier=045F"
    if "OK" not in dev[0].request(cmd):
        raise Exception("MSCS add failed")

    handle_mscs_req(hapd)
    wait_mscs_result(dev[0])

def test_mscs_post_assoc_change_response(dev, apdev):
    """MSCS configuration failure post assoc - AP response with MSCS Response frame change request"""
    mscs_run(dev, apdev, run_mscs_post_assoc_change_response)

def run_mscs_post_assoc_change_response(dev, apdev):
    hapd = add_mscs_ap(apdev[0])
    dev[0].connect("mscs", key_mgmt="NONE", scan_freq="2412")

    hapd.dump_monitor()
    hapd.set("ext_mgmt_frame_handling", "1")

    cmd = "MSCS add up_bitmap=F0 up_limit=7 stream_timeout=12345 frame_classifier=045F"
    if "OK" not in dev[0].request(cmd):
        raise Exception("MSCS add failed")

    change_params = {
        'up_bitmap': 6,
        'up_limit': 7,
        'stream_timeout': 1
    }

    handle_mscs_req(hapd, status_code=256, change_params=change_params)
    wait_mscs_result(dev[0], expect_status=256, change_params=change_params)

    # Verify we're able to send new valid MSCS request
    cmd = "MSCS add up_bitmap=F0 up_limit=7 stream_timeout=12345 frame_classifier=045F"
    if "OK" not in dev[0].request(cmd):
        raise Exception("MSCS add failed")

    handle_mscs_req(hapd)
    wait_mscs_result(dev[0])

def test_mscs_invalid_req_type_response(dev, apdev):
    """MSCS AP response with invalid request type"""
    mscs_run(dev, apdev, run_mscs_invalid_req_type_response)

def run_mscs_invalid_req_type_response(dev, apdev):
    hapd = add_mscs_ap(apdev[0])
    dev[0].connect("mscs", key_mgmt="NONE", scan_freq="2412")

    hapd.dump_monitor()
    hapd.set("ext_mgmt_frame_handling", "1")

    change_params = {
        'up_bitmap': 6,
        'up_limit': 7,
        'stream_timeout': 1
    }

    # Verify frames with invalid MSCS decriptor request type dropped
    for req_type in [3, 4, 10]:
        cmd = "MSCS add up_bitmap=F0 up_limit=7 stream_timeout=12345 frame_classifier=045F"
        if "OK" not in dev[0].request(cmd):
            raise Exception("MSCS add failed")

        change_params['req_type'] = req_type
        handle_mscs_req(hapd, status_code=256, change_params=change_params)
        ev = dev[0].wait_event(["CTRL-EVENT-MSCS-RESULT"], timeout=1)
        if ev is not None:
            raise Exception("Unexpected MSCS result reported")

def send_unsolicited_mscs_response(dev, hapd, status_code, params=None,
                                   wrong_dialog=False):
    dialog_token = 0 if not wrong_dialog else 10
    if params is not None:
        payload = fill_change_params(dialog_token, status_code, params)
    else:
        payload = struct.pack('<BBBH', 19, 5, dialog_token, status_code)

    msg = {
        'fc': 0xd0,
        'sa': hapd.own_addr(),
        'da': dev.own_addr(),
        'bssid': hapd.own_addr(),
        'payload': payload,
    }

    hapd.mgmt_tx(msg)
    ev = hapd.wait_event(["MGMT-TX-STATUS"], timeout=5)
    if ev is None or "stype=13 ok=1" not in ev:
        raise Exception("No TX status reported")

def test_mscs_unsolicited_response(dev, apdev):
    """MSCS configured and AP sends unsolicited response to terminate / change the session"""
    mscs_run(dev, apdev, run_mscs_unsolicited_response)

def run_mscs_unsolicited_response(dev, apdev):
    hapd = add_mscs_ap(apdev[0])
    dev[0].connect("mscs", key_mgmt="NONE", scan_freq="2412")

    hapd.dump_monitor()
    hapd.set("ext_mgmt_frame_handling", "1")

    cmd = "MSCS add up_bitmap=F0 up_limit=7 stream_timeout=12345 frame_classifier=045F"
    if "OK" not in dev[0].request(cmd):
        raise Exception("MSCS add failed")

    handle_mscs_req(hapd)
    wait_mscs_result(dev[0])

    status_code = 256

    # Verify unsolicited response with wrong dialog token (othar than 0) is
    # dropped
    send_unsolicited_mscs_response(dev[0], hapd, status_code=status_code,
                                   wrong_dialog=True)
    ev = dev[0].wait_event(["CTRL-EVENT-MSCS-RESULT"], timeout=1)
    if ev is not None:
        raise Exception("Unexpected MSCS result reported")

    # Verify unsolicited response without change request (termination)
    send_unsolicited_mscs_response(dev[0], hapd, status_code=status_code)
    wait_mscs_result(dev[0], expect_status=status_code)

    # Send new MSCS request
    cmd = "MSCS add up_bitmap=F0 up_limit=7 stream_timeout=12345 frame_classifier=045F"
    if "OK" not in dev[0].request(cmd):
        raise Exception("MSCS add failed")

    handle_mscs_req(hapd)
    wait_mscs_result(dev[0])

    params = {
        'up_bitmap': 6,
        'up_limit': 7,
        'stream_timeout': 1
    }

    # Verify unsolicited response handling with change request
    send_unsolicited_mscs_response(dev[0], hapd, status_code=status_code,
                                   params=params)
    wait_mscs_result(dev[0], expect_status=status_code, change_params=params)
