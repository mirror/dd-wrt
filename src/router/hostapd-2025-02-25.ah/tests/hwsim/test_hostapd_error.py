# hostapd error paths
# Copyright (c) 2024, Jouni Malinen <j@w1.fi>
#
# This software may be distributed under the terms of the BSD license.
# See README for more details.

import hostapd
from utils import *

def test_hostapd_error_drv_init(dev, apdev):
    """hostapd error path on driver interface initialization failure"""
    hapd = hostapd.add_ap(apdev[0], {"ssid": "ctrl"})
    with fail_test(hapd, 1, "nl80211_setup_ap"):
        hapd1 = hostapd.add_ap(apdev[1], {"ssid": "open"}, no_enable=True)
        if "FAIL" not in hapd1.request("ENABLE"):
            raise Exception("ENABLE succeeded unexpectedly")
