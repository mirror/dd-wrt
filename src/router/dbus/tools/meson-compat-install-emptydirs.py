#!/usr/bin/env python3
# Copyright 2022 Collabora Ltd.
# SPDX-License-Identifier: MIT

# Compatibility shim for installing empty directories with Meson < 0.60

import os
import sys
from pathlib import Path

for d in sys.argv[1].split(':'):
    if os.path.isabs(d) and 'DESTDIR' in os.environ:
        p = Path(d)
        d = p.relative_to(p.anchor)
        dest = os.path.join(os.environ['DESTDIR'], d)
    else:
        dest = os.path.join(os.environ['MESON_INSTALL_DESTDIR_PREFIX'], d)

    os.makedirs(dest, mode=0o755, exist_ok=True)
