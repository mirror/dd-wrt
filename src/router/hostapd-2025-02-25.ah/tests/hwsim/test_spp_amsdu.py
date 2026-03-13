# SPP A-MSDU tests
# Copyright (c) 2025, Intel Corporation
#
# This software may be distributed under the terms of the BSD license.
# See README for more details.

import logging
import hostapd
from utils import HwsimSkip, parse_ie
import hwsim_utils
import time
import os
from tshark import run_tshark

logger = logging.getLogger()

SPP_AMSDU_SUPP_CIPHERS = ['CCMP', 'GCMP', 'CCMP-256', 'GCMP-256']
SPP_AMSDU_STA_FLAG = '[SPP-A-MSDU]'
WLAN_EID_RSNX = 244
RSNX_SPP_AMSDU_CAPAB_MASK = 0x4000
RSNX_SPP_AMSDU_CAPAB_BIT = 14

def setup_ap(apdev, ssid, spp_amsdu, cipher):
    params = {'ssid': ssid}
    wpa_passphrase = ''

    if cipher != '':
        wpa_passphrase = '123456789'
        params.update({'wpa': '2',
                       'wpa_passphrase': wpa_passphrase,
                       'wpa_key_mgmt': 'WPA-PSK',
                       'rsn_pairwise': cipher,
                       'group_cipher': cipher})

    params['spp_amsdu'] = spp_amsdu

    return hostapd.add_ap(apdev, params), wpa_passphrase

def skip_unsupported_spp_amsdu(dev, hapd):
    flags2 = hapd.request("DRIVER_FLAGS2").splitlines()[1:]
    if "SPP_AMSDU" not in flags2:
        raise HwsimSkip('AP: SPP A-MSDU is not supported')

    # Sanity (should be the same for both AP/STA)
    flags2 = dev.request("DRIVER_FLAGS2").splitlines()[1:]
    if "SPP_AMSDU" not in flags2:
        raise HwsimSkip('STA: SPP A-MSDU is not supported')

def check_sta_rsnxe(spp_amsdu_enabled, logdir):
    """Check STA assoc request RSXNE"""
    # tshark returns the RSNXE data in a comma separated string
    out = run_tshark(os.path.join(logdir, 'hwsim0.pcapng'),
                     "wlan.fc.type_subtype == 0x0",
                     display=["wlan.rsnx"])
    hex_rsnxe = out.rstrip().split(',')
    if len(hex_rsnxe) < 2:
        if spp_amsdu_enabled:
            raise Exception('STA: RSNXE SPP A-MSDU is not set')
        else:
            return

    second_rsnxe_byte = int(hex_rsnxe[1], 16)
    spp_amsdu_set = second_rsnxe_byte & (1 << (RSNX_SPP_AMSDU_CAPAB_BIT % 8))
    logger.debug('STA RSNXE SPP A-MSDU capable bit is %sset' %
                 '' if spp_amsdu_set else 'not ')

    if spp_amsdu_enabled and not spp_amsdu_set:
        raise Exception('STA: SPP A-MSDU capab bit is not set in RSNXE')
    if not spp_amsdu_enabled and spp_amsdu_set:
        raise Exception('STA: Unexpected SPP A-MSDU capab bit set in RSNXE')

def check_ap_rsnxe(spp_amsdu_enabled, ies):
    if not WLAN_EID_RSNX in ies:
        if spp_amsdu_enabled:
            raise Exception('AP: RSNXE is not present')
        else:
            # nothing to check
            return

    rsnx_hex_str = ''.join([hex(byte)[2:].zfill(2) \
                            for byte in reversed(ies[WLAN_EID_RSNX])])
    rsnx_hex = int(rsnx_hex_str, 16)
    spp_amsdu_set = rsnx_hex & RSNX_SPP_AMSDU_CAPAB_MASK
    logger.debug('AP RSNXE SPP A-MSDU capable bit is %sset' %
                 '' if spp_amsdu_set else 'not ')
    if spp_amsdu_enabled and not spp_amsdu_set:
        raise Exception('AP: SPP A-MSDU capab bit is not set in RSNXE')
    if not spp_amsdu_enabled and spp_amsdu_set:
        raise Exception('AP: Unexpected SPP A-MSDU capab bit set in RSNXE')

