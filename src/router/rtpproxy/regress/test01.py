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

VALID_CMDS = ('U', 'L', 'D', 'P', 'R', 'C', 'S', 'V', 'I', 'Q', 'X')
INVALID_CMDS = ('A', 'B')
GARBAGE = ('dssj239', '238uwedguw', '39d9ashjo', 'ndkshf23', 'asjo083')

from twisted.internet import reactor

import sys
sys.path.append('~/projects/sippy.git')

from sippy.Rtp_proxy_client_local import Rtp_proxy_client_local

def caps_query2(rval, res_res):
    res_res.rcodes.append(rval)
    #print 'caps_query2:', res_res
    res_res.rremain -= 1
    if res_res.rremain == 0:
        reactor.stop()

global_config = {}
rtppc = Rtp_proxy_client_local(global_config, address = '/var/run/rtpproxy.sock')
res = []

class res(object):
    rcodes = None
    rremain = None

    def __init__(self, rremain):
        self.rcodes = []
        self.rremain = rremain

rres = res(0)
for command in INVALID_CMDS:
    command += ' aVF 20080403'
    rres.rremain += 1
    rtppc.send_command(command, caps_query2, rres)

for command in VALID_CMDS:
    for i in range(0, len(GARBAGE)):
        s = command
        for j in range(0, i):
            s = '%s %s' % (s, GARBAGE[j])
        rres.rremain += 1
        rtppc.send_command(s, caps_query2, rres)

reactor.run()
print 'main:', rres.rcodes
rtppc.shutdown()
