set -ve

CFLAGS="-O2 -march=athlon-xp -fomit-frame-pointer -pipe"
#CFLAGS="-O -pg -g"
#LFLAGS=$CFLAGS
#CC=gcc33
#CXX=g++33

rm -f *.o
find *.c -exec ccache $CC $CFLAGS -c \{\} \;
find *.cc -exec ccache $CXX $CFLAGS -c \{\} \;
$CXX $LFLAGS *.o -o lzma
