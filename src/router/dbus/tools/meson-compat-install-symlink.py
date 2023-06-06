#!/usr/bin/env python3
# Copyright 2022 Simon McVittie
# Copyright 2022 Collabora Ltd.
# SPDX-License-Identifier: MIT

# Compatibility shim for installing symlinks with Meson < 0.61

import os
import sys
from pathlib import Path

link_name, d, pointing_to = sys.argv[1:]

if os.path.isabs(d):
    p = Path(d)
    d = p.relative_to(p.anchor)
    dest = os.path.join(os.environ['DESTDIR'], d)
else:
    dest = os.path.join(os.environ['MESON_INSTALL_DESTDIR_PREFIX'], d)

os.makedirs(dest, mode=0o755, exist_ok=True)
os.symlink(pointing_to, os.path.join(dest, link_name))
