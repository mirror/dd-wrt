# EHT tests
# Copyright (c) 2022-2024, Qualcomm Innovation Center, Inc.
#
# This software may be distributed under the terms of the BSD license.
# See README for more details.

import binascii
import subprocess
import tempfile

import hostapd
from utils import *
from hwsim import HWSimRadio
import hwsim_utils
from wpasupplicant import WpaSupplicant
import re
import mld
from tshark import run_tshark
from test_gas import hs20_ap_params
from test_dpp import check_dpp_capab, wait_auth_success
from test_rrm import build_beacon_request, run_req_beacon, BeaconReport

def eht_verify_wifi_version(dev):
    status = dev.get_status()
    logger.info("station status: " + str(status))

    if 'wifi_generation' not in status:
        raise Exception("Missing wifi_generation information")
    if status['wifi_generation'] != "7":
        raise Exception("Unexpected wifi_generation value: " + status['wifi_generation'])

def _eht_get_links_bitmap(wpas, name):
    vfile = "/sys/kernel/debug/ieee80211/%s/netdev:%s/%s" % \
        (wpas.get_driver_status_field("phyname"), wpas.ifname, name)

    if wpas.cmd_execute(["ls", vfile])[0] != 0:
        logger_info("%s not supported in mac80211: %s" % (name, vfile))
        return 0

    res, out = wpas.cmd_execute(["cat", vfile], shell=True)
    if res != 0:
        raise Exception("Failed to read %s" % fname)

    logger.info("%s=%s" % (name, out))
    return int(out, 16)

def _eht_valid_links(wpas):
    return _eht_get_links_bitmap(wpas, "valid_links")

def _eht_active_links(wpas):
    return _eht_get_links_bitmap(wpas, "active_links")

def _eht_dormant_links(wpas):
    return _eht_get_links_bitmap(wpas, "dormant_links")

def _eht_verify_links(wpas, valid_links=0, active_links=0):
    vlinks = _eht_valid_links(wpas)
    if vlinks != valid_links:
        raise Exception("Unexpected valid links (0x%04x != 0x%04x)" % (vlinks, valid_links))

    alinks = _eht_active_links(wpas)
    if alinks != active_links:
        raise Exception("Unexpected active links (0x%04x != 0x%04x)" % (alinks, active_links))

def eht_verify_status(wpas, hapd, freq, bw, is_ht=False, is_vht=False,
                      mld=False, valid_links=0, active_links=0):
    status = hapd.get_status()

    logger.info("hostapd STATUS: " + str(status))
    if is_ht and status["ieee80211n"] != "1":
        raise Exception("Unexpected STATUS ieee80211n value")
    if is_vht and status["ieee80211ac"] != "1":
        raise Exception("Unexpected STATUS ieee80211ac value")
    if status["ieee80211ax"] != "1":
        raise Exception("Unexpected STATUS ieee80211ax value")
    if status["ieee80211be"] != "1":
        raise Exception("Unexpected STATUS ieee80211be value")

    sta = hapd.get_sta(wpas.own_addr())
    logger.info("hostapd STA: " + str(sta))
    if sta['addr'] == 'FAIL':
        raise Exception("hostapd " + hapd.ifname + " did not have a STA entry for the STA " + wpas.own_addr())
    if is_ht and "[HT]" not in sta['flags']:
        raise Exception("Missing STA flag: HT")
    if is_vht and "[VHT]" not in sta['flags']:
        raise Exception("Missing STA flag: VHT")
    if "[HE]" not in sta['flags']:
        raise Exception("Missing STA flag: HE")
    if "[EHT]" not in sta['flags']:
        raise Exception("Missing STA flag: EHT")

    sig = wpas.request("SIGNAL_POLL").splitlines()

    # TODO: With MLD connection, signal poll logic is still not implemented.
    # While mac80211 maintains the station using the MLD address, the
    # information is maintained in the link stations, but it is not sent to
    # user space yet.
    if not mld:
        if "FREQUENCY=%s" % freq not in sig:
            raise Exception("Unexpected SIGNAL_POLL value(1): " + str(sig))
        if "WIDTH=%s MHz" % bw not in sig:
            raise Exception("Unexpected SIGNAL_POLL value(2): " + str(sig))

    # Active links are updated in async work after the connection.
    # Sleep a bit to allow it to run.
    time.sleep(0.1)
    _eht_verify_links(wpas, valid_links, active_links)

def traffic_test(wpas, hapd, success=True, ifname2=None):
    hwsim_utils.test_connectivity(wpas, hapd, success_expected=success,
                                  ifname2=ifname2)

def test_eht_open(dev, apdev):
    """EHT AP with open mode configuration"""
    params = {"ssid": "eht",
              "ieee80211ax": "1",
              "ieee80211be": "1"}
    try:
        hapd = hostapd.add_ap(apdev[0], params)
    except Exception as e:
        if isinstance(e, Exception) and \
           str(e) == "Failed to set hostapd parameter ieee80211be":
            raise HwsimSkip("EHT not supported")
        raise
    if hapd.get_status_field("ieee80211be") != "1":
        raise Exception("AP STATUS did not indicate ieee80211be=1")
    dev[0].connect("eht", key_mgmt="NONE", scan_freq="2412")
    sta = hapd.get_sta(dev[0].own_addr())
    if "[EHT]" not in sta['flags']:
        raise Exception("Missing STA flag: EHT")
    status = dev[0].request("STATUS")
    if "wifi_generation=7" not in status:
        raise Exception("STA STATUS did not indicate wifi_generation=7")

def test_prefer_eht_20(dev, apdev):
    """EHT AP on a 20 MHz channel"""
    params = {"ssid": "eht",
              "channel": "1",
              "ieee80211ax": "1",
              "ieee80211be" : "1",
              "ieee80211n": "1"}
    try:
        hapd0 = hostapd.add_ap(apdev[0], params)

        params["ieee80211be"] = "0"
        hapd1 = hostapd.add_ap(apdev[1], params)
    except Exception as e:
        if isinstance(e, Exception) and \
           str(e) == "Failed to set hostapd parameter ieee80211be":
            raise HwsimSkip("EHT not supported")
        raise

    dev[0].connect("eht", key_mgmt="NONE")
    if dev[0].get_status_field('bssid') != apdev[0]['bssid']:
        raise Exception("dev[0] connected to unexpected AP")

    est = dev[0].get_bss(apdev[0]['bssid'])['est_throughput']
    if est != "172103":
      raise Exception("Unexpected BSS1 est_throughput: " + est)

def start_eht_sae_ap(apdev, ml=False, transition_mode=False,
                     anti_clogging_token=False):
    params = hostapd.wpa2_params(ssid="eht", passphrase="12345678")
    params["ieee80211ax"] = "1"
    params["ieee80211be"] = "1"
    params['ieee80211w'] = '1' if transition_mode else '2'
    params['rsn_pairwise'] = "CCMP GCMP-256" if transition_mode else "GCMP-256"
    params['group_cipher'] = "CCMP" if transition_mode else "GCMP-256"
    params["group_mgmt_cipher"] = "AES-128-CMAC" if transition_mode else "BIP-GMAC-256"
    params['beacon_prot'] = '1'
    params['wpa_key_mgmt'] = "SAE SAE-EXT-KEY WPA-PSK WPA-PSK-SHA256" if transition_mode else 'SAE-EXT-KEY'
    params['sae_groups'] = "19 20" if transition_mode else "20"
    params['sae_pwe'] = "2" if transition_mode else "1"
    if anti_clogging_token:
        params['sae_anti_clogging_threshold'] = '0'
    if ml:
        ml_elem = "ff0d6b" + "3001" + "0a" + "021122334455" + "01" + "00" + "00"
        params['vendor_elements'] = ml_elem
    try:
        hapd = hostapd.add_ap(apdev, params)
    except Exception as e:
        if isinstance(e, Exception) and \
           str(e) == "Failed to set hostapd parameter ieee80211be":
            raise HwsimSkip("EHT not supported")
        raise

def test_eht_sae(dev, apdev):
    """EHT AP with SAE"""
    check_sae_capab(dev[0])

    hapd = start_eht_sae_ap(apdev[0])
    try:
        dev[0].set("sae_groups", "20")
        dev[0].set("sae_pwe", "2")
        dev[0].connect("eht", key_mgmt="SAE-EXT-KEY", psk="12345678",
                       ieee80211w="2", beacon_prot="1",
                       pairwise="GCMP-256", group="GCMP-256",
                       group_mgmt="BIP-GMAC-256", scan_freq="2412")
    finally:
        dev[0].set("sae_groups", "")
        dev[0].set("sae_pwe", "0")

def test_eht_sae_mlo(dev, apdev):
    """EHT+MLO AP with SAE"""
    check_sae_capab(dev[0])

    hapd = start_eht_sae_ap(apdev[0], ml=True)
    try:
        dev[0].set("sae_groups", "20")
        dev[0].set("sae_pwe", "2")
        dev[0].connect("eht", key_mgmt="SAE-EXT-KEY", psk="12345678",
                       ieee80211w="2", beacon_prot="1",
                       pairwise="GCMP-256", group="GCMP-256",
                       group_mgmt="BIP-GMAC-256", scan_freq="2412")
    finally:
        dev[0].set("sae_groups", "")
        dev[0].set("sae_pwe", "0")

def test_eht_sae_mlo_tm(dev, apdev):
    """EHT+MLO AP with SAE and transition mode"""
    check_sae_capab(dev[0])
    check_sae_capab(dev[1])

    hapd = start_eht_sae_ap(apdev[0], ml=True, transition_mode=True)
    try:
        dev[0].set("sae_groups", "20")
        dev[0].set("sae_pwe", "2")
        dev[0].connect("eht", key_mgmt="SAE-EXT-KEY", psk="12345678",
                       ieee80211w="2", beacon_prot="1",
                       pairwise="GCMP-256", group="CCMP",
                       group_mgmt="AES-128-CMAC", scan_freq="2412")
        dev[1].set("sae_groups", "19")
        dev[1].connect("eht", key_mgmt="SAE-EXT-KEY", psk="12345678",
                       ieee80211w="2", beacon_prot="1",
                       pairwise="CCMP", group="CCMP",
                       group_mgmt="AES-128-CMAC", scan_freq="2412",
                       disable_eht="1")
        dev[2].connect("eht", key_mgmt="WPA-PSK", psk="12345678",
                       pairwise="CCMP", group="CCMP",
                       group_mgmt="AES-128-CMAC", scan_freq="2412",
                       disable_eht="1")
    finally:
        dev[0].set("sae_groups", "")
        dev[0].set("sae_pwe", "0")
        dev[1].set("sae_groups", "")

def test_eht_sae_mlo_anti_clogging_token(dev, apdev):
    """EHT+MLO AP with SAE and anti-clogging token"""
    check_sae_capab(dev[0])

    hapd = start_eht_sae_ap(apdev[0], ml=True, anti_clogging_token=True)
    try:
        dev[0].set("sae_groups", "20")
        dev[0].set("sae_pwe", "2")
        dev[0].connect("eht", key_mgmt="SAE-EXT-KEY", psk="12345678",
                       ieee80211w="2", beacon_prot="1",
                       pairwise="GCMP-256", group="GCMP-256",
                       group_mgmt="BIP-GMAC-256", scan_freq="2412")
    finally:
        dev[0].set("sae_groups", "")
        dev[0].set("sae_pwe", "0")

def eht_mld_enable_ap(iface, link_id, params):
    hapd = hostapd.add_mld_link(iface, link_id, params)
    hapd.enable()

    ev = hapd.wait_event(["AP-ENABLED", "AP-DISABLED"], timeout=1)
    if ev is None:
        raise Exception("AP startup timed out")
    if "AP-ENABLED" not in ev:
        raise Exception("AP startup failed")

    return hapd

def eht_mld_ap_wpa2_params(ssid, passphrase=None, key_mgmt="WPA-PSK-SHA256",
                           mfp="2", pwe=None, beacon_prot="1", bridge=False):
    params = hostapd.wpa2_params(ssid=ssid, passphrase=passphrase,
                                 wpa_key_mgmt=key_mgmt, ieee80211w=mfp)
    params['ieee80211n'] = '1'
    params['ieee80211ax'] = '1'
    params['ieee80211be'] = '1'
    params['channel'] = '1'
    params['hw_mode'] = 'g'
    params['group_mgmt_cipher'] = "AES-128-CMAC"
    params['beacon_prot'] = beacon_prot
    if bridge:
        params['bridge'] = 'ap-br0'

    if pwe is not None:
        params['sae_pwe'] = pwe

    return params

def _eht_mld_probe_req(wpas, hapd, tsf0, link_id=-1):
    if "OK" not in wpas.request("ML_PROBE_REQ bssid=%s mld_id=0 link_id=%d" % (hapd.own_addr(), link_id)):
        raise Exception("Failed to request ML probe request")

    ev = wpas.wait_event(["CTRL-EVENT-SCAN-STARTED"])
    if ev is None:
        raise Exception("Scan did not start")

    ev = wpas.wait_event(["CTRL-EVENT-SCAN-RESULTS"])
    if ev is None:
        raise Exception("Scan did not complete")

    logger.info("ML Probe request scan done")

    bss = wpas.get_bss(hapd.own_addr())
    if not bss:
        raise Exception("AP did not reply to ML probe request")

    tsf1 = int(bss['tsf'])
    logger.info("tsf0=%s, tsf1=%s" % (tsf0, tsf1))

    if tsf0 >= tsf1:
        raise Exception("AP was not found in ML probe request scan")

