#!/bin/sh

if [ ! -f ./TollBoothExample ]; then
    rm -r ./TollBoothExample
fi
if [ ! -f ./Billing ]; then
    rm -r ./Billing
fi
if [ ! -f ./Stolen ]; then
    rm -r ./Stolen
fi
if [ ! -f ./Traffic ]; then
    rm -r ./Traffic
fi
mkdir ./TollBoothExample
mkdir ./Billing
mkdir ./Stolen
mkdir ./Traffic

echo "Generating example data, this may take a minute."
../../../build_unix/ex_toll_data
echo "Simulating a day worth of cars passing through an automated toll booth."
../../../build_unix/ex_toll_event &
echo "Event Processor will bill the car owners, checking for stolen cars, and sending traffic alerts."
../../../build_unix/ex_toll_booth
