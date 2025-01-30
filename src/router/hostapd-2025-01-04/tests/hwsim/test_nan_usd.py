# Test cases for Wi-Fi Aware unsynchronized service discovery (NAN USD)
# Copyright (c) 2024, Qualcomm Innovation Center, Inc.
#
# This software may be distributed under the terms of the BSD license.
# See README for more details.

import time

import logging
logger = logging.getLogger()

import hostapd
from utils import *

def check_nan_usd_capab(dev):
    capa = dev.request("GET_CAPABILITY nan")
    if "USD" not in capa:
        raise HwsimSkip("NAN USD not supported")

def test_nan_usd_publish_invalid_param(dev):
    """NAN USD Publish with invalid parameters"""
    check_nan_usd_capab(dev[0])

    # Both solicited and unsolicited disabled is invalid
    cmd = "NAN_PUBLISH service_name=_test solicited=0 unsolicited=0"
    id0 = dev[0].request(cmd)
    if "FAIL" not in id0:
        raise Exception("NAN_PUBLISH accepts both solicited=0 and unsolicited=0")

def test_nan_usd_publish(dev, apdev):
    """NAN USD Publish"""
    check_nan_usd_capab(dev[0])
    cmd = "NAN_PUBLISH service_name=_test srv_proto_type=2 ssi=6677"
    id0 = dev[0].request(cmd)
    if "FAIL" in id0:
        raise Exception("NAN_PUBLISH failed")

    cmd = "NAN_UPDATE_PUBLISH publish_id=" + id0 + " ssi=1122334455"
    if "FAIL" in dev[0].request(cmd):
        raise Exception("NAN_UPDATE_PUBLISH failed")

    cmd = "NAN_CANCEL_PUBLISH publish_id=" + id0
    if "FAIL" in dev[0].request(cmd):
        raise Exception("NAN_CANCEL_PUBLISH failed")

    ev = dev[0].wait_event(["NAN-PUBLISH-TERMINATED"], timeout=1)
    if ev is None:
        raise Exception("PublishTerminated event not seen")
    if "publish_id=" + id0 not in ev:
        raise Exception("Unexpected publish_id: " + ev)
    if "reason=user-request" not in ev:
        raise Exception("Unexpected reason: " + ev)

    cmd = "NAN_PUBLISH service_name=_test"
    count = 0
    for i in range(256):
        if "FAIL" in dev[0].request(cmd):
            break
        count += 1
    logger.info("Maximum services: %d" % count)
    for i in range(count):
        cmd = "NAN_CANCEL_PUBLISH publish_id=%s" % (i + 1)
        if "FAIL" in dev[0].request(cmd):
            raise Exception("NAN_CANCEL_PUBLISH failed")

        ev = dev[0].wait_event(["NAN-PUBLISH-TERMINATED"], timeout=1)
        if ev is None:
            raise Exception("PublishTerminated event not seen")

def test_nan_usd_subscribe(dev, apdev):
    """NAN USD Subscribe"""
    check_nan_usd_capab(dev[0])
    cmd = "NAN_SUBSCRIBE service_name=_test active=1 srv_proto_type=2 ssi=1122334455"
    id0 = dev[0].request(cmd)
    if "FAIL" in id0:
        raise Exception("NAN_SUBSCRIBE failed")

    cmd = "NAN_CANCEL_SUBSCRIBE subscribe_id=" + id0
    if "FAIL" in dev[0].request(cmd):
        raise Exception("NAN_CANCEL_SUBSCRIBE failed")

    ev = dev[0].wait_event(["NAN-SUBSCRIBE-TERMINATED"], timeout=1)
    if ev is None:
        raise Exception("SubscribeTerminated event not seen")
    if "subscribe_id=" + id0 not in ev:
        raise Exception("Unexpected subscribe_id: " + ev)
    if "reason=user-request" not in ev:
        raise Exception("Unexpected reason: " + ev)

def test_nan_usd_match(dev, apdev):
    """NAN USD Publish/Subscribe match"""
    check_nan_usd_capab(dev[0])
    cmd = "NAN_SUBSCRIBE service_name=_test srv_proto_type=2 ssi=1122334455"
    id0 = dev[0].request(cmd)
    if "FAIL" in id0:
        raise Exception("NAN_SUBSCRIBE failed")

    cmd = "NAN_PUBLISH service_name=_test srv_proto_type=2 ssi=6677 ttl=5"
    id1 = dev[1].request(cmd)
    if "FAIL" in id1:
        raise Exception("NAN_PUBLISH failed")

    ev = dev[0].wait_event(["NAN-DISCOVERY-RESULT"], timeout=5)
    if ev is None:
        raise Exception("DiscoveryResult event not seen")
    if "srv_proto_type=2" not in ev.split(' '):
        raise Exception("Unexpected srv_proto_type: " + ev)
    if "ssi=6677" not in ev.split(' '):
        raise Exception("Unexpected ssi: " + ev)

    dev[0].request("NAN_CANCEL_SUBSCRIBE id=" + id0)
    dev[1].request("NAN_CANCEL_PUBLISH id=" + id1)

