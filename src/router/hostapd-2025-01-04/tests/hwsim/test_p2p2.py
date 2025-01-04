# Test cases for Wi-Fi Direct R2 features like unsynchronized service discovery
# (P2P USD), Bootstrapping and Pairing.
# Copyright (c) 2024, Qualcomm Innovation Center, Inc.
#
# This software may be distributed under the terms of the BSD license.
# See README for more details.

import binascii
import logging

from hwsim import HWSimRadio

logger = logging.getLogger()
import os
import hwsim_utils

from wpasupplicant import WpaSupplicant
from test_nan_usd import check_nan_usd_capab
from test_pasn import check_pasn_capab

def check_p2p2_capab(dev):
    check_nan_usd_capab(dev)
    check_pasn_capab(dev)

def set_p2p2_configs(dev):
    dev.global_request("P2P_SET pasn_type 3")
    dev.global_request("P2P_SET supported_bootstrapmethods 6")
    dev.global_request("P2P_SET pairing_setup 1")
    dev.global_request("P2P_SET pairing_cache 1")

def test_p2p_usd_publish_invalid_param(dev):
    """P2P USD Publish with invalid parameters"""
    check_p2p2_capab(dev[0])

    # Both solicited and unsolicited disabled is invalid
    cmd = "NAN_PUBLISH service_name=_test solicited=0 unsolicited=0 p2p=1"
    id0 = dev[0].global_request(cmd)
    if "FAIL" not in id0:
        raise Exception("NAN_PUBLISH accepts both solicited=0 and unsolicited=0 with p2p=1")

def test_p2p_usd_publish(dev, apdev):
    """P2P USD Publish"""
    check_p2p2_capab(dev[0])
    cmd = "NAN_PUBLISH service_name=_test unsolicited=0 srv_proto_type=2 ssi=6677 p2p=1"
    id0 = dev[0].global_request(cmd)
    if "FAIL" in id0:
        raise Exception("NAN_PUBLISH for P2P failed")

    cmd = "NAN_UPDATE_PUBLISH publish_id=" + id0 + " ssi=1122334455"
    if "FAIL" in dev[0].global_request(cmd):
        raise Exception("NAN_UPDATE_PUBLISH for P2P failed")

    cmd = "NAN_CANCEL_PUBLISH publish_id=" + id0
    if "FAIL" in dev[0].global_request(cmd):
        raise Exception("NAN_CANCEL_PUBLISH for P2P failed")

    ev = dev[0].wait_global_event(["NAN-PUBLISH-TERMINATED"], timeout=1)
    if ev is None:
        raise Exception("PublishTerminated event not seen")
    if "publish_id=" + id0 not in ev:
        raise Exception("Unexpected publish_id: " + ev)
    if "reason=user-request" not in ev:
        raise Exception("Unexpected reason: " + ev)

    cmd = "NAN_PUBLISH service_name=_test p2p=1"
    count = 0
    for i in range(256):
        if "FAIL" in dev[0].global_request(cmd):
            break
        count += 1
    logger.info("Maximum services: %d" % count)
    for i in range(count):
        cmd = "NAN_CANCEL_PUBLISH publish_id=%s" % (i + 1)
        if "FAIL" in dev[0].global_request(cmd):
            raise Exception("NAN_CANCEL_PUBLISH failed")

        ev = dev[0].wait_global_event(["NAN-PUBLISH-TERMINATED"], timeout=1)
        if ev is None:
            raise Exception("PublishTerminated event not seen")

