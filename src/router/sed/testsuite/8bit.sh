#!/bin/sh

# Adapted from sed's old "8bit" test

# Copyright (C) 2017-2022 Free Software Foundation, Inc.

# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.

# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.
. "${srcdir=.}/testsuite/init.sh"; path_prepend_ ./sed
print_ver_ sed

# Original comment from '8bit.sed':

# The first poem from the Man'yoshu.  I like Hitomaro's poems better
# but I couldn't find a copy of any of them in Japanese.  This version
# of this poem is from $BNc2r8E8l<-E5(B($BBh;0HG(B)$B;0>JF2(B.
#
# Speaking of Hitomaro, here is the english translation of one of my
# favorites.  I just know that everyone reading these test cases wants
# to see this.
#
#  In the autumn mountains
#  The yellow leaves are so thick.
#  Alas, how shall I seek my love
#  Who has wandered away?
#
#  I see the messenger come
#  As the yellow leaves are falling.
#  Oh, well I remember
#  How on such a day we used to meet--
#  My lover and I!
#       -- Kakinomoto Hitomaro

# The program is:
#   s/ÂçÏÂ/ÆüËÜ/
printf "s/\302\347\317\302/\306\374\313\334/\n" > 8bit-prog.sed \
    || framework_failure_


sed -f 8bit-prog.sed < "$abs_top_srcdir/testsuite/8bit.inp" > out || fail=1
remove_cr_inplace out
compare "$abs_top_srcdir/testsuite/8bit.good" out || fail=1


Exit $fail
