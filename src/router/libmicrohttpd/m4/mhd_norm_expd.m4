# SYNOPSIS
#
#   _mhd_norm_expd([macro])
#
# DESCRIPTION
#
#   Normalize string after expansion of the macros.
#
#
# LICENSE
#
#   Copyright (c) 2022 Karlson2k (Evgeny Grin) <k2k@narod.ru>
#
#   Copying and distribution of this file, with or without modification, are
#   permitted in any medium without royalty provided the copyright notice
#   and this notice are preserved. This file is offered as-is, without any
#   warranty.

#serial 1

AC_DEFUN([_mhd_norm_expd],[m4_normalize(m4_expand([$1]))])