def check_ap_sta_flags(dev, hapd, spp_amsdu_enabled):
    """Check AP SPP A-MSDU STA flags"""
    sta = hapd.get_sta(dev.own_addr())
    spp_amsdu_flag_set = SPP_AMSDU_STA_FLAG in sta['flags']

    logger.debug('AP SPP A-MSDU STA flag is %sset' %
                 '' if spp_amsdu_flag_set else 'not ')
    if spp_amsdu_enabled and not spp_amsdu_flag_set:
        raise Exception('SPP-A-MSDU flag not present for STA')
    if not spp_amsdu_enabled and spp_amsdu_flag_set:
        raise Exception('Unexpected SPP-A-MSDU flag present for STA')

def _run(dev, apdev, logdir, spp_amsdu, cipher=''):
    """
    1. Connect to AP (SPP A-MSDU enabled/disabled)
    2. test connectivity
    3. Verify AP/STA advertised SPP A-MSDU capabilities (enabled/disabled)
    """
    # Sanity
    if cipher != '' and cipher not in dev.get_capability('pairwise'):
        raise HwsimSkip('%s not supported' % cipher)

    ssid='test-spp-amsdu-%s' % cipher if cipher != '' else 'open'
    hapd, wpa_passphrase = setup_ap(apdev, ssid, spp_amsdu=spp_amsdu,
                                    cipher=cipher)
    skip_unsupported_spp_amsdu(dev, hapd)

    dev.connect(ssid, key_mgmt='NONE' if cipher == '' else '',
                psk=wpa_passphrase, pairwise=cipher, group=cipher)
    hapd.wait_sta()
    time.sleep(0.1)
    hwsim_utils.test_connectivity(dev, hapd)

    if spp_amsdu != '0' and cipher in SPP_AMSDU_SUPP_CIPHERS:
        spp_amsdu_enabled = True
    else:
        spp_amsdu_enabled = False

    # Verify AP capabilities
    bss = dev.get_bss(hapd.own_addr())
    bss_ies = parse_ie(bss['ie'])
    check_ap_rsnxe(spp_amsdu_enabled, bss_ies)
    check_ap_sta_flags(dev, hapd, spp_amsdu_enabled)

    # Verify STA capabilities
    check_sta_rsnxe(spp_amsdu_enabled, logdir)

def test_spp_amsdu_ccmp(dev, apdev, params):
    """SPP A-MSDU with CCMP"""
    _run(dev[0], apdev[0], params['logdir'], spp_amsdu='1', cipher='CCMP')

def test_spp_amsdu_ccmp256(dev, apdev, params):
    """SPP A-MSDU with CCMP-256"""
    _run(dev[0], apdev[0], params['logdir'], spp_amsdu='1', cipher='CCMP-256')

def test_spp_amsdu_gcmp(dev, apdev, params):
    """SPP A-MSDU with GCMP"""
    _run(dev[0], apdev[0], params['logdir'], spp_amsdu='1', cipher='GCMP')

def test_spp_amsdu_gcmp256(dev, apdev, params):
    """SPP A-MSDU with GCMP-256"""
    _run(dev[0], apdev[0], params['logdir'], spp_amsdu='1', cipher='GCMP-256')

def test_spp_amsdu_tkip(dev, apdev, params):
    """SPP A-MSDU disabled with TKIP"""
    _run(dev[0], apdev[0], params['logdir'], spp_amsdu='1', cipher='TKIP')

def test_spp_amsdu_open(dev, apdev, params):
    """SPP A-MSDU disabled in open network"""
    _run(dev[0], apdev[0], params['logdir'], spp_amsdu='1')

def test_spp_amsdu_disabled(dev, apdev, params):
    """SPP A-MSDU explicitly disabled"""
    _run(dev[0], apdev[0], params['logdir'], spp_amsdu='0', cipher='CCMP')
