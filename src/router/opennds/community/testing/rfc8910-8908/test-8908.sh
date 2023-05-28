#!/bin/bash
#Copyright (C) The openNDS Contributors 2004-2022
#Copyright (C) BlueWave Projects and Services 2015-2023
#This software is released under the GNU GPL license.
#
# Warning - shebang bash is for compatibliity with BASH shell (Generic Linux)
# This script is expected to be run on a client device
#

echo
echo "Checking RFC8908 Response....."

json_reply=$(wget -q --save-headers -O - --header "Accept: application/captive+json" http://status.client)

received_headers=$(echo "$json_reply" | awk -F"{" 'length>2 {print $1}')
received_json="{"$(echo "$json_reply" | awk -F"{" 'length>2 {printf "%s", $2}')

echo
echo "Received Headers:"
echo "$received_headers"

echo
echo "Received Content:"
echo "$received_json"
echo
