#!/bin/sh
# (Re)Generate configure script and all build related files for i18n

autoreconf -W portability -vism --force >/dev/null