def test_eht_mld_discovery(dev, apdev):
    """EHT MLD AP discovery"""
    with HWSimRadio(use_mlo=True) as (hapd_radio, hapd_iface), \
        HWSimRadio(use_mlo=True) as (wpas_radio, wpas_iface):

        wpas = WpaSupplicant(global_iface='/tmp/wpas-wlan5')
        wpas.interface_add(wpas_iface)

        ssid = "mld_ap"
        link0_params = {"ssid": ssid,
                        "hw_mode": "g",
                        "channel": "1"}
        link1_params = {"ssid": ssid,
                        "hw_mode": "g",
                        "channel": "2"}

        hapd0 = eht_mld_enable_ap(hapd_iface, 0, link0_params)
        hapd1 = eht_mld_enable_ap(hapd_iface, 1, link1_params)

        # Only scan link 0
        res = wpas.request("SCAN freq=2412")
        if "FAIL" in res:
            raise Exception("Failed to start scan")

        ev = wpas.wait_event(["CTRL-EVENT-SCAN-STARTED"])
        if ev is None:
            raise Exception("Scan did not start")

        ev = wpas.wait_event(["CTRL-EVENT-SCAN-RESULTS"])
        if ev is None:
            raise Exception("Scan did not complete")

        logger.info("Scan done")

        rnr_pattern = re.compile(".*ap_info.*, mld ID=0, link ID=",
                                 re.MULTILINE)
        ml_pattern = re.compile(".*multi-link:.*, MLD addr=.*", re.MULTILINE)

        bss = wpas.request("BSS " + hapd0.own_addr())
        logger.info("BSS 0: " + str(bss))

        if rnr_pattern.search(bss) is None:
            raise Exception("RNR element not found for first link")

        if ml_pattern.search(bss) is None:
            raise Exception("ML element not found for first link")

        # Save the tsf0 for checking ML Probe request scan later
        tsf0 = int(wpas.get_bss(hapd0.own_addr())['tsf'])

        if wpas.get_bss(hapd1.own_addr()) is not None:
            raise Exception("BSS for link 1 found without ML probe request")

        # Now send an ML probe request (for all links)
        _eht_mld_probe_req(wpas, hapd0, tsf0)
        tsf0 = int(wpas.get_bss(hapd0.own_addr())['tsf'])

        # NOTE: hostapd incorrectly reports a TSF offset of zero
        # This only works because the source is always the ML probe response
        tsf1 = int(wpas.get_bss(hapd1.own_addr())['tsf'])

        bss = wpas.request("BSS " + hapd1.own_addr())
        logger.info("BSS 1: " + str(bss))

        if rnr_pattern.search(bss) is None:
            raise Exception("RNR element not found for second link")

        if ml_pattern.search(bss) is None:
            raise Exception("ML element not found for second link")

        _eht_mld_probe_req(wpas, hapd0, tsf0, link_id=1)
        if int(wpas.get_bss(hapd1.own_addr())['tsf']) <= tsf1:
            raise Exception("Probe for link ID did not update BSS")
        tsf0 = int(wpas.get_bss(hapd0.own_addr())['tsf'])
        tsf1 = int(wpas.get_bss(hapd1.own_addr())['tsf'])

        # Probing the wrong link ID should not update second link
        _eht_mld_probe_req(wpas, hapd0, tsf0, link_id=4)
        if int(wpas.get_bss(hapd1.own_addr())['tsf']) != tsf1:
            raise Exception("Probe for other link ID not updated BSS")

def test_eht_mld_owe_two_links(dev, apdev):
    """EHT MLD AP with MLD client OWE connection using two links"""
    _eht_mld_owe_two_links(dev, apdev)

def test_eht_mld_owe_two_links_scan_second(dev, apdev):
    """EHT MLD AP with MLD client OWE connection using two links; scan only second"""
    _eht_mld_owe_two_links(dev, apdev, scan_only_second_link=True)

def _eht_mld_owe_two_links(dev, apdev, second_link_disabled=False,
                           only_one_link=False, scan_only_second_link=False):
    with HWSimRadio(use_mlo=True) as (hapd0_radio, hapd0_iface), \
        HWSimRadio(use_mlo=True) as (hapd1_radio, hapd1_iface), \
        HWSimRadio(use_mlo=True) as (wpas_radio, wpas_iface):

        wpas = WpaSupplicant(global_iface='/tmp/wpas-wlan5')
        wpas.interface_add(wpas_iface)

        ssid = "mld_ap_owe_two_link"
        params = eht_mld_ap_wpa2_params(ssid, key_mgmt="OWE", mfp="2")

        hapd0 = eht_mld_enable_ap(hapd0_iface, 0, params)

        params['channel'] = '6'
        if second_link_disabled:
            params['mld_indicate_disabled'] = '1'

        hapd1 = eht_mld_enable_ap(hapd0_iface, 1, params)
        # Check legacy client connection
        dev[0].connect(ssid, scan_freq="2437", key_mgmt="OWE", ieee80211w="2")

        if only_one_link:
            link0 = hapd0.get_status_field("link_addr")
            wpas.set("bssid_filter", link0)
        scan_freq = "2437" if scan_only_second_link else "2412 2437"
        wpas.connect(ssid, scan_freq=scan_freq, key_mgmt="OWE",
                     ieee80211w="2")

        active_links = 3
        valid_links = 3
        if second_link_disabled:
            dlinks = _eht_dormant_links(wpas)
            if dlinks != 2:
                raise Exception("Unexpected dormant links")
            active_links = 1
        if only_one_link:
            active_links = 1
            valid_links = 1

        eht_verify_status(wpas, hapd0, 2412, 20, is_ht=True, mld=True,
                          valid_links=valid_links, active_links=active_links)
        eht_verify_wifi_version(wpas)
        traffic_test(wpas, hapd0)

        if not second_link_disabled:
            traffic_test(wpas, hapd1)

        if only_one_link:
            wpas.set("bssid_filter", "")

def test_eht_mld_owe_two_links_one_disabled(dev, apdev):
    """AP MLD with MLD client OWE connection when one of the AP MLD links is disabled"""
    _eht_mld_owe_two_links(dev, apdev, second_link_disabled=True)

def test_eht_mld_owe_two_links_only_one_negotiated(dev, apdev):
    """AP MLD with MLD client OWE connection when only one of the links is negotiated"""
    _eht_mld_owe_two_links(dev, apdev, only_one_link=True)

def test_eht_mld_sae_single_link(dev, apdev):
    """EHT MLD AP with MLD client SAE H2E connection using single link"""
    run_eht_mld_sae_single_link(dev, apdev)

def test_eht_mld_sae_single_link_anti_clogging_token(dev, apdev):
    """EHT MLD AP with MLD client SAE H2E connection using single link and SAE anti-clogging token"""
    run_eht_mld_sae_single_link(dev, apdev, anti_clogging_token=True)

def run_eht_mld_sae_single_link(dev, apdev, anti_clogging_token=False):
    with HWSimRadio(use_mlo=True) as (hapd_radio, hapd_iface), \
            HWSimRadio(use_mlo=True) as (wpas_radio, wpas_iface):
        wpas = WpaSupplicant(global_iface='/tmp/wpas-wlan5')
        wpas.interface_add(wpas_iface)

        passphrase = 'qwertyuiop'
        ssid = "mld_ap_sae_single_link"
        params = eht_mld_ap_wpa2_params(ssid, passphrase, key_mgmt="SAE",
                                        mfp="2", pwe='2')
        if anti_clogging_token:
            params['sae_anti_clogging_threshold'] = '0'

        hapd0 = eht_mld_enable_ap(hapd_iface, 0, params)

        wpas.set("sae_pwe", "1")
        wpas.connect(ssid, sae_password=passphrase, scan_freq="2412",
                     key_mgmt="SAE", ieee80211w="2")

        eht_verify_status(wpas, hapd0, 2412, 20, is_ht=True, mld=True,
                          valid_links=1, active_links=1)
        eht_verify_wifi_version(wpas)
        traffic_test(wpas, hapd0)

def run_eht_mld_sae_two_links(dev, apdev, beacon_prot="1",
                              disable_enable=False, bridge=False):
    with HWSimRadio(use_mlo=True) as (hapd_radio, hapd_iface), \
        HWSimRadio(use_mlo=True) as (wpas_radio, wpas_iface):

        wpas = WpaSupplicant(global_iface='/tmp/wpas-wlan5')
        wpas.interface_add(wpas_iface)

        passphrase = 'qwertyuiop'
        ssid = "mld_ap_sae_two_link"
        params = eht_mld_ap_wpa2_params(ssid, passphrase,
                                        key_mgmt="SAE", mfp="2", pwe='1',
                                        beacon_prot=beacon_prot,
                                        bridge=bridge)

        hapd0 = eht_mld_enable_ap(hapd_iface, 0, params)

        params['channel'] = '6'

        hapd1 = eht_mld_enable_ap(hapd_iface, 1, params)

        if bridge:
            hapd0.cmd_execute(['brctl', 'setfd', 'ap-br0', '0'])
            hapd0.cmd_execute(['ip', 'link', 'set', 'dev', 'ap-br0', 'up'])

        wpas.set("sae_pwe", "1")

        # The first authentication attempt tries to use group 20 and the
        # authentication is expected to fail. The next authentication should
        # use group 19 and succeed.
        wpas.set("sae_groups", "20 19")

        wpas.connect(ssid, sae_password=passphrase, scan_freq="2412 2437",
                     key_mgmt="SAE", ieee80211w="2", beacon_prot="1")

        eht_verify_status(wpas, hapd0, 2412, 20, is_ht=True, mld=True,
                          valid_links=3, active_links=3)
        eht_verify_wifi_version(wpas)

        if wpas.get_status_field('sae_group') != '19':
            raise Exception("Expected SAE group not used")

        traffic_test(wpas, hapd0, ifname2='ap-br0' if bridge else None)
        traffic_test(wpas, hapd1, ifname2='ap-br0' if bridge else None)

        if disable_enable:
            if "OK" not in hapd0.request("DISABLE_MLD"):
                raise Exception("DISABLE_MLD failed")
            ev = hapd0.wait_event(["AP-DISABLED"], timeout=1)
            if ev is None:
                raise Exception("AP-DISABLED not received (0)")
            ev = hapd1.wait_event(["AP-DISABLED"], timeout=1)
            if ev is None:
                raise Exception("AP-DISABLED not received (1)")

            # mac80211 does not seem to detect beacon loss or deauthentication
            # in non-AP MLD case?! For now, ignore that and just force
            # disconnection locally on the STA.
            wpas.request("DISCONNECT")
            wpas.wait_disconnected()

            if "OK" not in hapd0.request("ENABLE_MLD"):
                raise Exception("ENABLE_MLD failed")
            ev = hapd0.wait_event(["AP-ENABLED"], timeout=1)
            if ev is None:
                raise Exception("AP-ENABLED not received (0)")
            ev = hapd1.wait_event(["AP-ENABLED"], timeout=1)
            if ev is None:
                raise Exception("AP-ENABLED not received (1)")

            # TODO: Figure out why this fails without PMKSA_FLUSH. Things should
            # fall back to full SAE from failed PMKSA caching attempt
            # automatically.
            wpas.request("PMKSA_FLUSH")

            # flush the BSS table before reconnect as otherwise the old
            # AP MLD BSSs would be in the BSS list
            wpas.request("BSS_FLUSH 0")
            wpas.request("RECONNECT")
            wpas.wait_connected()
            hapd0.wait_sta()
            hapd1.wait_sta()
            traffic_test(wpas, hapd0, ifname2='ap-br0' if bridge else None)
            traffic_test(wpas, hapd1, ifname2='ap-br0' if bridge else None)

def test_eht_mld_sae_two_links(dev, apdev):
    """EHT MLD AP with MLD client SAE H2E connection using two links"""
    run_eht_mld_sae_two_links(dev, apdev)

def test_eht_mld_sae_two_links_no_beacon_prot(dev, apdev):
    """EHT MLD AP with MLD client SAE H2E connection using two links and no beacon protection"""
    run_eht_mld_sae_two_links(dev, apdev, beacon_prot="0")

def test_eht_mld_sae_two_links_disable_enable(dev, apdev):
    """AP MLD with two links and disabling/enabling full AP MLD"""
    run_eht_mld_sae_two_links(dev, apdev, disable_enable=True)

def test_eht_mld_sae_two_links_bridge(dev, apdev):
    """AP MLD with two links in a bridge"""
    run_eht_mld_sae_two_links(dev, apdev, bridge=True)

def test_eht_mld_sae_ext_one_link(dev, apdev):
    """EHT MLD AP with MLD client SAE-EXT H2E connection using single link"""
    with HWSimRadio(use_mlo=True) as (hapd_radio, hapd_iface), \
        HWSimRadio(use_mlo=True) as (wpas_radio, wpas_iface):

        wpas = WpaSupplicant(global_iface='/tmp/wpas-wlan5')
        wpas.interface_add(wpas_iface)

        passphrase = 'qwertyuiop'
        ssid = "mld_ap_sae_ext_single_link"
        params = eht_mld_ap_wpa2_params(ssid, passphrase, key_mgmt="SAE-EXT-KEY")

        hapd0 = eht_mld_enable_ap(hapd_iface, 0, params)

        wpas.connect(ssid, sae_password=passphrase, scan_freq="2412",
                     key_mgmt="SAE-EXT-KEY", ieee80211w="2")

        eht_verify_status(wpas, hapd0, 2412, 20, is_ht=True, mld=True,
                          valid_links=1, active_links=1)
        eht_verify_wifi_version(wpas)
        traffic_test(wpas, hapd0)

