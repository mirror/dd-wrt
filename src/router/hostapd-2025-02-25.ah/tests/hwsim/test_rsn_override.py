# Test cases for RSNE/RSNXE overriding
# Copyright (c) 2023-2024, Qualcomm Innovation Center, Inc.
#
# This software may be distributed under the terms of the BSD license.
# See README for more details.

import hostapd
from utils import *
from hwsim import HWSimRadio
from wpasupplicant import WpaSupplicant
from test_eht import eht_mld_enable_ap, eht_verify_status, eht_verify_wifi_version, traffic_test

def test_rsn_override(dev, apdev):
    """RSNE=WPA2-Personal/PMF-optional override=WPA3-Personal/PMF-required (with MLO parameters)"""
    check_sae_capab(dev[0])

    ssid = "test-rsn-override"
    params = hostapd.wpa2_params(ssid=ssid,
                                 passphrase="12345678",
                                 ieee80211w='1')
    params['rsn_override_key_mgmt'] = 'SAE SAE-EXT-KEY'
    params['rsn_override_pairwise'] = 'CCMP GCMP-256'
    params['rsn_override_mfp'] = '2'
    params['beacon_prot'] = '1'
    params['sae_groups'] = '19 20'
    params['sae_require_mfp'] = '1'
    params['sae_pwe'] = '2'
    hapd = hostapd.add_ap(apdev[0], params)
    bssid = hapd.own_addr()

    try:
        dev[0].set("rsn_overriding", "1")
        dev[0].scan_for_bss(bssid, freq=2412)
        bss = dev[0].get_bss(bssid)
        flags = bss['flags']
        if "PSK" in flags:
            raise Exception("Unexpected BSS flags: " + flags)
        if "-SAE+SAE-EXT-KEY-" not in flags:
            raise Exception("Unexpected BSS flags: " + flags)
        if "-GCMP-256+CCMP" not in flags:
            raise Exception("Unexpected BSS flags: " + flags)

        dev[0].set("sae_pwe", "2")
        dev[0].set("sae_groups", "")
        dev[0].connect(ssid, sae_password="12345678", key_mgmt="SAE",
                       ieee80211w="2", scan_freq="2412")
    finally:
        dev[0].set("sae_pwe", "0")
        dev[0].set("rsn_overriding", "0")

def test_rsn_override2(dev, apdev):
    """RSNE=WPA2-Personal/PMF-disabled override=WPA3-Personal/PMF-required (with MLO parameters)"""
    check_sae_capab(dev[0])

    ssid = "test-rsn-override"
    params = hostapd.wpa2_params(ssid=ssid,
                                 passphrase="12345678",
                                 ieee80211w='0')
    params['rsn_override_key_mgmt'] = 'SAE SAE-EXT-KEY'
    params['rsn_override_pairwise'] = 'CCMP GCMP-256'
    params['rsn_override_mfp'] = '2'
    params['beacon_prot'] = '1'
    params['sae_groups'] = '19 20'
    params['sae_require_mfp'] = '1'
    params['sae_pwe'] = '2'
    hapd = hostapd.add_ap(apdev[0], params)
    bssid = hapd.own_addr()

    try:
        dev[0].set("rsn_overriding", "1")
        dev[0].scan_for_bss(bssid, freq=2412)
        bss = dev[0].get_bss(bssid)
        flags = bss['flags']
        if "PSK" in flags:
            raise Exception("Unexpected BSS flags: " + flags)
        if "-SAE+SAE-EXT-KEY-" not in flags:
            raise Exception("Unexpected BSS flags: " + flags)
        if "-GCMP-256+CCMP" not in flags:
            raise Exception("Unexpected BSS flags: " + flags)

        dev[0].set("sae_pwe", "2")
        dev[0].set("sae_groups", "")
        dev[0].connect(ssid, sae_password="12345678", key_mgmt="SAE",
                       ieee80211w="2", scan_freq="2412")
    finally:
        dev[0].set("sae_pwe", "0")
        dev[0].set("rsn_overriding", "0")

