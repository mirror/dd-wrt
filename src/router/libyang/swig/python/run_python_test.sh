#!/bin/sh
#
# @file run_python_test.c
# @author: Mislav Novakovic <mislav.novakovic@sartura.hr>
# @brief unit tests for functions from libyang.h header
#
# Copyright (C) 2018 Deutsche Telekom AG.
#
# Author: Mislav Novakovic <mislav.novakovic@sartura.hr>
#
# This source code is licensed under BSD 3-Clause License (the "License").
# You may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     https://opensource.org/licenses/BSD-3-Clause
#

if [ "$#" -ne 5 ] ; then
  echo "Usage: $0 PATH PythonPath PythonExecutable Test" >&2
  exit 1
fi
PTH="$1"
PYTHON_EXE="$3"
TEST_FILE="$4"
export PATH=$PTH:$PATH
export PYTHONPATH="$2"
export LIBYANG_EXTENSIONS_PLUGINS_DIR="$5/src/extensions"

$PYTHON_EXE $TEST_FILE