def test_p2p_usd_subscribe(dev, apdev):
    """P2P USD Subscribe"""
    check_p2p2_capab(dev[0])
    cmd = "NAN_SUBSCRIBE service_name=_test active=1 srv_proto_type=2 ssi=1122334455 p2p=1"
    id0 = dev[0].global_request(cmd)
    if "FAIL" in id0:
        raise Exception("NAN_SUBSCRIBE for P2P failed")

    cmd = "NAN_CANCEL_SUBSCRIBE subscribe_id=" + id0
    if "FAIL" in dev[0].global_request(cmd):
        raise Exception("NAN_CANCEL_SUBSCRIBE for P2P failed")

    ev = dev[0].wait_global_event(["NAN-SUBSCRIBE-TERMINATED"], timeout=1)
    if ev is None:
        raise Exception("SubscribeTerminated event not seen")
    if "subscribe_id=" + id0 not in ev:
        raise Exception("Unexpected subscribe_id: " + ev)
    if "reason=user-request" not in ev:
        raise Exception("Unexpected reason: " + ev)

def test_p2p_usd_match(dev, apdev):
    """P2P USD Publish/Subscribe match"""
    check_p2p2_capab(dev[0])
    check_p2p2_capab(dev[1])
    cmd = "NAN_SUBSCRIBE service_name=_test active=1 srv_proto_type=2 ssi=1122334455 p2p=1"
    id0 = dev[0].global_request(cmd)
    if "FAIL" in id0:
        raise Exception("NAN_SUBSCRIBE for P2P failed")

    cmd = "NAN_PUBLISH service_name=_test unsolicited=0 srv_proto_type=2 ssi=6677 ttl=5 p2p=1"
    id1 = dev[1].global_request(cmd)
    if "FAIL" in id1:
        raise Exception("NAN_PUBLISH for P2P failed")

    ev = dev[0].wait_global_event(["P2P-DEVICE-FOUND"], timeout=5)
    if ev is None:
        raise Exception("Peer not found")
    ev = dev[1].wait_global_event(["P2P-DEVICE-FOUND"], timeout=5)
    if ev is None:
        raise Exception("Peer not found")

    ev = dev[0].wait_global_event(["NAN-DISCOVERY-RESULT"], timeout=5)
    if ev is None:
        raise Exception("DiscoveryResult event not seen")
    if "srv_proto_type=2" not in ev.split(' '):
        raise Exception("Unexpected srv_proto_type: " + ev)
    if "ssi=6677" not in ev.split(' '):
        raise Exception("Unexpected ssi: " + ev)

    dev[0].global_request("NAN_CANCEL_SUBSCRIBE subscribe_id=" + id0)
    dev[1].global_request("NAN_CANCEL_PUBLISH publish_id=" + id1)

def run_p2p_pairing_password(dev):
    check_p2p2_capab(dev[0])
    check_p2p2_capab(dev[1])

    set_p2p2_configs(dev[0])
    set_p2p2_configs(dev[1])

    cmd = "NAN_SUBSCRIBE service_name=_test active=1 srv_proto_type=2 ssi=1122334455 ttl=10 p2p=1"
    id0 = dev[0].global_request(cmd)
    if "FAIL" in id0:
        raise Exception("NAN_SUBSCRIBE for P2P failed")

    cmd = "NAN_PUBLISH service_name=_test unsolicited=0 srv_proto_type=2 ssi=6677 ttl=10 p2p=1"
    id1 = dev[1].global_request(cmd)
    if "FAIL" in id1:
        raise Exception("NAN_PUBLISH for P2P failed")

    ev = dev[0].wait_global_event(["P2P-DEVICE-FOUND"], timeout=5)
    if ev is None:
        raise Exception("Peer not found")
    ev = dev[1].wait_global_event(["P2P-DEVICE-FOUND"], timeout=5)
    if ev is None:
        raise Exception("Peer not found")

    ev = dev[0].wait_global_event(["NAN-DISCOVERY-RESULT"], timeout=5)
    if ev is None:
        raise Exception("DiscoveryResult event not seen")
    if "srv_proto_type=2" not in ev.split(' '):
        raise Exception("Unexpected srv_proto_type: " + ev)
    if "ssi=6677" not in ev.split(' '):
        raise Exception("Unexpected ssi: " + ev)

    cmd = "NAN_CANCEL_SUBSCRIBE subscribe_id=" + id0
    if "FAIL" in dev[0].global_request(cmd):
        raise Exception("NAN_CANCEL_SUBSCRIBE for P2P failed")
    cmd = "NAN_CANCEL_PUBLISH publish_id=" + id1
    if "FAIL" in dev[1].global_request(cmd):
        raise Exception("NAN_CANCEL_PUBLISH for P2P failed")

    cmd = "P2P_CONNECT " + dev[0].p2p_dev_addr() + " pair he go_intent=15 p2p2 bstrapmethod=2 auth password=975310123 freq=2437"
    id0 = dev[1].global_request(cmd)
    if "FAIL" in id0:
        raise Exception("P2P_CONNECT auth Failed")

    cmd = "P2P_CONNECT " + dev[1].p2p_dev_addr() + " pair he go_intent=5 p2p2 bstrapmethod=32 password=975310123"
    id0 = dev[0].global_request(cmd)
    if "FAIL" in id0:
        raise Exception("P2P_CONNECT Failed")

    ev = dev[0].wait_global_event(["P2P-GROUP-STARTED"], timeout=10)
    if ev is None:
        raise Exception("Group formation timed out")
    dev[0].group_form_result(ev, no_pwd=True)
    dev[0].dump_monitor()

    ev = dev[1].wait_global_event(["P2P-GROUP-STARTED"], timeout=10)
    if ev is None:
        raise Exception("Group formation timed out(2)")
    dev[1].group_form_result(ev)

    dev[1].remove_group()
    dev[0].wait_go_ending_session()
    dev[0].dump_monitor()