def test_eht_mld_sae_ext_two_links(dev, apdev):
    """EHT MLD AP with MLD client SAE-EXT H2E connection using two links"""
    with HWSimRadio(use_mlo=True) as (hapd_radio, hapd_iface), \
        HWSimRadio(use_mlo=True) as (wpas_radio, wpas_iface):

        wpas = WpaSupplicant(global_iface='/tmp/wpas-wlan5')
        wpas.interface_add(wpas_iface)

        passphrase = 'qwertyuiop'
        ssid = "mld_ap_sae_two_link"
        params = eht_mld_ap_wpa2_params(ssid, passphrase,
                                        key_mgmt="SAE-EXT-KEY")

        hapd0 = eht_mld_enable_ap(hapd_iface, 0, params)

        params['channel'] = '6'

        hapd1 = eht_mld_enable_ap(hapd_iface, 1, params)

        wpas.connect(ssid, sae_password=passphrase, scan_freq="2412 2437",
                     key_mgmt="SAE-EXT-KEY", ieee80211w="2")

        eht_verify_status(wpas, hapd0, 2412, 20, is_ht=True, mld=True,
                          valid_links=3, active_links=3)
        eht_verify_wifi_version(wpas)
        traffic_test(wpas, hapd0)
        traffic_test(wpas, hapd1)

def test_eht_mld_sae_legacy_client(dev, apdev):
    """EHT MLD AP with legacy client SAE H2E connection"""
    with HWSimRadio(use_mlo=True) as (hapd_radio, hapd_iface):
        passphrase = 'qwertyuiop'
        ssid = "mld_ap_sae_two_link"
        params = eht_mld_ap_wpa2_params(ssid, passphrase,
                                        key_mgmt="SAE", mfp="2", pwe='1')

        hapd0 = eht_mld_enable_ap(hapd_iface, 0, params)

        params['channel'] = '6'

        hapd1 = eht_mld_enable_ap(hapd_iface, 1, params)

        try:
            dev[0].set("sae_groups", "")
            dev[0].set("sae_pwe", "1")
            dev[0].connect(ssid, sae_password=passphrase, scan_freq="2412",
                           key_mgmt="SAE", ieee80211w="2", beacon_prot="1")
            logger.info("wpa_supplicant STATUS:\n" + dev[0].request("STATUS"))
            bssid = dev[0].get_status_field("bssid")
            if hapd0.own_addr() == bssid:
                hapd0.wait_sta();
            elif hapd1.own_addr() == bssid:
                hapd1.wait_sta();
            else:
                raise Exception("Unknown BSSID: " + bssid)

            eht_verify_status(dev[0], hapd0, 2412, 20, is_ht=True)
            traffic_test(dev[0], hapd0)
        finally:
            dev[0].set("sae_groups", "")
            dev[0].set("sae_pwe", "0")

def test_eht_mld_sae_transition(dev, apdev):
    """EHT MLD AP in SAE/PSK transition mode with MLD client connection using two links"""
    with HWSimRadio(use_mlo=True) as (hapd_radio, hapd_iface), \
        HWSimRadio(use_mlo=True) as (wpas_radio, wpas_iface):

        wpas = WpaSupplicant(global_iface='/tmp/wpas-wlan5')
        wpas.interface_add(wpas_iface)

        passphrase = 'qwertyuiop'
        ssid = "mld_ap_sae_two_link"
        params = eht_mld_ap_wpa2_params(ssid, passphrase,
                                        key_mgmt="SAE-EXT-KEY SAE WPA-PSK WPA-PSK-SHA256",
                                        mfp="1")

        hapd0 = eht_mld_enable_ap(hapd_iface, 0, params)

        params['channel'] = '6'

        hapd1 = eht_mld_enable_ap(hapd_iface, 1, params)

        wpas.connect(ssid, sae_password=passphrase, scan_freq="2412 2437",
                     key_mgmt="SAE-EXT-KEY", ieee80211w="2")

        eht_verify_status(wpas, hapd0, 2412, 20, is_ht=True, mld=True,
                          valid_links=3, active_links=3)
        eht_verify_wifi_version(wpas)
        traffic_test(wpas, hapd0)
        traffic_test(wpas, hapd1)

        dev[0].set("sae_groups", "")
        dev[0].connect(ssid, sae_password=passphrase, scan_freq="2412",
                       key_mgmt="SAE", ieee80211w="2", beacon_prot="1")
        dev[1].connect(ssid, psk=passphrase, scan_freq="2412",
                       key_mgmt="WPA-PSK", ieee80211w="0")

def test_eht_mld_ptk_rekey(dev, apdev):
    """EHT MLD AP and PTK rekeying with MLD client connection using two links"""
    with HWSimRadio(use_mlo=True) as (hapd_radio, hapd_iface), \
        HWSimRadio(use_mlo=True) as (wpas_radio, wpas_iface):

        wpas = WpaSupplicant(global_iface='/tmp/wpas-wlan5')
        wpas.interface_add(wpas_iface)

        passphrase = 'qwertyuiop'
        ssid = "mld_ap_sae_two_link"
        params = eht_mld_ap_wpa2_params(ssid, passphrase,
                                        key_mgmt="SAE-EXT-KEY SAE WPA-PSK WPA-PSK-SHA256",
                                        mfp="1")
        params['wpa_ptk_rekey'] = '5'

        hapd0 = eht_mld_enable_ap(hapd_iface, 0, params)

        params['channel'] = '6'

        hapd1 = eht_mld_enable_ap(hapd_iface, 1, params)

        wpas.connect(ssid, sae_password=passphrase, scan_freq="2412 2437",
                     key_mgmt="SAE-EXT-KEY", ieee80211w="2")
        ev0 = hapd0.wait_event(["AP-STA-CONNECT"], timeout=1)
        if ev0 is None:
            ev1 = hapd1.wait_event(["AP-STA-CONNECT"], timeout=1)
        traffic_test(wpas, hapd0)
        traffic_test(wpas, hapd1)

        ev = wpas.wait_event(["WPA: Key negotiation completed",
                              "CTRL-EVENT-DISCONNECTED"], timeout=10)
        if ev is None:
            raise Exception("PTK rekey timed out")
        if "CTRL-EVENT-DISCONNECTED" in ev:
            raise Exception("Disconnect instead of rekey")

        time.sleep(0.1)
        traffic_test(wpas, hapd0)
        traffic_test(wpas, hapd1)

def test_eht_mld_gtk_rekey(dev, apdev):
    """AP MLD and GTK rekeying with MLD client connection using two links"""
    with HWSimRadio(use_mlo=True) as (hapd_radio, hapd_iface), \
        HWSimRadio(use_mlo=True) as (wpas_radio, wpas_iface):

        wpas = WpaSupplicant(global_iface='/tmp/wpas-wlan5')
        wpas.interface_add(wpas_iface)

        passphrase = 'qwertyuiop'
        ssid = "mld_ap_sae_two_link"
        params = eht_mld_ap_wpa2_params(ssid, passphrase,
                                        key_mgmt="SAE-EXT-KEY SAE WPA-PSK WPA-PSK-SHA256",
                                        mfp="1")
        params['wpa_group_rekey'] = '5'

        hapd0 = eht_mld_enable_ap(hapd_iface, 0, params)

        params['channel'] = '6'

        hapd1 = eht_mld_enable_ap(hapd_iface, 1, params)

        wpas.connect(ssid, sae_password=passphrase, scan_freq="2412 2437",
                     key_mgmt="SAE-EXT-KEY", ieee80211w="2")
        ev0 = hapd0.wait_event(["AP-STA-CONNECT"], timeout=1)
        if ev0 is None:
            ev1 = hapd1.wait_event(["AP-STA-CONNECT"], timeout=1)
        traffic_test(wpas, hapd0)
        traffic_test(wpas, hapd1)

        for i in range(2):
            ev = wpas.wait_event(["MLO RSN: Group rekeying completed",
                                  "CTRL-EVENT-DISCONNECTED"], timeout=10)
            if ev is None:
                raise Exception("GTK rekey timed out")
            if "CTRL-EVENT-DISCONNECTED" in ev:
                raise Exception("Disconnect instead of rekey")

            time.sleep(0.1)
            traffic_test(wpas, hapd0)
            traffic_test(wpas, hapd1)

def test_eht_mld_gtk_rekey_failure(dev, apdev):
    """AP MLD and GTK rekeying failure with MLD client connection using two links"""
    with HWSimRadio(use_mlo=True) as (hapd_radio, hapd_iface), \
        HWSimRadio(use_mlo=True) as (wpas_radio, wpas_iface):

        wpas = WpaSupplicant(global_iface='/tmp/wpas-wlan5')
        wpas.interface_add(wpas_iface)

        passphrase = 'qwertyuiop'
        ssid = "mld_ap_sae_two_link"
        params = eht_mld_ap_wpa2_params(ssid, passphrase,
                                        key_mgmt="SAE-EXT-KEY SAE WPA-PSK WPA-PSK-SHA256",
                                        mfp="1")
        params['wpa_group_rekey'] = '5'

        hapd0 = eht_mld_enable_ap(hapd_iface, 0, params)

        params['channel'] = '6'

        hapd1 = eht_mld_enable_ap(hapd_iface, 1, params)

        wpas.connect(ssid, sae_password=passphrase, scan_freq="2412 2437",
                     key_mgmt="SAE-EXT-KEY", ieee80211w="2")
        ev0 = hapd0.wait_event(["AP-STA-CONNECT"], timeout=1)
        if ev0 is None:
            ev1 = hapd1.wait_event(["AP-STA-CONNECT"], timeout=1)

        # Force group handshake to time out
        hapd0.request("SET ext_eapol_frame_io 1")
        hapd1.request("SET ext_eapol_frame_io 1")

        ev = wpas.wait_event(["CTRL-EVENT-DISCONNECTED"], timeout=20)
        if ev is None:
            raise Exception("Disconnection not reported")
        if "reason=16" not in ev.split():
            raise Exception("Unexpected disconnection reason: " + ev)

        hapd0.dump_monitor()
        hapd1.dump_monitor()

        # Re-enable automatic EAPOL frame processing
        hapd0.request("SET ext_eapol_frame_io 0")
        hapd1.request("SET ext_eapol_frame_io 0")

        wpas.wait_connected()

def test_eht_ml_probe_req(dev, apdev):
    """AP MLD with two links and non-AP MLD sending ML Probe Request"""
    with HWSimRadio(use_mlo=True) as (hapd_radio, hapd_iface), \
        HWSimRadio(use_mlo=True) as (wpas_radio, wpas_iface):

        wpas = WpaSupplicant(global_iface='/tmp/wpas-wlan5')
        wpas.interface_add(wpas_iface)

        passphrase = 'qwertyuiop'
        ssid = "mld_ap_sae_two_link"
        params = eht_mld_ap_wpa2_params(ssid, passphrase,
                                        key_mgmt="SAE-EXT-KEY")

        hapd0 = eht_mld_enable_ap(hapd_iface, 0, params)

        params['channel'] = '6'

        hapd1 = eht_mld_enable_ap(hapd_iface, 1, params)

        bssid = hapd0.own_addr()
        wpas.scan_for_bss(bssid, freq=2412)

        time.sleep(1)
        cmd = "ML_PROBE_REQ bssid=" + bssid + " mld_id=0"
        if "OK" not in wpas.request(cmd):
            raise Exception("Failed to run: " + cmd)
        ev = wpas.wait_event(["CTRL-EVENT-SCAN-RESULTS",
                              "CTRL-EVENT-SCAN-FAILED"], timeout=10)
        if ev is None:
            raise Exception("ML_PROBE_REQ did not result in scan results")

        time.sleep(1)
        cmd = "ML_PROBE_REQ bssid=" + bssid + " mld_id=0 link_id=2"
        if "OK" not in wpas.request(cmd):
            raise Exception("Failed to run: " + cmd)
        ev = wpas.wait_event(["CTRL-EVENT-SCAN-RESULTS",
                              "CTRL-EVENT-SCAN-FAILED"], timeout=10)
        if ev is None:
            raise Exception("ML_PROBE_REQ did not result in scan results")

def test_eht_mld_connect_probes(dev, apdev, params):
    """MLD client sends ML probe to connect to not discovered links"""
    with HWSimRadio(use_mlo=True) as (hapd_radio, hapd_iface), \
        HWSimRadio(use_mlo=True) as (wpas_radio, wpas_iface):

        wpas = WpaSupplicant(global_iface='/tmp/wpas-wlan5')
        wpas.interface_add(wpas_iface)

        ssid = "mld_ap"
        passphrase = 'qwertyuiop'
        link_params = eht_mld_ap_wpa2_params(ssid, passphrase, mfp="2",
                                             key_mgmt="SAE", pwe='2')
        link_params['channel'] = '1'
        link_params['bssid'] = '00:11:22:33:44:01'
        hapd0 = eht_mld_enable_ap(hapd_iface, 0, link_params)

        link_params['channel'] = '6'
        link_params['bssid'] = '00:11:22:33:44:02'
        hapd1 = eht_mld_enable_ap(hapd_iface, 1, link_params)

        wpas.set("sae_pwe", "1")
        wpas.connect(ssid, sae_password= passphrase, ieee80211w="2",
                     key_mgmt="SAE", scan_freq="2412")

        out = run_tshark(os.path.join(params['logdir'], 'hwsim0.pcapng'),
                         'wlan.fc.type_subtype == 0x0004 && wlan.ext_tag.length == 8 && wlan.ext_tag.number == 107 && wlan.eht.multi_link_control == 0x0011 && wlan.eht.multi_link.common_info.length == 2 && wlan.eht.multi_link.common_info.mld_id == 0 && wlan.eht.multi_link.sta_profile.subelt_id == 0 && wlan.eht.multi_link.sta_profile.subelt_len == 2 && wlan.eht.multi_link.type_1.sta_profile_count == 1 && wlan.eht.multi_link.sta_profile_id_list == 1',
                         display=['frame.number'])
        if not out.splitlines():
            # Fall back to older tshark version without support for EHT dissecting
            out = run_tshark(os.path.join(params['logdir'], 'hwsim0.pcapng'),
                             'wlan.fc.type_subtype == 0x0004 && wlan.ext_tag.number == 107 && wlan.ext_tag.data == 11:00:02:00:00:02:11:00',
                             display=['frame.number'])
            if not out.splitlines():
                raise Exception('ML probe request not found')

        # Probe Response frame has the ML element, which will be fragmented
        out = run_tshark(os.path.join(params['logdir'], "hwsim0.pcapng"),
                         "wlan.fc.type_subtype == 0x0005 && wlan.ext_tag.number == 107 && wlan.ext_tag.length == 254",
                         display=['frame.number'])
        if not out.splitlines():
            # This requires new tshark (e.g., 4.0.6); for now, ignore the issue
            # to avoid forcing such upgrade.
            logger.info('ML probe response not found')
            #raise Exception('ML probe response not found')

        eht_verify_status(wpas, hapd0, 2412, 20, is_ht=True, mld=True,
                          valid_links=3, active_links=3)
        traffic_test(wpas, hapd0)
        traffic_test(wpas, hapd1)