def test_nan_usd_match2(dev, apdev):
    """NAN USD Publish/Subscribe match (2)"""
    check_nan_usd_capab(dev[0])
    cmd = "NAN_PUBLISH service_name=_test srv_proto_type=2 ssi=6677 ttl=10 fsd=0"
    id0 = dev[1].request(cmd)
    if "FAIL" in id0:
        raise Exception("NAN_PUBLISH failed")

    time.sleep(1)

    cmd = "NAN_SUBSCRIBE service_name=_test srv_proto_type=2 ssi=1122334455 active=1"
    id0 = dev[0].request(cmd)
    if "FAIL" in id0:
        raise Exception("NAN_SUBSCRIBE failed")

    ev = dev[0].wait_event(["NAN-DISCOVERY-RESULT"], timeout=5)
    if ev is None:
        raise Exception("DiscoveryResult event not seen")
    if "srv_proto_type=2" not in ev.split(' '):
        raise Exception("Unexpected srv_proto_type: " + ev)
    if "ssi=6677" not in ev.split(' '):
        raise Exception("Unexpected ssi: " + ev)

    # Check for publisher and subscriber functionality to time out
    ev = dev[0].wait_event(["NAN-SUBSCRIBE-TERMINATED"], timeout=2)
    if ev is None:
        raise Exception("Subscribe not terminated")
    ev = dev[1].wait_event(["NAN-PUBLISH-TERMINATED"], timeout=10)
    if ev is None:
        raise Exception("Publish not terminated")

def test_nan_usd_match3(dev, apdev):
    """NAN USD Publish/Subscribe match (3)"""
    check_nan_usd_capab(dev[0])
    cmd = "NAN_SUBSCRIBE service_name=_test srv_proto_type=2 ssi=1122334455 active=1"
    id0 = dev[0].request(cmd)
    if "FAIL" in id0:
        raise Exception("NAN_SUBSCRIBE failed")

    time.sleep(0.05)

    cmd = "NAN_PUBLISH service_name=_test srv_proto_type=2 ssi=6677 ttl=10"
    id1 = dev[1].request(cmd)
    if "FAIL" in id1:
        raise Exception("NAN_PUBLISH failed")

    ev = dev[0].wait_event(["NAN-DISCOVERY-RESULT"], timeout=5)
    if ev is None:
        raise Exception("DiscoveryResult event not seen")
    if "srv_proto_type=2" not in ev.split(' '):
        raise Exception("Unexpected srv_proto_type: " + ev)
    if "ssi=6677" not in ev.split(' '):
        raise Exception("Unexpected ssi: " + ev)

    dev[0].request("NAN_CANCEL_SUBSCRIBE id=" + id0)
    dev[1].request("NAN_CANCEL_PUBLISH id=" + id1)

def split_nan_event(ev):
    vals = dict()
    for p in ev.split(' ')[1:]:
        name, val = p.split('=')
        vals[name] = val
    return vals

def test_nan_usd_followup(dev, apdev):
    """NAN USD Publish/Subscribe match and follow-up"""
    check_nan_usd_capab(dev[0])
    run_nan_usd_followup(dev[0], dev[1])

def test_nan_usd_followup_multi_chan(dev, apdev):
    """NAN USD Publish/Subscribe match and follow-up with multi channels"""
    check_nan_usd_capab(dev[0])
    run_nan_usd_followup(dev[0], dev[1], multi_chan=True)

def test_nan_usd_followup_hostapd(dev, apdev):
    """NAN USD Publish/Subscribe match and follow-up with hostapd"""
    check_nan_usd_capab(dev[0])
    hapd = hostapd.add_ap(apdev[0], {"ssid": "open",
                                     "channel": "6"})
    run_nan_usd_followup(hapd, dev[1])

