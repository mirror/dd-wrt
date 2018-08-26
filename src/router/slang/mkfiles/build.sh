#! /bin/sh

if [ ! -f src/slang.c ] ; then
   echo ERROR: this script must be invoked from the top of the S-Lang distribution
   exit 1
fi

if [ $# -eq 0 ] ; then
   echo Usage: `basename $0` PLATFORM COMPILER [DLL]
   echo 	Parameters should be specified in upper case
   exit 1
fi

echo
echo This script assumes you working from a CygWin/MinGW shell, with GNU make
echo

echo - generating Makefile in src...
src/mkfiles/mkmake $1 $2 $3 $4 $5 < src/mkfiles/makefile.all > src/Makefile

echo - generating Makefile in slsh...
src/mkfiles/mkmake $1 $2 $3 $4 $5 < slsh/mkfiles/makefile.all > slsh/Makefile

echo - generating Makefile in modules...
src/mkfiles/mkmake $1 $2 $3 $4 $5 < modules/mkfiles/makefile.all > modules/Makefile

cp mkfiles/makefile.sh Makefile
make
