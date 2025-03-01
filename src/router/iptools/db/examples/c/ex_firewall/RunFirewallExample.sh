#!/bin/sh

if [ ! -f ./FirewallExample ]; then
    rm -r ./FirewallExample
fi
if [ ! -f ./Destination_Env ]; then
    rm -r ./Destination_Env
fi
mkdir ./FirewallExample
mkdir ./Destination_Env

echo "Starting message simulator and generating 100,000 messages."
../../../build_unix/ex_firewall_msg &
echo "Starting destination site."
../../../build_unix/ex_firewall_dest -d destination -h Destination_Env &
echo "Starting firewall."
../../../build_unix/ex_firewall
