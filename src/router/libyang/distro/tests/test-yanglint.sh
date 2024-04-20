#!/bin/bash
set -ex

version=`yanglint --version`
echo "$version" | grep '2\.[0-9.]\+'
