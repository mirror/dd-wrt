#!/bin/sh

if [ ! -f ./AdvertisingExample ]; then
    rm -r ./AdvertisingExample
fi
if [ ! -f ./Stores ]; then
    rm -r ./Stores
fi
if [ ! -f ./Users ]; then
    rm -r ./Users
fi
mkdir ./AdvertisingExample
mkdir ./Stores
mkdir ./Users

echo "Generating example data, this may take a minute."
../../../build_unix/ex_ad_data
echo "Simulating GPS coordinates arriving at the central advertising server."
../../../build_unix/ex_ad_event &
echo "The advertising server will return ads that match the customer's location and shopping preferences."
../../../build_unix/ex_advertising
