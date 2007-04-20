#!/bin/sh
######################################################################
# This program is free software; you can redistribute it and/or      #
# modify it under the terms of the GNU General Public License as     #
# published by the Free Software Foundation; either version 2 of the #
# License, or (at your option) any later version.                    #
#                                                                    #
# This program is distributed in the hope that it will be useful,    #
# but WITHOUT ANY WARRANTY; without even the implied warranty of     #
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the      #
# GNU General Public License for more details.                       #
#                                                                    #
# You should have received a copy of the GNU General Public License  #
# along with this program; if not, write to the                      #
# Free Software Foundation, Inc.,                                    #
# 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA             #
######################################################################
# This file is part of a Milkfish Embedded OpenSER SIP Router Setup  #
# More information can be found at http://www.milkfish.org           #
#                                                                    #
# The Milkfish Router Services - SIP Tracer Shell Script             #
#                                                                    #
# Built/Version:  20060930                                           #
# Author/Contact: Michael Poehnl <budrus@sipwerk.com>                #
# Copyright (C) 2007 by sipwerk - All rights reserved.               #
#                                                                    #
# Please note that this software is under development and comes with #
# absolutely no warranty, to the extend permitted by applicable law. #
######################################################################

[ $(nvram get milkfish_siptrace) = "off" ] && exit 0
if [ -z $1 ]; then
 while true
   do
   read input
   echo $input >> /tmp/sip_trace.tmp
   if [ -z $input ]; then
     [ -e /var/log/sip_trace.log ] && head -n500 /var/log/sip_trace.log >> /tmp/sip_trace.tmp
     mv /tmp/sip_trace.tmp /var/log/sip_trace.log
     exit 0
   fi
 done
fi 