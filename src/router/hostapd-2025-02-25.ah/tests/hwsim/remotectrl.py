#!/usr/bin/python
#
# wpa_supplicant/hostapd control interface using Python
# Copyright (c) 2024, Jouni Malinen <j@w1.fi>
#
# This software may be distributed under the terms of the BSD license.
# See README for more details.

from wpaspy import Ctrl
import remotehost

class RemoteCtrl(Ctrl):
    def __init__(self, path, port=9877, hostname=None, ifname=None):
        self.started = False
        self.attached = False
        self.path = path
        self.port = port
        self.ifname = ifname
        self.hostname = hostname
        self.proc = None

        self.host = remotehost.Host(hostname)
        self.started = True

    def __del__(self):
        self.close()

    def close(self):
        if self.attached:
            try:
                self.detach()
            except Exception as e:
                # Need to ignore this allow the socket to be closed
                self.attached = False
                pass

        if self.host and self.started:
            self.started = False

    def request(self, cmd, timeout=10):
        if self.host:
            cmd = '\'' + cmd + '\''
            if self.ifname:
                _cmd = ['wpa_cli', '-p', self.path, '-i', self.ifname, "raw " + cmd]
            else:
                _cmd = ['wpa_cli', '-g', self.path, "raw " + cmd]
            status, buf = self.host.execute(_cmd)
            return buf

    def attach(self):
        if self.attached:
            return

        if self.host:
            if self.ifname:
                _cmd = [ "wpa_cli", "-p", self.path, "-i", self.ifname ]
            else:
                _cmd = [ "wpa_cli", '-g', self.path]
            self.proc = self.host.proc_run(_cmd)
            self.attached = True

    def detach(self):
        if not self.attached:
            return

        if self.hostname and self.proc:
            self.request("DETACH")
            self.request("QUIT")
            self.host.proc_stop(self.proc)
            self.attached = False
            self.proc = None

    def terminate(self):
        if self.attached:
            try:
                self.detach()
            except Exception as e:
                # Need to ignore this to allow the socket to be closed
                self.attached = False
        self.request("TERMINATE")
        self.close()

    def pending(self, timeout=0):
        if self.host and self.proc:
            return self.host.proc_pending(self.proc, timeout=timeout)
        return False

    def recv(self):
        if self.host and self.proc:
            res = self.host.proc_read(self.proc)
            return res
        return ""