def test_eht_tx_link_rejected_connect_other(dev, apdev, params):
    """EHT MLD AP with MLD client being rejected on TX link, but then connecting on second link"""
    with HWSimRadio(use_mlo=True) as (hapd_radio, hapd_iface), \
        HWSimRadio(use_mlo=True) as (wpas_radio, wpas_iface):

        wpas = WpaSupplicant(global_iface='/tmp/wpas-wlan5')
        wpas.interface_add(wpas_iface)

        ssid = "mld_ap"
        passphrase = 'qwertyuiop'
        link_params = eht_mld_ap_wpa2_params(ssid, passphrase, mfp="2",
                                             key_mgmt="SAE", pwe='2')
        link_params['channel'] = '1'
        link_params['bssid'] = '00:11:22:33:44:01'
        hapd0 = eht_mld_enable_ap(hapd_iface, 0, link_params)

        link_params['channel'] = '6'
        link_params['bssid'] = '00:11:22:33:44:02'
        hapd1 = eht_mld_enable_ap(hapd_iface, 1, link_params)

        wpas.set("sae_pwe", "1")
        with fail_test(hapd0, 1, "hostapd_get_aid"):
            wpas.connect(ssid, sae_password=passphrase, ieee80211w="2",
                         key_mgmt="SAE", scan_freq="2412")

        eht_verify_status(wpas, hapd1, 2437, 20, is_ht=True, mld=True,
                          valid_links=2, active_links=2)
        traffic_test(wpas, hapd0)
        traffic_test(wpas, hapd1)

def test_eht_all_links_rejected(dev, apdev, params):
    """EHT MLD AP with MLD client ignores all rejected links"""
    with HWSimRadio(use_mlo=True) as (hapd_radio, hapd_iface), \
        HWSimRadio(use_mlo=True) as (wpas_radio, wpas_iface):

        wpas = WpaSupplicant(global_iface='/tmp/wpas-wlan5')
        wpas.interface_add(wpas_iface)

        ssid = "mld_ap"
        passphrase = 'qwertyuiop'
        link_params = eht_mld_ap_wpa2_params(ssid, passphrase, mfp="2",
                                             key_mgmt="SAE", pwe='2')
        link_params['channel'] = '1'
        link_params['bssid'] = '00:11:22:33:44:01'
        hapd0 = eht_mld_enable_ap(hapd_iface, 0, link_params)

        link_params['channel'] = '6'
        link_params['bssid'] = '00:11:22:33:44:02'
        hapd1 = eht_mld_enable_ap(hapd_iface, 1, link_params)
        wpas.set("mld_connect_bssid_pref", "00:11:22:33:44:01")
        wpas.set("sae_pwe", "1")

        with fail_test(hapd0, 1, "hostapd_get_aid",
                       1, "hostapd_process_assoc_ml_info"):
            wpas.connect(ssid, sae_password=passphrase, ieee80211w="2",
                         key_mgmt="SAE", scan_freq="2412", wait_connect=False)
            ev = wpas.wait_event(['CTRL-EVENT-ASSOC-REJECT'])
            if not ev:
                raise Exception('Rejection not found')

            ev1 = wpas.wait_event(['Added BSSID'])
            ev2 = wpas.wait_event(['Added BSSID'])
            if (not ev1 or not ev2) or \
                not ((hapd0.own_addr() in ev1 and hapd1.own_addr() in ev2) or
                     (hapd1.own_addr() in ev1 and hapd0.own_addr() in ev2)):
                raise Exception('Not all BSSs were added to the ignore list')

            # After this message, a new scan clears the ignore and the STA
            # connects.
            wpas.wait_connected(timeout=15)

def test_eht_connect_invalid_link(dev, apdev, params):
    """EHT MLD AP where one link is incorrectly configured and rejected by mac80211"""
    with HWSimRadio(use_mlo=True) as (hapd_radio, hapd_iface), \
        HWSimRadio(use_mlo=True) as (wpas_radio, wpas_iface):

        wpas = WpaSupplicant(global_iface='/tmp/wpas-wlan5')
        wpas.interface_add(wpas_iface)

        ssid = "mld_ap"
        passphrase = 'qwertyuiop'
        ssid = "mld_ap"
        passphrase = 'qwertyuiop'
        link_params = eht_mld_ap_wpa2_params(ssid, passphrase, mfp="2",
                                             key_mgmt="SAE", pwe='2')
        link_params['channel'] = '1'
        link_params['bssid'] = '00:11:22:33:44:01'
        hapd0 = eht_mld_enable_ap(hapd_iface, 0, link_params)

        link_params['channel'] = '6'
        link_params['bssid'] = '00:11:22:33:44:02'
        hapd1 = eht_mld_enable_ap(hapd_iface, 1, link_params)

        # We scan for both APs, then try to connect to link 0, but only the
        # second attempt will work if mac80211 rejects the second link.
        wpas.set("mld_connect_bssid_pref", "00:11:22:33:44:01")
        wpas.set("sae_pwe", "1")
        with fail_test(wpas, 1, "assoc;wpa_driver_nl80211_associate",
                             2, "link;wpa_driver_nl80211_associate"):
            wpas.connect(ssid, sae_password=passphrase, ieee80211w="2",
                         key_mgmt="SAE", scan_freq="2412")

        eht_verify_status(wpas, hapd0, 2412, 20, is_ht=True, mld=True,
                          valid_links=1, active_links=1)

        out = run_tshark(os.path.join(params['logdir'], 'hwsim0.pcapng'),
                         'wlan.fc.type_subtype == 0x0000 && wlan.ext_tag.length == 11 && wlan.ext_tag.number == 107 && wlan.eht.multi_link_control == 0x0100 && wlan.eht.multi_link.common_info.length == 9 && wlan.eht.multi_link.common_info.mld_mac_address == %s && wlan.eht.multi_link.common_info.mld_capabilities == 0x0000' % wpas.own_addr(),
                         display=['frame.number'])
        if out.splitlines():
            return

        # Fall back to older tshark version without support for EHT dissecting
        out = run_tshark(os.path.join(params['logdir'], 'hwsim0.pcapng'),
                         'wlan.fc.type_subtype == 0x0000 && wlan.ext_tag.data == 00:01:09:%s:00:00' % wpas.own_addr(),
                         display=['frame.number'])
        if not out.splitlines():
            raise Exception('Association request send by mac80211 had unexpected ML element content (probably it contained a second link)')

def test_eht_mld_link_removal(dev, apdev):
    """EHT MLD with two links. Links removed during association"""

    with HWSimRadio(use_mlo=True) as (hapd0_radio, hapd0_iface), \
        HWSimRadio(use_mlo=True) as (wpas_radio, wpas_iface):

        wpas = WpaSupplicant(global_iface='/tmp/wpas-wlan5')
        wpas.interface_add(wpas_iface)

        ssid = "mld_ap_owe_two_link"
        params = eht_mld_ap_wpa2_params(ssid, key_mgmt="OWE", mfp="2")
        hapd0 = eht_mld_enable_ap(hapd0_iface, 0, params)

        params['channel'] = '6'
        hapd1 = eht_mld_enable_ap(hapd0_iface, 1, params)

        wpas.connect(ssid, scan_freq="2412 2437", key_mgmt="OWE",
                     ieee80211w="2")
        eht_verify_status(wpas, hapd0, 2412, 20, is_ht=True, mld=True,
                          valid_links=3, active_links=3)
        eht_verify_wifi_version(wpas)
        traffic_test(wpas, hapd0)

        logger.info("Disable the 2nd link in 4 beacon intervals")
        hapd1.link_remove(4)
        time.sleep(0.6)

        logger.info("Test traffic after 2nd link disabled")
        traffic_test(wpas, hapd0)

        if "OK" not in hapd0.request("REKEY_GTK"):
            raise Exception("REKEY_GTK failed")

        ev = wpas.wait_event(["MLO RSN: Group rekeying completed"], timeout=2)
        if ev is None:
            raise Exception("GTK rekey timed out")

        traffic_test(wpas, hapd0)

        logger.info("Disable the 1st link in 20 beacon intervals")
        hapd0.link_remove(20)
        time.sleep(1)

        logger.info("Verify that traffic is valid before the link is removed")
        traffic_test(wpas, hapd0)
        time.sleep(2)

        logger.info("Test traffic after 1st link disabled")
        traffic_test(wpas, hapd0, success=False)

def test_eht_mld_bss_trans_mgmt_link_removal_imminent(dev, apdev):
    """EHT MLD with two links. BSS transition management with link removal imminent"""

    with HWSimRadio(use_mlo=True) as (hapd0_radio, hapd0_iface), \
        HWSimRadio(use_mlo=True) as (wpas_radio, wpas_iface):

        wpas = WpaSupplicant(global_iface='/tmp/wpas-wlan5')
        wpas.interface_add(wpas_iface)

        ssid = "mld_ap_owe_two_link"
        params = eht_mld_ap_wpa2_params(ssid, key_mgmt="OWE", mfp="2")
        params["bss_transition"] = "1"
        params["mbo"] = "1"

        hapd0 = eht_mld_enable_ap(hapd0_iface, 0, params)

        params['channel'] = '6'

        hapd1 = eht_mld_enable_ap(hapd0_iface, 1, params)

        wpas.connect(ssid, scan_freq="2412 2437", key_mgmt="OWE",
                     ieee80211w="2")
        eht_verify_status(wpas, hapd0, 2412, 20, is_ht=True, mld=True,
                          valid_links=3, active_links=3)
        eht_verify_wifi_version(wpas)
        hapd0.wait_sta()
        hapd1.wait_sta()
        traffic_test(wpas, hapd0)

        addr = wpas.own_addr()
        cmd = "BSS_TM_REQ " + addr + " disassoc_timer=3 disassoc_imminent=1 link_removal_imminent=1 bss_term=0,1"
        if "OK" not in hapd0.request(cmd):
            raise Exception("BSS_TM_REQ command failed")

        # Only one link is terminate, so the STA is expected to remain
        # associated and not start a scan.
        ev = hapd0.wait_event(['BSS-TM-RESP'], timeout=5)
        # For now, allow this to pass without the BSS TM response since that
        # functionality with MLD needs a recent kernel change.
        #if ev is None:
        #    raise Exception("No BSS TM response received")
        if ev and "status_code=0" not in ev:
            raise Exception("Unexpected BSS TM response contents: " + ev)

        ev = wpas.wait_event(["CTRL-EVENT-SCAN-STARTED",
                              "CTRL-EVENT-DISCONNECTED"], timeout=10)
        if ev is not None:
            raise Exception("Unexpected action on STA: " + ev)

def send_check(hapd, frame, no_tx_status=False):
        cmd = "MGMT_RX_PROCESS freq=2412 datarate=0 ssi_signal=-30 frame="
        hapd.request(cmd + frame)
        if no_tx_status:
            return
        ev = hapd.wait_event(["MGMT-TX-STATUS"], timeout=1)
        if ev is None:
            raise Exception("No TX status")

