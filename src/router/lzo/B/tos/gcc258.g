# /* TOS (Atari ST) - gcc 2.5.8
#  * a very simple make driver (gulam shell)
#  * Copyright (C) 1996-2002 Markus F.X.J. Oberhumer
#  */

# --- 16-bit integers ---
alias gcc 'gcc -mshort -DSIZEOF_SIZE_T=4 -Iinclude'
set MYLIB lzo16.olb

# --- 32-bit integers ---
alias gcc 'gcc -Iinclude'
set MYLIB lzo.olb

set batch_echo 0
set env_style mw

rm $MYLIB
rm *.o

foreach f { src\*.c }
  echo "Compiling $f"
  gcc -Wall -W -Wno-uninitialized -O2 -fomit-frame-pointer -c $f
endfor

### gcc-ar rcs $MYLIB *.o
foreach f { *.o }
  gcc-ar rs $MYLIB $f
endfor

set batch_echo 1
gcc -s -Wall -O2       ltest\ltest.c $MYLIB -o ltest.ttp
gcc -s -Wall -O2 -Isrc tests\align.c       $MYLIB -o align.ttp
gcc -s -Wall -O2       tests\chksum.c      $MYLIB -o chksum.ttp
gcc -s -Wall -O2       tests\sizes.c       $MYLIB -o sizes.ttp
gcc -s -Wall -O2       examples\dict.c     $MYLIB -o dict.ttp
gcc -s -Wall -O2       examples\lpack.c    $MYLIB -o lpack.ttp
gcc -s -Wall -O2       examples\overlap.c  $MYLIB -o overlap.ttp
gcc -s -Wall -O2       examples\precomp.c  $MYLIB -o precomp.ttp
gcc -s -Wall -O2       examples\precomp2.c $MYLIB -o precomp2.ttp
gcc -s -Wall -O2       examples\simple.c   $MYLIB -o simple.ttp
set batch_echo 0

unalias gcc

echo Done.
