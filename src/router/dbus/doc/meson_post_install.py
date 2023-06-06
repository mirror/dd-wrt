#!/usr/bin/env python3
# Copyright © 2019-2020 Salamandar <felix@piedallu.me>
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

import os
import sys
import shutil
from pathlib import Path


if __name__ == "__main__":
    arg_builddir = sys.argv[1]
    arg_docdir = sys.argv[2]
    arg_qch = sys.argv[3]
    arg_qchdir = sys.argv[4]
    env_destdir = os.getenv('MESON_INSTALL_DESTDIR_PREFIX')

    builddir = Path(arg_builddir)
    docdir = Path(arg_docdir)
    qch = Path(arg_qch)
    qchdir = Path(arg_qchdir)
    destdir = Path(env_destdir)
    apidir = Path(destdir /docdir / 'api')
    shutil.rmtree(apidir, ignore_errors=True)
    shutil.copytree(builddir / 'api/html', apidir)
    if qch.is_file():
        shutil.copy(qch, destdir / qchdir)
