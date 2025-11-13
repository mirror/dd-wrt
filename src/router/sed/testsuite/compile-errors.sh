#!/bin/sh
# Test compilation errors

# Copyright (C) 2016-2022 Free Software Foundation, Inc.

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

#
# Excess P modifier to s//  (EXCESS_P_OPT)
#
cat <<\EOF >exp-exs-p || framework_failure_
sed: -e expression #1, char 8: multiple `p' options to `s' command
EOF
returns_ 1 sed 's/./x/pp' </dev/null 2>err-exs-p || fail=1
compare_ exp-exs-p err-exs-p || fail=1

#
# Excess G modifier to s//  (EXCESS_G_OPT)
#
cat <<\EOF >exp-exs-g || framework_failure_
sed: -e expression #1, char 8: multiple `g' options to `s' command
EOF
returns_ 1 sed 's/./x/gg' </dev/null 2>err-exs-g || fail=1
compare_ exp-exs-g err-exs-g || fail=1

#
# zero numeric modifier to s//  (ZERO_N_OPT)
#
cat <<\EOF >exp-exs-0 || framework_failure_
sed: -e expression #1, char 7: number option to `s' command may not be zero
EOF
returns_ 1 sed 's/./x/0' </dev/null 2>err-exs-0 || fail=1
compare_ exp-exs-0 err-exs-0 || fail=1


#
# Multiple number modifiers to s// (EXCESS_N_OPT)
#
cat <<\EOF >exp-exs-n || framework_failure_
sed: -e expression #1, char 9: multiple number options to `s' command
EOF
returns_ 1 sed 's/./x/2p3' </dev/null 2>err-exs-n || fail=1
compare_ exp-exs-n err-exs-n || fail=1


#
# Unknown s/// modifier letter
#
cat << \EOF >exp-unk-s-opt || framework_failure_
sed: -e expression #1, char 7: unknown option to `s'
EOF
returns_ 1 sed 's/./x/Q' </dev/null 2>err-unk-s-opt || fail=1
compare_ exp-unk-s-opt err-unk-s-opt || fail=1

#
# Special case: s/// followed by \r alone
#
printf "s/./x/\r" > s-opt-r-in || framework_failure_
cat << \EOF >exp-s-opt-r || framework_failure_
sed: file s-opt-r-in line 1: unknown option to `s'
EOF
returns_ 1 sed -f s-opt-r-in </dev/null 2>err-s-opt-r || fail=1
compare_ exp-s-opt-r err-s-opt-r || fail=1


#
# Step-address as first address (BAD_STEP)
# (both +N and ~N addresses)
cat <<\EOF >exp-step-addr || framework_failure_
sed: -e expression #1, char 2: invalid usage of +N or ~N as first address
EOF
returns_ 1 sed '~1d' </dev/null 2>err-step-addr1 || fail=1
compare_ exp-step-addr err-step-addr1 || fail=1
returns_ 1 sed '+1d' </dev/null 2>err-step-addr2 || fail=1
compare_ exp-step-addr err-step-addr2 || fail=1


#
# Multiple '!' (BAD_BANG)
#
cat <<\EOF >exp-bad-bang || framework_failure_
sed: -e expression #1, char 3: multiple `!'s
EOF
returns_ 1 sed '1!!d' </dev/null 2>err-bad-bang || fail=1
compare_ exp-bad-bang err-bad-bang || fail=1


#
# GNU extension commands, not accepted in --posix mode
# (bad_command(OPT))
for opt in e F v z L Q T R W ;
do
    cat <<EOF >exp-posix-cmd-$opt || framework_failure_
sed: -e expression #1, char 2: unknown command: \`$opt'
EOF
    returns_ 1 sed --posix "1$opt" </dev/null 2>err-posix-cmd-$opt || fail=1
    compare_ exp-posix-cmd-$opt err-posix-cmd-$opt || fail=1
done

#
# GNU extension commands, accepting only one address in --posix mode
# (ONE_ADDR)
cat <<\EOF >exp-one-addr || framework_failure_
sed: -e expression #1, char 4: command only uses one address
EOF
for opt in a i l = r ;
do
    returns_ 1 sed --posix "1,2$opt" </dev/null 2>err-posix-cmd-$opt || fail=1
    compare_ exp-one-addr err-posix-cmd-$opt || fail=1
done

# q/Q always accept one address (Q is gnu extension, can't use --posix, above)
for opt in q Q ;
do
    returns_ 1 sed "1,2$opt" </dev/null 2>err-posix-cmd-$opt || fail=1
    compare_ exp-one-addr err-posix-cmd-$opt || fail=1
done

#
# Comment with address (NO_CLOSE_BRACE_ADDR)
#
cat <<\EOF >exp-no-sharp || framework_failure_
sed: -e expression #1, char 2: comments don't accept any addresses
EOF
returns_ 1 sed '1#foo' </dev/null 2>err-no-sharp || fail=1
compare_ exp-no-sharp err-no-sharp || fail=1


#
# Unexpected closing braces (EXCESS_CLOSE_BRACE)
#
cat <<\EOF >exp-unexp-brace || framework_failure_
sed: -e expression #1, char 2: unexpected `}'
EOF
returns_ 1 sed '1}' </dev/null 2>err-unexp-brace || fail=1
compare_ exp-unexp-brace err-unexp-brace || fail=1


#
# Umatched opening braces (EXCESS_OPEN_BRACE)
# TODO: why 'char 0' ?
cat <<\EOF >exp-unmatched-braces || framework_failure_
sed: -e expression #1, char 0: unmatched `{'
EOF
returns_ 1 sed '1{' </dev/null 2>err-unmatched-braces || fail=1
compare_ exp-unmatched-braces err-unmatched-braces || fail=1