def test_p2p_pairing_password(dev, apdev):
    """P2P Pairing with Password"""
    run_p2p_pairing_password(dev)

def test_p2p_pairing_password_dev(dev, apdev):
    """P2P Pairing with Password with dedicated P2P device"""
    with HWSimRadio(use_p2p_device=True) as (radio, iface):
        wpas = WpaSupplicant(global_iface='/tmp/wpas-wlan5')
        wpas.interface_add(iface)
        run_p2p_pairing_password([dev[0], wpas])

def test_p2p_pairing_password_dev2(dev, apdev):
    """P2P Pairing with Password with dedicated P2P device (reversed) and no group interface"""
    with HWSimRadio(use_p2p_device=True) as (radio, iface):
        wpas = WpaSupplicant(global_iface='/tmp/wpas-wlan5')
        wpas.interface_add(iface)
        wpas.global_request("SET p2p_no_group_iface 1")
        run_p2p_pairing_password([wpas, dev[0]])

def test_p2p_pairing_opportunistic(dev, apdev):
    """P2P Pairing with Opportunistic"""
    check_p2p2_capab(dev[0])
    check_p2p2_capab(dev[1])

    set_p2p2_configs(dev[0])
    set_p2p2_configs(dev[1])

    cmd = "NAN_SUBSCRIBE service_name=_test active=1 srv_proto_type=2 ssi=1122334455 ttl=10 p2p=1"
    id0 = dev[0].global_request(cmd)
    if "FAIL" in id0:
        raise Exception("NAN_SUBSCRIBE for P2P failed")

    cmd = "NAN_PUBLISH service_name=_test unsolicited=0 srv_proto_type=2 ssi=6677 ttl=10 p2p=1"
    id1 = dev[1].global_request(cmd)
    if "FAIL" in id1:
        raise Exception("NAN_PUBLISH for P2P failed")

    ev = dev[0].wait_global_event(["P2P-DEVICE-FOUND"], timeout=5)
    if ev is None:
        raise Exception("Peer not found")
    ev = dev[1].wait_global_event(["P2P-DEVICE-FOUND"], timeout=5)
    if ev is None:
        raise Exception("Peer not found")

    ev = dev[0].wait_global_event(["NAN-DISCOVERY-RESULT"], timeout=5)
    if ev is None:
        raise Exception("DiscoveryResult event not seen")
    if "srv_proto_type=2" not in ev.split(' '):
        raise Exception("Unexpected srv_proto_type: " + ev)
    if "ssi=6677" not in ev.split(' '):
        raise Exception("Unexpected ssi: " + ev)

    cmd = "NAN_CANCEL_SUBSCRIBE subscribe_id=" + id0
    if "FAIL" in dev[0].global_request(cmd):
        raise Exception("NAN_CANCEL_SUBSCRIBE for P2P failed")
    cmd = "NAN_CANCEL_PUBLISH publish_id=" + id1
    if "FAIL" in dev[1].global_request(cmd):
        raise Exception("NAN_CANCEL_PUBLISH for P2P failed")

    cmd = "P2P_CONNECT " + dev[0].p2p_dev_addr() + " pair he go_intent=15 p2p2 bstrapmethod=1 auth freq=2437"
    id0 = dev[1].global_request(cmd)
    if "FAIL" in id0:
        raise Exception("P2P_CONNECT auth Failed")

    cmd = "P2P_CONNECT " + dev[1].p2p_dev_addr() + " pair he go_intent=5 p2p2 bstrapmethod=1"
    id0 = dev[0].global_request(cmd)
    if "FAIL" in id0:
        raise Exception("P2P_CONNECT Failed")

    ev = dev[0].wait_global_event(["P2P-GROUP-STARTED"], timeout=10)
    if ev is None:
        raise Exception("Group formation timed out")
    #dev[0].group_form_result(ev)
    dev[0].dump_monitor()

    ev = dev[1].wait_global_event(["P2P-GROUP-STARTED"], timeout=10)
    if ev is None:
        raise Exception("Group formation timed out(2)")
    dev[1].group_form_result(ev)
    dev[1].wait_sta()

    dev[1].remove_group()
    dev[0].wait_go_ending_session()
    dev[0].dump_monitor()