def test_eht_ap_mld_proto(dev, apdev):
    """AP MLD protocol testing"""
    with HWSimRadio(use_mlo=True) as (hapd0_radio, hapd0_iface), \
        HWSimRadio(use_mlo=True) as (hapd1_radio, hapd1_iface):

        ssid = "mld_ap_owe_two_link"
        params = eht_mld_ap_wpa2_params(ssid, key_mgmt="OWE", mfp="2")

        hapd0 = eht_mld_enable_ap(hapd0_iface, 0, params)

        params['channel'] = '6'

        hapd1 = eht_mld_enable_ap(hapd0_iface, 1, params)

        ap_mld_addr = hapd0.get_status_field("mld_addr[0]").replace(':', '')
        bssid0 = hapd0.own_addr().replace(':', '')
        bssid1 = hapd1.own_addr().replace(':', '')

        time.sleep(1)
        hapd0.set("ext_mgmt_frame_handling", "1")
        hapd1.set("ext_mgmt_frame_handling", "1")

        # Truncated EML missing MLD Capabilities And operations field
        hapd0.note("Truncated EML missing MLD Capabilities And operations field")
        addr0 = "021122334400"
        addr1 = "021122334401"
        mld_addr = "02112233440f"
        hdr = "b0003a01" + bssid0 + addr0 + bssid0 + "1000"
        mle = "ff0a6b000007" + mld_addr
        auth = hdr + "0000" + "0100" + "0000" + mle
        send_check(hapd0, auth)

        hdr = "00000000" + bssid0 + mld_addr + bssid0 + "1000"
        ssid = "00136d6c645f61705f6f77655f74776f5f6c696e6b"
        supp_rates = "010802040b160c121824"
        ext_supp_rates = "32043048606c"
        rsne = "301a0100000fac040100000fac040100000fac12cc000000000fac06"
        ht_capab = "2d1afe131bffff000000000000000000000100000000000000000000"
        ext_capab = "7f0a04004a02014000400001"
        he_capab = "ff16230178c81a400000bfce0000000000000000fafffaff"
        eht_capab = "ff126c07007c0000feffff7f0100888888880000"
        supp_op_classes = "3b155151525354737475767778797a7b7c7d7e7f808182"
        dh_param = "ff23201300ea85e693343a079500cf4d461011a0ff90ec4de1af40165adbea94a3f36eb071"
        wmm = "dd070050f202000100"
        assocreq_start = "3004" + "0500" + ssid + supp_rates + ext_supp_rates + rsne + ht_capab + ext_capab + he_capab
        assocreq_end = eht_capab + supp_op_classes + dh_param + wmm

        # --> Not enough bytes for common info
        mle = "ff0a6b000109" + mld_addr
        send_check(hapd0, hdr + assocreq_start + mle + assocreq_end)

        # Truncated Non-Inheritance element
        hapd0.note("Truncated Non-Inheritance element")
        addr0 = "021122334410"
        addr1 = "021122334411"
        mld_addr = "02112233441f"
        hdr = "b0003a01" + bssid0 + addr0 + bssid0 + "1000"
        mle = "ff0a6b000007" + mld_addr
        auth = hdr + "0000" + "0100" + "0000" + mle
        send_check(hapd0, auth)

        # --> MLD: Invalid inheritance
        mle = "ff7d6b000109" + mld_addr + "0000"
        mle += "0067" + "3100" + "07" + addr1
        mle += "3004" + "010802040b160c121824" + "32043048606c" + "2d1afe131bffff000000000000000000000100000000000000000000" + "ff16230178c81a400000bfce0000000000000000fafffaff" + "ff126c07007c0000feffff7f0100888888880000"
        # Non-Inhericance element
        mle += "ff023800"
        # Unknown optional subelement
        mle += "aa00"
        # Vendor-Specific subelement
        mle += "dd0411223344"
        hdr = "00000000" + bssid0 + mld_addr + bssid0 + "1000"
        send_check(hapd0, hdr + assocreq_start + mle + assocreq_end,
                   no_tx_status=True)

        # Empty Non-Inheritance element
        hapd0.note("Empty Non-Inheritance element")
        addr0 = "021122334420"
        addr1 = "021122334421"
        mld_addr = "02112233442f"
        hdr = "b0003a01" + bssid0 + addr0 + bssid0 + "1000"
        mle = "ff0a6b000007" + mld_addr
        auth = hdr + "0000" + "0100" + "0000" + mle
        send_check(hapd0, auth)

        mle = "ff7e6b000109" + mld_addr + "0000"
        mle += "0068" + "3100" + "07" + addr1
        mle += "3004" + "010802040b160c121824" + "32043048606c" + "2d1afe131bffff000000000000000000000100000000000000000000" + "ff16230178c81a400000bfce0000000000000000fafffaff" + "ff126c07007c0000feffff7f0100888888880000"
        # Non-Inhericance element
        mle += "ff03380000"
        # Unknown optional subelement
        mle += "aa00"
        # Vendor-Specific subelement
        mle += "dd0411223344"
        hdr = "00000000" + bssid0 + mld_addr + bssid0 + "1000"
        send_check(hapd0, hdr + assocreq_start + mle + assocreq_end)

        # Non-Inheritance element
        hapd0.note("Non-Inheritance element")
        addr0 = "021122334430"
        addr1 = "021122334431"
        mld_addr = "02112233443f"
        hdr = "b0003a01" + bssid0 + addr0 + bssid0 + "1000"
        mle = "ff0a6b000007" + mld_addr
        auth = hdr + "0000" + "0100" + "0000" + mle
        send_check(hapd0, auth)

        mle = "ff9e6b000109" + mld_addr + "0000"
        mle += "0088" + "3100" + "07" + addr1
        mle += "3004" + "010802040b160c121824" + "32043048606c" + "2d1afe131bffff000000000000000000000100000000000000000000" + "ff16230178c81a400000bfce0000000000000000fafffaff" + "ff126c07007c0000feffff7f0100888888880000"
        # Non-Inhericance element
        mle += "ff2338" + "1010032a362137387172756b548bedeff0" + "106b01020304050607080c0d21643b3a36"
        # Unknown optional subelement
        mle += "aa00"
        # Vendor-Specific subelement
        mle += "dd0411223344"
        hdr = "00000000" + bssid0 + mld_addr + bssid0 + "1000"
        send_check(hapd0, hdr + assocreq_start + mle + assocreq_end)

        # No Non-Inheritance element
        hapd0.note("No Non-Inheritance element")
        addr0 = "021122334440"
        addr1 = "021122334441"
        mld_addr = "02112233444f"
        hdr = "b0003a01" + bssid0 + addr0 + bssid0 + "1000"
        mle = "ff0a6b000007" + mld_addr
        auth = hdr + "0000" + "0100" + "0000" + mle
        send_check(hapd0, auth)

        mle = "ff716b000109" + mld_addr + "0000"
        mle += "0063" + "3100" + "07" + addr1
        mle += "3004010802040b160c12182432043048606c2d1afe131bffff000000000000000000000100000000000000000000ff16230178c81a400000bfce0000000000000000fafffaffff126c07007c0000feffff7f0100888888880000"
        hdr = "00000000" + bssid0 + mld_addr + bssid0 + "1000"
        send_check(hapd0, hdr + assocreq_start + mle + assocreq_end)

def _5ghz_chanwidth_to_bw(op):
    return {
        0: "40",
        1: "80",
        2: "160",
        3: "80+80",
    }.get(op, "20")

def _test_eht_5ghz(dev, apdev, channel, chanwidth, ccfs1, ccfs2=0,
                   eht_oper_puncturing_override=None,
                   he_ccfs1=None, he_oper_chanwidth=None):
    if he_ccfs1 is None:
        he_ccfs1 = ccfs1
    if he_oper_chanwidth is None:
        he_oper_chanwidth = chanwidth

    try:
        params = {"ssid": "eht",
                  "country_code": "US",
                  "hw_mode": "a",
                  "channel": str(channel),
                  "ieee80211n": "1",
                  "ieee80211ac": "1",
                  "ieee80211ax": "1",
                  "ieee80211be": "1",
                  "vht_oper_chwidth": str(he_oper_chanwidth),
                  "vht_oper_centr_freq_seg0_idx": str(he_ccfs1),
                  "vht_oper_centr_freq_seg1_idx": str(ccfs2),
                  "he_oper_chwidth": str(he_oper_chanwidth),
                  "he_oper_centr_freq_seg0_idx": str(he_ccfs1),
                  "he_oper_centr_freq_seg1_idx": str(ccfs2),
                  "eht_oper_centr_freq_seg0_idx": str(ccfs1),
                  "eht_oper_chwidth": str(chanwidth)}

        if he_oper_chanwidth == 0:
            if channel < he_ccfs1:
                  params["ht_capab"] = "[HT40+]"
            elif channel > he_ccfs1:
                  params["ht_capab"] = "[HT40-]"
        else:
            params["ht_capab"] = "[HT40+]"
            if he_oper_chanwidth == 2:
                params["vht_capab"] = "[VHT160]"
            elif he_oper_chanwidth == 3:
                params["vht_capab"] = "[VHT160-80PLUS80]"

        if eht_oper_puncturing_override:
            params['eht_oper_puncturing_override'] = eht_oper_puncturing_override

        freq = 5000 + channel * 5
        bw = "20"
        if chanwidth != 0 or channel != ccfs1:
            bw = _5ghz_chanwidth_to_bw(chanwidth)

        hapd = hostapd.add_ap(apdev[0], params)
        dev[0].connect("eht", key_mgmt="NONE", scan_freq=str(freq))
        hapd.wait_sta()

        eht_verify_status(dev[0], hapd, freq, bw, is_ht=True, is_vht=True)
        eht_verify_wifi_version(dev[0])
        hwsim_utils.test_connectivity(dev[0], hapd)

        if eht_oper_puncturing_override:
            hapd.set("eht_oper_puncturing_override", "0x0")
            hapd.request("UPDATE_BEACON")
            time.sleep(1)
    finally:
        dev[0].request("DISCONNECT")
        dev[0].wait_disconnected()
        hapd.wait_sta_disconnect()
        set_world_reg(apdev[0], None, dev[0])

def test_eht_5ghz_20mhz(dev, apdev):
    """EHT with 20 MHz channel width on 5 GHz"""
    _test_eht_5ghz(dev, apdev, 36, 0, 36, 0)

def test_eht_5ghz_40mhz_low(dev, apdev):
    """EHT with 40 MHz channel width on 5 GHz - secondary channel above"""
    _test_eht_5ghz(dev, apdev, 36, 0, 38, 0)

def test_eht_5ghz_40mhz_high(dev, apdev):
    """EHT with 80 MHz channel width on 5 GHz - secondary channel below"""
    _test_eht_5ghz(dev, apdev, 40, 0, 38, 0)

def test_eht_5ghz_80mhz_1(dev, apdev):
    """EHT with 80 MHz channel width on 5 GHz - primary=149"""
    _test_eht_5ghz(dev, apdev, 36, 1, 42, 0)

def test_eht_5ghz_80mhz_2(dev, apdev):
    """EHT with 80 MHz channel width on 5 GHz - primary=149"""
    _test_eht_5ghz(dev, apdev, 149, 1, 155, 0)

def test_eht_5ghz_80mhz_puncturing_override_1(dev, apdev):
    """EHT with 80 MHz channel width on 5 GHz - primary=36 - puncturing override (2nd)"""

    # The 2nd 20 MHz is punctured
    _test_eht_5ghz(dev, apdev, 36, 1, 42, 0,
                   eht_oper_puncturing_override="0x0002",
                   he_ccfs1=36, he_oper_chanwidth=0)

def test_eht_5ghz_80mhz_puncturing_override_2(dev, apdev):
    """EHT with 80 MHz channel width on 5 GHz - primary=149 - puncturing override (3rd)"""

    # The 3rd 20 MHz is punctured
    _test_eht_5ghz(dev, apdev, 149, 1, 155, 0,
                   eht_oper_puncturing_override="0x0004",
                   he_ccfs1=151, he_oper_chanwidth=0)

def test_eht_5ghz_80mhz_puncturing_override_3(dev, apdev):
    """EHT with 80 MHz channel width on 5 GHz - primary=149 - puncturing override (4th)"""

    # The 4th 20 MHz is punctured
    _test_eht_5ghz(dev, apdev, 149, 1, 155, 0,
                   eht_oper_puncturing_override="0x0008",
                   he_ccfs1=151, he_oper_chanwidth=0)

def test_eht_5ghz_80p80mhz(dev, apdev):
    """EHT with 80+80 MHz channel width on 5 GHz"""
    _test_eht_5ghz(dev, apdev, 36, 3, 42, 155)

def _6ghz_op_class_to_bw(op):
    return {
        131: "20",
        132: "40",
        133: "80",
        134: "160",
        137: "320",
    }.get(op, "20")

def _test_eht_6ghz(dev, apdev, channel, op_class, ccfs1):
    check_sae_capab(dev[0])

    # CA enables 320 MHz channels without NO-IR restriction
    dev[0].cmd_execute(['iw', 'reg', 'set', 'CA'])
    wait_regdom_changes(dev[0])

    try:
        ssid = "eht_6ghz_sae"
        passphrase = "12345678"
        params = hostapd.he_wpa2_params(ssid=ssid, passphrase=passphrase)
        params["ieee80211be"] = "1"
        params["channel"] = str(channel)
        params["op_class"] = str(op_class)
        params["he_oper_centr_freq_seg0_idx"] = str(ccfs1)
        params["eht_oper_centr_freq_seg0_idx"] = str(ccfs1)
        params["country_code"] = "CA"

        if not he_6ghz_supported():
            raise HwsimSkip("6 GHz frequency is not supported")
        if op_class == 137 and not eht_320mhz_supported():
            raise HwsimSkip("320 MHz channels are not supported")

        hapd = hostapd.add_ap(apdev[0], params)
        status = hapd.get_status()
        logger.info("hostapd STATUS: " + str(status))
        if hapd.get_status_field("ieee80211ax") != "1":
            raise Exception("STATUS did not indicate ieee80211ax=1")

        if hapd.get_status_field("ieee80211be") != "1":
            raise Exception("STATUS did not indicate ieee80211be=1")

        dev[0].set("sae_pwe", "1")
        dev[0].set("sae_groups", "")

        freq = 5950 + channel * 5
        bw = _6ghz_op_class_to_bw(op_class)

        dev[0].connect(ssid, key_mgmt="SAE", psk=passphrase, ieee80211w="2",
                       scan_freq=str(freq))
        hapd.wait_sta()

        eht_verify_status(dev[0], hapd, freq, bw)
        eht_verify_wifi_version(dev[0])
        sta = hapd.get_sta(dev[0].own_addr())
        if 'supp_op_classes' not in sta:
            raise Exception("supp_op_classes not indicated")
        supp_op_classes = binascii.unhexlify(sta['supp_op_classes'])
        if op_class not in supp_op_classes:
            raise Exception("STA did not indicate support for opclass %d" % op_class)
        hwsim_utils.test_connectivity(dev[0], hapd)
        dev[0].request("DISCONNECT")
        dev[0].wait_disconnected()
        hapd.wait_sta_disconnect()
        hapd.disable()
    finally:
        dev[0].set("sae_pwe", "0")
        dev[0].cmd_execute(['iw', 'reg', 'set', '00'])
        wait_regdom_changes(dev[0])

