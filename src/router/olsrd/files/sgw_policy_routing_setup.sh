#!/bin/bash

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

set -e
set -u


declare script="$0"
declare scriptName="$(basename "${0%\.*}")"
declare -a arguments=( ${@} )
declare -i argc=$#


###############################################################################
#
# OVERVIEW
#
###############################################################################

# Tables (from SmartGatewayTablesOffset):
#                                               +-----------------+-----------------+---------------+
#                                               | sgwsrvtun table | egressif tables | sgwtun tables |
#                                               +-----------------+-----------------+---------------+
# Example:                                              90               91 92            93 94 ...
#
#
# Rules (from SmartGatewayRulesOffset):
# +-----------------------+---------------------+-----------------+-----------------+---------------+
# | egressif bypass rules | olsrif bypass rules | sgwsrvtun rule  | egressif rules  | sgwtun rules  |
# +-----------------------+---------------------+-----------------+-----------------+---------------+
# Example:  84 85               86 87 88 89             90               91 92            93 94 ...


###############################################################################
#
# SETTINGS
#
###############################################################################

declare IPVERSION_4="ipv4"
declare IPVERSION_6="ipv6"

declare MODE_CLEANUP="cleanup"
declare MODE_GENERIC="generic"
declare MODE_OLSRIF="olsrif"
declare MODE_SGWSRVTUN="sgwsrvtun"
declare MODE_EGRESSIF="egressif"
declare MODE_SGWTUN="sgwtun"

declare ADDMODE_ADD="add"
declare ADDMODE_DEL="del"

declare -i MODE_CLEANUP_ARGC=0
declare -i MODE_GENERIC_ARGC=0
declare -i MODE_OLSRIF_ARGC=2
declare -i MODE_EGRESSIF_ARGC=4
declare -i MODE_SGWSRVTUN_ARGC=3
declare -i MODE_SGWTUN_ARGC=3


###############################################################################
#
# HELPER FUNCTIONS
#
###############################################################################

function usage() {
  echo ""
  echo "The script was called as:"
  echo "  $script ${arguments[@]:-}"
  echo ""
  echo "Usage:"
  echo "  $script instanceId ipVersion mode addMode ifName tableNr ruleNr bypassRuleNr"
  echo "    - instanceId  : the olsrd instance id"
  echo "    - ipVersion   : $IPVERSION_4 or $IPVERSION_6"
  echo "    - mode        : $MODE_CLEANUP, $MODE_GENERIC, $MODE_OLSRIF, $MODE_EGRESSIF, $MODE_SGWSRVTUN or $MODE_SGWTUN"
  echo "    - addMode     : $ADDMODE_ADD or $ADDMODE_DEL"
  echo "    - ifName      : the interface name       , only relevant for modes $MODE_EGRESSIF, $MODE_SGWSRVTUN, $MODE_SGWTUN"
  echo "    - tableNr     : the routing table number , only relevant for modes $MODE_EGRESSIF, $MODE_SGWSRVTUN, $MODE_SGWTUN"
  echo "    - ruleNr      : the ip rule number       , only relevant for modes $MODE_EGRESSIF, $MODE_SGWSRVTUN, $MODE_SGWTUN"
  echo "    - bypassRuleNr: the bypass ip rule number, only relevant for mode  $MODE_EGRESSIF, $MODE_OLSRIF"
}

