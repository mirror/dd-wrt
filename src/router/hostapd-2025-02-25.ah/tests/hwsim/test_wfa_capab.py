# Test cases for Wi-Fi Alliance capabilities indication
# Copyright (c) 2024, Qualcomm Innovation Center, Inc.
#
# This software may be distributed under the terms of the BSD license.
# See README for more details.

import hostapd
from utils import *

def test_wfa_gen_capa_protected(dev, apdev):
    """WFA generational capabilities indication (protected)"""
    try:
        dev[0].set("wfa_gen_capa", "1")
        run_wfa_gen_capa(dev, apdev)
    finally:
        dev[0].set("wfa_gen_capa", "0")

def test_wfa_gen_capa_unprotected(dev, apdev):
    """WFA generational capabilities indication (unprotected)"""
    try:
        dev[0].set("wfa_gen_capa", "2")
        run_wfa_gen_capa(dev, apdev)
    finally:
        dev[0].set("wfa_gen_capa", "0")

def test_wfa_gen_capa_protected_cert(dev, apdev):
    """WFA generational capabilities indication (protected, cert)"""
    try:
        dev[0].set("wfa_gen_capa", "1")
        run_wfa_gen_capa(dev, apdev, cert=True)
    finally:
        dev[0].set("wfa_gen_capa", "0")

def test_wfa_gen_capa_unprotected_cert(dev, apdev):
    """WFA generational capabilities indication (unprotected, cert)"""
    try:
        dev[0].set("wfa_gen_capa", "2")
        run_wfa_gen_capa(dev, apdev, cert=True)
    finally:
        dev[0].set("wfa_gen_capa", "0")

def test_wfa_gen_capa_automatic(dev, apdev):
    """WFA generational capabilities indication (automatic)"""
    try:
        dev[0].set("wfa_gen_capa", "1")
        run_wfa_gen_capa(dev, apdev, automatic=True)
    finally:
        dev[0].set("wfa_gen_capa", "0")

def run_wfa_gen_capa(dev, apdev, cert=False, automatic=False):
    check_sae_capab(dev[0])

    params = hostapd.wpa3_params(ssid="wfa-capab", password="12345678")
    hapd = hostapd.add_ap(apdev[0], params)

    dev[0].set("sae_groups", "")
    if automatic:
        dev[0].set("wfa_gen_capa_supp", "")
        dev[0].set("wfa_gen_capa_cert", "")
    else:
        dev[0].set("wfa_gen_capa_supp", "07")
        if cert:
            dev[0].set("wfa_gen_capa_cert", "07")
        else:
            dev[0].set("wfa_gen_capa_cert", "")
    dev[0].connect("wfa-capab", sae_password="12345678", key_mgmt="SAE",
                   ieee80211w="2", scan_freq="2412")
    ev = hapd.wait_event(["WFA-GEN-CAPAB"], timeout=10)
    if ev is None:
        raise Exception("Generational capabilities indication not received")
    _, addr, val = ev.split()
    if addr != dev[0].own_addr():
        raise Exception("Unexpected STA address in the indication")
    if automatic:
        if not val.startswith("01"):
            raise Exception("Unexpected indication value: " + val)
    else:
        if val != ("01070107" if cert else "0107"):
            raise Exception("Unexpected indication value: " + val)