def test_rsn_override_no_pairwise(dev, apdev):
    """RSN overriding and no pairwise cipher match in RSNEO"""
    check_sae_capab(dev[0])

    ssid = "test-rsn-override"
    params = hostapd.wpa2_params(ssid=ssid,
                                 passphrase="12345678",
                                 ieee80211w='1')
    params['rsn_override_key_mgmt'] = 'SAE SAE-EXT-KEY'
    params['rsn_override_pairwise'] = 'GCMP-256'
    params['rsn_override_mfp'] = '2'
    params['beacon_prot'] = '1'
    params['sae_groups'] = '19 20'
    params['sae_require_mfp'] = '1'
    hapd = hostapd.add_ap(apdev[0], params)
    bssid = hapd.own_addr()

    try:
        dev[0].set("rsn_overriding", "1")
        dev[0].scan_for_bss(bssid, freq=2412)

        dev[0].set("sae_groups", "")
        dev[0].connect(ssid, psk="12345678", key_mgmt="WPA-PSK SAE",
                       pairwise="CCMP", ieee80211w="1", scan_freq="2412")
    finally:
        dev[0].set("sae_pwe", "0")
        dev[0].set("rsn_overriding", "0")

def test_rsn_override_mld(dev, apdev):
    """AP MLD and RSNE=WPA2-Personal/PMF-disabled override=WPA3-Personal/PMF-required"""
    run_rsn_override_mld(dev, apdev, False)

def test_rsn_override_mld_mixed(dev, apdev):
    """AP MLD and RSNE=WPA2-Personal/PMF-disabled override=WPA3-Personal/PMF-required on one link"""
    run_rsn_override_mld(dev, apdev, True)

def test_rsn_override_mld_only_sta(dev, apdev):
    """AP MLD and RSN overriding only on STA"""
    run_rsn_override_mld(dev, apdev, False, only_sta=True)

def test_rsn_override_mld_too_long_elems(dev, apdev):
    """AP MLD and RSN overriding with too long elements"""
    run_rsn_override_mld(dev, apdev, False, too_long_elems=True)

