#! /bin/sh -e

while :; do

sudo nmap code.and.org dict www-cache \
          illiter.at www.illiter.at localhost 127.0.0.1 \
          127.0.0.2 127.0.1.2 127.2.2.2 127.4.3.2 -A -p 8008
sleep 0.4
done