def test_p2p_auto_go_and_client_join(dev, apdev):
    """A new client joining a group using P2P Pairing/Opportunistic"""
    check_p2p2_capab(dev[0])
    check_p2p2_capab(dev[1])

    set_p2p2_configs(dev[0])
    set_p2p2_configs(dev[1])

    cmd = "NAN_SUBSCRIBE service_name=_test active=1 srv_proto_type=2 ssi=1122334455 ttl=10 p2p=1"
    id0 = dev[0].global_request(cmd)
    if "FAIL" in id0:
        raise Exception("NAN_SUBSCRIBE for P2P failed")

    cmd = "NAN_PUBLISH service_name=_test unsolicited=0 srv_proto_type=2 ssi=6677 ttl=10 p2p=1"
    id1 = dev[1].global_request(cmd)
    if "FAIL" in id1:
        raise Exception("NAN_PUBLISH for P2P failed")

    ev = dev[0].wait_global_event(["P2P-DEVICE-FOUND"], timeout=5)
    if ev is None:
        raise Exception("Peer not found")
    ev = dev[1].wait_global_event(["P2P-DEVICE-FOUND"], timeout=5)
    if ev is None:
        raise Exception("Peer not found")

    ev = dev[0].wait_global_event(["NAN-DISCOVERY-RESULT"], timeout=5)
    if ev is None:
        raise Exception("DiscoveryResult event not seen")
    if "srv_proto_type=2" not in ev.split(' '):
        raise Exception("Unexpected srv_proto_type: " + ev)
    if "ssi=6677" not in ev.split(' '):
        raise Exception("Unexpected ssi: " + ev)

    cmd = "NAN_CANCEL_SUBSCRIBE subscribe_id=" + id0
    if "FAIL" in dev[0].global_request(cmd):
        raise Exception("NAN_CANCEL_SUBSCRIBE for P2P failed")
    cmd = "NAN_CANCEL_PUBLISH publish_id=" + id1
    if "FAIL" in dev[1].global_request(cmd):
        raise Exception("NAN_CANCEL_PUBLISH for P2P failed")

    cmd = "P2P_GROUP_ADD p2p2"
    res = dev[1].global_request(cmd)
    if "FAIL" in res:
        raise Exception("P2P_GROUP_ADD failed")

    ev = dev[1].wait_global_event(["P2P-GROUP-STARTED"], timeout=10)
    if ev is None:
        raise Exception("Group formation timed out(2)")

    cmd = "P2P_CONNECT " + dev[0].p2p_dev_addr() + " pair he go_intent=15 p2p2 bstrapmethod=1 join auth"
    id0 = dev[1].global_request(cmd)
    if "FAIL" in id0:
        raise Exception("P2P_CONNECT auth failed")

    cmd = "P2P_CONNECT " + dev[1].p2p_dev_addr() + " pair p2p2 join bstrapmethod=1"
    id0 = dev[0].global_request(cmd)
    if "FAIL" in id0:
        raise Exception("P2P_CONNECT failed")

    ev = dev[0].wait_global_event(["P2P-GROUP-STARTED"], timeout=10)
    if ev is None:
        raise Exception("Group formation timed out")
    #dev[0].group_form_result(ev)
    dev[0].dump_monitor()

    dev[1].wait_sta()

    dev[1].remove_group()
    dev[0].wait_go_ending_session()
    dev[0].dump_monitor()

