#!/usr/bin/env python
#
# Copyright (c) 2014 Sippy Software, Inc. All rights reserved.
#
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without modification,
# are permitted provided that the following conditions are met:
#
# 1. Redistributions of source code must retain the above copyright notice, this
# list of conditions and the following disclaimer.
#
# 2. Redistributions in binary form must reproduce the above copyright notice,
# this list of conditions and the following disclaimer in the documentation and/or
# other materials provided with the distribution.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
# ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
# ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
# (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
# LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
# ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
# SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

NCMDS = 100000
CMD_TEMPLATE_U = 'U %s 127.0.0.2 %d %s'
CMD_TEMPLATE_L = 'L %s 127.0.0.3 %d %s %s'

from twisted.internet import reactor
from random import random

import sys
sys.path.append('../../sippy.git')

from sippy.Rtp_proxy_client_local import Rtp_proxy_client_local
from sippy.Timeout import Timeout

sys.path.append('../../sip_test_data/python')

from pickrandom import pickrandom

class sess_data(object):
    command_u = None
    command_l = None

class res(object):
    rcodes = None
    rremain = None
    nsent = None
    rtppc = None
    prand_cid = None
    prand_tags = None

    def __init__(self, rremain):
        self.rcodes = []
        self.rremain = rremain
        self.nsent = 0
        global_config = {}
        self.prand_cid = pickrandom(kind = 'call_id')
        self.prand_tags = pickrandom(kind = 'to_from_tags')
        self.rtppc = Rtp_proxy_client_local(global_config, address = '/var/run/rtpproxy.sock')

    def issue_command(self):
        call_id = self.prand_cid.get()
        ftag = self.prand_tags.get()
        ttag = self.prand_tags.get()
        sd = sess_data()
        sd.command_u = CMD_TEMPLATE_U % (call_id, self.nsent + 1000, ftag)
        sd.command_l = CMD_TEMPLATE_L % (call_id, self.nsent + 2000, ftag, ttag)
        self.rtppc.send_command(sd.command_u, self.rtpp_reply_u, sd)
        #print command

    def rtpp_reply_u(self, rval, sdata):
        #print 'rtpp_reply'
        if rval == 'E72':
            print 'bingo 1'
            Timeout(self.issue_command, 3 * random())
            return
        if rval == None or rval.startswith('E'):
            print ('error: %s' % str(rval))
            reactor.stop()
            return

        self.rcodes.append(rval)
        self.issue_command_l(sdata)

    def issue_command_l(self, sdata):
        self.rtppc.send_command(sdata.command_l, self.rtpp_reply_l, sdata)
        #print command
        self.nsent += 1

    def rtpp_reply_l(self, rval, sdata):
        #print 'rtpp_reply'
        if rval == 'E71':
            print 'bingo 2'
            Timeout(self.issue_command_l, 2 * random(), 1, sdata)
            return
        if rval == None or rval.startswith('E'):
            print ('error: %s' % str(rval))
            reactor.stop()
            return

        self.rcodes.append(rval)
        self.rremain -= 1
        if self.rremain == 0:
            reactor.stop()
        self.issue_command()

rres = res(NCMDS)
rres.issue_command()

reactor.run()
#print 'main:', rres.rcodes
rres.rtppc.shutdown()
