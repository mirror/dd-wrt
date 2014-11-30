#!/bin/bash

set -e
set -u


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

declare MODE_GENERIC="generic"
declare MODE_OLSRIF="olsrif"
declare MODE_SGWSRVTUN="sgwsrvtun"
declare MODE_EGRESSIF="egressif"
declare MODE_SGWTUN="sgwtun"

declare ADDMODE_ADD="add"
declare ADDMODE_DEL="del"

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
  echo "  $script ipVersion mode addMode ifName tableNr ruleNr bypassRuleNr"
  echo "    - ipVersion   : $IPVERSION_4 or $IPVERSION_6"
  echo "    - mode        : $MODE_GENERIC, $MODE_OLSRIF, $MODE_EGRESSIF, $MODE_SGWSRVTUN or $MODE_SGWTUN"
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
# MODE FUNCTIONS
#
###############################################################################

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
  "$IP"       $IP_ARGS       rule      "$ADDMODE_IP" fwmark "$ruleNr" table "$tableNr" priority "$ruleNr"
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

declare script="$0"
declare -a arguments=( ${@} )
declare -i argc=$#

# we always need 3 arguments, check it
if [ $argc -lt 3 ]; then
  error "Need at least 3 arguments"
  usage
  exit 1
fi

# get first 3 arguments
declare ipVersion=$1
declare mode="$2"
declare addMode="$3"
shift 3
argc=$#

# check IP version argument
if [ ! "$ipVersion" == "$IPVERSION_4" ] && \
   [ ! "$ipVersion" == "$IPVERSION_6" ]; then
  error "Illegal IP version"
  usage
  exit 1
fi

# check mode argument
if [ ! "$mode" == "$MODE_GENERIC" ] && \
   [ ! "$mode" == "$MODE_OLSRIF" ] && \
   [ ! "$mode" == "$MODE_SGWSRVTUN" ] && \
   [ ! "$mode" == "$MODE_EGRESSIF" ] && \
   [ ! "$mode" == "$MODE_SGWTUN" ]; then
  error "Illegal mode"
  usage
  exit 1
fi

# check addMode argument
if [ ! "$addMode" == "$ADDMODE_ADD" ] && \
   [ ! "$addMode" == "$ADDMODE_DEL" ]; then
  error "Illegal addMode"
  usage
  exit 1
fi

# check argument count for all modes
if ([ "$mode" == "$MODE_GENERIC" ]   && [ $argc -lt $MODE_GENERIC_ARGC   ]) || \
   ([ "$mode" == "$MODE_OLSRIF" ]    && [ $argc -lt $MODE_OLSRIF_ARGC    ]) || \
   ([ "$mode" == "$MODE_EGRESSIF"  ] && [ $argc -lt $MODE_EGRESSIF_ARGC  ]) || \
   ([ "$mode" == "$MODE_SGWSRVTUN" ] && [ $argc -lt $MODE_SGWSRVTUN_ARGC ]) || \
   ([ "$mode" == "$MODE_SGWTUN"  ]   && [ $argc -lt $MODE_SGWTUN_ARGC    ]); then
  if [ $argc -eq 0 ]; then
    error "Not enough arguments arguments ($argc) for mode $mode"
  else
    error "Not enough arguments arguments ($argc) for mode $mode" "Arguments: ${@}"
  fi
  usage
  exit 1
fi

# check argument count for all modes
if ([ "$mode" == "$MODE_GENERIC" ]   && [ $argc -gt $MODE_GENERIC_ARGC   ]) || \
   ([ "$mode" == "$MODE_OLSRIF" ]    && [ $argc -gt $MODE_OLSRIF_ARGC    ]) || \
   ([ "$mode" == "$MODE_EGRESSIF"  ] && [ $argc -gt $MODE_EGRESSIF_ARGC  ]) || \
   ([ "$mode" == "$MODE_SGWSRVTUN" ] && [ $argc -gt $MODE_SGWSRVTUN_ARGC ]) || \
   ([ "$mode" == "$MODE_SGWTUN"  ]   && [ $argc -gt $MODE_SGWTUN_ARGC    ]); then
  if [ $argc -eq 0 ]; then
    error "Not enough arguments arguments ($argc) for mode $mode"
  else
    error "Not enough arguments arguments ($argc) for mode $mode" "Arguments: ${@}"
  fi
  usage
  exit 1
fi

# process ipVersion argument
declare IPTABLES="iptables"
declare IPTABLES_ARGS=""
declare IP="ip"
declare IP_ARGS="-4"
if [ "$ipVersion" == "$IPVERSION_6" ]; then
  IPTABLES="ip6tables"
  IPTABLES_ARGS=""
  IP="ip"
  IP_ARGS="-6"
fi

# process addMode argument
declare ADDMODE_IPTABLES="-D"
declare ADDMODE_IP="delete"
if [ "$addMode" == "$ADDMODE_ADD" ]; then
  # first call the delete mode to remove any left-over rules
  set +e
  "$mode" "${@}" 2> /dev/null
  set -e

  ADDMODE_IPTABLES="-I"
  ADDMODE_IP="add"
fi

# call the mode
"$mode" "${@}"
