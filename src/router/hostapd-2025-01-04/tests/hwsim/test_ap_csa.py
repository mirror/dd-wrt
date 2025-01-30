# AP CSA tests
# Copyright (c) 2013, Luciano Coelho <luciano.coelho@intel.com>
#
# This software may be distributed under the terms of the BSD license.
# See README for more details.

from remotehost import remote_compatible
import time
import logging
logger = logging.getLogger()

import hwsim_utils
import hostapd
from utils import *

def connect(dev, apdev, scan_freq="2412", **kwargs):
    params = {"ssid": "ap-csa",
              "channel": "1"}
    params.update(kwargs)
    ap = hostapd.add_ap(apdev[0], params)
    dev.connect("ap-csa", key_mgmt="NONE", scan_freq=scan_freq)
    return ap

def switch_channel(ap, count, freq, extra=None):
    cmd = "CHAN_SWITCH " + str(count) + " " + str(freq)
    if extra:
        cmd += " " + extra
    ap.request(cmd)

    ev = ap.wait_event(["CTRL-EVENT-STARTED-CHANNEL-SWITCH"], timeout=10)
    if ev is None:
        raise Exception("Channel switch start event not seen")
    if "freq=" + str(freq) not in ev:
        raise Exception("Unexpected channel in CS started event")

    ev = ap.wait_event(["CTRL-EVENT-CHANNEL-SWITCH"], timeout=10)
    if ev is None:
        raise Exception("Channel switch completed event not seen")
    if "freq=" + str(freq) not in ev:
        raise Exception("Unexpected channel in CS completed event")

    ev = ap.wait_event(["AP-CSA-FINISHED"], timeout=10)
    if ev is None:
        raise Exception("CSA finished event timed out")
    if "freq=" + str(freq) not in ev:
        raise Exception("Unexpected channel in CSA finished event")

def wait_channel_switch(dev, freq):
    ev = dev.wait_event(["CTRL-EVENT-STARTED-CHANNEL-SWITCH"], timeout=5)
    if ev is None:
        raise Exception("Channel switch start not reported")
    if "freq=%d" % freq not in ev:
        raise Exception("Unexpected frequency in channel switch started: " + ev)

    ev = dev.wait_event(["CTRL-EVENT-CHANNEL-SWITCH"], timeout=5)
    if ev is None:
        raise Exception("Channel switch not reported")
    if "freq=%d" % freq not in ev:
        raise Exception("Unexpected frequency: " + ev)
    ev = dev.wait_event(["CTRL-EVENT-DISCONNECTED"], timeout=1)
    if ev is not None:
        raise Exception("Unexpected disconnection on channel switch")

@remote_compatible
def test_ap_csa_1_switch(dev, apdev):
    """AP Channel Switch, one switch"""
    csa_supported(dev[0])
    freq = int(dev[0].get_driver_status_field("freq"))
    if freq != 0:
        raise Exception("Unexpected driver freq=%d in beginning" % freq)
    ap = connect(dev[0], apdev)
    freq = int(dev[0].get_driver_status_field("freq"))
    if freq != 2412:
        raise Exception("Unexpected driver freq=%d after association" % freq)

    hwsim_utils.test_connectivity(dev[0], ap)
    switch_channel(ap, 10, 2462, extra="ht")
    wait_channel_switch(dev[0], 2462)
    hwsim_utils.test_connectivity(dev[0], ap)
    freq = int(dev[0].get_driver_status_field("freq"))
    if freq != 2462:
        raise Exception("Unexpected driver freq=%d after channel switch" % freq)
    dev[0].request("DISCONNECT")
    dev[0].wait_disconnected()
    freq = int(dev[0].get_driver_status_field("freq"))
    if freq != 0:
        raise Exception("Unexpected driver freq=%d after disconnection" % freq)

@remote_compatible
def test_ap_csa_2_switches(dev, apdev):
    """AP Channel Switch, two switches"""
    csa_supported(dev[0])
    ap = connect(dev[0], apdev)

    hwsim_utils.test_connectivity(dev[0], ap)
    switch_channel(ap, 10, 2462, extra="ht")
    wait_channel_switch(dev[0], 2462)
    hwsim_utils.test_connectivity(dev[0], ap)
    switch_channel(ap, 10, 2412, extra="ht")
    wait_channel_switch(dev[0], 2412)
    hwsim_utils.test_connectivity(dev[0], ap)

@remote_compatible
def test_ap_csa_1_switch_count_0(dev, apdev):
    """AP Channel Switch, one switch with count 0"""
    csa_supported(dev[0])
    ap = connect(dev[0], apdev)

    hwsim_utils.test_connectivity(dev[0], ap)
    switch_channel(ap, 0, 2462)
    # this does not result in CSA currently, so do not bother checking
    # connectivity

@remote_compatible
def test_ap_csa_2_switches_count_0(dev, apdev):
    """AP Channel Switch, two switches with count 0"""
    csa_supported(dev[0])
    ap = connect(dev[0], apdev)

    hwsim_utils.test_connectivity(dev[0], ap)
    switch_channel(ap, 0, 2462)
    # this does not result in CSA currently, so do not bother checking
    # connectivity
    switch_channel(ap, 0, 2412)
    # this does not result in CSA currently, so do not bother checking
    # connectivity

@remote_compatible
def test_ap_csa_1_switch_count_1(dev, apdev):
    """AP Channel Switch, one switch with count 1"""
    csa_supported(dev[0])
    ap = connect(dev[0], apdev)

    hwsim_utils.test_connectivity(dev[0], ap)
    switch_channel(ap, 1, 2462)
    # this does not result in CSA currently, so do not bother checking
    # connectivity