def run_nan_usd_followup(dev0, dev1, multi_chan=False):
    cmd = "NAN_SUBSCRIBE service_name=_test srv_proto_type=3 ssi=1122334455"
    if multi_chan:
        cmd += " freq=2462"
    id0 = dev0.request(cmd)
    if "FAIL" in id0:
        raise Exception("NAN_SUBSCRIBE failed")

    cmd = "NAN_PUBLISH service_name=_test srv_proto_type=3 ssi=6677 ttl=10"
    if multi_chan:
        cmd += " freq=2412 freq_list=2437,2462"
    id1 = dev1.request(cmd)
    if "FAIL" in id1:
        raise Exception("NAN_PUBLISH failed")

    ev = dev0.wait_event(["NAN-DISCOVERY-RESULT"], timeout=10)
    if ev is None:
        raise Exception("DiscoveryResult event not seen")
    vals = split_nan_event(ev)
    if vals['srv_proto_type'] != '3':
        raise Exception("Unexpected srv_proto_type: " + ev)
    if vals['ssi'] != '6677':
        raise Exception("Unexpected ssi: " + ev)
    if vals['subscribe_id'] != id0:
        raise Exception("Unexpected subscribe_id: " + ev)
    if vals['publish_id'] != id1:
        raise Exception("Unexpected publish_id: " + ev)
    addr1 = vals['address']

    # Automatically sent Follow-up message without ssi
    ev = dev1.wait_event(["NAN-RECEIVE"], timeout=5)
    if ev is None:
        raise Exception("Receive event not seen")
    vals2 = split_nan_event(ev)
    if vals2['ssi'] != '':
        raise Exception("Unexpected ssi in Follow-up: " + ev)

    # Follow-up from subscriber to publisher
    time.sleep(0.2)
    cmd = "NAN_TRANSMIT handle={} req_instance_id={} address={} ssi=8899".format(vals['subscribe_id'], vals['publish_id'], addr1)
    if "FAIL" in dev0.request(cmd):
        raise Exception("NAN_TRANSMIT failed")

    ev = dev1.wait_event(["NAN-RECEIVE"], timeout=5)
    if ev is None:
        raise Exception("Receive event not seen")
    vals = split_nan_event(ev)
    if vals['ssi'] != '8899':
        raise Exception("Unexpected ssi in Follow-up: " + ev)
    if vals['id'] != id1:
        raise Exception("Unexpected id: " + ev)
    if vals['peer_instance_id'] != id0:
        raise Exception("Unexpected peer_instance_id: " + ev)
    addr0 = vals['address']

    # Follow-up from publisher to subscriber
    cmd = "NAN_TRANSMIT handle={} req_instance_id={} address={} ssi=aabbccdd".format(id1, vals['peer_instance_id'], addr0)
    if "FAIL" in dev1.request(cmd):
        raise Exception("NAN_TRANSMIT failed")

    ev = dev0.wait_event(["NAN-RECEIVE"], timeout=5)
    if ev is None:
        raise Exception("Receive event not seen")
    vals = split_nan_event(ev)
    if vals['ssi'] != 'aabbccdd':
        raise Exception("Unexpected ssi in Follow-up: " + ev)
    if vals['id'] != id0:
        raise Exception("Unexpected id: " + ev)
    if vals['peer_instance_id'] != id1:
        raise Exception("Unexpected peer_instance_id: " + ev)

    # Another Follow-up message from publisher to subscriber
    cmd = "NAN_TRANSMIT handle={} req_instance_id={} address={} ssi=eeff".format(id1, vals['peer_instance_id'], addr0)
    if "FAIL" in dev1.request(cmd):
        raise Exception("NAN_TRANSMIT failed")

    ev = dev0.wait_event(["NAN-RECEIVE"], timeout=5)
    if ev is None:
        raise Exception("Receive event not seen")
    vals = split_nan_event(ev)
    if vals['ssi'] != 'eeff':
        raise Exception("Unexpected ssi in Follow-up: " + ev)
    if vals['id'] != id0:
        raise Exception("Unexpected id: " + ev)
    if vals['peer_instance_id'] != id1:
        raise Exception("Unexpected peer_instance_id: " + ev)

    # And one more Follow-up message from publisher to subscriber after some
    # delay.
    time.sleep(0.5)
    cmd = "NAN_TRANSMIT handle={} req_instance_id={} address={} ssi=22334455".format(id1, vals['peer_instance_id'], addr0)
    if "FAIL" in dev1.request(cmd):
        raise Exception("NAN_TRANSMIT failed")

    ev = dev0.wait_event(["NAN-RECEIVE"], timeout=5)
    if ev is None:
        raise Exception("Receive event not seen")
    vals = split_nan_event(ev)
    if vals['ssi'] != '22334455':
        raise Exception("Unexpected ssi in Follow-up: " + ev)
    if vals['id'] != id0:
        raise Exception("Unexpected id: " + ev)
    if vals['peer_instance_id'] != id1:
        raise Exception("Unexpected peer_instance_id: " + ev)

    dev0.request("NAN_CANCEL_SUBSCRIBE id=" + id0)
    dev1.request("NAN_CANCEL_PUBLISH id=" + id1)