def run_rsn_override_mld(dev, apdev, mixed, only_sta=False,
                         too_long_elems=False):
    with HWSimRadio(use_mlo=True) as (hapd_radio, hapd_iface), \
        HWSimRadio(use_mlo=True) as (wpas_radio, wpas_iface):

        wpas = WpaSupplicant(global_iface='/tmp/wpas-wlan5')
        wpas.interface_add(wpas_iface)
        check_sae_capab(wpas)

        passphrase = 'qwertyuiop'
        ssid = "AP MLD RSN override"
        params = hostapd.wpa2_params(ssid=ssid, passphrase=passphrase)
        params['ieee80211n'] = '1'
        params['ieee80211ax'] = '1'
        params['ieee80211be'] = '1'
        params['channel'] = '1'
        params['hw_mode'] = 'g'
        params['beacon_prot'] = '1'
        params['sae_groups'] = '19 20'
        params['sae_require_mfp'] = '1'
        params['sae_pwe'] = '2'
        if only_sta:
            params['wpa_key_mgmt'] = 'SAE SAE-EXT-KEY'
            params['rsn_pairwise'] = 'CCMP GCMP-256'
            params['ieee80211w'] = '2'
        elif not mixed:
            params['rsn_override_key_mgmt'] = 'SAE'
            params['rsn_override_key_mgmt_2'] = 'SAE-EXT-KEY'
            params['rsn_override_pairwise'] = 'CCMP'
            params['rsn_override_pairwise_2'] = 'GCMP-256'
            params['rsn_override_mfp'] = '1'
            params['rsn_override_mfp_2'] = '2'

        params1 = dict(params)

        if mixed:
            params['wpa_key_mgmt'] = 'SAE SAE-EXT-KEY'
            params['rsn_pairwise'] = 'CCMP GCMP-256'
            params['ieee80211w'] = '2'
            params['rsn_override_key_mgmt_2'] = 'SAE SAE-EXT-KEY'
            params['rsn_override_pairwise_2'] = 'CCMP GCMP-256'
            params['rsn_override_mfp_2'] = '2'

            params1['rsn_override_key_mgmt_2'] = 'SAE SAE-EXT-KEY'
            params1['rsn_override_pairwise_2'] = 'CCMP GCMP-256'
            params1['rsn_override_mfp_2'] = '2'

        hapd0 = eht_mld_enable_ap(hapd_iface, 0, params)

        params1['channel'] = '6'
        if too_long_elems:
            params1['rsnoe_override'] = 'ddff506f9a29' + 251*'cc'
        hapd1 = eht_mld_enable_ap(hapd_iface, 1, params1)

        wpas.set("sae_pwe", "1")
        wpas.set("rsn_overriding", "1")
        wpas.connect(ssid, sae_password=passphrase, scan_freq="2412 2437",
                     key_mgmt="SAE-EXT-KEY", ieee80211w="2", beacon_prot="1",
                     pairwise="GCMP-256 CCMP", wait_connect=not too_long_elems)
        if too_long_elems:
            ev = wpas.wait_event(['Associated with'], timeout=10)
            if ev is None:
                raise Exception("Association not reported")
            ev = wpas.wait_event(['EAPOL-RX'], timeout=1)
            if ev is None:
                raise Exception("EAPOL-Key M1 not reported")
            ev = wpas.wait_event(['EAPOL-RX', 'CTRL-EVENT-DISCONNECTED'],
                                 timeout=20)
            if ev is None:
                raise Exception("Disconnection not reported")
            # The AP is expected to fail to send M3 due to RSNOE/RSNO2E/RSNXOE
            # being too long to fit into the RSN Override Link KDE.
            if 'EAPOL-RX' in ev:
                raise Exception("Unexpected EAPOL-Key M3 reported")
            return

        eht_verify_status(wpas, hapd0, 2412, 20, is_ht=True, mld=True,
                          valid_links=3, active_links=3)
        eht_verify_wifi_version(wpas)
        traffic_test(wpas, hapd0)
        traffic_test(wpas, hapd1)
        if only_sta:
            return

        dev[0].set("rsn_overriding", "0")
        dev[0].connect(ssid, psk=passphrase, key_mgmt="WPA-PSK",
                       scan_freq="2412 2437")

        status = wpas.get_status()
        logger.debug("wpas STATUS:\n" + str(status))
        if status['key_mgmt'] != 'SAE-EXT-KEY' or \
           'pmf' not in status or \
           status['pmf'] != '2' or \
           status['pairwise_cipher'] != 'GCMP-256':
            raise Exception("Unexpected result for new STA")

        status = dev[0].get_status()
        logger.debug("dev[0] STATUS:\n" + str(status))
        if status['key_mgmt'] != 'WPA2-PSK' or \
           status['pairwise_cipher'] != 'CCMP':
            raise Exception("Unexpected result for legacy STA")

def test_rsn_override_connect_cmd(dev, apdev):
    """RSNE=WPA2-Personal/PMF-optional override=WPA3-Personal/PMF-required using cfg80211 connect command"""
    wpas = WpaSupplicant(global_iface='/tmp/wpas-wlan5')
    wpas.interface_add("wlan5", drv_params="force_connect_cmd=1 rsn_override_in_driver=1")
    check_sae_capab(wpas)

    ssid = "test-rsn-override"
    params = hostapd.wpa2_params(ssid=ssid,
                                 passphrase="12345678",
                                 ieee80211w='1')
    params['rsn_override_key_mgmt'] = 'WPA-PSK-SHA256'
    params['rsn_override_pairwise'] = 'CCMP GCMP-256'
    params['rsn_override_mfp'] = '2'
    params['beacon_prot'] = '1'
    hapd = hostapd.add_ap(apdev[0], params)

    wpas.set("rsn_overriding", "1")
    wpas.connect(ssid, psk="12345678", key_mgmt="WPA-PSK-SHA256",
                 ieee80211w="2", scan_freq="2412")