def test_eht_6ghz_20mhz(dev, apdev):
    """EHT with 20 MHz channel width on 6 GHz"""
    _test_eht_6ghz(dev, apdev, 5, 131, 5)

def test_eht_6ghz_40mhz(dev, apdev):
    """EHT with 40 MHz channel width on 6 GHz"""
    _test_eht_6ghz(dev, apdev, 5, 132, 3)

def test_eht_6ghz_80mhz(dev, apdev):
    """EHT with 80 MHz channel width on 6 GHz"""
    _test_eht_6ghz(dev, apdev, 5, 133, 7)

def test_eht_6ghz_160mhz(dev, apdev):
    """EHT with 160 MHz channel width on 6 GHz"""
    _test_eht_6ghz(dev, apdev, 5, 134, 15)

def test_eht_6ghz_320mhz(dev, apdev):
    """EHT with 320 MHz channel width on 6 GHz"""
    _test_eht_6ghz(dev, apdev, 5, 137, 31)

def test_eht_6ghz_320mhz_2(dev, apdev):
    """EHT with 320 MHz channel width on 6 GHz center 63"""
    _test_eht_6ghz(dev, apdev, 37, 137, 63)

def test_eht_6ghz_320mhz_3(dev, apdev):
    """EHT with 320 MHz channel width on 6 GHz center 31 primary 37"""
    _test_eht_6ghz(dev, apdev, 37, 137, 31)

def check_anqp(dev, bssid):
    if "OK" not in dev.request("ANQP_GET " + bssid + " 258"):
        raise Exception("ANQP_GET command failed")

    ev = dev.wait_event(["GAS-QUERY-START"], timeout=5)
    if ev is None:
        raise Exception("GAS query start timed out")

    ev = dev.wait_event(["GAS-QUERY-DONE"], timeout=10)
    if ev is None:
        raise Exception("GAS query timed out")

    ev = dev.wait_event(["RX-ANQP"], timeout=1)
    if ev is None or "Venue Name" not in ev:
        raise Exception("Did not receive Venue Name")

    ev = dev.wait_event(["ANQP-QUERY-DONE"], timeout=10)
    if ev is None:
        raise Exception("ANQP-QUERY-DONE event not seen")
    if "result=SUCCESS" not in ev:
        raise Exception("Unexpected result: " + ev)

def test_eht_mld_gas(dev, apdev):
    """GAS/ANQP during MLO association"""
    params = hs20_ap_params()
    bssid = apdev[0]['bssid']
    params['hessid'] = bssid
    params['channel'] = "11"
    hapd = hostapd.add_ap(apdev[0], params)

    with HWSimRadio(use_mlo=True) as (hapd0_radio, hapd0_iface), \
        HWSimRadio(use_mlo=True) as (hapd1_radio, hapd1_iface), \
        HWSimRadio(use_mlo=True) as (wpas_radio, wpas_iface):

        wpas = WpaSupplicant(global_iface='/tmp/wpas-wlan5')
        wpas.interface_add(wpas_iface)
        wpas.scan_for_bss(bssid, freq="2462")

        ssid = "owe_two_link"
        params = eht_mld_ap_wpa2_params(ssid, key_mgmt="OWE", mfp="2")
        params['interworking'] = "1"
        params['venue_group'] = "7"
        params['venue_type'] = "1"
        params['venue_name'] = "eng:Example venue"
        hapd0 = eht_mld_enable_ap(hapd0_iface, 0, params)
        bssid0 = hapd0.own_addr()

        params['channel'] = '6'
        hapd1 = eht_mld_enable_ap(hapd0_iface, 1, params)
        bssid1 = hapd1.own_addr()

        wpas.scan_for_bss(bssid0, freq="2412")
        wpas.scan_for_bss(bssid1, freq="2437")

        wpas.connect(ssid, scan_freq="2412 2437", key_mgmt="OWE",
                     ieee80211w="2")
        hapd0.wait_sta()
        hapd1.wait_sta()

        eht_verify_status(wpas, hapd0, 2412, 20, is_ht=True, mld=True,
                          valid_links=3, active_links=3)

        check_anqp(wpas, bssid)
        check_anqp(wpas, bssid0)
        check_anqp(wpas, bssid1)

def test_eht_mld_dpp_responder_while_assoc(dev, apdev):
    """DPP responder while ML associated"""
    check_dpp_capab(dev[0])

    with HWSimRadio(use_mlo=True) as (hapd0_radio, hapd0_iface), \
        HWSimRadio(use_mlo=True) as (hapd1_radio, hapd1_iface), \
        HWSimRadio(use_mlo=True) as (wpas_radio, wpas_iface):

        wpas = WpaSupplicant(global_iface='/tmp/wpas-wlan5')
        wpas.interface_add(wpas_iface)
        check_dpp_capab(wpas)

        ssid = "owe_two_link"
        params = eht_mld_ap_wpa2_params(ssid, key_mgmt="OWE", mfp="2")
        hapd0 = eht_mld_enable_ap(hapd0_iface, 0, params)

        params['channel'] = '6'
        hapd1 = eht_mld_enable_ap(hapd0_iface, 1, params)

        wpas.connect(ssid, scan_freq="2412 2437", key_mgmt="OWE",
                     ieee80211w="2")
        hapd0.wait_sta()
        hapd1.wait_sta()

        eht_verify_status(wpas, hapd0, 2412, 20, is_ht=True, mld=True,
                          valid_links=3, active_links=3)

        id = wpas.dpp_bootstrap_gen(chan="81/11", mac=True)
        uri = wpas.request("DPP_BOOTSTRAP_GET_URI %d" % id)
        wpas.dpp_listen(2462)
        dev[0].dpp_auth_init(uri=uri)
        wait_auth_success(dev[0], wpas)

def _eht_mld_disconnect(dev, apdev, disassoc=True):
    with HWSimRadio(use_mlo=True) as (hapd0_radio, hapd0_iface), \
        HWSimRadio(use_mlo=True) as (wpas_radio, wpas_iface):

        wpas = WpaSupplicant(global_iface='/tmp/wpas-wlan5')
        wpas.interface_add(wpas_iface)

        ssid = "mld_ap_owe_two_link"
        params = eht_mld_ap_wpa2_params(ssid, key_mgmt="OWE", mfp="2")
        hapd0 = eht_mld_enable_ap(hapd0_iface, 0, params)

        params['channel'] = '6'
        hapd1 = eht_mld_enable_ap(hapd0_iface, 1, params)

        wpas.connect(ssid, scan_freq="2412 2437", key_mgmt="OWE",
                     ieee80211w="2")
        hapd0.wait_sta()
        hapd1.wait_sta()
        eht_verify_status(wpas, hapd0, 2412, 20, is_ht=True, mld=True,
                          valid_links=3, active_links=3)
        eht_verify_wifi_version(wpas)
        traffic_test(wpas, hapd0)

        cmd = "DISASSOCIATE " if disassoc else "DEAUTHENTICATE "

        cmd += wpas.own_addr()
        for i in range(0, 3):
            time.sleep(1)

            if "OK" not in hapd0.request(cmd):
                raise Exception("Failed to request: " + cmd)
            hapd0.wait_sta_disconnect()
            hapd1.wait_sta_disconnect()

            wpas.wait_disconnected(timeout=1)
            wpas.wait_connected(timeout=5)
            hapd0.wait_sta()
            hapd1.wait_sta()

            eht_verify_status(wpas, hapd0, 2412, 20, is_ht=True, mld=True,
                              valid_links=3, active_links=3)
            eht_verify_wifi_version(wpas)
            traffic_test(wpas, hapd0)

def test_eht_mld_disassociate(dev, apdev):
    """EHT MLD with two links. Disassociate and reconnect"""
    _eht_mld_disconnect(dev, apdev, disassoc=True)

def test_eht_mld_deauthenticate(dev, apdev):
    """EHT MLD with two links. Deauthenticate and reconnect"""
    _eht_mld_disconnect(dev, apdev, disassoc=False)

def test_eht_mld_non_pref_chan(dev, apdev):
    """EHT MLD with one link. MBO non preferred channels"""

    with HWSimRadio(use_mlo=True) as (hapd0_radio, hapd0_iface), \
        HWSimRadio(use_mlo=True) as (wpas_radio, wpas_iface):

        wpas = WpaSupplicant(global_iface='/tmp/wpas-wlan5')
        wpas.interface_add(wpas_iface)

        # Start the first AP
        ssid = "mld_ap_one_link_mbo"
        params = eht_mld_ap_wpa2_params(ssid, key_mgmt="OWE", mfp="2")
        params["bss_transition"] = "1"
        params["mbo"] = "1"

        hapd0 = eht_mld_enable_ap(hapd0_iface, 0, params)

        if "OK" not in wpas.request("SET non_pref_chan 81:7:200:1 81:9:100:2"):
            raise Exception("Failed to set non-preferred channel list")

        id = wpas.connect(ssid, scan_freq="2412", key_mgmt="OWE",
                          ieee80211w="2", owe_only="1")
        hapd0.wait_sta()
        eht_verify_status(wpas, hapd0, 2412, 20, is_ht=True, mld=True,
                          valid_links=1, active_links=1)
        eht_verify_wifi_version(wpas)
        traffic_test(wpas, hapd0)

        # Validate information received from the Association Request frame
        addr = wpas.own_addr()
        sta = hapd0.get_sta(addr)
        logger.debug("STA: " + str(sta))

        if 'non_pref_chan[0]' not in sta:
            raise Exception("Missing non_pref_chan[0] value (assoc)")
        if sta['non_pref_chan[0]'] != '81:200:1:7':
            raise Exception("Unexpected non_pref_chan[0] value (assoc)")
        if 'non_pref_chan[1]' not in sta:
            raise Exception("Missing non_pref_chan[1] value (assoc)")
        if sta['non_pref_chan[1]'] != '81:100:2:9':
            raise Exception("Unexpected non_pref_chan[1] value (assoc)")
        if 'non_pref_chan[2]' in sta:
            raise Exception("Unexpected non_pref_chan[2] value (assoc)")

        # Verify operating class
        if 'supp_op_classes' not in sta:
            raise Exception("No supp_op_classes")
        supp = bytearray(binascii.unhexlify(sta['supp_op_classes']))
        if supp[0] != 81:
            raise Exception("Unexpected current operating class %d" % supp[0])
        if 115 not in supp:
            raise Exception("Operating class 115 missing")

        # Validate information from WNM action
        if "OK" not in wpas.request("SET non_pref_chan 81:9:100:2"):
            raise Exception("Failed to update non-preferred channel list")

        time.sleep(0.1)
        sta = hapd0.get_sta(addr)
        logger.debug("STA: " + str(sta))

        if 'non_pref_chan[0]' not in sta:
            raise Exception("Missing non_pref_chan[0] value (update 1)")
        if sta['non_pref_chan[0]'] != '81:100:2:9':
            raise Exception("Unexpected non_pref_chan[0] value (update 1)")
        if 'non_pref_chan[1]' in sta:
            raise Exception("Unexpected non_pref_chan[1] value (update 1)")

        # Validate information from WNM action with multiple entries
        if "OK" not in wpas.request("SET non_pref_chan 81:9:100:2 81:10:100:2 81:8:100:2 81:7:100:1 81:5:100:1"):
            raise Exception("Failed to update non-preferred channel list")
        time.sleep(0.1)
        sta = hapd0.get_sta(addr)
        logger.debug("STA: " + str(sta))

        if 'non_pref_chan[0]' not in sta:
            raise Exception("Missing non_pref_chan[0] value (update 2)")
        if sta['non_pref_chan[0]'] != '81:100:1:7,5':
            raise Exception("Unexpected non_pref_chan[0] value (update 2)")
        if 'non_pref_chan[1]' not in sta:
            raise Exception("Missing non_pref_chan[1] value (update 2)")
        if sta['non_pref_chan[1]'] != '81:100:2:9,10,8':
            raise Exception("Unexpected non_pref_chan[1] value (update 2)")
        if 'non_pref_chan[2]' in sta:
            raise Exception("Unexpected non_pref_chan[2] value (update 2)")