def test_nan_usd_solicited_publisher(dev, apdev):
    """NAN USD Publish/Subscribe match with solicited-only Publisher"""
    check_nan_usd_capab(dev[0])
    cmd = "NAN_PUBLISH service_name=_test unsolicited=0 srv_proto_type=2 ssi=6677"
    id1 = dev[1].request(cmd)
    if "FAIL" in id1:
        raise Exception("NAN_PUBLISH failed")

    cmd = "NAN_SUBSCRIBE service_name=_test active=1 srv_proto_type=2 ssi=1122334455"
    id0 = dev[0].request(cmd)
    if "FAIL" in id0:
        raise Exception("NAN_SUBSCRIBE failed")

    ev = dev[0].wait_event(["NAN-DISCOVERY-RESULT"], timeout=5)
    if ev is None:
        raise Exception("DiscoveryResult event not seen")
    vals = split_nan_event(ev)
    if vals['srv_proto_type'] != "2":
        raise Exception("Unexpected ssi: " + ev)
    if vals['ssi'] != "6677":
        raise Exception("Unexpected ssi: " + ev)

    ev = dev[1].wait_event(["NAN-REPLIED"], timeout=5)
    if ev is None:
        raise Exception("Replied event not seen")
    vals = split_nan_event(ev)
    if vals['publish_id'] != id1:
        raise Exception("Unexpected publish_id: " + ev)
    if vals['subscribe_id'] != id0:
        raise Exception("Unexpected subscribe_id: " + ev)
    if vals['address'] != dev[0].own_addr():
        raise Exception("Unexpected address: " + ev)
    if vals['srv_proto_type'] != "2":
        raise Exception("Unexpected ssi: " + ev)
    if vals['ssi'] != "1122334455":
        raise Exception("Unexpected ssi: " + ev)

def test_nan_usd_solicited_publisher_timeout(dev, apdev):
    """NAN USD solicited Publisher timeout"""
    check_nan_usd_capab(dev[0])
    cmd = "NAN_PUBLISH service_name=_test unsolicited=0 ttl=10 srv_proto_type=2 ssi=6677"
    id = dev[0].request(cmd)
    if "FAIL" in id:
        raise Exception("NAN_PUBLISH failed")
    ev = dev[0].wait_event(["NAN-PUBLISH-TERMINATED"], timeout=2)
    if ev is not None:
        raise Exception("Too quick Publish termination")

    ev = dev[0].wait_event(["NAN-PUBLISH-TERMINATED"], timeout=10)
    if ev is None:
        raise Exception("Publish not terminated")
    if "reason=timeout" not in ev:
        raise Exception("Unexpected reason: " + ev)

def test_nan_usd_unsolicited_publisher_timeout(dev, apdev):
    """NAN USD unsolicited Publisher timeout"""
    check_nan_usd_capab(dev[0])
    cmd = "NAN_PUBLISH service_name=_test solicited=0 ttl=10 srv_proto_type=2 ssi=6677"
    id = dev[0].request(cmd)
    if "FAIL" in id:
        raise Exception("NAN_PUBLISH failed")
    ev = dev[0].wait_event(["NAN-PUBLISH-TERMINATED"], timeout=2)
    if ev is not None:
        raise Exception("Too quick Publish termination")

    ev = dev[0].wait_event(["NAN-PUBLISH-TERMINATED"], timeout=10)
    if ev is None:
        raise Exception("Publish not terminated")
    if "reason=timeout" not in ev:
        raise Exception("Unexpected reason: " + ev)

def test_nan_usd_publish_all_chans(dev, apdev):
    """NAN USD Publish - all channels"""
    check_nan_usd_capab(dev[0])
    cmd = "NAN_PUBLISH service_name=_test srv_proto_type=2 ssi=6677 freq_list=all ttl=10"
    id0 = dev[0].request(cmd)
    if "FAIL" in id0:
        raise Exception("NAN_PUBLISH failed")

    ev = dev[0].wait_event(["NAN-PUBLISH-TERMINATED"], timeout=15)
    if ev is None:
        raise Exception("PublishTerminated event not seen")