def test_rsn_override_omit_rsnxe(dev, apdev):
    """RSN overriding with RSNXE explicitly omitted"""
    check_sae_capab(dev[0])

    ssid = "test-rsn-override"
    params = hostapd.wpa2_params(ssid=ssid,
                                 passphrase="12345678",
                                 ieee80211w='1')
    params['rsn_override_key_mgmt'] = 'SAE SAE-EXT-KEY'
    params['rsn_override_pairwise'] = 'CCMP GCMP-256'
    params['rsn_override_mfp'] = '2'
    params['beacon_prot'] = '1'
    params['sae_groups'] = '19 20'
    params['sae_require_mfp'] = '1'
    params['sae_pwe'] = '2'
    params['ssid_protection'] = '1'
    params['rsn_override_omit_rsnxe'] = '1'
    hapd = hostapd.add_ap(apdev[0], params)
    bssid = hapd.own_addr()

    try:
        dev[0].set("rsn_overriding", "1")
        dev[0].scan_for_bss(bssid, freq=2412)
        dev[0].set("sae_pwe", "2")
        dev[0].set("sae_groups", "")
        dev[0].connect(ssid, sae_password="12345678", key_mgmt="SAE",
                       ieee80211w="2", ssid_protection="1",
                       scan_freq="2412")
    finally:
        dev[0].set("sae_pwe", "0")
        dev[0].set("rsn_overriding", "0")

def test_rsn_override_replace_ies(dev, apdev):
    """RSN overriding and replaced AP IEs"""
    check_sae_capab(dev[0])

    ssid = "test-rsn-override"
    params = hostapd.wpa2_params(ssid=ssid,
                                 passphrase="12345678",
                                 ieee80211w='1')
    params['rsn_override_key_mgmt'] = 'SAE'
    params['rsn_override_key_mgmt_2'] = 'SAE-EXT-KEY'
    params['rsn_override_pairwise'] = 'CCMP'
    params['rsn_override_pairwise_2'] = 'GCMP-256'
    params['rsn_override_mfp'] = '1'
    params['rsn_override_mfp_2'] = '2'
    params['beacon_prot'] = '1'
    params['sae_groups'] = '19 20'
    params['sae_require_mfp'] = '1'
    params['sae_pwe'] = '2'
    params['ssid_protection'] = '1'
    params['rsne_override'] = '30180100000fac040100000fac040200000facff000fac020c00'
    params['rsnxe_override'] = 'f40320eeee'
    params['rsnoe_override'] = 'dd1c506f9a290100000fac040100000fac040200000facff000fac088c00'
    params['rsno2e_override'] = 'dd1c506f9a2a0100000fac040100000fac090200000facff000fac18cc00'
    params['rsnxoe_override'] = 'dd07506f9a2b20bbbb'
    hapd = hostapd.add_ap(apdev[0], params)
    bssid = hapd.own_addr()

    try:
        dev[0].set("rsn_overriding", "1")
        dev[0].scan_for_bss(bssid, freq=2412)
        dev[0].set("sae_pwe", "2")
        dev[0].set("sae_groups", "")
        dev[0].connect(ssid, sae_password="12345678", key_mgmt="SAE",
                       ieee80211w="2", ssid_protection="1",
                       scan_freq="2412")
    finally:
        dev[0].set("sae_pwe", "0")
        dev[0].set("rsn_overriding", "0")

def test_rsn_override_rsnxe_extensibility(dev, apdev):
    """RSN overriding and RSNXE extensibility"""
    check_sae_capab(dev[0])

    ssid = "test-rsn-override"
    params = hostapd.wpa2_params(ssid=ssid,
                                 passphrase="12345678",
                                 ieee80211w='1')
    params['rsn_override_key_mgmt'] = 'SAE SAE-EXT-KEY'
    params['rsn_override_pairwise'] = 'CCMP GCMP-256'
    params['rsn_override_mfp'] = '2'
    params['beacon_prot'] = '1'
    params['sae_groups'] = '19 20'
    params['sae_require_mfp'] = '1'
    params['sae_pwe'] = '2'
    params['rsnxe_override'] = 'f4182f0000ffffffffffffffffffffffffffeeeeeeeeeeeeeeee'
    params['rsnxoe_override'] = 'dd1c506f9a2b2f0000ffffffffffffffffffffffffffeeeeeeeeeeeeeeee'
    hapd = hostapd.add_ap(apdev[0], params)
    bssid = hapd.own_addr()

    try:
        dev[0].set("rsn_overriding", "1")
        dev[0].scan_for_bss(bssid, freq=2412)
        dev[0].set("sae_pwe", "2")
        dev[0].set("sae_groups", "")
        dev[0].connect(ssid, sae_password="12345678", key_mgmt="SAE",
                       ieee80211w="2", ssid_protection="1",
                       scan_freq="2412")
    finally:
        dev[0].set("sae_pwe", "0")
        dev[0].set("rsn_overriding", "0")