def test_eht_mld_rrm_beacon_req(dev, apdev):
    """EHT MLD with one link. RRM beacon request"""

    with HWSimRadio(use_mlo=True) as (hapd0_radio, hapd0_iface), \
        HWSimRadio(use_mlo=True) as (hapd1_radio, hapd1_iface), \
        HWSimRadio(use_mlo=True) as (wpas_radio, wpas_iface):

        wpas = WpaSupplicant(global_iface='/tmp/wpas-wlan5')
        wpas.interface_add(wpas_iface)

        # Start the first AP and connect
        ssid = "mld_ap_one_link_rrm1"
        params = eht_mld_ap_wpa2_params(ssid, key_mgmt="OWE", mfp="2")
        params["bss_transition"] = "1"
        params["mbo"] = "1"
        params["rrm_beacon_report"] = "1"

        hapd0 = eht_mld_enable_ap(hapd0_iface, 0, params)

        wpas.connect(ssid, scan_freq="2412", key_mgmt="OWE", ieee80211w="2",
                     owe_only="1")
        hapd0.wait_sta()
        eht_verify_status(wpas, hapd0, 2412, 20, is_ht=True, mld=True,
                          valid_links=1, active_links=1)
        eht_verify_wifi_version(wpas)
        traffic_test(wpas, hapd0)

        # Start the second AP
        other_ssid = "other"
        params = eht_mld_ap_wpa2_params(other_ssid, key_mgmt="OWE", mfp="2")
        params["channel"] = '6'
        hapd1 = eht_mld_enable_ap(hapd1_iface, 0, params)

        # Issue a beacon request for the second AP
        addr = wpas.own_addr()
        req = build_beacon_request(mode=1, chan=6, duration=50)

        # Send the request with SSID, Detail, Last Beacon Report Indication, and
        # Extended Request subelements. The Extended Request elements includes
        # the Multi-Link element ID.
        run_req_beacon(hapd0, addr,
                       req + "0000" + "020101" + "a40101" + "0b02ff6b")

        ev = hapd0.wait_event(["BEACON-RESP-RX"], timeout=3)
        if ev is None:
            raise Exception("Beacon report response not received")

        fields = ev.split(' ')
        report = BeaconReport(binascii.unhexlify(fields[4]))
        logger.info("Received beacon report: " + str(report))
        if report.bssid_str != hapd1.own_addr() or report.opclass != 81 or \
           report.channel != 6:
            raise Exception("Incorrect bssid/op class/channel for hapd1")

        if not report.last_indication:
            raise Exception("Last Beacon Report Indication subelement missing")

def test_eht_mld_legacy_stas(dev, apdev):
    """EHT AP MLD and multiple non-MLD STAs"""
    for i in range(3):
        check_sae_capab(dev[i])

    with HWSimRadio(use_mlo=True) as (hapd_radio, hapd_iface):
        password = 'qwertyuiop'
        ssid = "ap_mld_sae"
        params = eht_mld_ap_wpa2_params(ssid, password,
                                        key_mgmt="SAE SAE-EXT-KEY",
                                        mfp="2", pwe='2')
        params['rsn_pairwise'] = "CCMP GCMP-256"
        params['sae_groups'] = "19 20"
        hapd0 = eht_mld_enable_ap(hapd_iface, 0, params)

        for i in range(3):
            dev[i].set("sae_groups", "")
            dev[i].connect(ssid, sae_password=password, scan_freq="2412",
                           key_mgmt="SAE", ieee80211w="2", disable_eht="1")
        hapd0.wait_sta()
        hapd0.wait_sta()
        hapd0.wait_sta()
        aid = []
        for i in range(3):
            aid.append(int(hapd0.get_sta(dev[i].own_addr())['aid']))
            traffic_test(dev[i], hapd0)
        logger.info("Assigned AIDs: " + str(aid))
        if len(set(aid)) != 3:
            raise Exception("AP did not assign unique AID to each STA")

def test_eht_mld_and_mlds(dev, apdev):
    """EHT AP MLD and multiple non-AP MLDs"""
    check_sae_capab(dev[0])

    with HWSimRadio(use_mlo=True) as (hapd_radio, hapd_iface), \
            HWSimRadio(use_mlo=True) as (wpas_radio, wpas_iface), \
            HWSimRadio(use_mlo=True) as (wpas_radio2, wpas_iface2):
        wpas = WpaSupplicant(global_iface='/tmp/wpas-wlan5')
        wpas.interface_add(wpas_iface)
        check_sae_capab(wpas)

        wpas2 = WpaSupplicant(global_iface='/tmp/wpas-wlan5')
        wpas2.interface_add(wpas_iface2)
        check_sae_capab(wpas2)

        password = 'qwertyuiop'
        ssid = "ap_mld_sae"
        params = eht_mld_ap_wpa2_params(ssid, password,
                                        key_mgmt="SAE SAE-EXT-KEY",
                                        mfp="2", pwe='2')
        params['rsn_pairwise'] = "CCMP GCMP-256"
        params['sae_groups'] = "19 20"
        hapd0 = eht_mld_enable_ap(hapd_iface, 0, params)

        wpas.set("sae_pwe", "1")
        wpas.connect(ssid, sae_password=password, scan_freq="2412",
                     key_mgmt="SAE-EXT-KEY", ieee80211w="2")
        wpas2.set("sae_pwe", "1")
        wpas2.connect(ssid, sae_password=password, scan_freq="2412",
                      key_mgmt="SAE-EXT-KEY", ieee80211w="2")

        hapd0.wait_sta()
        hapd0.wait_sta()
        aid = []
        aid.append(int(hapd0.get_sta(wpas.own_addr())['aid']))
        traffic_test(wpas, hapd0)
        aid.append(int(hapd0.get_sta(wpas2.own_addr())['aid']))
        traffic_test(wpas2, hapd0)
        logger.info("Assigned AIDs: " + str(aid))
        if len(set(aid)) != 2:
            raise Exception("AP MLD did not assign unique AID to each non-AP MLD")

def mlo_perform_csa(hapd, command, freq, dev):
        match_str = "freq=" + str(freq)
        hapd.request(command)

        ev = hapd.wait_event(["CTRL-EVENT-STARTED-CHANNEL-SWITCH"], timeout=10)
        if ev is None:
            raise Exception("Channel switch start event not seen")
        if match_str not in ev:
            raise Exception("Unexpected channel in CS started")

        ev = hapd.wait_event(["CTRL-EVENT-CHANNEL-SWITCH"], timeout=10)
        if ev is None:
            raise Exception("Channel switch completion event not seen")
        if match_str not in ev:
            raise Exception("Unexpected channel in CS completed")

        ev = hapd.wait_event(["AP-CSA-FINISHED"], timeout=10)
        if ev is None:
            raise Exception("CSA finished event timed out")
        if match_str not in ev:
            raise Exception("Unexpected channel in CSA finished event")

        ev = dev.wait_event(["CTRL-EVENT-LINK-CHANNEL-SWITCH"], timeout=10)
        if ev is None:
            raise Exception("Non-AP MLD did not report CS")
        if match_str not in ev:
            raise Exception("Unexpected channel in CS event from non-AP MLD")

        time.sleep(0.5)

def test_eht_mlo_csa(dev, apdev):
        """EHT MLD AP connected to non-AP MLD. Seamless channel switch"""
        csa_supported(dev[0])

        with HWSimRadio(use_mlo=True) as (hapd_radio, hapd_iface), \
            HWSimRadio(use_mlo=True) as (wpas_radio, wpas_iface):

            wpas = WpaSupplicant(global_iface='/tmp/wpas-wlan5')
            wpas.interface_add(wpas_iface)

            ssid = "mld_ap"
            passphrase = 'qwertyuiop'

            params = eht_mld_ap_wpa2_params(ssid, passphrase,
                                            key_mgmt="SAE", mfp="2", pwe='1')
            hapd0 = eht_mld_enable_ap(hapd_iface, 0, params)

            params['channel'] = '6'
            hapd1 = eht_mld_enable_ap(hapd_iface, 1, params)

            wpas.set("sae_pwe", "1")
            wpas.connect(ssid, sae_password=passphrase, scan_freq="2412 2437",
                         key_mgmt="SAE", ieee80211w="2")

            eht_verify_status(wpas, hapd0, 2412, 20, is_ht=True, mld=True,
                              valid_links=3, active_links=3)
            eht_verify_wifi_version(wpas)
            traffic_test(wpas, hapd0)

            logger.info("Perform CSA on 1st link")
            mlo_perform_csa(hapd0, "CHAN_SWITCH 5 2462 ht he eht blocktx",
                            2462, wpas)

            logger.info("Test traffic after 1st link CSA completes")
            traffic_test(wpas, hapd0)

            logger.info("Perform CSA on 2nd link")
            mlo_perform_csa(hapd1, "CHAN_SWITCH 5 2412 ht he eht blocktx",
                            2412, wpas)


            logger.info("Test traffic after 2nd link CSA completes")
            traffic_test(wpas, hapd1)

            logger.info("Perform CSA on 2nd link and bring it back to original channel")
            mlo_perform_csa(hapd1, "CHAN_SWITCH 5 2437 ht he eht blocktx",
                            2437, wpas)

            logger.info("Test traffic again after 2nd link CSA completes")
            traffic_test(wpas, hapd1)

            logger.info("Perform CSA on 1st link and bring it back to original channel")
            mlo_perform_csa(hapd0, "CHAN_SWITCH 5 2412 ht he eht blocktx",
                            2412, wpas)

            logger.info("Test traffic again after 1st link CSA completes")
            traffic_test(wpas, hapd0)

def create_base_conf_file(iface, channel, prefix='hostapd-', hw_mode='g',
                          op_class=None):
    # Create configuration file and add phy characteristics
    fd, fname = tempfile.mkstemp(dir='/tmp',
                                 prefix=prefix + iface + "-chan-" + str(channel) + "-")
    f = os.fdopen(fd, 'w')

    f.write("driver=nl80211\n")
    f.write("hw_mode=" + str(hw_mode) + "\n")
    f.write("ieee80211n=1\n")
    if hw_mode == 'a' and \
       (op_class is None or \
        op_class not in [131, 132, 133, 134, 135, 136, 137]):
        f.write("ieee80211ac=1\n")
    f.write("ieee80211ax=1\n")
    f.write("ieee80211be=1\n")
    f.write("channel=" + str(channel) + "\n")

    return f, fname

def append_bss_conf_to_file(f, ifname, params, first=False):
    # Add BSS specific characteristics
    config = "bss"

    if first:
        config = "interface"

    f.write("\n" + config + "=%s\n" % ifname)

    for k, v in list(params.items()):
        f.write("{}={}\n".format(k,v))

    f.write("mld_ap=1\n")

def dump_config(fname):
    with open(fname, 'r') as f:
        cfg = f.read()
        logger.debug("hostapd config: " + str(fname) + "\n" + cfg)

def get_config(iface, count, ssid, passphrase, channel, bssid_regex,
               rnr=False, debug=False, start_idx=0):
    f, fname = create_base_conf_file(iface, channel=channel)
    hapds = []

    i = start_idx
    while count:
        if i == 0:
            ifname = iface
        else:
            ifname = iface + "-" + str(i)

        set_ssid = ssid + str(i)
        set_passphrase = passphrase + str(i)
        params = hostapd.wpa2_params(ssid=set_ssid, passphrase=set_passphrase,
                                     wpa_key_mgmt="SAE", ieee80211w="2")
        params['sae_pwe'] = "2"
        params['group_mgmt_cipher'] = "AES-128-CMAC"
        params['beacon_prot'] = "1"
        params["ctrl_interface"] = "/var/run/hostapd/"
        params["bssid"] = bssid_regex % (i + 1)

        if rnr:
            params["rnr"] = "1"

        append_bss_conf_to_file(f, ifname, params, first=(i == start_idx))

        hapds.append([ifname, i])
        count = count - 1
        i = i + 1

    f.close()

    if debug:
        dump_config(fname)

    return fname, hapds

def start_ap(prefix, configs):
    pid = prefix + ".hostapd.pid"
    configs = configs.split()

    cmd = ['../../hostapd/hostapd', '-ddKtB', '-P', pid, '-f',
           prefix + ".hostapd-log"]

    cmd = cmd + configs

    logger.info("Starting APs")
    res = subprocess.check_call(cmd)
    if res != 0:
        raise Exception("Could not start hostapd: %s" % str(res))

    # Wait for hostapd to complete initialization and daemonize.
    time.sleep(2)
    for i in range(20):
        if os.path.exists(pid):
            break
        time.sleep(0.2)

    if not os.path.exists(pid):
        raise Exception("hostapd did not create PID file.")

def get_mld_devs(hapd_iface, count, prefix, rnr=False):
    fname1, hapds1 = get_config(hapd_iface, count=count, ssid="mld-",
                                passphrase="qwertyuiop-", channel=1,
                                bssid_regex="02:00:00:00:07:%02x",
                                rnr=rnr, debug=True)
    fname2, hapds2 = get_config(hapd_iface, count=count, ssid="mld-",
                                passphrase="qwertyuiop-", channel=6,
                                bssid_regex="02:00:00:00:08:%02x",
                                rnr=rnr, debug=True)

    start_ap(prefix, fname1 + " " + fname2)

    hapd_mld1_link0 = hostapd.Hostapd(ifname=hapds1[0][0], bssidx=hapds1[0][1],
                                      link=0)
    hapd_mld1_link1 = hostapd.Hostapd(ifname=hapds2[0][0], bssidx=hapds2[0][1],
                                      link=1)

    hapd_mld2_link0 = hostapd.Hostapd(ifname=hapds1[1][0], bssidx=hapds1[1][1],
                                      link=0)
    hapd_mld2_link1 = hostapd.Hostapd(ifname=hapds2[1][0], bssidx=hapds2[1][1],
                                      link=1)

    if not hapd_mld1_link0.ping():
        raise Exception("Could not ping hostapd")

    if not hapd_mld1_link1.ping():
        raise Exception("Could not ping hostapd")

    if not hapd_mld2_link0.ping():
        raise Exception("Could not ping hostapd")

    if not hapd_mld2_link1.ping():
        raise Exception("Could not ping hostapd")

    os.remove(fname1)
    os.remove(fname2)

    return [hapd_mld1_link0, hapd_mld1_link1, hapd_mld2_link0, hapd_mld2_link1]

def stop_mld_devs(hapds, pid):
    pid = pid + ".hostapd.pid"

    if "OK" not in hapds[0].request("TERMINATE"):
        raise Exception("Failed to terminate hostapd process")

    ev = hapds[0].wait_event(["CTRL-EVENT-TERMINATING"], timeout=15)
    if ev is None:
        raise Exception("CTRL-EVENT-TERMINATING not seen")

    time.sleep(0.5)

    for i in range(30):
        time.sleep(0.1)
        if not os.path.exists(pid):
            break
    if os.path.exists(pid):
        raise Exception("PID file exits after process termination")

