#!/bin/sh

grep UTS_RELEASE $1/include/linux/version.h | cut -d\" -f2
