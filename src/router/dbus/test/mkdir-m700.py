#!/usr/bin/env python3
# Copyright 2022 Collabora Ltd.
# SPDX-License-Identifier: MIT

import os
import sys

# Note that we can't create the XDG_RUNTIME_DIR with permissions 0700
# on MSYS2, which rejects attempts to change permissions, hence "|| true".
os.makedirs(sys.argv[1], exist_ok=True)

try:
    os.chmod(sys.argv[1], 0o700)
except OSError:
    pass
