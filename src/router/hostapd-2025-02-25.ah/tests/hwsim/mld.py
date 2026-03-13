# Python class for controlling Multi Link Device
# Copyright (c) 2024, Jouni Malinen <j@w1.fi>
#
# This software may be distributed under the terms of the BSD license.
# See README for more details.

import os
import logging
import wpaspy

logger = logging.getLogger()
hapd_ctrl = '/var/run/hostapd'

class MultiLinkDevice:
    def __init__(self, ifname, ctrl=hapd_ctrl, port=8877):
        self.ifname = ifname
        self.ctrl = wpaspy.Ctrl(os.path.join(ctrl, ifname))
        self.dbg = ifname

    def close_ctrl(self):
        self.ctrl.close()
        self.ctrl = None

    def request(self, cmd):
        logger.debug(self.dbg + ": MLD CTRL: " + cmd)
        return self.ctrl.request(cmd)

    def ping(self):
        return "PONG" in self.request("PING")

def get_mld_obj(ifname, ctrl=hapd_ctrl, port=8877):
    mld = MultiLinkDevice(ifname, ctrl, port)
    if not mld.ping():
        raise Exception("Could not ping MLD %s" % ifname)

    return mld
