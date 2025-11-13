#!/usr/bin/perl
# Test the --debug feature

# Copyright (C) 2018-2022 Free Software Foundation, Inc.

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

use strict;
use File::stat;

(my $program_name = $0) =~ s|.*/||;

# Turn off localization of executable's output.
@ENV{qw(LANGUAGE LANG LC_ALL)} = ('C') x 3;

my $prog = 'sed';

print "PATH = $ENV{PATH}\n";

=pod
This list contains a template for the tests.
Two 'foreach' loops below add the '{IN/OUT}' hash entries
of typical coreutils tests, adds single-quotes around the sed program,
and adds the --debug command line option.

NOTE: test names with "_" character will be checked with NULL input,
but not with non-empty input (eg. to avoid executing external programs).
=cut
my @Tests =
    (
     ## Test parsing of SED commands, without any execution
     ['c0',   '='      ],
     ['c1',   ':FOO'   ],
     ['c2',   '{=}'    ],
     ['c3',   '#FOO'   ],
     ['c4',   'aFOO'   ],
     ['c5',   'b'      ],
     ['c6',   'bx;:x'  ],
     ['c7',   'cFOO'   ],
     ['c8',   'D'      ],
     ['c9',   'd'      ],
     ['c10_', 'e'      ],
     ['c11_', 'ew'     ],
     ['c12',  'F'      ],
     ['c13',  'G'      ],
     ['c14',  'g'      ],
     ['c15',  'H'      ],
     ['c16',  'h'      ],
     ['c17',  'iFOO'   ],
     ['c18',  'l'      ],
     ['c19',  'l3'     ],
     ['c20',  'N'      ],
     ['c21',  'n'      ],
     ['c22',  'P'      ],
     ['c23',  'p'      ],
     ['c24',  'Q'      ],
     ['c25_', 'Q3'     ],
     ['c26',  'q'      ],
     ['c27_', 'q3'     ],
     ['c28',  'Rx'     ],
     ['c29',  'rx'     ],
     ['c30',  's/x//'  ],
     ['c31',  'T'      ],
     ['c32',  'Tx;:x'  ],
     ['c33',  't'      ],
     ['c34',  'tx;:x'  ],
     ['c35',  'v'      ],
     ['c36',  'Wx'     ],
     ['c37',  'wx'     ],
     ['c38',  'x'      ],
     ['c39',  'y/x/y/' ],
     ['c40',  'z'      ],
     ['c41',  ''       ],

     ## Test parsing of SED addresses, without any execution
     ['a0',  '1='        ],
     ['a1',  '1!='       ],
     ['a2',  '1,2='      ],
     ['a3',  '1,2!='     ],
     ['a4',  '$='        ],
     ['a5',  '$!='       ],
     ['a6',  '1~3='      ],
     ['a7',  '1~3='      ],
     ['a8',  '50~0='     ],
     ['a9',  '/foo/='    ],
     ['a10', '/foo/!='   ],
     ['a11', '\@foo@='   ],
     ['a12', '0,/foo/='  ],
     ['a13', '1,/foo/='  ],
     ['a14', '/foo/,1='  ],
     ['a15', '1,+10='    ],
     ['a16', '1,~10='    ],
     ['a17', '/foo/,+10='],
     ['a18', '/foo/,~10='],

     ## Test strings with special characters
     ['s1', '/\\a/='  ],
     ['s2', '/\\b/='  ],
     ['s3', '/\\f/='  ],
     ['s4', '/\\r/='  ],
     ['s5', '/\\t/='  ],
     ['s6', '/\\v/='  ],
     ['s7', '/\\n/='  ],
     ['s8', '/\\\\/=' ],
     ['s9', '/\x01/=' ],
     ['s10','/\//='   ],

     ## Address Regex variations
     ['r0', '/a/= ; //='  ],
     ['r1', '/a/I='       ],
     ['r2', '/a/M='       ],
     ['r3', '/a/IM='      ],

     ## substitute variations
     ['t0', 's/a/b/'     ],
     ['t1', 's/a/b/g'    ],
     ['t2', 's/a/b/i'    ],
     ['t3', 's/a/b/I'    ],
     ['t4', 's/a/b/m'    ],
     ['t5', 's/a/b/M'    ],
     ['t6', 's/a/b/wX'   ],
     ['t7', 's/a/b/p'    ],
     ['t8', 's/a/b/e'    ],
     ['t9', 's/a/b/3'    ],
     ['t10','s/a/b/iMg5p'],

     ['t20','s/\\(a\\)/\\1/'  ],
     ['t21','s/a/\\Ua/'       ],
     ['t22','s/a/\\ua/'       ],
     ['t23','s/a/\\La/'       ],
     ['t24','s/a/\\la/'       ],
     ['t25','s/a/\\U\\Ea/'    ],
     ['t26','s/a/&/'          ],

     ## Some special cases
     ['l1', 'a\\'             ],
     ['l2', 'c\\'             ],
     ['l3', 'i\\'             ],
     ['l4', 's/[0-9]/&/'      ], # report matched regex register
     ['l5', 'n;N;D'           ], # n/N/D with patterns containing \n.
     ['l6', 'n;n;n;n;n'       ], # n causing end-of-cycle
     ['l7', 's/^/a/'          ], # zero-length regex match
     ['l8', 's/\\($\\)/a/'    ], # zero-length regex match
    );


foreach my $t (@Tests)
{
    my $name = shift @$t;
    my $cmd = shift @$t;

    # Add "--debug" and single-quotes around the sed program.
    $cmd = "--debug '" . $cmd . "'";
    unshift @$t, $cmd;
    unshift @$t, $name;

    # Add the typical coreutils hash entries.
    # With empty input, the sed program will be printed (due to --debug),
    # but not executed.
    push @$t, {IN=>''};
    push @$t, {OUT=>''};
    push @$t, {OUT_SUBST=>'s/.*//s'};
}

# Repeat the tests with some input, to test --debug during execution.
# Discard the output, the exact debug output is not set in stone.
my @xtests;
Test:
foreach my $t (@Tests)
{
    # Remove the '{IN}' hash
    my @newt = grep { ! ( ref $_ eq 'HASH' && exists $_->{IN} ) } @$t;
    next if $newt[0] =~ /_/;

    # Rename the test (add "x_" prefix, for execution)
    $newt[0] = 'x_' . $newt[0];

    # Add non-empty input.
    push @newt, {IN=>"1\n2\n3\n4\n"};
    push @xtests, \@newt;
}

push @Tests, @xtests;

my $save_temps = $ENV{SAVE_TEMPS};
my $verbose = $ENV{VERBOSE};

my $fail = run_tests ($program_name, $prog, \@Tests, $save_temps, $verbose);
exit $fail;