def test_p2p_auto_go_and_client_join_sae(dev, apdev):
    """A new client joining a group using P2P Pairing/SAE"""
    check_p2p2_capab(dev[0])
    check_p2p2_capab(dev[1])

    set_p2p2_configs(dev[0])
    set_p2p2_configs(dev[1])

    cmd = "NAN_SUBSCRIBE service_name=_test active=1 srv_proto_type=2 ssi=1122334455 ttl=10 p2p=1"
    id0 = dev[0].global_request(cmd)
    if "FAIL" in id0:
        raise Exception("NAN_SUBSCRIBE for P2P failed")

    cmd = "NAN_PUBLISH service_name=_test unsolicited=0 srv_proto_type=2 ssi=6677 ttl=10 p2p=1"
    id1 = dev[1].global_request(cmd)
    if "FAIL" in id1:
        raise Exception("NAN_PUBLISH for P2P failed")

    ev = dev[0].wait_global_event(["P2P-DEVICE-FOUND"], timeout=5)
    if ev is None:
        raise Exception("Peer not found")
    ev = dev[1].wait_global_event(["P2P-DEVICE-FOUND"], timeout=5)
    if ev is None:
        raise Exception("Peer not found")

    ev = dev[0].wait_global_event(["NAN-DISCOVERY-RESULT"], timeout=5)
    if ev is None:
        raise Exception("DiscoveryResult event not seen")
    if "srv_proto_type=2" not in ev.split(' '):
        raise Exception("Unexpected srv_proto_type: " + ev)
    if "ssi=6677" not in ev.split(' '):
        raise Exception("Unexpected ssi: " + ev)

    cmd = "NAN_CANCEL_SUBSCRIBE subscribe_id=" + id0
    if "FAIL" in dev[0].global_request(cmd):
        raise Exception("NAN_CANCEL_SUBSCRIBE for P2P failed")
    cmd = "NAN_CANCEL_PUBLISH publish_id=" + id1
    if "FAIL" in dev[1].global_request(cmd):
        raise Exception("NAN_CANCEL_PUBLISH for P2P failed")

    cmd = "P2P_GROUP_ADD p2p2"
    res = dev[1].global_request(cmd)
    if "FAIL" in res:
        raise Exception("P2P_GROUP_ADD failed")

    ev = dev[1].wait_global_event(["P2P-GROUP-STARTED"], timeout=10)
    if ev is None:
        raise Exception("Group formation timed out(2)")

    cmd = "P2P_CONNECT " + dev[0].p2p_dev_addr() + " pair he go_intent=15 p2p2 bstrapmethod=2 join auth password=975310123"
    id0 = dev[1].global_request(cmd)
    if "FAIL" in id0:
        raise Exception("P2P_CONNECT auth failed")

    cmd = "P2P_CONNECT " + dev[1].p2p_dev_addr() + " pair p2p2 join bstrapmethod=32 password=975310123"
    id0 = dev[0].global_request(cmd)
    if "FAIL" in id0:
        raise Exception("P2P_CONNECT failed")

    ev = dev[0].wait_global_event(["P2P-GROUP-STARTED"], timeout=10)
    if ev is None:
        raise Exception("Group formation timed out")
    #dev[0].group_form_result(ev)
    dev[0].dump_monitor()

    dev[1].wait_sta()

    dev[1].remove_group()
    dev[0].wait_go_ending_session()
    dev[0].dump_monitor()