function error() {
  local -i firstLine=1
  while [ $# -gt 0 ]; do
    if [ $firstLine -eq 1 ]; then
      echo "Error: $1"
    else
      echo "       $1"
    fi
    firstLine=0
    shift 1
  done
}


###############################################################################
#
# HELPER FUNCTIONS
#
###############################################################################

function updateLogFile() {
  local logLine="$ipVersion $mode $ADDMODE_DEL"
  while [ $# -gt 0 ]; do
    logLine="$logLine $1"
    shift 1
  done

  echo "$logLine" >> "$logFile"
}


###############################################################################
#
# MODE FUNCTIONS
#
###############################################################################

function cleanup() {
  if [ ! -e "$logFile" ]; then
    return
  fi

  if [ "$addMode" = "$ADDMODE_ADD" ] && \
     [ -s "$logFile" ]; then
    # read logFile
    local ifsOrg="$IFS"
    IFS=$'\n'
    local -a lines=( $(cat "$logFile" | sed -r '/^[[:space:]]*$/ d') )
    IFS="$ifsOrg"

    local -i index=${#lines[*]}
    index+=-1
    while [ $index -ge 0 ]; do
      set +e
      "$script" "$instanceId" ${lines[$index]}
      set -e
      index+=-1
    done
  fi

  rm -f "$logFile"
}

function generic() {
  "$IPTABLES" $IPTABLES_ARGS -t mangle "$ADDMODE_IPTABLES" PREROUTING  -m conntrack ! --ctstate NEW -j CONNMARK --restore-mark
  "$IPTABLES" $IPTABLES_ARGS -t mangle "$ADDMODE_IPTABLES" OUTPUT      -m conntrack ! --ctstate NEW -j CONNMARK --restore-mark
}

function olsrif() {
  local interfaceName="$1"
  local bypassRuleNr="$2"

  "$IP"       $IP_ARGS        rule      "$ADDMODE_IP" iif    "$interfaceName" table main       priority "$bypassRuleNr"
}

function egressif() {
  local interfaceName="$1"
  local tableNr="$2"
  local ruleNr="$3"
  local bypassRuleNr="$4"

  "$IPTABLES" $IPTABLES_ARGS -t mangle "$ADDMODE_IPTABLES" POSTROUTING -m conntrack --ctstate NEW -o "$interfaceName" -j CONNMARK --set-mark "$ruleNr"
  "$IPTABLES" $IPTABLES_ARGS -t mangle "$ADDMODE_IPTABLES" INPUT       -m conntrack --ctstate NEW -i "$interfaceName" -j CONNMARK --set-mark "$ruleNr"
  "$IP"       $IP_ARGS       rule      "$ADDMODE_IP" fwmark "$ruleNr"        table "$tableNr" priority "$ruleNr"
  "$IP"       $IP_ARGS       rule      "$ADDMODE_IP" iif    "$interfaceName" table main       priority "$bypassRuleNr"
}

function sgwsrvtun() {
  local interfaceName="$1"
  local tableNr="$2"
  local ruleNr="$3"

  "$IPTABLES" $IPTABLES_ARGS -t mangle "$ADDMODE_IPTABLES" PREROUTING  -m conntrack --ctstate NEW -i "$interfaceName" -j CONNMARK --set-mark "$ruleNr"
  "$IP"       $IP_ARGS       rule      "$ADDMODE_IP" iif    "$interfaceName" table "$tableNr" priority "$ruleNr"
  "$IP"       $IP_ARGS       rule      "$ADDMODE_IP" fwmark "$ruleNr"        table "$tableNr" priority "$ruleNr"
}

function sgwtun() {
  local interfaceName="$1"
  local tableNr="$2"
  local ruleNr="$3"

  "$IPTABLES" $IPTABLES_ARGS -t mangle "$ADDMODE_IPTABLES" POSTROUTING -m conntrack --ctstate NEW -o "$interfaceName" -j CONNMARK --set-mark "$ruleNr"
  "$IP"       $IP_ARGS       rule      "$ADDMODE_IP" fwmark "$ruleNr" table "$tableNr" priority "$ruleNr"
}


###############################################################################
#
# MAIN
#
###############################################################################

# we always need 4 arguments, check it
if [ $argc -lt 4 ]; then
  error "Need at least 4 arguments"
  usage
  exit 1
fi

# get first 4 arguments
declare instanceId="$1"
declare ipVersion="$2"
declare mode="$3"
declare addMode="$4"
shift 4
declare logFile="/var/run/$scriptName-$instanceId.log"
argc=$#

# check IP version argument
if [ ! "$ipVersion" = "$IPVERSION_4" ] && \
   [ ! "$ipVersion" = "$IPVERSION_6" ]; then
  error "Illegal IP version"
  usage
  exit 1
fi

# check mode argument
if [ ! "$mode" = "$MODE_CLEANUP" ] && \
   [ ! "$mode" = "$MODE_GENERIC" ] && \
   [ ! "$mode" = "$MODE_OLSRIF" ] && \
   [ ! "$mode" = "$MODE_SGWSRVTUN" ] && \
   [ ! "$mode" = "$MODE_EGRESSIF" ] && \
   [ ! "$mode" = "$MODE_SGWTUN" ]; then
  error "Illegal mode"
  usage
  exit 1
fi

# check addMode argument
if [ ! "$addMode" = "$ADDMODE_ADD" ] && \
   [ ! "$addMode" = "$ADDMODE_DEL" ]; then
  error "Illegal addMode"
  usage
  exit 1
fi

# check argument count for all modes
if ([ "$mode" = "$MODE_CLEANUP" ]   && [ $argc -lt $MODE_CLEANUP_ARGC   ]) || \
   ([ "$mode" = "$MODE_GENERIC" ]   && [ $argc -lt $MODE_GENERIC_ARGC   ]) || \
   ([ "$mode" = "$MODE_OLSRIF" ]    && [ $argc -lt $MODE_OLSRIF_ARGC    ]) || \
   ([ "$mode" = "$MODE_EGRESSIF"  ] && [ $argc -lt $MODE_EGRESSIF_ARGC  ]) || \
   ([ "$mode" = "$MODE_SGWSRVTUN" ] && [ $argc -lt $MODE_SGWSRVTUN_ARGC ]) || \
   ([ "$mode" = "$MODE_SGWTUN"  ]   && [ $argc -lt $MODE_SGWTUN_ARGC    ]); then
  if [ $argc -eq 0 ]; then
    error "Not enough arguments arguments ($argc) for mode $mode"
  else
    error "Not enough arguments arguments ($argc) for mode $mode" "Arguments: ${@}"
  fi
  usage
  exit 1
fi

# check argument count for all modes
if ([ "$mode" = "$MODE_CLEANUP" ]   && [ $argc -gt $MODE_CLEANUP_ARGC   ]) || \
   ([ "$mode" = "$MODE_GENERIC" ]   && [ $argc -gt $MODE_GENERIC_ARGC   ]) || \
   ([ "$mode" = "$MODE_OLSRIF" ]    && [ $argc -gt $MODE_OLSRIF_ARGC    ]) || \
   ([ "$mode" = "$MODE_EGRESSIF"  ] && [ $argc -gt $MODE_EGRESSIF_ARGC  ]) || \
   ([ "$mode" = "$MODE_SGWSRVTUN" ] && [ $argc -gt $MODE_SGWSRVTUN_ARGC ]) || \
   ([ "$mode" = "$MODE_SGWTUN"  ]   && [ $argc -gt $MODE_SGWTUN_ARGC    ]); then
  if [ $argc -eq 0 ]; then
    error "Too many arguments arguments ($argc) for mode $mode"
  else
    error "Too many arguments arguments ($argc) for mode $mode" "Arguments: ${@}"
  fi
  usage
  exit 1
fi

# process ipVersion argument
declare IPTABLES="iptables"
declare IPTABLES_ARGS="-w"
declare IP="ip"
declare IP_ARGS="-4"
if [ "$ipVersion" = "$IPVERSION_6" ]; then
  IPTABLES="ip6tables"
  IPTABLES_ARGS="-w"
  IP="ip"
  IP_ARGS="-6"
fi

# process addMode argument
declare ADDMODE_IPTABLES="-D"
declare ADDMODE_IP="delete"
if [ "$addMode" = "$ADDMODE_ADD" ]; then
  # first call the delete mode to remove any left-over rules
  set +e
  "$mode" "${@}" 2> /dev/null
  set -e

  ADDMODE_IPTABLES="-I"
  ADDMODE_IP="add"
fi

# call the mode
if [ "$addMode" = "$ADDMODE_ADD" ] && [ ! "$mode" = "$MODE_CLEANUP" ]; then
  updateLogFile "${@}"
fi
"$mode" "${@}"
