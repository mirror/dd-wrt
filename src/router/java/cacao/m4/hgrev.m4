dnl m4/hgrev.m4
dnl
dnl Copyright (C) 1996-2012
dnl CACAOVM - Verein zur Foerderung der freien virtuellen Maschine CACAO
dnl 
dnl This file is part of CACAO.
dnl 
dnl This program is free software; you can redistribute it and/or
dnl modify it under the terms of the GNU General Public License as
dnl published by the Free Software Foundation; either version 2, or (at
dnl your option) any later version.
dnl 
dnl This program is distributed in the hope that it will be useful, but
dnl WITHOUT ANY WARRANTY; without even the implied warranty of
dnl MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
dnl General Public License for more details.
dnl 
dnl You should have received a copy of the GNU General Public License
dnl along with this program; if not, write to the Free Software
dnl Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
dnl 02110-1301, USA.


dnl Mercurial revision

AC_DEFUN([AC_CHECK_HGREV],[
AC_MSG_CHECKING(for the Mercurial revision)
CACAO_HGREV=`grep ^node: "$srcdir"/.hg_archival.txt 2>/dev/null | awk '{print substr($[]2, 1, 12)}'`
if test x"$CACAO_HGREV" = "x"; then
    if test -d "$srcdir"/.hg; then
        dnl Strip the "+" suffix because icedtea builds are always slightly modified
        CACAO_HGREV=`cd "$srcdir" && hg id -i | sed -e 's/\([[0-9a-f]]*\).*/\1/'`
    fi
fi
AC_MSG_RESULT(${CACAO_HGREV-none})
AC_DEFINE_UNQUOTED([CACAO_HGREV], "${CACAO_HGREV}", [CACAO's Mercurial revision])
AC_SUBST(CACAO_HGREV)
])