def eht_parse_rnr(bss, rnr=False, exp_bssid=None):
        partner_rnr_pattern = re.compile(".*ap_info.*, mld ID=0, link ID=",
                                         re.MULTILINE)
        ml_pattern = re.compile(".*multi-link:.*, MLD addr=.*", re.MULTILINE)

        if partner_rnr_pattern.search(bss) is None:
            raise Exception("RNR element not found for first link of first MLD")

        if ml_pattern.search(bss) is None:
            raise Exception("ML element not found for first link of first MLD")

        if not rnr:
            return

        coloc_rnr_pattern = re.compile(".*ap_info.*, mld ID=255, link ID=..",
                                       re.MULTILINE)

        if coloc_rnr_pattern.search(bss) is None:
            raise Exception("RNR element not found for co-located BSS")

        line = coloc_rnr_pattern.search(bss).group()
        if line.count('bssid') > 1:
            raise Exception("More than one BSS found for co-located RNR")

        # Get the BSSID carried in the RNR
        index = line.rindex('bssid')
        bssid = line[index+len('bssid')+1:].split(',')[0]

        # Get the MLD ID carried in the RNR
        index = line.rindex('link ID')
        link_id = line[index+len('link ID')+1:].split(',')[0]

        if link_id != "15":
            raise Exception("Unexpected link ID for co-located BSS which is not own partner")

        if bssid != exp_bssid:
            raise Exception("Unexpected BSSID for co-located BSS")

def eht_mld_cohosted_discovery(dev, apdev, params, rnr=False):
    with HWSimRadio(use_mlo=True, n_channels=2) as (hapd_radio, hapd_iface), \
        HWSimRadio(use_mlo=True) as (wpas_radio, wpas_iface):

        wpas = WpaSupplicant(global_iface='/tmp/wpas-wlan5')
        wpas.interface_add(wpas_iface)

        hapds = get_mld_devs(hapd_iface=hapd_iface, count=2,
                             prefix=params['prefix'], rnr=rnr)

        # Only scan link 0
        res = wpas.request("SCAN freq=2412")
        if "FAIL" in res:
            raise Exception("Failed to start scan")

        ev = wpas.wait_event(["CTRL-EVENT-SCAN-STARTED"])
        if ev is None:
            raise Exception("Scan did not start")

        ev = wpas.wait_event(["CTRL-EVENT-SCAN-RESULTS"])
        if ev is None:
            raise Exception("Scan did not complete")

        logger.info("Scan done")

        bss = wpas.request("BSS " + hapds[0].own_addr())
        logger.info("BSS 0_0: " + str(bss))
        eht_parse_rnr(bss, rnr, hapds[2].own_addr())

        bss = wpas.request("BSS " + hapds[2].own_addr())
        logger.info("BSS 1_0: " + str(bss))
        eht_parse_rnr(bss, rnr, hapds[0].own_addr())

        stop_mld_devs(hapds, params['prefix'])

def test_eht_mld_cohosted_discovery(dev, apdev, params):
    """EHT 2 AP MLDs discovery"""
    eht_mld_cohosted_discovery(dev, apdev, params)

def test_eht_mld_cohosted_discovery_with_rnr(dev, apdev, params):
    """EHT 2 AP MLDs discovery (with co-location RNR)"""
    eht_mld_cohosted_discovery(dev, apdev, params, rnr=True)

def test_eht_mld_cohosted_connectivity(dev, apdev, params):
    """EHT 2 AP MLDs with 2 MLD clients connection"""
    check_sae_capab(dev[0])

    with HWSimRadio(use_mlo=True, n_channels=2) as (hapd_radio, hapd_iface), \
        HWSimRadio(use_mlo=True) as (wpas_radio, wpas_iface), \
        HWSimRadio(use_mlo=True) as (wpas_radio1, wpas_iface1):

        wpas = WpaSupplicant(global_iface='/tmp/wpas-wlan5')
        wpas.interface_add(wpas_iface)
        check_sae_capab(wpas)

        wpas1 = WpaSupplicant(global_iface='/tmp/wpas-wlan5')
        wpas1.interface_add(wpas_iface1)
        check_sae_capab(wpas1)

        hapds = get_mld_devs(hapd_iface=hapd_iface, count=2,
                             prefix=params['prefix'], rnr=False)

        passphrase = "qwertyuiop-"
        ssid = "mld-"

        # Connect one client to first AP MLD and verify traffic on both links
        wpas.set("sae_pwe", "1")
        wpas.connect(ssid + "0", sae_password=passphrase+"0", scan_freq="2412",
                     key_mgmt="SAE", ieee80211w="2")

        eht_verify_status(wpas, hapds[0], 2412, 20, is_ht=True, mld=True,
                          valid_links=3, active_links=3)
        eht_verify_wifi_version(wpas)

        traffic_test(wpas, hapds[0])
        traffic_test(wpas, hapds[1])

        # Connect another client to second AP MLD and verify traffic on both
        # links
        wpas1.set("sae_pwe", "1")
        wpas1.connect(ssid + "1", sae_password=passphrase+"1", scan_freq="2437",
                      key_mgmt="SAE", ieee80211w="2")

        eht_verify_status(wpas1, hapds[3], 2437, 20, is_ht=True, mld=True,
                          valid_links=3, active_links=3)
        eht_verify_wifi_version(wpas1)

        traffic_test(wpas1, hapds[3])
        traffic_test(wpas1, hapds[2])

        stop_mld_devs(hapds, params['prefix'])

def test_eht_mlo_color_change(dev, apdev):
    """AP MLD and Color Change Announcement"""
    with HWSimRadio(use_mlo=True) as (hapd_radio, hapd_iface), \
        HWSimRadio(use_mlo=True) as (wpas_radio, wpas_iface):

        ssid = "mld_ap"
        passphrase = 'qwertyuiop'

        params = eht_mld_ap_wpa2_params(ssid, passphrase,
                                        key_mgmt="SAE", mfp="2", pwe='1')
        params['he_bss_color'] = '42'

        hapd0 = eht_mld_enable_ap(hapd_iface, 0, params)

        params['channel'] = '6'
        params['he_bss_color'] = '24'

        hapd1 = eht_mld_enable_ap(hapd_iface, 1, params)

        logger.info("Perform CCA on 1st link")
        if "OK" not in hapd0.request("COLOR_CHANGE 10"):
            raise Exception("COLOR_CHANGE failed")

        time.sleep(1.5)

        color = hapd0.get_status_field("he_bss_color")
        if color != "10":
            raise Exception("Expected current he_bss_color to be 10; was " + color)

        logger.info("Perform CCA on 1st link again")
        if "OK" not in hapd0.request("COLOR_CHANGE 60"):
            raise Exception("COLOR_CHANGE failed")
        time.sleep(1.5)

        color = hapd0.get_status_field("he_bss_color")
        if color != "60":
            raise Exception("Expected current he_bss_color to be 60; was " + color)

        logger.info("Perform CCA on 2nd link")
        if "OK" not in hapd1.request("COLOR_CHANGE 25"):
            raise Exception("COLOR_CHANGE failed")
        time.sleep(1.5)

        color = hapd1.get_status_field("he_bss_color")
        if color != "25":
            raise Exception("Expected current he_bss_color to be 25; was " + color)

        logger.info("Perform CCA on 2nd link again")
        if "OK" not in hapd1.request("COLOR_CHANGE 5"):
            raise Exception("COLOR_CHANGE failed")
        time.sleep(1.5)

        color = hapd1.get_status_field("he_bss_color")
        if color != "5":
            raise Exception("Expected current he_bss_color to be 5; was " + color)

        hapd0.dump_monitor()
        hapd1.dump_monitor()

def test_eht_mld_control_socket_connectivity(dev, apdev):
    """AP MLD control socket connectivity"""
    with HWSimRadio(use_mlo=True) as (hapd_radio, hapd_iface), \
        HWSimRadio(use_mlo=True) as (wpas_radio, wpas_iface):

        ssid = "mld_ap"
        link0_params = {"ssid": ssid,
                        "hw_mode": "g",
                        "channel": "1"}
        link1_params = {"ssid": ssid,
                        "hw_mode": "g",
                        "channel": "2"}

        hapd0 = eht_mld_enable_ap(hapd_iface, 0, link0_params)
        hapd1 = eht_mld_enable_ap(hapd_iface, 1, link1_params)

        mld_dev = mld.get_mld_obj(hapd_iface)

        # Check status of each link
        res = str(mld_dev.request("LINKID 0 STATUS"))
        logger.info("LINK 0 STATUS:\n" + res)
        if "state" not in res:
            raise Exception("Failed to get link 0 status via MLD socket")
        if 'link_id=0' not in res.splitlines():
            raise Exception("link_id=0 not reported for link 0")

        res = mld_dev.request("LINKID 1 STATUS")
        logger.info("LINK 1 STATUS:\n" + res)
        if "state" not in res:
            raise Exception("Failed to get link 1 status via MLD socket")
        if 'link_id=1' not in res.splitlines():
            raise Exception("link_id=0 not reported for link 1")

def test_eht_mlo_single_drv(dev, apdev, params):
    "EHT MLO AP simple test but having single drv object"
    with HWSimRadio(use_mlo=True, n_channels=2) as (hapd_radio, hapd_iface), \
        HWSimRadio(use_mlo=True) as (wpas_radio, wpas_iface), \
        HWSimRadio(use_mlo=True) as (wpas_radio1, wpas_iface1):

        wpas = WpaSupplicant(global_iface='/tmp/wpas-wlan5')
        wpas.interface_add(wpas_iface)
        check_sae_capab(wpas)

        wpas1 = WpaSupplicant(global_iface='/tmp/wpas-wlan5')
        wpas1.interface_add(wpas_iface1)
        check_sae_capab(wpas1)

        fname1, hapds1 = get_config(hapd_iface, count=1, ssid="mld-",
                                    passphrase="qwertyuiop-", channel=1,
                                    bssid_regex="02:00:00:00:07:%02x",
                                    rnr=False, debug=True, start_idx=1)
        fname2, hapds2 = get_config(hapd_iface, count=2, ssid="mld-",
                                    passphrase="qwertyuiop-", channel=6,
                                    bssid_regex="02:00:00:00:08:%02x",
                                    rnr=False, debug=True)

        # Configs will be in this form:
        #
        #   +--------------------+     +------------------+
        #   |     config 2       |     |      config 1    |
        #   |                    |     |                  |
        #   |                    |     |                  |
        #   | +----------------+ |     |                  |
        #   | |     BSS 1      | |     |                  |
        #   | | ssid: mld-0    | |     |                  |
        #   | +----------------+ |     |                  |
        #   |                    |     |                  |
        # +---------------------------------------------------------------+
        # | | +----------------+ |     | +--------------+ |               |
        # | | |     BSS 2      | |     | |     BSS 1    | |               |
        # | | | ssid: mld-1    | |     | | ssid: mld-1  | | 2 Link MLO AP |
        # | | +----------------+ |     | +--------------+ |               |
        # | +--------------------+     +------------------+               |
        # +---------------------------------------------------------------+

        # Since config 1 will be passed first, it will expect an interface
        # to be created already. Hence, create the interface for it
        cmd = ['iw', 'phy' + str(hapd_radio), 'interface', 'add', 'wlan7-1',
               'type', '__ap']
        proc = subprocess.Popen(cmd, stderr=subprocess.STDOUT,
                                stdout=subprocess.PIPE, shell=False)
        out, err = proc.communicate()
        logger.debug("iw output: " + out.decode())

        # Start hostapd
        start_ap(params['prefix'], fname1 + " " + fname2)

        hapd_mld1_link0 = hostapd.Hostapd(ifname=hapds1[0][0],
                                          bssidx=hapds1[0][1],
                                          link=0)
        hapd_mld1_link1 = hostapd.Hostapd(ifname=hapds2[1][0],
                                          bssidx=hapds2[1][1],
                                          link=1)

        hapd_mld2_link0 = hostapd.Hostapd(ifname=hapds2[0][0],
                                          bssidx=hapds2[0][1],
                                          link=0)

        hapds = [hapd_mld1_link0, hapd_mld1_link1, hapd_mld2_link0]

        if not hapd_mld1_link0.ping():
            raise Exception("Could not ping hostapd")

        if not hapd_mld2_link0.ping():
            raise Exception("Could not ping hostapd")

        if not hapd_mld1_link1.ping():
            raise Exception("Could not ping hostapd")

        os.remove(fname1)
        os.remove(fname2)

        passphrase = "qwertyuiop-"
        ssid = "mld-"

        # Connect one client to the first AP MLD 1 and verify traffic
        wpas.set("sae_pwe", "1")
        wpas.connect(ssid + "1", sae_password=passphrase + "1",
                     scan_freq="2412", key_mgmt="SAE", ieee80211w="2")

        eht_verify_status(wpas, hapds[0], 2412, 20, is_ht=True, mld=True,
                          valid_links=3, active_links=3)
        eht_verify_wifi_version(wpas)

        traffic_test(wpas, hapds[1])

        # Connect another client to the second AP MLD 2 and verify traffic
        wpas1.set("sae_pwe", "1")
        wpas1.connect(ssid + "0", sae_password=passphrase + "0",
                      scan_freq="2437", key_mgmt="SAE", ieee80211w="2")

        eht_verify_status(wpas1, hapds[2], 2437, 20, is_ht=True, mld=True,
                          valid_links=1, active_links=1)
        eht_verify_wifi_version(wpas1)

        traffic_test(wpas1, hapds[2])

        stop_mld_devs(hapds, params['prefix'])