#
# '}' with address (NO_CLOSE_BRACE_ADDR)
#
cat <<\EOF >exp-brace-addr || framework_failure_
sed: -e expression #1, char 3: `}' doesn't want any addresses
EOF
returns_ 1 sed '{1}' </dev/null 2>err-brace-addr || fail=1
compare_ exp-brace-addr err-brace-addr || fail=1


#
# Too new version requested (ANCIENT_VERSION)
# (i.e. this version of SED is too old)
cat <<\EOF >exp-anc-ver || framework_failure_
sed: -e expression #1, char 4: expected newer version of sed
EOF
returns_ 1 sed 'v9.0' </dev/null 2>err-anc-ver || fail=1
compare_ exp-anc-ver err-anc-ver || fail=1


#
# Junk after command (EXCESS_JUNK)
# notes: EOF, \n or ';' are allowed after a command.
#        multiple places abort with EXCESS_JUNK, check them all.
#        dummy addresses ensure the offending char is the same.
cat <<\EOF >exp-junk || framework_failure_
sed: -e expression #1, char 7: extra characters after command
EOF
returns_ 1 sed '11111=d' </dev/null 2>err-junk || fail=1
compare_ exp-junk err-junk || fail=1
returns_ 1 sed 'y/a/b/d' </dev/null 2>err-junk-y || fail=1
compare_ exp-junk err-junk-y || fail=1
returns_ 1 sed '1111{}d' </dev/null 2>err-junk-braces || fail=1
compare_ exp-junk err-junk-braces || fail=1
returns_ 1 sed '22222ld' </dev/null 2>err-junk-braces || fail=1
compare_ exp-junk err-junk-braces || fail=1


#
# Slash after a/c/i (EXPECTED_SLASH)
# note: GNU extensions are less strict than --posix.
cat <<\EOF >exp-junk || framework_failure_
sed: -e expression #1, char 2: expected \ after `a', `c' or `i'
EOF
for opt in a c i ;
do
  # EOF after command
  returns_ 1 sed "1$opt" </dev/null 2>err-junk-$opt || fail=1
  compare_ exp-junk err-junk-$opt || fail=1

  # no slash after command, in GNU extension mode - accepted.
  sed "1${opt}foo" </dev/null >/dev/null || fail=1

  # no slash after command, in --posix mode - rejected.
  returns_ 1 sed --posix "${opt}foo" </dev/null 2>err-junk-$opt-psx || fail=1
  compare_ exp-junk err-junk-$opt-psx || fail=1
done


#
# ':' with address (NO_COLON_ADDR)
#
cat <<\EOF >exp-colon-addr || framework_failure_
sed: -e expression #1, char 2: : doesn't want any addresses
EOF
returns_ 1 sed '2:' </dev/null 2>err-colon-addr || fail=1
compare_ exp-colon-addr err-colon-addr || fail=1



#
# q/Q need one address (ONE_ADDR)
#
cat <<\EOF >exp-colon-addr || framework_failure_
sed: -e expression #1, char 2: : doesn't want any addresses
EOF
returns_ 1 sed '2:' </dev/null 2>err-colon-addr || fail=1
compare_ exp-colon-addr err-colon-addr || fail=1


#
# unterminated Y commands (UNTERM_Y_CMD)
# NOTE: the code calls bad_proc(UNTERM_Y_CMD)
# in multiple places due to varied conditions - check them all.
# dummy addresses ensures the offending character is always 5.
cat <<\EOF >exp-unterm-y || framework_failure_
sed: -e expression #1, char 5: unterminated `y' command
EOF
returns_ 1 sed '1111y' </dev/null 2>err-unterm-y1 || fail=1
compare_ exp-unterm-y err-unterm-y1 || fail=1
returns_ 1 sed '111y/' </dev/null 2>err-unterm-y2 || fail=1
compare_ exp-unterm-y err-unterm-y2 || fail=1
returns_ 1 sed '11y/a' </dev/null 2>err-unterm-y3 || fail=1
compare_ exp-unterm-y err-unterm-y3 || fail=1
returns_ 1 sed '1y/a/' </dev/null 2>err-unterm-y4 || fail=1
compare_ exp-unterm-y err-unterm-y4 || fail=1
returns_ 1 sed 'y/a/a' </dev/null 2>err-unterm-y5 || fail=1
compare_ exp-unterm-y err-unterm-y5 || fail=1

#
# Y command with bad legth (Y_CMD_LEN)
# TODO: check with multibyte strings.
cat <<\EOF >exp-bad-y-len || framework_failure_
sed: -e expression #1, char 7: strings for `y' command are different lengths
EOF
returns_ 1 sed 'y/a/bb/' </dev/null 2>err-bad-y-len || fail=1
compare_ exp-bad-y-len err-bad-y-len || fail=1


#
# GNU Extension: allow a/c/i to continue in next 'program'.
# in --posix mode, reject it with "incomplete command" (INCOMPLETE_CMD)
#
cat <<\EOF >exp-inc-cmd || framework_failure_
sed: -e expression #1, char 2: incomplete command
EOF
for opt in a c i ;
do
    # works as a gnu extension
    sed -e "$opt\\" -e foo < /dev/null || fail=1

    # rejected in posix mode
    returns_ 1 sed --posix -e "$opt\\" -e foo </dev/null 2>err-inc-cmd-$opt \
        || fail=1
    compare_ exp-inc-cmd err-inc-cmd-$opt || fail=1
done


Exit $fail
