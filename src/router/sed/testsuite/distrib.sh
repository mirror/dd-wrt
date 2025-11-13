#!/bin/sh

# Test runner for the old 'distrib' test

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

#   This is straight out of C News
#
#
# All this does is massage the headers so they look like what news
# software expects.  To:, Cc: and Resent-*: headers are masked.
# Reply-To: is turned into references, which is questionable (could
# just as well be dropped.
#
# The From: line is rewritten to use the "address (comments)" form
# instead of "phrase <route>" form our mailer uses.  Also, addresses
# with no "@domainname" are assumed to originate locally, and so are
# given a domain.
#
# The Sender: field below reflects the address of the person who
# maintains our mailing lists.  The Approved: field is in a special
# form, so that we can do bidirectional gatewaying.  Any message
# in a newsgroup that bears this stamp will not be fed into the
# matching mailing list.
cat << \EOF > distrib.sed || framework_failure_
1i\
Path: mailnewsgateway
	:a
	/^[Rr]eceived:/b r
	/^[Nn]ewsgroups:/b r
	/^[Pp]ath:/b r
	/^[Tt][Oo]:/s/^/Original-/
	/^[Cc][Cc]:/s/^/Original-/
	/^[Rr][Ee][Ss][Ee][Nn][Tt]-.*/s/^/Original-/
	/^[Mm][Ee][Ss][Ss][Aa][Gg][Ee]-[Ii][Dd]:/s/@/.alt.buddha.fat.short.guy@/
	s/^[Ii]n-[Rr]eply-[Tt]o:/References:/
	/^From:/{
		s/<\([^@]*\)>$/<\1@$thissite>/
		s/^From:[ 	][	]*\(.*\)  *<\(.*\)>$/From: \2 (\1)/
		}
	s/-[Ii]d:/-ID:/
	s/^[Ss][Uu][Bb][Jj][Ee][Cc][Tt]:[ 	]*$/Subject: (none)/
	s/^\([^:]*:\)[	 ]*/\1 /
	/^$/{i\
Newsgroups: alt.buddha.short.fat.guy\
Distribution: world\
Sender: news@cygnus.com\
Approved: alt.buddha.short.fat.guy@cygnus.com
	b e
	}
	p
	n
	b a
	:r
	s/.*//g
	n
	/^[ 	]/b r
	b a
	:e
	p
	n
	b e
EOF


# The expected output
cat << \EOF > distrib-exp || framework_failure_
Path: mailnewsgateway
From crash@cygnus.com  Wed Mar  8 18: 02:42 1995
From: crash@cygnus.com (Jason Molenda)
Message-ID: <9503090202.AA06931.alt.buddha.fat.short.guy@phydeaux.cygnus.com>
Subject: Note for sed testsuite
Original-To: molenda@msi.umn.edu
Date: Wed, 8 Mar 1995 18:02:24 -0800 (PST)
X-Mailer: ELM [version 2.4 PL23]
Newsgroups: alt.buddha.short.fat.guy
Distribution: world
Sender: news@cygnus.com
Approved: alt.buddha.short.fat.guy@cygnus.com

                _Summum Bonum_

    All the breath and the bloom of the
            year in the bag of one bee:
    All the wonder and wealth of the mine in
         the heart of one gem:
    In the core of one pearl all the shade and the
           shine of the sea:
    Breath and bloom, shade and shine, -- wonder,
        wealth, and -- how far above them --
          Truth, thats brighter than gem,
          Trust, that's purer than pearl, --
    Brightest truth, purest trust in the universe --
              all were for me
                 In the kiss of one girl.
        -- Robert Browning
EOF

# NOTE:
# The input has lines wider than 80 characters, and is kept as a separate file.

# location of external test files
dir="$abs_top_srcdir/testsuite"


sed -n -f distrib.sed < "$dir/distrib.inp" > distrib-out || fail=1
remove_cr_inplace distrib-out
compare distrib-exp distrib-out || fail=1


Exit $fail