def test_nan_usd_publish_multi_chan(dev, apdev):
    """NAN USD Publish - multi channel"""
    check_nan_usd_capab(dev[0])
    cmd = "NAN_PUBLISH service_name=_test srv_proto_type=2 ssi=6677 freq_list=2412,2462 ttl=10"
    id0 = dev[0].request(cmd)
    if "FAIL" in id0:
        raise Exception("NAN_PUBLISH failed")

    ev = dev[0].wait_event(["NAN-PUBLISH-TERMINATED"], timeout=15)
    if ev is None:
        raise Exception("PublishTerminated event not seen")

def test_nan_usd_publish_multi_chan_solicited(dev, apdev):
    """NAN USD Publish - multi channel - solicited"""
    check_nan_usd_capab(dev[0])
    cmd = "NAN_PUBLISH service_name=_test unsolicited=0 srv_proto_type=2 ssi=6677 freq_list=2412,2462 ttl=10"
    id0 = dev[0].request(cmd)
    if "FAIL" in id0:
        raise Exception("NAN_PUBLISH failed")

    ev = dev[0].wait_event(["NAN-PUBLISH-TERMINATED"], timeout=15)
    if ev is None:
        raise Exception("PublishTerminated event not seen")

def test_nan_usd_publish_multi_chan_pause(dev, apdev):
    """NAN USD Publish - multi channel"""
    check_nan_usd_capab(dev[0])
    cmd = "NAN_PUBLISH service_name=_test srv_proto_type=2 ssi=6677 freq_list=2412,2462 ttl=10"
    id0 = dev[0].request(cmd)
    if "FAIL" in id0:
        raise Exception("NAN_PUBLISH failed")

    time.sleep(1)

    cmd = "NAN_SUBSCRIBE service_name=_test srv_proto_type=2 ssi=1122334455"
    id1 = dev[1].request(cmd)
    if "FAIL" in id1:
        raise Exception("NAN_SUBSCRIBE failed")

    cmd = "NAN_SUBSCRIBE service_name=_test srv_proto_type=2 ssi=8899 active=1"
    id2 = dev[2].request(cmd)
    if "FAIL" in id2:
        raise Exception("NAN_SUBSCRIBE failed")

    ev = dev[0].wait_event(["NAN-RECEIVE"], timeout=10)
    if ev is None:
        raise Exception("Receive event not seen")
    if "address=" + dev[1].own_addr() in ev.split():
        dev1 = dev[1]
        dev2 = dev[2]
    elif "address=" + dev[2].own_addr() in ev.split():
        dev1 = dev[2]
        dev2 = dev[1]
    else:
        raise Exception("Unexpected address in NAN-RECEIVE: " + ev)

    ev = dev1.wait_event(["NAN-DISCOVERY-RESULT"], timeout=5)
    if ev is None:
        raise Exception("DiscoveryResult event not seen (1)")
    vals = split_nan_event(ev)

    cmd = "NAN_TRANSMIT handle={} req_instance_id={} address={} ssi=8899".format(vals['subscribe_id'], vals['publish_id'], dev[0].own_addr())
    if "FAIL" in dev1.request(cmd):
        raise Exception("NAN_TRANSMIT failed")
    ev = dev[0].wait_event(["NAN-RECEIVE"], timeout=5)
    if ev is None:
        raise Exception("Receive event not seen for follow-up (1)")
    vals = split_nan_event(ev)
    cmd = "NAN_UNPAUSE_PUBLISH publish_id={} peer_instance_id={} peer={}".format(vals['id'], vals['peer_instance_id'], vals['address'])
    if "OK" not in dev[0].request(cmd):
        raise Exception("NAN_UNPAUSE_PUBLISH failed")

    ev = dev2.wait_event(["NAN-DISCOVERY-RESULT"], timeout=5)
    if ev is None:
        raise Exception("DiscoveryResult event not seen (2)")
    vals = split_nan_event(ev)
    cmd = "NAN_TRANSMIT handle={} req_instance_id={} address={} ssi=8899".format(vals['subscribe_id'], vals['publish_id'], dev[0].own_addr())
    if "FAIL" in dev2.request(cmd):
        raise Exception("NAN_TRANSMIT failed")
    ev = dev[0].wait_event(["NAN-RECEIVE"], timeout=5)
    if ev is None:
        raise Exception("Receive event not seen for follow-up (2)")

    ev = dev[0].wait_event(["NAN-PUBLISH-TERMINATED"], timeout=15)
    if ev is None:
        raise Exception("PublishTerminated event not seen")