def test_p2p_pairing_verification(dev, apdev):
    """P2P Pairing with Verification"""

    """wpa_supplicant config file for pairing verification"""
    config = "/tmp/test_p2p.conf"
    if os.path.exists(config):
        os.remove(config)

    wpas = WpaSupplicant(global_iface='/tmp/wpas-wlan5')

    with open(config, "w") as f:
         f.write("update_config=1\n")

    wpas.interface_add("wlan5", config=config)

    check_p2p2_capab(wpas)
    check_p2p2_capab(dev[1])

    set_p2p2_configs(wpas)
    set_p2p2_configs(dev[1])

    wpas.global_request("SET update_config 1")
    wpas.global_request("SAVE_CONFIG")

    cmd = "NAN_SUBSCRIBE service_name=_test active=1 srv_proto_type=2 ssi=1122334455 ttl=10 p2p=1"
    id0 = wpas.global_request(cmd)
    if "FAIL" in id0:
        raise Exception("NAN_SUBSCRIBE for P2P failed")

    cmd = "NAN_PUBLISH service_name=_test unsolicited=0 srv_proto_type=2 ssi=6677 ttl=10 p2p=1"
    id1 = dev[1].global_request(cmd)
    if "FAIL" in id1:
        raise Exception("NAN_PUBLISH for P2P failed")

    ev = wpas.wait_global_event(["P2P-DEVICE-FOUND"], timeout=5)
    if ev is None:
        raise Exception("Peer not found")
    ev = dev[1].wait_global_event(["P2P-DEVICE-FOUND"], timeout=5)
    if ev is None:
        raise Exception("Peer not found")

    ev = wpas.wait_global_event(["NAN-DISCOVERY-RESULT"], timeout=5)
    if ev is None:
        raise Exception("DiscoveryResult event not seen")
    if "srv_proto_type=2" not in ev.split(' '):
        raise Exception("Unexpected srv_proto_type: " + ev)
    if "ssi=6677" not in ev.split(' '):
        raise Exception("Unexpected ssi: " + ev)

    cmd = "NAN_CANCEL_SUBSCRIBE subscribe_id=" + id0
    if "FAIL" in wpas.global_request(cmd):
        raise Exception("NAN_CANCEL_SUBSCRIBE for P2P failed")
    cmd = "NAN_CANCEL_PUBLISH publish_id=" + id1
    if "FAIL" in dev[1].global_request(cmd):
        raise Exception("NAN_CANCEL_PUBLISH for P2P failed")

    cmd = "P2P_CONNECT " + wpas.p2p_dev_addr() + " pair he go_intent=15 p2p2 bstrapmethod=2 auth password=975310123 freq=2437 persistent"
    id0 = dev[1].global_request(cmd)
    if "FAIL" in id0:
        raise Exception("P2P_CONNECT auth Failed")

    cmd = "P2P_CONNECT " + dev[1].p2p_dev_addr() + " pair he go_intent=5 p2p2 bstrapmethod=32 password=975310123 persistent"
    id0 = wpas.global_request(cmd)
    if "FAIL" in id0:
        raise Exception("P2P_CONNECT Failed")

    ev = dev[1].wait_global_event(["P2P-GROUP-STARTED"], timeout=30)
    if ev is None:
        raise Exception("Group formation timed out")
    dev[1].group_form_result(ev)

    ev = wpas.wait_global_event(["P2P-GROUP-STARTED",
                                   "WPA: 4-Way Handshake failed"], timeout=30)
    if ev is None:
        raise Exception("Group formation timed out (2)")
    wpas.dump_monitor()

    dev[1].remove_group()
    wpas.wait_go_ending_session()
    wpas.dump_monitor()

    wpas.interface_remove("wlan5")
    wpas.interface_add("wlan5", config=config)

    cmd = "NAN_SUBSCRIBE service_name=_test active=1 srv_proto_type=2 ssi=1122334455 ttl=10 p2p=1"
    id0 = wpas.global_request(cmd)
    if "FAIL" in id0:
        raise Exception("NAN_SUBSCRIBE for P2P failed (2)")

    cmd = "NAN_PUBLISH service_name=_test unsolicited=0 srv_proto_type=2 ssi=6677 ttl=10 p2p=1"
    id1 = dev[1].global_request(cmd)
    if "FAIL" in id1:
        raise Exception("NAN_PUBLISH for P2P failed (2)")

    ev = wpas.wait_global_event(["P2P-DEVICE-FOUND"], timeout=5)
    if ev is None:
        raise Exception("Peer not found")
    ev = dev[1].wait_global_event(["P2P-DEVICE-FOUND"], timeout=5)
    if ev is None:
        raise Exception("Peer not found")

    ev = wpas.wait_global_event(["NAN-DISCOVERY-RESULT"], timeout=5)
    if ev is None:
        raise Exception("DiscoveryResult event not seen")
    if "srv_proto_type=2" not in ev.split(' '):
        raise Exception("Unexpected srv_proto_type: " + ev)
    if "ssi=6677" not in ev.split(' '):
        raise Exception("Unexpected ssi: " + ev)

    cmd = "NAN_CANCEL_SUBSCRIBE subscribe_id=" + id0
    if "FAIL" in wpas.global_request(cmd):
        raise Exception("NAN_CANCEL_SUBSCRIBE for P2P failed (2)")
    cmd = "NAN_CANCEL_PUBLISH publish_id=" + id1
    if "FAIL" in dev[1].global_request(cmd):
        raise Exception("NAN_CANCEL_PUBLISH for P2P failed (2)")

    wpas.global_request("SET persistent_reconnect 1")
    dev[1].global_request("SET persistent_reconnect 1")
    peer = wpas.get_peer(dev[1].p2p_dev_addr())
    if 'persistent' not in peer:
        raise Exception("Missing information on persistent group for the peer")
    cmd = "P2P_INVITE persistent=" + peer['persistent'] + " peer=" + dev[1].p2p_dev_addr() + " p2p2"
    id0 = wpas.global_request(cmd)
    if "FAIL" in id0:
        raise Exception("P2P_INVITE Failed")

    ev = dev[1].wait_global_event(["P2P-GROUP-STARTED"], timeout=30)
    if ev is None:
        raise Exception("Group re-invoke failed")
    dev[1].group_form_result(ev)

    ev = wpas.wait_global_event(["P2P-GROUP-STARTED",
                                 "WPA: 4-Way Handshake failed"], timeout=30)
    if ev is None:
        raise Exception("Group re-invoke failed (2)")
    if "P2P-GROUP-STARTED" not in ev:
        raise Exception("Failed to complete group start on reinvocation")
    wpas.dump_monitor()

    dev[1].remove_group()
    wpas.wait_go_ending_session()
    wpas.dump_monitor()

