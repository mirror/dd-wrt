#!/usr/bin/perl
# Test misc.

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

use strict;
use File::stat;

(my $program_name = $0) =~ s|.*/||;

# Turn off localization of executable's output.
@ENV{qw(LANGUAGE LANG LC_ALL)} = ('C') x 3;

my $prog = 'sed';

print "PATH = $ENV{PATH}\n";

my @Tests =
    (
     ['empty', qw(-e ''), {IN=>''}, {OUT=>''}],
     ['empty2', q('s/^ *//'), {IN=>"x\n\n"}, {OUT=>"x\n\n"}],

     ['head', qw(3q), {IN=>"1\n2\n3\n4\n"}, {OUT=>"1\n2\n3\n"}],
     ['space', q('s/_\S/XX/g;s/\s/_/g'),
      {IN=>  "Hello World\t!\nSecond_line_ of tests\n" },
      {OUT=> "Hello_World_!\nSecondXXine__of_tests\n" }],

     ['zero-anchor', qw(-z), q('N;N;s/^/X/g;s/^/X/mg;s/$/Y/g;s/$/Y/mg'),
      {IN=>"a\0b\0c\0" },
      {OUT=>"XXaY\0XbY\0XcYY\0" }],

     ['case-insensitive', qw(-n), q('h;s/Version: *//p;g;s/version: *//Ip'),
      {IN=>"Version: 1.2.3\n" },
      {OUT=>"1.2.3\n1.2.3\n" },
      ],

     ['preserve-missing-EOL-at-EOF', q('s/$/x/'),
      {IN=> "a\nb" },
      {OUT=>"ax\nbx" },
      ],

     ['y-bracket', q('y/[/ /'),
      {IN => "Are you sure (y/n)? [y]\n" },
      {OUT=> "Are you sure (y/n)?  y]\n" },
      ],

     ['y-zero', q('y/b/\x00/'),
      {IN => "abc\n" },
      {OUT=> "a\0c\n" },
      ],

     ['y-newline', q('H
G
y/Ss\nYy/yY$sS/'),
      {IN => "Are you sure (y/n)? [y]\n" },
      {OUT=> 'Are Sou Yure (S/n)? [S]$$Are Sou Yure (S/n)? [S]'."\n"},
      ],

     ['allsub', q('s/foo/bar/g'),
      {IN => "foo foo fo oo f oo foo foo foo foo foo foo foo foo foo\n"},
      {OUT=> "bar bar fo oo f oo bar bar bar bar bar bar bar bar bar\n"},
      ],

     ['insert-nl', qw(-f), {IN => "/foo/i\\\n"},
      {IN => "bar\nfoo\n" },
      {OUT=> "bar\n\nfoo\n" },
      ],

     ['recall',
      # Check that the empty regex recalls the last *executed* regex,
      # not the last *compiled* regex
      qw(-f), {IN => "p;s/e/X/p;:x;s//Y/p;/f/bx"},
      {IN => "eeefff\n" },
      {OUT=> "eeefff\n"
	   . "Xeefff\n"
	   . "XYefff\n"
	   . "XYeYff\n"
	   . "XYeYYf\n"
	   . "XYeYYY\n"
	   . "XYeYYY\n"
      },
      ],

     ['recall2',
      # Starting from sed 4.1.3, regexes are compiled with REG_NOSUB
      # if they are used in an address, so that the matcher does not
      # have to obey leftmost-longest.  The tricky part is to recompile
      # them if they are then used in a substitution.
      qw(-f), {IN => '/\(ab*\)\+/ s//>\1</g'},
      {IN => "ababb||abbbabbbb\n" },
      {OUT=> ">abb<||>abbbb<\n" },
      ],

     ['0range',
      # Test address 0 (GNU extension)
      # FIXME: This test does NOT actually fail if the address is changed to 1.
      qw(-e '0,/aaa/d'),
      {IN => "1\n"
           . "2\n"
           . "3\n"
           . "4\n"
           . "aaa\n"
           . "yes\n"},
      {OUT => "yes\n"}
     ],

     ['amp-escape',
      # Test ampersand as escape sequence (ASCII 0x26), which should
      # not have a special meaning (i.e. the 'matched pattern')
      qw(-e 's/yes/yes\x26/'),
      {IN => "yes\n"},
      {OUT => "yes&\n"}
     ],

     ['appquit',
      # Test 'a'ppend command before 'q'uit
      qw(-f),
      {IN => q(a\
ok
q)},
      {IN => "doh\n"},
      {OUT => "doh\n"
            . "ok\n"}
     ],


     ['brackets',
      qw(-f),
      {IN => q(s/[[]/a/
s/[[[]/b/
s/[[[[]/c/
s/[[[[[]/d/
s/[[[[[[]/e/
s/[[[[[[[]/f/
s/[[[[[[[[]/g/
s/[[[[[[[[[]/h/
)},
      {IN => "[[[[[[[[[\n"},
      {OUT => "abcdefgh[\n"}
     ],


     ['bkslashes',
      # Test backslashes in regex
      # bug in sed 4.0b
      qw(-f),
      {IN => q(s/$/\\\\\
/
)},
      {IN => "a\n"},
      {OUT => "a\\\n"
            . "\n"}
     ],

     ['classes',
      # inspired by an autoconf generated configure script.
      qw(-n -f),
      {IN => 's/^\([/[:lower:]A-Z0-9]*_cv_[[:lower:][:upper:]/[:digit:]]*\)'.
             '=\(.*\)/: \${\1=\'\2\'}/p'},
      {IN => "_cv_=emptyvar\n"
           . "ac_cv_prog/RANLIB=/usr/bin/ranlib\n"
           . "ac_cv_prog/CC=/usr/unsupported/\\ \\ /lib/_cv_/cc\n"
           . "a/c_cv_prog/CPP=/usr/bin/cpp\n"
           . "SHELL=bash\n"
           . "GNU=GNU!UNIX\n"},
      {OUT => ": \${_cv_='emptyvar'}\n"
            . ": \${ac_cv_prog/RANLIB='/usr/bin/ranlib'}\n"
            . ": \${ac_cv_prog/CC='/usr/unsupported/\\ \\ /lib/_cv_/cc'}\n"
            . ": \${a/c_cv_prog/CPP='/usr/bin/cpp'}\n"}
     ],


     ['cv-vars',
      # inspired by an autoconf generated configure script.
      qw(-n -f),
      {IN => q|s/^\([a-zA-Z0-9_]*_cv_[a-zA-Z0-9_]*\)=\(.*\)/: \${\1='\2'}/p|},
      {IN => "_cv_=emptyvar\n"
           . "ac_cv_prog_RANLIB=/usr/bin/ranlib\n"
           . "ac_cv_prog_CC=/usr/unsupported/\ \ /lib/_cv_/cc\n"
           . "ac_cv_prog_CPP=/usr/bin/cpp\n"
           . "SHELL=bash\n"
           . "GNU=GNU!UNIX\n"},
      {OUT => ": \${_cv_='emptyvar'}\n"
            . ": \${ac_cv_prog_RANLIB='/usr/bin/ranlib'}\n"
            . ": \${ac_cv_prog_CC='/usr/unsupported/\ \ /lib/_cv_/cc'}\n"
            . ": \${ac_cv_prog_CPP='/usr/bin/cpp'}\n"}
     ],

     ['quiet',
      # the old 'quiet' test: --quiet instead of -n
      qw(--quiet -f),
      {IN => q|s/^\([a-zA-Z0-9_]*_cv_[a-zA-Z0-9_]*\)=\(.*\)/: \${\1='\2'}/p|},
      {IN => "_cv_=emptyvar\n"
           . "ac_cv_prog_RANLIB=/usr/bin/ranlib\n"
           . "ac_cv_prog_CC=/usr/unsupported/\ \ /lib/_cv_/cc\n"
           . "ac_cv_prog_CPP=/usr/bin/cpp\n"
           . "SHELL=bash\n"
           . "GNU=GNU!UNIX\n"},
      {OUT => ": \${_cv_='emptyvar'}\n"
            . ": \${ac_cv_prog_RANLIB='/usr/bin/ranlib'}\n"
            . ": \${ac_cv_prog_CC='/usr/unsupported/\ \ /lib/_cv_/cc'}\n"
            . ": \${ac_cv_prog_CPP='/usr/bin/cpp'}\n"}
     ],

     ['file',
      # the old 'file' test: --file instead of -f
      qw(-n --file),
      {IN => q|s/^\([a-zA-Z0-9_]*_cv_[a-zA-Z0-9_]*\)=\(.*\)/: \${\1='\2'}/p|},
      {IN => "_cv_=emptyvar\n"
           . "ac_cv_prog_RANLIB=/usr/bin/ranlib\n"
           . "ac_cv_prog_CC=/usr/unsupported/\ \ /lib/_cv_/cc\n"
           . "ac_cv_prog_CPP=/usr/bin/cpp\n"
           . "SHELL=bash\n"
           . "GNU=GNU!UNIX\n"},
      {OUT => ": \${_cv_='emptyvar'}\n"
            . ": \${ac_cv_prog_RANLIB='/usr/bin/ranlib'}\n"
            . ": \${ac_cv_prog_CC='/usr/unsupported/\ \ /lib/_cv_/cc'}\n"
            . ": \${ac_cv_prog_CPP='/usr/bin/cpp'}\n"}
     ],


     ['dollar',
      # Test replacement on the last line (address '$')
      qw(-e '$s/^/space /'),
      {IN => "I can't quite remember where I heard it,\n"
           . "but I can't seem to get out of my head\n"
           . "the phrase\n"
           . "the final frontier\n"},
      {OUT => "I can't quite remember where I heard it,\n"
            . "but I can't seem to get out of my head\n"
            . "the phrase\n"
            . "space the final frontier\n"}
     ],

     ['enable',
      # inspired by an autoconf generated configure script.
      qw(-e 's/-*enable-//;s/=.*//'),
      {IN => "--enable-targets=sparc-sun-sunos4.1.3,srec\n"
           . "--enable-x11-testing=on\n"
           . "--enable-wollybears-in-minnesota=yes-id-like-that\n"},
      {OUT => "targets\n"
            . "x11-testing\n"
            . "wollybears-in-minnesota\n"}
     ],

     ['fasts',
      # test `fast' substitutions
      qw(-f),
      {IN => q(
h
s/a//
p
g
s/a//g
p
g
s/^a//p
g
s/^a//g
p
g
s/not present//g
p
g
s/^[a-z]//g
p
g
s/a$//
p
g

y/a/b/
h
s/b//
p
g
s/b//g
p
g
s/^b//p
g
s/^b//g
p
g
s/^[a-z]//g
p
g
s/b$//
p
g
)},
      {IN => "aaaaaaabbbbbbaaaaaaa\n"},
      {OUT => "aaaaaabbbbbbaaaaaaa\n"
            . "bbbbbb\n"
            . "aaaaaabbbbbbaaaaaaa\n"
            . "aaaaaabbbbbbaaaaaaa\n"
            . "aaaaaaabbbbbbaaaaaaa\n"
            . "aaaaaabbbbbbaaaaaaa\n"
            . "aaaaaaabbbbbbaaaaaa\n"
            . "bbbbbbbbbbbbbbbbbbb\n"
            . "\n"
            . "bbbbbbbbbbbbbbbbbbb\n"
            . "bbbbbbbbbbbbbbbbbbb\n"
            . "bbbbbbbbbbbbbbbbbbb\n"
            . "bbbbbbbbbbbbbbbbbbb\n"
            . "bbbbbbbbbbbbbbbbbbbb\n"}
     ],



     ['factor',
      # Compute a few common factors for speed.  Clear the subst flag
      # These are placed here to make the flow harder to understand :-)
      # The quotient of dividing by 11 is a limit to the remaining prime factors
      # Pattern space looks like CANDIDATE\nNUMBER.  When a candidate is valid,
      # the number is divided and the candidate is tried again
      # We have a prime factor in CANDIDATE! Print it
      # If NUMBER = 1, we don't have any more factors
      qw(-n -f),
      {IN => q~
s/.*/&;9aaaaaaaaa8aaaaaaaa7aaaaaaa6aaaaaa5aaaaa4aaaa3aaa2aa1a0/
:encode
s/\(a*\)\([0-9]\)\([0-9]*;.*\2\(a*\)\)/\1\1\1\1\1\1\1\1\1\1\4\3/
tencode
s/;.*//

t7a

:2
a\
2
b2a
:3
a\
3
b3a
:5
a\
5
b5a
:7
a\
7

:7a
s/^\(aa*\)\1\{6\}$/\1/
t7
:5a
s/^\(aa*\)\1\{4\}$/\1/
t5
:3a
s/^\(aa*\)\1\1$/\1/
t3
:2a
s/^\(aa*\)\1$/\1/
t2

/^a$/b

s/^\(aa*\)\1\{10\}/\1=&/

:factor
/^\(a\{7,\}\)=\1\1*$/! {
  # Decrement CANDIDATE, and search again if it is still >1
  s/^a//
  /^aa/b factor

  # Print the last remaining factor: since it is stored in the NUMBER
  # rather than in the CANDIDATE, swap 'em: now NUMBER=1
  s/\(.*\)=\(.*\)/\2=\1/
}

h
s/=.*/;;0a1aa2aaa3aaaa4aaaaa5aaaaaa6aaaaaaa7aaaaaaaa8aaaaaaaaa9/

:decode
s/^\(a*\)\1\{9\}\(a\{0,9\}\)\([0-9]*;.*[^a]\2\([0-9]\)\)/\1\4\3/
/^a/tdecode
s/;.*//p

g
:divide
s/^\(a*\)\(=b*\)\1/\1\2b/
tdivide
y/b/a/

/aa$/bfactor
~},

      {IN => "2\n"
           . "3\n"
           . "4\n"
           . "5\n"
           . "8\n"
           . "11\n"
           . "16\n"
           . "143\n"},
      {OUT => "2\n"
           . "3\n"
           . "2\n"
           . "2\n"
           . "5\n"
           . "2\n"
           . "2\n"
           . "2\n"
           . "11\n"
           . "2\n"
           . "2\n"
           . "2\n"
           . "2\n"
           . "13\n"
           . "11\n"}
     ],


     ['flipcase',
      qw(-f),
      {IN => q|s,\([^A-Za-z]*\)\([A-Za-z]*\),\1\L\u\2,g|},
      {IN => "09 - 02 - 2002 00.00 Tg La7 La7 -\n"
           . "09 - 02 - 2002 00.00 Brand New Tmc 2 -\n"
           . "09 - 02 - 2002 00.10 Tg1 Notte Rai Uno -\n"
           . "09 - 02 - 2002 00.15 Tg Parlamento Rai Due -\n"
           . "09 - 02 - 2002 00.15 Kung Fu - La Leggenda Continua La7 -\n"
           . "09 - 02 - 2002 00.20 Berserk - La CoNFESSIONE Di Gatz"
             . " Italia 1 Cartoon\n"
           . "09 - 02 - 2002 00.20 Tg3 - Tg3 Meteo Rai TrE -\n"
           . "09 - 02 - 2002 00.25 Meteo 2 Rai Due -\n"
           . "09 - 02 - 2002 00.30 Appuntamento Al CinEMA RaI Due -\n"
           . "09 - 02 - 2002 00.30 Rai Educational - Mediamente Rai Tre -\n"
           . "09 - 02 - 2002 00.35 Profiler Rai Due -\n"
           . "09 - 02 - 2002 00.35 Stampa OggI - Che Tempo Fa Rai Uno -\n"
           . "09 - 02 - 2002 00.45 Rai Educational - Babele: Euro Rai Uno -\n"
           . "09 - 02 - 2002 00.45 BollettINO Della NEVE RETE 4 News\n"
           . "09 - 02 - 2002 00.50 STUDIO Aperto - La Giornata Italia 1 News\n"
           . "09 - 02 - 2002 00.50 BOCCA A Bocca - 2 Tempo Rete 4 Film\n"
           . "09 - 02 - 2002 01.00 AppuntAMENTO Al Cinema Rai Tre -\n"
           . "09 - 02 - 2002 01.00 Music NoN Stop Tmc 2 -\n"
           . "09 - 02 - 2002 01.00 Studio SpORT Italia 1 SporT\n"
           . "09 - 02 - 2002 01.00 Tg 5 - Notte Canale 5 News\n"
           . "09 - 02 - 2002 01.05 Fuori Orario. CosE (Mai) Viste Rai Tre -\n"
           . "09 - 02 - 2002 01.15 RAINOTTE Rai Due -\n"
           . "09 - 02 - 2002 01.15 Sottovoce Rai Uno -\n"
           . "09 - 02 - 2002 01.15 GiOCHI Olimpici InVERNALI - CERIMONIA"
             . " Di Apertura Rai Tre -\n"
           . "09 - 02 - 2002 01.17 Italia Interroga Rai Due -\n"},
      {OUT => "09 - 02 - 2002 00.00 Tg La7 La7 -\n"
            . "09 - 02 - 2002 00.00 Brand New Tmc 2 -\n"
            . "09 - 02 - 2002 00.10 Tg1 Notte Rai Uno -\n"
            . "09 - 02 - 2002 00.15 Tg Parlamento Rai Due -\n"
            . "09 - 02 - 2002 00.15 Kung Fu - La Leggenda Continua La7 -\n"
            . "09 - 02 - 2002 00.20 Berserk - La Confessione Di Gatz"
              . " Italia 1 Cartoon\n"
            . "09 - 02 - 2002 00.20 Tg3 - Tg3 Meteo Rai Tre -\n"
            . "09 - 02 - 2002 00.25 Meteo 2 Rai Due -\n"
            . "09 - 02 - 2002 00.30 Appuntamento Al Cinema Rai Due -\n"
            . "09 - 02 - 2002 00.30 Rai Educational - Mediamente Rai Tre -\n"
            . "09 - 02 - 2002 00.35 Profiler Rai Due -\n"
            . "09 - 02 - 2002 00.35 Stampa Oggi - Che Tempo Fa Rai Uno -\n"
            . "09 - 02 - 2002 00.45 Rai Educational - Babele: Euro Rai Uno -\n"
            . "09 - 02 - 2002 00.45 Bollettino Della Neve Rete 4 News\n"
            . "09 - 02 - 2002 00.50 Studio Aperto - La Giornata Italia 1 News\n"
            . "09 - 02 - 2002 00.50 Bocca A Bocca - 2 Tempo Rete 4 Film\n"
            . "09 - 02 - 2002 01.00 Appuntamento Al Cinema Rai Tre -\n"
            . "09 - 02 - 2002 01.00 Music Non Stop Tmc 2 -\n"
            . "09 - 02 - 2002 01.00 Studio Sport Italia 1 Sport\n"
            . "09 - 02 - 2002 01.00 Tg 5 - Notte Canale 5 News\n"
            . "09 - 02 - 2002 01.05 Fuori Orario. Cose (Mai) Viste Rai Tre -\n"
            . "09 - 02 - 2002 01.15 Rainotte Rai Due -\n"
            . "09 - 02 - 2002 01.15 Sottovoce Rai Uno -\n"
            . "09 - 02 - 2002 01.15 Giochi Olimpici Invernali - Cerimonia"
              . " Di Apertura Rai Tre -\n"
            . "09 - 02 - 2002 01.17 Italia Interroga Rai Due -\n"}
       ],


     ['inclib',
      # inspired by an autoconf generated configure script.
      qw(-e 's;lib;include;'),
      {IN => "	/usr/X11R6/lib\n"
           . "	/usr/X11R5/lib\n"
           . "	/usr/X11R4/lib\n"
           . "\n"
           . "	/usr/lib/X11R6\n"
           . "	/usr/lib/X11R5\n"
           . "	/usr/lib/X11R4\n"
           . "\n"
           . "	/usr/local/X11R6/lib\n"
           . "	/usr/local/X11R5/lib\n"
           . "	/usr/local/X11R4/lib\n"
           . "\n"
           . "	/usr/local/lib/X11R6\n"
           . "	/usr/local/lib/X11R5\n"
           . "	/usr/local/lib/X11R4\n"
           . "\n"
           . "	/usr/X11/lib\n"
           . "	/usr/lib/X11\n"
           . "	/usr/local/X11/lib\n"
           . "	/usr/local/lib/X11\n"
           . "\n"
           . "	/usr/X386/lib\n"
           . "	/usr/x386/lib\n"
           . "	/usr/XFree86/lib/X11\n"
           . "\n"
           . "	/usr/lib\n"
           . "	/usr/local/lib\n"
           . "	/usr/unsupported/lib\n"
           . "	/usr/athena/lib\n"
           . "	/usr/local/x11r5/lib\n"
           . "	/usr/lpp/Xamples/lib\n"
           . "\n"
           . "	/usr/openwin/lib\n"
           . "	/usr/openwin/share/lib\n"},
      {OUT => "	/usr/X11R6/include\n"
            . "	/usr/X11R5/include\n"
            . "	/usr/X11R4/include\n"
            . "\n"
            . "	/usr/include/X11R6\n"
            . "	/usr/include/X11R5\n"
            . "	/usr/include/X11R4\n"
            . "\n"
            . "	/usr/local/X11R6/include\n"
            . "	/usr/local/X11R5/include\n"
            . "	/usr/local/X11R4/include\n"
            . "\n"
            . "	/usr/local/include/X11R6\n"
            . "	/usr/local/include/X11R5\n"
            . "	/usr/local/include/X11R4\n"
            . "\n"
            . "	/usr/X11/include\n"
            . "	/usr/include/X11\n"
            . "	/usr/local/X11/include\n"
            . "	/usr/local/include/X11\n"
            . "\n"
            . "	/usr/X386/include\n"
            . "	/usr/x386/include\n"
            . "	/usr/XFree86/include/X11\n"
            . "\n"
            . "	/usr/include\n"
            . "	/usr/local/include\n"
            . "	/usr/unsupported/include\n"
            . "	/usr/athena/include\n"
            . "	/usr/local/x11r5/include\n"
            . "	/usr/lpp/Xamples/include\n"
            . "\n"
            . "	/usr/openwin/include\n"
            . "	/usr/openwin/share/include\n"}
     ],

     ['khadafy',
      # The Khadafy test is brought to you by Scott Anderson . . .
      qw(-f),
      {IN => '/M[ou]\'\{0,1\}am\{1,2\}[ae]r' .
             ' .*' .
             '\([AEae]l[- ]\)\{0,1\}' .
             '[GKQ]h\{0,1\}[aeu]\{1,\}\([dtz][dhz]\{0,1\}\)\{1,\}af[iy]/!d'},
      {IN => "1)  Muammar Qaddafi\n"
           . "2)  Mo'ammar Gadhafi\n"
           . "3)  Muammar Kaddafi\n"
           . "4)  Muammar Qadhafi\n"
           . "5)  Moammar El Kadhafi\n"
           . "6)  Muammar Gadafi\n"
           . "7)  Mu'ammar al-Qadafi\n"
           . "8)  Moamer El Kazzafi\n"
           . "9)  Moamar al-Gaddafi\n"
           . "10) Mu'ammar Al Qathafi\n"
           . "11) Muammar Al Qathafi\n"
           . "12) Mo'ammar el-Gadhafi\n"
           . "13) Moamar El Kadhafi\n"
           . "14) Muammar al-Qadhafi\n"
           . "15) Mu'ammar al-Qadhdhafi\n"
           . "16) Mu'ammar Qadafi\n"
           . "17) Moamar Gaddafi\n"
           . "18) Mu'ammar Qadhdhafi\n"
           . "19) Muammar Khaddafi\n"
           . "20) Muammar al-Khaddafi\n"
           . "21) Mu'amar al-Kadafi\n"
           . "22) Muammar Ghaddafy\n"
           . "23) Muammar Ghadafi\n"
           . "24) Muammar Ghaddafi\n"
           . "25) Muamar Kaddafi\n"
           . "26) Muammar Quathafi\n"
           . "27) Muammar Gheddafi\n"
           . "28) Muamar Al-Kaddafi\n"
           . "29) Moammar Khadafy\n"
           . "30) Moammar Qudhafi\n"
           . "31) Mu'ammar al-Qaddafi\n"
           . "32) Mulazim Awwal Mu'ammar Muhammad Abu Minyar al-Qadhafi\n"},
      {OUT => "1)  Muammar Qaddafi\n"
            . "2)  Mo'ammar Gadhafi\n"
            . "3)  Muammar Kaddafi\n"
            . "4)  Muammar Qadhafi\n"
            . "5)  Moammar El Kadhafi\n"
            . "6)  Muammar Gadafi\n"
            . "7)  Mu'ammar al-Qadafi\n"
            . "8)  Moamer El Kazzafi\n"
            . "9)  Moamar al-Gaddafi\n"
            . "10) Mu'ammar Al Qathafi\n"
            . "11) Muammar Al Qathafi\n"
            . "12) Mo'ammar el-Gadhafi\n"
            . "13) Moamar El Kadhafi\n"
            . "14) Muammar al-Qadhafi\n"
            . "15) Mu'ammar al-Qadhdhafi\n"
            . "16) Mu'ammar Qadafi\n"
            . "17) Moamar Gaddafi\n"
            . "18) Mu'ammar Qadhdhafi\n"
            . "19) Muammar Khaddafi\n"
            . "20) Muammar al-Khaddafi\n"
            . "21) Mu'amar al-Kadafi\n"
            . "22) Muammar Ghaddafy\n"
            . "23) Muammar Ghadafi\n"
            . "24) Muammar Ghaddafi\n"
            . "25) Muamar Kaddafi\n"
            . "26) Muammar Quathafi\n"
            . "27) Muammar Gheddafi\n"
            . "28) Muamar Al-Kaddafi\n"
            . "29) Moammar Khadafy\n"
            . "30) Moammar Qudhafi\n"
            . "31) Mu'ammar al-Qaddafi\n"
            . "32) Mulazim Awwal Mu'ammar Muhammad Abu Minyar al-Qadhafi\n"}
     ],

     ['linecnt',
      qw(-e '='),
      {IN => "A dialogue on poverty\n"
           . "\n"
           . "	On the night when the rain beats,\n"
           . "	Driven by the wind,\n"
           . "	On the night when the snowflakes mingle\n"
           . "	With a sleety rain,\n"
           . "	I feel so helplessly cold.\n"
           . "	I nibble at a lump of salt,\n"
           . "	Sip the hot, oft-diluted dregs of _sake_;\n"
           . "	And coughing, snuffling,\n"
           . "	And stroking my scanty beard,\n"
           . "	I say in my pride,\n"
           . "	\"There's none worthy, save I!\"\n"
           . "	But I shiver still with cold.\n"
           . "	I pull up my hempen bedclothes,\n"
           . "	Wear what few sleeveless clothes I have,\n"
           . "	But cold and bitter is the night!\n"
           . "	As for those poorer than myself,\n"
           . "	Their parents must be cold and hungry,\n"
           . "	Their wives and children beg and cry.\n"
           . "	Then, how do you struggle through life?\n"
           . "\n"
           . "	Wide as they call the heaven and earth,\n"
           . "	For me they have shrunk quite small;\n"
           . "	Bright though they call the sun and moon,\n"
           . "	They never shine for me.\n"
           . "	Is it the same with all men,\n"
           . "	Or for me alone?\n"
           . "	By rare chance I was born a man\n"
           . "	And no meaner than my fellows,\n"
           . "	But, wearing unwadded sleeveless clothes\n"
           . "	In tatters, like weeds waving in the sea,\n"
           . "	Hanging from my shoulders,\n"
           . "	And under the sunken roof,\n"
           . "	Within the leaning walls,\n"
           . "	Here I lie on straw\n"
           . "	Spread on bare earth,\n"
           . "	With my parents at my pillow,\n"
           . "	And my wife and children at my feet,\n"
           . "	All huddled in grief and tears.\n"
           . "	No fire sends up smoke\n"
           . "	At the cooking-place,\n"
           . "	And in the cauldron\n"
           . "	A spider spins its web.\n"
           . "	With not a grain to cook,\n"
           . "	We moan like the night thrush.\n"
           . "	Then, \"to cut,\" as the saying is,\n"
           . "	\"The ends of what is already too short,\"\n"
           . "	The village headman comes,\n"
           . "	With rod in hand, to our sleeping place,\n"
           . "	Growling for his dues.\n"
           . "	Must it be so hopeless --\n"
           . "	The way of this world?\n"
           . "\n"
           . "	-- Yamanoue Okura\n"},
      {OUT => "1\n"
            . "A dialogue on poverty\n"
            . "2\n"
            . "\n"
            . "3\n"
            . "	On the night when the rain beats,\n"
            . "4\n"
            . "	Driven by the wind,\n"
            . "5\n"
            . "	On the night when the snowflakes mingle\n"
            . "6\n"
            . "	With a sleety rain,\n"
            . "7\n"
            . "	I feel so helplessly cold.\n"
            . "8\n"
            . "	I nibble at a lump of salt,\n"
            . "9\n"
            . "	Sip the hot, oft-diluted dregs of _sake_;\n"
            . "10\n"
            . "	And coughing, snuffling,\n"
            . "11\n"
            . "	And stroking my scanty beard,\n"
            . "12\n"
            . "	I say in my pride,\n"
            . "13\n"
            . "	\"There's none worthy, save I!\"\n"
            . "14\n"
            . "	But I shiver still with cold.\n"
            . "15\n"
            . "	I pull up my hempen bedclothes,\n"
            . "16\n"
            . "	Wear what few sleeveless clothes I have,\n"
            . "17\n"
            . "	But cold and bitter is the night!\n"
            . "18\n"
            . "	As for those poorer than myself,\n"
            . "19\n"
            . "	Their parents must be cold and hungry,\n"
            . "20\n"
            . "	Their wives and children beg and cry.\n"
            . "21\n"
            . "	Then, how do you struggle through life?\n"
            . "22\n"
            . "\n"
            . "23\n"
            . "	Wide as they call the heaven and earth,\n"
            . "24\n"
            . "	For me they have shrunk quite small;\n"
            . "25\n"
            . "	Bright though they call the sun and moon,\n"
            . "26\n"
            . "	They never shine for me.\n"
            . "27\n"
            . "	Is it the same with all men,\n"
            . "28\n"
            . "	Or for me alone?\n"
            . "29\n"
            . "	By rare chance I was born a man\n"
            . "30\n"
            . "	And no meaner than my fellows,\n"
            . "31\n"
            . "	But, wearing unwadded sleeveless clothes\n"
            . "32\n"
            . "	In tatters, like weeds waving in the sea,\n"
            . "33\n"
            . "	Hanging from my shoulders,\n"
            . "34\n"
            . "	And under the sunken roof,\n"
            . "35\n"
            . "	Within the leaning walls,\n"
            . "36\n"
            . "	Here I lie on straw\n"
            . "37\n"
            . "	Spread on bare earth,\n"
            . "38\n"
            . "	With my parents at my pillow,\n"
            . "39\n"
            . "	And my wife and children at my feet,\n"
            . "40\n"
            . "	All huddled in grief and tears.\n"
            . "41\n"
            . "	No fire sends up smoke\n"
            . "42\n"
            . "	At the cooking-place,\n"
            . "43\n"
            . "	And in the cauldron\n"
            . "44\n"
            . "	A spider spins its web.\n"
            . "45\n"
            . "	With not a grain to cook,\n"
            . "46\n"
            . "	We moan like the night thrush.\n"
            . "47\n"
            . "	Then, \"to cut,\" as the saying is,\n"
            . "48\n"
            . "	\"The ends of what is already too short,\"\n"
            . "49\n"
            . "	The village headman comes,\n"
            . "50\n"
            . "	With rod in hand, to our sleeping place,\n"
            . "51\n"
            . "	Growling for his dues.\n"
            . "52\n"
            . "	Must it be so hopeless --\n"
            . "53\n"
            . "	The way of this world?\n"
            . "54\n"
            . "\n"
            . "55\n"
            . "	-- Yamanoue Okura\n"}
     ],

     ['manis',
      # straight out of an autoconf-generated configure.
      # The input should look just like the input after this is run.
      #
      # Protect against being on the right side of a sed subst in config.status.
      qw(-f),
      {IN => q(s/%@/@@/; s/@%/@@/; s/%g$/@g/; /@g$/s/[\\\\&%]/\\\\&/g;
s/@@/%@/; s/@@/@%/; s/@g$/%g/
)},
      {IN => "s\%\@CFLAGS\@\%\%g\n"
           . "s\%\@CPPFLAGS\@\%-I/\%g\n"
           . "s\%\@CXXFLAGS\@\%-x c++\%g\n"
           . "s\%\@DEFS\@\%\$DEFS\%g\n"
           . "s\%\@LDFLAGS\@\%-L/usr/lib\%g\n"
           . "s\%\@LIBS\@\%-lgnu -lbfd\%g\n"
           . "s\%\@exec_prefix\@\%\%g\n"
           . "s\%\@prefix\@\%\$prefix\%g\n"
           . "s\%\@RANLIB\@\%\$RANLIB\%g\n"
           . "s\%\@CC\@\%/usr/local/bin/gcc\%g\n"
           . "s\%\@CPP\@\%\$CPP\%g\n"
           . "s\%\@XCFLAGS\@\%\$XCFLAGS\%g\n"
           . "s\%\@XINCLUDES\@\%\$XINCLUDES\%g\n"
           . "s\%\@XLIBS\@\%\$XLIBS\%g\n"
           . "s\%\@XPROGS\@\%\$XPROGS\%g\n"
           . "s\%\@TCLHDIR\@\%\$TCLHDIR\%g\n"
           . "s\%\@TCLLIB\@\%\$TCLLIB\%g\n"
           . "s\%\@TKHDIR\@\%\$TKHDIR\%g\n"
           . "s\%\@TKLIB\@\%\$TKLIB\%g\n"
           . "s\%\@PTY_TYPE\@\%\$PTY_TYPE\%g\n"
           . "s\%\@EVENT_TYPE\@\%\$EVENT_TYPE\%g\n"
           . "s\%\@SETUID\@\%\$SETUID\%g\n"},
      {OUT => "s\%\@CFLAGS\@\%\%g\n"
            . "s\%\@CPPFLAGS\@\%-I/\%g\n"
            . "s\%\@CXXFLAGS\@\%-x c++\%g\n"
            . "s\%\@DEFS\@\%\$DEFS\%g\n"
            . "s\%\@LDFLAGS\@\%-L/usr/lib\%g\n"
            . "s\%\@LIBS\@\%-lgnu -lbfd\%g\n"
            . "s\%\@exec_prefix\@\%\%g\n"
            . "s\%\@prefix\@\%\$prefix\%g\n"
            . "s\%\@RANLIB\@\%\$RANLIB\%g\n"
            . "s\%\@CC\@\%/usr/local/bin/gcc\%g\n"
            . "s\%\@CPP\@\%\$CPP\%g\n"
            . "s\%\@XCFLAGS\@\%\$XCFLAGS\%g\n"
            . "s\%\@XINCLUDES\@\%\$XINCLUDES\%g\n"
            . "s\%\@XLIBS\@\%\$XLIBS\%g\n"
            . "s\%\@XPROGS\@\%\$XPROGS\%g\n"
            . "s\%\@TCLHDIR\@\%\$TCLHDIR\%g\n"
            . "s\%\@TCLLIB\@\%\$TCLLIB\%g\n"
            . "s\%\@TKHDIR\@\%\$TKHDIR\%g\n"
            . "s\%\@TKLIB\@\%\$TKLIB\%g\n"
            . "s\%\@PTY_TYPE\@\%\$PTY_TYPE\%g\n"
            . "s\%\@EVENT_TYPE\@\%\$EVENT_TYPE\%g\n"
            . "s\%\@SETUID\@\%\$SETUID\%g\n"}
     ],

     ['modulo',
      qw(-e '0~2d;='),
      {IN => "s\%\@CFLAGS\@\%\%g\n"
           . "s\%\@CPPFLAGS\@\%-I/\%g\n"
           . "s\%\@CXXFLAGS\@\%-x c++\%g\n"
           . "s\%\@DEFS\@\%\$DEFS\%g\n"
           . "s\%\@LDFLAGS\@\%-L/usr/lib\%g\n"
           . "s\%\@LIBS\@\%-lgnu -lbfd\%g\n"
           . "s\%\@exec_prefix\@\%\%g\n"
           . "s\%\@prefix\@\%\$prefix\%g\n"
           . "s\%\@RANLIB\@\%\$RANLIB\%g\n"
           . "s\%\@CC\@\%/usr/local/bin/gcc\%g\n"
           . "s\%\@CPP\@\%\$CPP\%g\n"
           . "s\%\@XCFLAGS\@\%\$XCFLAGS\%g\n"
           . "s\%\@XINCLUDES\@\%\$XINCLUDES\%g\n"
           . "s\%\@XLIBS\@\%\$XLIBS\%g\n"
           . "s\%\@XPROGS\@\%\$XPROGS\%g\n"
           . "s\%\@TCLHDIR\@\%\$TCLHDIR\%g\n"
           . "s\%\@TCLLIB\@\%\$TCLLIB\%g\n"
           . "s\%\@TKHDIR\@\%\$TKHDIR\%g\n"
           . "s\%\@TKLIB\@\%\$TKLIB\%g\n"
           . "s\%\@PTY_TYPE\@\%\$PTY_TYPE\%g\n"
           . "s\%\@EVENT_TYPE\@\%\$EVENT_TYPE\%g\n"
           . "s\%\@SETUID\@\%\$SETUID\%g\n"},
      {OUT => "1\n"
            . "s\%\@CFLAGS\@\%\%g\n"
            . "3\n"
            . "s\%\@CXXFLAGS\@\%-x c++\%g\n"
            . "5\n"
            . "s\%\@LDFLAGS\@\%-L/usr/lib\%g\n"
            . "7\n"
            . "s\%\@exec_prefix\@\%\%g\n"
            . "9\n"
            . "s\%\@RANLIB\@\%\$RANLIB\%g\n"
            . "11\n"
            . "s\%\@CPP\@\%\$CPP\%g\n"
            . "13\n"
            . "s\%\@XINCLUDES\@\%\$XINCLUDES\%g\n"
            . "15\n"
            . "s\%\@XPROGS\@\%\$XPROGS\%g\n"
            . "17\n"
            . "s\%\@TCLLIB\@\%\$TCLLIB\%g\n"
            . "19\n"
            . "s\%\@TKLIB\@\%\$TKLIB\%g\n"
            . "21\n"
            . "s\%\@EVENT_TYPE\@\%\$EVENT_TYPE\%g\n"}
      ],

     ['middle',
      qw(-n -e '3,5p'),
      {IN => q(   "...by imposing a tiny bit of order in a communication you are
   translating, you are carving out a little bit of order in the
   universe.  You will never succeed.  Everything will fail and come
   to an end finally.  But you have a chance to carve a little bit
   of order and maybe even beauty out of the raw materials that
   surround you everywhere, and I think there is no greater meaning
   in life."

             Donald L. Philippi, Oct 1930 - Jan 1993
)},
    {OUT =>
q(   universe.  You will never succeed.  Everything will fail and come
   to an end finally.  But you have a chance to carve a little bit
   of order and maybe even beauty out of the raw materials that
)}
     ],

     ['newline-anchor',
      qw(-f),
      {IN => q(N
N
s/^/X/g
s/^/X/mg
s/$/Y/g
s/$/Y/mg
)},
      {IN => "a\n"
           . "b\n"
           . "c\n"},
      {OUT => "XXaY\n"
            . "XbY\n"
            . "XcYY\n"}
     ],

     ['noeolw',
      qw(-n -f),
      # The sed program:
      # generates two output files (in addition to STDOUT)
      {IN => q(w noeolw.1out
$ {
	x
	w noeolw.1out
	x
}
h
1,3w noeolw.2out
p
p
)},
      # The input file (was: noeolw.inp).
      # NOTE: in the old test, the input file was given twice.
      #       here we specify two (identical) input files.
      {IN => "This file is unique\n" .
	     "in that it does\n" .
	     "end in a newline."},
      {IN => "This file is unique\n" .
	     "in that it does\n" .
	     "end in a newline."},

      # The expected STDOUT (was: noeolw.good)
      {OUT => "This file is unique\n" .
	      "This file is unique\n" .
	      "in that it does\n" .
	      "in that it does\n" .
	      "end in a newline.\n" .
	      "end in a newline.\n" .
	      "This file is unique\n" .
	      "This file is unique\n" .
	      "in that it does\n" .
	      "in that it does\n" .
	      "end in a newline.\n" .
	      "end in a newline."},

      # The expected content of 'noeolw.1out' (was: noeolw.1good)
      {CMP => [ "This file is unique\n" .
		"in that it does\n" .
		"end in a newline.\n" .
		"This file is unique\n" .
		"in that it does\n" .
		"end in a newline.\n" .
		"in that it does\n",
		{ 'noeolw.1out' => undef }]},

      # The expected content of 'noeolw.2out' (was: noeolw.2good)
      {CMP => [ "This file is unique\n" .
		"in that it does\n" .
		"end in a newline.",
		{ 'noeolw.2out' => undef }]},
     ],

     ['numsub',
      qw(-f),
      {IN => q(
# the first one matches, the second doesn't
1s/foo/bar/10
2s/foo/bar/20

# The second line should be deleted.  ssed 3.55-3.58 do not.
t
d
)},
      {IN =>
q(foo foo fo oo f oo foo foo foo foo foo foo foo foo foo foo foo foo foo
foo foo fo oo f oo foo foo foo foo foo foo foo foo foo foo foo foo foo
)},
      {OUT => "foo foo fo oo f oo foo foo foo foo "
            . "foo foo foo bar foo foo foo foo foo\n"}
     ],


     ['numsub2',
      qw(-n -e 's/a*/b/2'),
      {IN => "\n"},
      {OUT => ""}
     ],


     ['numsub3',
      qw(-n -e 's/^a*/b/2'),
      {IN => "\n"},
      {OUT => ""}
     ],


     ['numsub4',
      qw(-n -e 's/^a*/b/2p'),
      {IN => "z\n"},
      {OUT => ""}
     ],


     ['numsub5',
      qw(-n -e 's/a*/b/3p'),
      {IN => "z\n"},
      {OUT => ""}
     ],

     ['readin',
      qw(-f),
      {IN => q(/\.$/r readin.in2
/too\.$/q
)},
      {AUX => { 'readin.in2' => "MOO\n" }},
      {IN => "``Democracy will not come today, this year,\n"
           . "  nor ever through compromise and fear.\n"
           . "  I have as much right as the other fellow has\n"
           . "  to stand on my two feet and own the land.\n"
           . "  I tire so of hearing people say\n"
           . "  let things take their course,\n"
           . "  tomorrow is another day.\n"
           . "  I do not need my freedom when I'm dead.\n"
           . "  I cannot live on tomorrow's bread.\n"
           . "  Freedom is a strong seed\n"
           . "  planted in a great need.\n"
           . "  I live here, too.\n"
           . "  I want freedom just as you.''\n"
           . "    ``The Weary Blues'', Langston Hughes\n"},
      {OUT => "``Democracy will not come today, this year,\n"
            . "  nor ever through compromise and fear.\n"
            . "MOO\n"
            . "  I have as much right as the other fellow has\n"
            . "  to stand on my two feet and own the land.\n"
            . "MOO\n"
            . "  I tire so of hearing people say\n"
            . "  let things take their course,\n"
            . "  tomorrow is another day.\n"
            . "MOO\n"
            . "  I do not need my freedom when I'm dead.\n"
            . "MOO\n"
            . "  I cannot live on tomorrow's bread.\n"
            . "MOO\n"
            . "  Freedom is a strong seed\n"
            . "  planted in a great need.\n"
            . "MOO\n"
            . "  I live here, too.\n"
            . "MOO\n"}
     ],


     ['sep',
      # inspired by an autoconf generated configure script.
      qw(-f),
      {IN => q(s%/[^/][^/]*$%%
s%[\/][^\/][^\/]*$%%
s,.*[^\/],,
)},
      {IN => "miss mary mack mack//mack/ran down/the track  track  track\n"
           . "slashes\aren't%used enough/in/casual-conversation///\n"
           . "possibly sentences would be more attractive if they ended"
             . "in two slashes//\n"},
      {OUT => "\n"
            . "///\n"
            . "//\n"}
     ],

     ['subwrite',
      # test s///w option
      qw(-e 's/you/YoU/w subwrite.wout'),
      {IN => "Not some church, and not the state,\n"
           . "Not some dark capricious fate.\n"
           . "Who you are, and when you lose,\n"
           . "Comes only from the things you choose.\n"},
      # The expected STDOUT
      {OUT => "Not some church, and not the state,\n"
            . "Not some dark capricious fate.\n"
            . "Who YoU are, and when you lose,\n"
            . "Comes only from the things YoU choose.\n"},
      # The expected content of 'writeout.wout'
      {CMP => [  "Who YoU are, and when you lose,\n"
               . "Comes only from the things YoU choose.\n",
                 { 'subwrite.wout' => undef }]}
     ],

     ['writeout',
      # Test 'w' command
      qw(-e '/^Facts ar/w writeout.wout'),
      {IN => "Facts are simple and facts are straight\n"
           . "Facts are lazy and facts are late\n"
           . "Facts all come with points of view\n"
           . "Facts don't do what I want them to\n"},
      # The expected STDOUT
      {OUT => "Facts are simple and facts are straight\n"
            . "Facts are lazy and facts are late\n"
            . "Facts all come with points of view\n"
            . "Facts don't do what I want them to\n"},
      # The expected content of 'writeout.wout'
      {CMP => [ "Facts are simple and facts are straight\n"
                . "Facts are lazy and facts are late\n",
                { 'writeout.wout' => undef }]}
     ],

     ['xabcx',
      # from the ChangeLog (Fri May 21 1993)
      # Regex address with custom character (\xREGEXx)
      qw(-e '\xfeetxs/blue/too/'),
      {IN => "roses are red\n"
           . "violets are blue\n"
           . "my feet are cold\n"
           . "your feet are blue\n"},
      {OUT => "roses are red\n"
            . "violets are blue\n"
            . "my feet are cold\n"
            . "your feet are too\n"}
     ],


     ['xbxcx',
      # from the ChangeLog (Wed Sep 5 2001)
      qw(-e 's/a*/x/g'),
      {IN => "\n"
           . "b\n"
           . "bc\n"
           . "bac\n"
           . "baac\n"
           . "baaac\n"
           . "baaaac\n"},
      {OUT => "x\n"
            . "xbx\n"
            . "xbxcx\n"
            . "xbxcx\n"
            . "xbxcx\n"
            . "xbxcx\n"
            . "xbxcx\n"}
      ],

     ['xbxcx3',
      # Test s///N replacements (GNU extension)
      qw(-e 's/a*/x/3'),
      {IN => "\n"
           . "b\n"
           . "bc\n"
           . "bac\n"
           . "baac\n"
           . "baaac\n"
           . "baaaac\n"},
      {OUT => "\n"
           . "b\n"
           . "bcx\n"
           . "bacx\n"
           . "baacx\n"
           . "baaacx\n"
           . "baaaacx\n"}
     ],


     # Four backslashes (2 pairs of "\\") to pass through two interpolations:
     # once in Perl, then the shell command line argument.
     # sed will see one backslash character in the s/// command.
     ['bug30794_1', "s/z/\\\\x5cA/",  {IN=>'z'}, {OUT => "\\A"}],
     ['bug30794_2', "s/z/\\\\x5c/",   {IN=>'z'}, {OUT => "\\"}],
     ['bug30794_3', "s/z/\\\\x5c1/",  {IN=>'z'}, {OUT => "\\1"}],

     ['bug40242', q('sn\nnXn'),  {IN=>'n'}, {OUT => 'X'}],
    );

my $save_temps = $ENV{SAVE_TEMPS};
my $verbose = $ENV{VERBOSE};

my $fail = run_tests ($program_name, $prog, \@Tests, $save_temps, $verbose);
exit $fail;
