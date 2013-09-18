#!/bin/bash

set -e
set -u

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


###############################################################################
#
# HELPER FUNCTIONS
#
###############################################################################

function usage() {
  echo ""
  echo "The script was called as:"
  echo "  ${script} ${arguments[@]:-}"
  echo ""
  echo "Usage:"
  echo "  ${script} ipVersion mode addMode [ifname [ifmark]]"
  echo "    - ipVersion: ${IPVERSION_4} or ${IPVERSION_6}"
  echo "    - mode     : ${MODE_GENERIC}, ${MODE_OLSRIF}, ${MODE_SGWSRVTUN}, ${MODE_EGRESSIF} or ${MODE_SGWTUN}"
  echo "    - addMode  : ${ADDMODE_ADD} or ${ADDMODE_DEL}"
  echo "    - ifname   : an interface name, not relevant for generic mode"
  echo "    - ifmark   : an interface marking (number), only relevant for ${MODE_EGRESSIF} and ${MODE_SGWTUN} modes"
}

function error() {
  local -i firstLine=1
  while [ ${#} -gt 0 ]; do
    if [ ${firstLine} -eq 1 ]; then
      echo "Error: ${1}"
    else
      echo "       ${1}"
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
  "${IPTABLES}" ${IPTABLES_ARGS} -t mangle "${ADDMODE_IPTABLES}" OUTPUT -j CONNMARK --restore-mark
}

function olsrif() {
  "${IPTABLES}" ${IPTABLES_ARGS} -t mangle "${ADDMODE_IPTABLES}" PREROUTING -i "${1}" -j CONNMARK --restore-mark
}

function sgwsrvtun() {
  "${IPTABLES}" ${IPTABLES_ARGS} -t mangle "${ADDMODE_IPTABLES}" PREROUTING -i "${1}" -j CONNMARK --restore-mark
}

function egressif() {
  "${IPTABLES}" ${IPTABLES_ARGS} -t mangle "${ADDMODE_IPTABLES}" POSTROUTING -m conntrack --ctstate NEW -o "${1}" -j CONNMARK --set-mark "${2}"
  "${IPTABLES}" ${IPTABLES_ARGS} -t mangle "${ADDMODE_IPTABLES}" INPUT       -m conntrack --ctstate NEW -i "${1}" -j CONNMARK --set-mark "${2}"
  "${IP}" ${IP_ARGS} rule "${ADDMODE_IP}" fwmark "${2}" table "${2}" pref "${2}"
}

function sgwtun() {
  "${IPTABLES}" ${IPTABLES_ARGS} -t mangle "${ADDMODE_IPTABLES}" POSTROUTING -m conntrack --ctstate NEW -o "${1}" -j CONNMARK --set-mark "${2}"
  "${IP}" ${IP_ARGS} rule "${ADDMODE_IP}" fwmark "${2}" table "${2}" pref "${2}"
}


###############################################################################
#
# MAIN
#
###############################################################################

declare script="${0}"
declare -a arguments=( ${@} )
declare -i argc=${#}

# we always need 3 arguments, check it
if [ ${argc} -lt 3 ]; then
  error "Need at least 3 arguments"
  usage
  exit 1
fi

# get first 3 arguments
declare ipVersion=${1}
declare mode="${2}"
declare addMode="${3}"
shift 3
argc=${#}

# check IP version argument
if [ ! "${ipVersion}" == "${IPVERSION_4}" ] && \
   [ ! "${ipVersion}" == "${IPVERSION_6}" ]; then
  error "Illegal IP version"
  usage
  exit 1
fi

# check mode argument
if [ ! "${mode}" == "${MODE_GENERIC}" ] && \
   [ ! "${mode}" == "${MODE_OLSRIF}" ] && \
   [ ! "${mode}" == "${MODE_SGWSRVTUN}" ] && \
   [ ! "${mode}" == "${MODE_EGRESSIF}" ] && \
   [ ! "${mode}" == "${MODE_SGWTUN}" ]; then
  error "Illegal mode"
  usage
  exit 1
fi

# check addMode argument
if [ ! "${addMode}" == "${ADDMODE_ADD}" ] && \
   [ ! "${addMode}" == "${ADDMODE_DEL}" ]; then
  error "Illegal addMode"
  usage
  exit 1
fi

# check argument count for all modes
if ([ "${mode}" == "${MODE_GENERIC}" ]   && [ ${argc} -ne 0 ]) || \
   ([ "${mode}" == "${MODE_OLSRIF}" ]    && [ ${argc} -ne 1 ]) || \
   ([ "${mode}" == "${MODE_SGWSRVTUN}" ] && [ ${argc} -ne 1 ]) || \
   ([ "${mode}" == "${MODE_EGRESSIF}"  ] && [ ${argc} -ne 2 ]) || \
   ([ "${mode}" == "${MODE_SGWTUN}"  ]   && [ ${argc} -ne 2 ]); then
  error "Not enough arguments or too many arguments"
  usage
  exit 1
fi

# process ipVersion argument
declare IPTABLES="iptables"
declare IPTABLES_ARGS=""
declare IP="ip"
declare IP_ARGS="-4"
if [ "${ipVersion}" == "${IPVERSION_6}" ]; then
  IPTABLES="ip6tables"
  IPTABLES_ARGS=""
  IP="ip"
  IP_ARGS="-6"
fi

# process addMode argument
declare ADDMODE_IPTABLES="-D"
declare ADDMODE_IP="del"
if [ "${addMode}" == "${ADDMODE_ADD}" ]; then
  # first call the delete mode to remove any left-over rules
  set +e
  "${mode}" "${@}" 2> /dev/null
  set -e

  ADDMODE_IPTABLES="-I"
  ADDMODE_IP="add"
fi

# call the mode
"${mode}" "${@}"