def test_rsn_override_sta_only(dev, apdev):
    """RSN overriding enabled only on the STA"""
    check_sae_capab(dev[0])
    params = hostapd.wpa2_params(ssid="test-sae",
                                 passphrase="12345678")
    params['wpa_key_mgmt'] = 'SAE'
    hapd = hostapd.add_ap(apdev[0], params)

    dev[0].set("sae_groups", "")
    try:
        dev[0].set("rsn_overriding", "1")
        dev[0].connect("test-sae", psk="12345678", key_mgmt="SAE",
                       scan_freq="2412")
    finally:
        dev[0].set("rsn_overriding", "0")

def test_rsn_override_compatibility_mode(dev, apdev):
    """RSN overriding and WPA3-Personal Compatibility Mode"""
    check_sae_capab(dev[0])

    ssid = "test-rsn-override"
    params = hostapd.wpa2_params(ssid=ssid,
                                 passphrase="12345678")
    params['rsn_override_key_mgmt'] = 'SAE'
    params['rsn_override_key_mgmt_2'] = 'SAE-EXT-KEY'
    params['rsn_override_pairwise'] = 'CCMP'
    params['rsn_override_pairwise_2'] = 'GCMP-256'
    params['rsn_override_mfp'] = '2'
    params['rsn_override_mfp_2'] = '2'
    params['beacon_prot'] = '1'
    params['sae_groups'] = '19 20'
    params['sae_require_mfp'] = '1'
    params['sae_pwe'] = '2'
    hapd = hostapd.add_ap(apdev[0], params)
    bssid = hapd.own_addr()

    try:
        logger.info("RSN overriding capable STA using RSNO2E")
        dev[0].set("rsn_overriding", "1")
        dev[0].scan_for_bss(bssid, freq=2412)
        dev[0].set("sae_pwe", "2")
        dev[0].set("sae_groups", "")
        dev[0].connect(ssid, sae_password="12345678",
                       pairwise="GCMP-256", key_mgmt="SAE-EXT-KEY",
                       ieee80211w="2", scan_freq="2412")
        hapd.wait_sta()
        dev[0].request("REMOVE_NETWORK all")
        dev[0].wait_disconnected()
        hapd.wait_sta_disconnect()

        logger.info("RSN overriding capable STA using RSNOE")
        dev[0].set("sae_pwe", "0")
        dev[0].connect(ssid, sae_password="12345678",
                       pairwise="CCMP", key_mgmt="SAE",
                       ieee80211w="2", scan_freq="2412")
        hapd.wait_sta()
        dev[0].request("REMOVE_NETWORK all")
        dev[0].wait_disconnected()
        hapd.wait_sta_disconnect()

        logger.info("RSN overriding capable STA using RSNE")
        dev[0].connect(ssid, psk="12345678",
                       pairwise="CCMP", key_mgmt="WPA-PSK",
                       ieee80211w="0", scan_freq="2412")
        hapd.wait_sta()
        dev[0].request("REMOVE_NETWORK all")
        dev[0].wait_disconnected()
        hapd.wait_sta_disconnect()

        logger.info("RSN overriding uncapable STA using RSNE")
        dev[0].set("rsn_overriding", "0")
        dev[0].connect(ssid, psk="12345678",
                       pairwise="CCMP", key_mgmt="WPA-PSK",
                       ieee80211w="0", scan_freq="2412")
        hapd.wait_sta()
        dev[0].request("REMOVE_NETWORK all")
        dev[0].wait_disconnected()
        hapd.wait_sta_disconnect()
    finally:
        dev[0].set("sae_pwe", "0")
        dev[0].set("rsn_overriding", "0")
