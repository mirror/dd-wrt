#!/usr/bin/env python3
# Copyright © 2019-2020 Salamandar <felix@piedallu.me>
# Copyright 2022 Collabora Ltd.
# SPDX-License-Identifier: MIT

import glob
import sys

print('\n'.join(glob.glob(sys.argv[1] + '/*.[ch]')))
