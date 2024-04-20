#!/bin/bash
set -ex

version=`pkg-config --modversion libyang`
echo "$version" | grep '2\.[0-9.]\+'