def test_p2p_auto_go_pcc_with_two_cli(dev, apdev):
    """P2P autonomous GO in PCC mode with PSK and SAE clients"""
    check_p2p2_capab(dev[0])
    set_p2p2_configs(dev[0])

    cmd = "P2P_GROUP_ADD p2p2 p2pmode=2 freq=2462"
    res = dev[0].global_request(cmd)
    if "FAIL" in res:
        raise Exception("P2P_GROUP_ADD failed")
    ev = dev[0].wait_global_event(["P2P-GROUP-STARTED"], timeout=10)
    if ev is None:
        raise Exception("Group formation timed out")

    res = dev[0].group_form_result(ev)
    if dev[0].get_group_status_field("passphrase", extra="WPS") != res['passphrase']:
        raise Exception("passphrase mismatch")
    if dev[0].group_request("P2P_GET_PASSPHRASE") != res['passphrase']:
        raise Exception("passphrase mismatch(2)")

    logger.info("Connect legacy non-WPS P2P client")
    dev[1].global_request("P2P_SET disabled 1")
    dev[0].dump_monitor()
    dev[1].connect(ssid=res['ssid'], psk=res['passphrase'], proto='RSN',
                   key_mgmt='WPA-PSK', pairwise='CCMP',
                   group='CCMP', scan_freq=res['freq'])
    dev[0].wait_sta(addr=dev[1].own_addr(), wait_4way_hs=True)

    try:
        logger.info("Connect P2P2 client")
        dev[2].global_request("P2P_SET disabled 1")
        dev[0].dump_monitor()
        dev[2].set("rsn_overriding", "1")
        dev[2].set("sae_pwe", "2")
        dev[2].set("sae_groups", "")
        dev[2].connect(ssid=res['ssid'], sae_password=res['passphrase'],
                       proto='WPA2', key_mgmt='SAE', ieee80211w='2',
                       pairwise='GCMP-256 CCMP', group='GCMP-256 CCMP',
                       scan_freq=res['freq'])
        dev[0].wait_sta(addr=dev[2].own_addr(), wait_4way_hs=True)

        hwsim_utils.test_connectivity_p2p_sta(dev[1], dev[2])

        dev[2].request("DISCONNECT")
        dev[2].wait_disconnected()
        dev[0].wait_sta_disconnect(addr=dev[2].own_addr())
    finally:
        dev[2].set("sae_pwe", "0")
        dev[2].set("rsn_overriding", "0")

    dev[1].request("DISCONNECT")
    dev[1].wait_disconnected()
    dev[0].wait_sta_disconnect(addr=dev[1].own_addr())

    dev[0].remove_group()

