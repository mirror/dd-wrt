# The olsr.org Optimized Link-State Routing daemon (olsrd)
#
# (c) by the OLSR project
#
# See our Git repository to find out who worked on this file
# and thus is a copyright holder on it.
#
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#
# * Redistributions of source code must retain the above copyright
#   notice, this list of conditions and the following disclaimer.
# * Redistributions in binary form must reproduce the above copyright
#   notice, this list of conditions and the following disclaimer in
#   the documentation and/or other materials provided with the
#   distribution.
# * Neither the name of olsr.org, olsrd nor the names of its
#   contributors may be used to endorse or promote products derived
#   from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
# FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
# COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
# INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
# BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
# LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
# CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
# LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
# ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
# POSSIBILITY OF SUCH DAMAGE.
#
# Visit http://www.olsr.org for more information.
#
# If you find this software useful feel free to make a donation
# to the project. For more information see the website or contact
# the copyright holders.
#

sizex=1920
sizey=1080

set terminal svg dynamic enhanced fname 'Helvetica' fsize 16 mousing name "SmartGatewayCost" butt solid size sizex,sizey
set output 'SmartGatewayCost.svg'

set grid xtics lt 0 lw 1 lc rgb "#bbbbbb"
set grid ytics lt 0 lw 1 lc rgb "#bbbbbb"
set grid ztics lt 0 lw 1 lc rgb "#bbbbbb"

set dummy x,y
set isosamples 100, 100


set title "Smart Gateway Cost (Symmetric Link)" 

#                     WexitU   WexitD   Wetx
# path_cost_weight =  ------ + ------ + ---- * path_cost
#                     exitUm   exitDm   Detx

WexitU=1.0
WexitD=1.0
Wetx=1.0
Detx=4096.0

set xlabel "Uplink / Downlink (Mbps)" 
xlow=0.0
xhigh=1.0
set xlabel  offset character 0, 0, 0 font "Helvetica" textcolor lt -1 norotate
set xrange [ xlow : xhigh ] noreverse nowriteback

set ylabel "Path Cost ( = ETX * 1024 )"
ylow=1000.0
yhigh=10000.0
set ylabel  offset character 0, 0, 0 font "Helvetica" textcolor lt -1 rotate by -270
set yrange [ ylow : yhigh ] noreverse nowriteback


set zlabel "Costs" 
set zlabel  offset character -2, 0, 0 font "" textcolor lt -1 norotate

splot (WexitU/x)+(WexitD/x)+((Wetx/Detx)*y)
