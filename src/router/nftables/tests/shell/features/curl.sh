#!/bin/sh

# check whether curl is installed and supports ftp
curl --version | grep "^Protocols: "| grep -q " ftp"