def test_p2p_auto_go_pcc_with_p2p2_cli(dev, apdev):
    """P2P autonomous GO in PCC mode with P2P2 clients"""
    check_p2p2_capab(dev[0])
    check_p2p2_capab(dev[1])
    set_p2p2_configs(dev[0])

    cmd = "P2P_GROUP_ADD p2p2 p2pmode=2 freq=2462"
    res = dev[0].global_request(cmd)
    if "FAIL" in res:
        raise Exception("P2P_GROUP_ADD failed")
    ev = dev[0].wait_global_event(["P2P-GROUP-STARTED"], timeout=10)
    if ev is None:
        raise Exception("Group formation timed out")

    res = dev[0].group_form_result(ev)
    if dev[0].get_group_status_field("passphrase", extra="WPS") != res['passphrase']:
        raise Exception("passphrase mismatch")
    if dev[0].group_request("P2P_GET_PASSPHRASE") != res['passphrase']:
        raise Exception("passphrase mismatch(2)")

    ssidhex =  binascii.hexlify(res['ssid'].encode()).decode()
    cmd = "P2P_CONNECT " + dev[0].p2p_dev_addr() + " pair p2p2 skip_prov join password=" + res['passphrase'] + " ssid=" + ssidhex
    id0 = dev[1].global_request(cmd)
    if "FAIL" in id0:
        raise Exception("P2P_CONNECT failed")

    ev = dev[1].wait_global_event(["CTRL-EVENT-CONNECTED"], timeout=10)
    if ev is None:
        raise Exception("Group formation timed out")
    dev[1].dump_monitor()

    dev[0].wait_sta()

    dev[0].remove_group()
    dev[1].dump_monitor()