@remote_compatible
def test_ap_csa_2_switches_count_1(dev, apdev):
    """AP Channel Switch, two switches with count 1"""
    csa_supported(dev[0])
    ap = connect(dev[0], apdev)

    hwsim_utils.test_connectivity(dev[0], ap)
    switch_channel(ap, 1, 2462)
    # this does not result in CSA currently, so do not bother checking
    # connectivity
    switch_channel(ap, 1, 2412)
    # this does not result in CSA currently, so do not bother checking
    # connectivity

@remote_compatible
def test_ap_csa_1_switch_count_2(dev, apdev):
    """AP Channel Switch, one switch with count 2"""
    csa_supported(dev[0])
    ap = connect(dev[0], apdev)

    hwsim_utils.test_connectivity(dev[0], ap)
    switch_channel(ap, 2, 2462, extra="ht")
    wait_channel_switch(dev[0], 2462)
    hwsim_utils.test_connectivity(dev[0], ap)

@remote_compatible
def test_ap_csa_ecsa_only(dev, apdev):
    """AP Channel Switch, one switch with only ECSA IE"""
    csa_supported(dev[0])
    ap = connect(dev[0], apdev, ecsa_ie_only="1")

    hwsim_utils.test_connectivity(dev[0], ap)
    switch_channel(ap, 10, 2462, extra="ht")
    wait_channel_switch(dev[0], 2462)
    hwsim_utils.test_connectivity(dev[0], ap)

@remote_compatible
def test_ap_csa_invalid(dev, apdev):
    """AP Channel Switch - invalid channel"""
    csa_supported(dev[0])
    ap = connect(dev[0], apdev)

    vals = [2461, 4900, 4901, 5181, 5746, 5699, 5895, 5899]
    for val in vals:
        if "FAIL" not in ap.request("CHAN_SWITCH 1 %d" % val):
            raise Exception("Invalid channel accepted: %d" % val)

def test_ap_csa_disable(dev, apdev):
    """AP Channel Switch and DISABLE command before completion"""
    csa_supported(dev[0])
    ap = connect(dev[0], apdev, scan_freq="2412 2462")
    if "OK" not in ap.request("CHAN_SWITCH 10 2462"):
        raise Exception("CHAN_SWITCH failed")
    ap.disable()
    ap.enable()
    dev[0].wait_disconnected()
    dev[0].wait_connected()

def _assoc_while_csa(dev, apdev, freq_to, blocktx):
    params = {
        "ssid": "ap-csa",
        "hw_mode": "a",
        "country_code": "FI",
        "channel": "36",
        "ieee80211n": "0",
    }
    ap = hostapd.add_ap(apdev[0], params)
    count = 100 if blocktx else 20
    delay = 1 + count / 10
    cmd = f"CHAN_SWITCH {count} {freq_to}"
    if blocktx:
        cmd += " blocktx"
    ap.request(cmd)

    ev = ap.wait_event(["CTRL-EVENT-STARTED-CHANNEL-SWITCH"], timeout=10)
    if ev is None:
        raise Exception("Channel switch start event not seen")
    if f"freq={freq_to}" not in ev:
        raise Exception("Unexpected channel in CS started event")
    try:
        dev[0].connect("ap-csa", key_mgmt="NONE", scan_freq="5180",
                       wait_connect=False)
        if blocktx or freq_to != 5180:
            ev = dev[0].wait_event(["CTRL-EVENT-SSID-TEMP-DISABLED",
                                    "CTRL-EVENT-CONNECTED"], timeout=9)
            if not ev: # this is fine, at least we didn't connect
                return
            if not "CTRL-EVENT-SSID-TEMP-DISABLED" in ev:
                raise Exception("Erroneously connected!")
            if not 'auth_failures=1' in ev:
                raise Exception(f'Should have auth failures in "{ev}"')
            # wait for CSA to finish and connect then
            time.sleep(delay)
            dev[0].connect("ap-csa", key_mgmt="NONE", scan_freq=str(freq_to))
        else:
            dev[0].wait_connected()
            if freq_to != 5180:
                wait_channel_switch(dev[0], freq_to)
    finally:
        dev[0].request("DISCONNECT")
        dev[0].wait_event(["CTRL-EVENT-DISCONNECTED"], timeout=1)
        clear_regdom(ap, dev)

@long_duration_test
def test_assoc_while_csa_same_blocktx(dev, apdev):
    """Check we don't associate while AP is doing quiet CSA (same channel)"""
    _assoc_while_csa(dev, apdev, 5180, True)

def test_assoc_while_csa_same(dev, apdev):
    """Check we _do_ associate while AP is doing CSA (same channel)"""
    _assoc_while_csa(dev, apdev, 5180, False)

@long_duration_test
def test_assoc_while_csa_diff_blocktx(dev, apdev):
    """Check we don't associate while AP is doing quiet CSA (different channel)"""
    _assoc_while_csa(dev, apdev, 5200, True)

@long_duration_test
def test_assoc_while_csa_diff(dev, apdev):
    """Check we don't associate while AP is doing CSA (different channel)"""
    _assoc_while_csa(dev, apdev, 5200, False)

def test_ap_stuck_ecsa(dev, apdev):
    """ECSA element stuck in Probe Response frame"""

    # Test behaving like an Asus RT-AC53, firmware 3.0.0.4.380_10760-g21a5898,
    # which has stuck ECSA element in the Probe Response frames.
    try:
        ap = connect(dev[0], apdev, scan_freq=None,
                     hw_mode='a', channel='36',
                     country_code='FI',
                     presp_elements="3c0401732409")
        ap.wait_sta()
        hwsim_utils.test_connectivity(dev[0], ap)
    finally:
        dev[0].request("DISCONNECT")
        dev[0].wait_event(["CTRL-EVENT-DISCONNECTED"], timeout=1)
        clear_regdom(ap, dev)
